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
  UniqueFileDescriptor fd(path, O_RDONLY | O_NOATIME | O_NOCTTY | O_NOFOLLOW);
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
  if (!std::filesystem::is_symlink(path))
    insertFromFileImpl(path, *this, fm);
}

void ExtentSet::insertFromDir(const char* path, bool stopOnError) {
  UniqueMAllocPtr<fiemap> fm(sizeof(fiemap)); // Reuse the same memory to save allocation calls
  std::filesystem::recursive_directory_iterator it(path), it_before;
  const std::filesystem::recursive_directory_iterator end; // The default constructor gives and end iterator
  std::error_code ec;
  while (it != end) {
    // Look into the file pointed by it
    try {
      if (!it->is_symlink() && it->is_regular_file())
        insertFromFileImpl(it->path().c_str(), *this, fm);
    } catch (const std::exception& ex) {
      std::cerr << it->path() << ": " << ex.what() << std::endl;
      if (stopOnError)
        throw;
    }
    // Try to loop to the next file, checking for errors and reporting them
    it_before = it; // Save the value of it, since increment() will make it invalid (i.e. an end pointer) on error
    it.increment(ec);
    if (ec) {
      // Check if we called disable_recursion_pending at the previous iteration.
      // If we did and `it` still failed to increment, we are in an infinite loop.
      // We break it with an exception, because there is nothing else to do.
      if (!it_before.recursion_pending()) {
        throw std::filesystem::filesystem_error("cannot increment recursive directory iterator even after skipping the previous file", ec);
      }
      // Take the path from it_before, since it will be invalid (i.e. points to (directory_entry*)nullptr) on error
      std::cerr << it_before->path() << ": cannot increment recursive directory iterator: " << ec.message() << std::endl;
      if (stopOnError)
        throw std::filesystem::filesystem_error("cannot increment recursive directory iterator", ec);
      it = it_before; // Restore the value before the error
      it.disable_recursion_pending(); // Try again at next loop, skipping the current directory
    }
  }
}
