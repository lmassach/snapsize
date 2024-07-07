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
#include <iostream>
#include <string>
using namespace std;

int main(int argc, char** argv) {
  cout << "Hello world!\nargc=" << argc;
  for (int i = 0; i < argc; ++i) {
    try {
      double d = stod(argv[i]);
      cout << "\nargv[" << i << "]=" << HumanSize(d);
      continue;
    } catch (const std::invalid_argument&) {} catch(const std::out_of_range&) {}
    cout << "\nargv[" << i << "]=" << argv[i];
  }
  cout << endl;
  return 0;
}
