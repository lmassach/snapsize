/** Entry point for the de utility.
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
#include "HumanSize.hh"
#include "Extents.hh"
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
using namespace std;
using namespace std::filesystem;
using namespace std::string_literals;

static path resolve_path(path p) {
  while (is_symlink(p))
    p = read_symlink(p);
  if (!exists(p))
    throw runtime_error("File does not exist: "s + p.string());
  return p;
}

int main(int argc, char** argv) {
  // Parse args
  bool printHelp = false, humanReadable = false;
  int nFiles = 0;
  for (int i = 1; i < argc; ++i) {
    if (argv[i][0] == '-') {
      if (argv[i] == "--help"s) {
        printHelp = true;
      } else if (argv[i] == "-h"s) {
        humanReadable = true;
      } else {
        printHelp = true;
        cerr << "Unrecognized option: " << argv[i] << endl;
      }
    } else {
      ++nFiles;
    }
  }
  if (argc < 2 || nFiles == 0 || printHelp) {
    cerr << "Usage: " << argv[0] << " [-h] FILE_OR_DIR [FILE_OR_DIR [...]]" << endl;
    return 1;
  }

  // Find and list file sizes
  ExtentSet es, total;
  for (int i = 1; i < argc; ++i) {
    if (argv[i][0] != '-') {
      es.clear();
      try {
        path p = resolve_path(argv[i]);
        if (is_directory(p))
          es.insertFromDir(p.c_str());
        else if (is_regular_file(p))
          es.insertFromFile(p.c_str());
        else
          throw runtime_error("Neither regular file nor directory");
        // Output
        if (humanReadable)
          cout << HumanSize(es.totalLength()) << '\t' << argv[i] << '\n';
        else
          cout << es.totalLength() << '\t' << argv[i] << '\n';
        total |= es;
      } catch (const exception& ex) {
        cerr << argv[i] << ": " << ex.what() << endl;
      }
    }
  }

  if (humanReadable)
    cout << HumanSize(es.totalLength()) << "\ttotal\n";
  else
    cout << total.totalLength() << "\ttotal\n";

  // TODO count also file metadata size, which is never shared

  return 0;
}
