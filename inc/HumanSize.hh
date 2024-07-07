/** Utility class to print file sizes in human-readable format.
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
#include <iostream>

class HumanSize {
public:
  explicit HumanSize() : HumanSize(0.0) {}
  HumanSize(double sz);

  friend std::ostream& operator<<(std::ostream& stream, const HumanSize& hsz);

private:
  double m_sz;
  char m_suffix;
};
