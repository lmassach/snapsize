/** Utilities to exploit the fiemap ioctl (implementation of functions
 * with ioctl and filesystem calls).
 *
 * Copyright 2024 Ludovico Massaccesi
 *
 * This file is part of snapsize.
 *
 * snapsize is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * snapsize is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with snapsize. If not, see <https://www.gnu.org/licenses/>.
 */
#include "Extents.hh"
#include "UniqueFileDescriptor.hh"
#include "UniqueMAllocPtr.hh"
#include <cstring>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <linux/fiemap.h>
#include <linux/fs.h>
#include <sys/ioctl.h>
#include <fcntl.h>

static void insertFromFileImpl(const char* path, ExtentSet& es, UniqueMAllocPtr<fiemap>& fm) {
  // Open file
  UniqueFileDescriptor fd(path, O_RDONLY | O_NOATIME);
  // Allocate fiemap (if necessary)
  fm.realloc(sizeof(fiemap));
  memset(fm, 0, sizeof(fiemap));
  fm->fm_length = ~((decltype(fm->fm_length))0);
  // Retrieve number of extents
  if (ioctl(fd, FS_IOC_FIEMAP, (void*)fm) < 0)
    throw std::runtime_error("ioctl FS_IOC_FIEMAP (1st) failed");
  // Resize fiemap
  fm.realloc(sizeof(fiemap) + sizeof(fiemap_extent) * fm->fm_mapped_extents);
  memset(fm->fm_extents, 0, sizeof(fiemap_extent) * fm->fm_mapped_extents);
  fm->fm_extent_count = fm->fm_mapped_extents;
  fm->fm_mapped_extents = 0;
  // Retrieve extents
  if (ioctl(fd, FS_IOC_FIEMAP, (void*)fm) < 0)
    throw std::runtime_error("ioctl FS_IOC_FIEMAP (2nd) failed");
  for (decltype(fm->fm_mapped_extents) i = 0; i < fm->fm_mapped_extents; ++i) {
    // Do not count unaligned blocks, as it typically is due to inline data,
    // i.e. data in the same block as metadata (happens for short files), which
    // means that typically a dummy extent is returned (block #0)
    if (fm->fm_extents[i].fe_flags & (FIEMAP_EXTENT_UNKNOWN | FIEMAP_EXTENT_NOT_ALIGNED)) {
      // std::cerr << "DEBUG "
      //   << "path=" << path
      //   << ", i=" << i << ", n=" << fm->fm_mapped_extents
      //   << ", logical=" << fm->fm_extents[i].fe_logical
      //   << ", physical=" << fm->fm_extents[i].fe_physical
      //   << ", length=" << fm->fm_extents[i].fe_length
      //   << ", flags=" << fm->fm_extents[i].fe_flags << '\n';
      continue;
    }
    es.insert(Extent(fm->fm_extents[i].fe_physical, fm->fm_extents[i].fe_length));
  }
}

void ExtentSet::insertFromFile(const char* path) {
  UniqueMAllocPtr<fiemap> fm(sizeof(fiemap));
  insertFromFileImpl(path, *this, fm);
}

void ExtentSet::insertFromDir(const char* path, bool stopOnError) {
  UniqueMAllocPtr<fiemap> fm(sizeof(fiemap)); // Reuse the same memory to save allocation calls
  std::string lastFilePath;
  bool stoppingOnError = false;
  try {
    for (
      const std::filesystem::directory_entry& fp : std::filesystem::recursive_directory_iterator(
        path, std::filesystem::directory_options::skip_permission_denied
        // skip_permission_denied will prevent us from reporting when a
        // directory is inaccessible; however, not using skip_permission_denied
        // would result in stopping at the first inaccessible directory, which
        // is even worse; follow_directory_symlink is not used *on purpose*
      )
    ) {
      lastFilePath = fp.path();
      try {
        if (!fp.is_symlink() && fp.is_regular_file())
          insertFromFileImpl(fp.path().c_str(), *this, fm);
      } catch (const std::exception& ex) {
        std::cerr << fp.path() << ": " << ex.what() << std::endl;
        if (stopOnError) {
          stoppingOnError = true;
          throw;
        }
      }
    }
  } catch (const std::exception& ex) {
    // This catch block is meant to report exceptions thrown by incrementing
    // the recursive_directory_iterator; exceptions from looking into a file
    // are already caught and reported by the try-catch block inside the for
    if (!stoppingOnError)
      std::cerr << "ExtentSet::insertFromDir: last file before error: " << lastFilePath
        << "\nExtentSet::insertFromDir: stopping on error: " << ex.what() << std::endl;
    throw;
  }
}
