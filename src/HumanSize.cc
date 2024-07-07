/** Utility class to print file sizes in human-readable format
 * (implementation).
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
#include <iomanip>
using namespace std;

HumanSize::HumanSize(double sz) : m_sz(sz), m_suffix(' ') {
    for (char suffix : {'K', 'M', 'G', 'T', 'P', 'E'}) {
      if (m_sz < 1000.0)
        break;
      m_sz /= 1024.0;
      m_suffix = suffix;
    }
  }

ostream& operator<<(ostream& stream, const HumanSize& hsz) {
  const int p = (hsz.m_sz < 10.0) ? 2 : (hsz.m_sz < 100.0) ? 1 : 0;
  return (stream << fixed << setprecision(p) << hsz.m_sz << hsz.m_suffix);
}
