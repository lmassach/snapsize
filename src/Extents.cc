/** Utilities to exploit the fiemap ioctl (implementation).
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
#include <algorithm>
#include <stdexcept>
using namespace std;

bool Extent::contains(const Extent& other) const {
  return m_start <= other.m_start && end() >= other.end();
}

bool Extent::overlaps(const Extent& other) const {
  return m_start < other.end() && end() > other.m_start;
}

bool Extent::joins(const Extent& other) const {
  return m_start <= other.end() && end() >= other.m_start;
}

Extent& Extent::operator&=(const Extent& rhs) {
  if (overlaps(rhs)) {
    __u64 newEnd = min(end(), rhs.end());
    m_start = max(m_start, rhs.m_start);
    m_length = newEnd - m_start;
  } else {
    m_start = m_length = 0;
  }
  return *this;
}

Extent& Extent::operator|=(const Extent& rhs) {
  if (!joins(rhs))
    throw domain_error("The extents you are trying to join are not contiguous");
  __u64 newEnd = max(end(), rhs.end());
  m_start = min(m_start, rhs.m_start);
  m_length = newEnd - m_start;
  return *this;
}


void ExtentSet::insert(Extent x) {
  m_totalSizeCache = 0; // Invalidate cache
  // Find the last element that starts before x
  iterator it = m_set.lower_bound(x);
  if (it != m_set.begin()) --it;
  // Iterate until all elements that may be joinable with x are found
  while (it != m_set.end() && x.end() >= it->start()) {
    if (x.joins(*it)) {
      // Join contiguous extents into x
      x |= *it;
      // Remove the old extent, which is now included in x. First `it`
      // is incremented, then its previous value is passed to erase,
      // which will invalidate the old value but not the new one.
      m_set.erase(it++);
    } else
      ++it;
  }
  // Insert the element
  m_set.insert(x);
}

const Extent& ExtentSet::first() const {
  if (empty())
    throw out_of_range("The ExtentSet is empty");
  return *m_set.begin();
}

const Extent& ExtentSet::last() const {
  if (empty())
    throw out_of_range("The ExtentSet is empty");
  return *(--m_set.end());
}

__u64 ExtentSet::totalLength() const {
  if (!m_totalSizeCache)
    for (const Extent& x : m_set)
      m_totalSizeCache += x.length();
  return m_totalSizeCache;
}

ExtentSet& ExtentSet::operator&=(const ExtentSet& rhs) {
  // No need to invalidate m_totalSizeCache (clear or assignment will do it)
  if (empty() || rhs.empty() || rhs.last().end() <= first().start() || last().end() <= rhs.first().start())
    clear();
  else
    *this = *this & rhs;
  return *this;
}

ExtentSet operator&(const ExtentSet& lhs, const ExtentSet& rhs) {
  ExtentSet res;
  if (lhs.empty() || rhs.empty() || rhs.last().end() <= lhs.first().start() || lhs.last().end() <= rhs.first().start())
    return res;

  ExtentSet::iterator a = lhs.begin(), b = rhs.begin();
  while (a != lhs.end()) { // Iterate on lhs
    if (a->end() <= b->start()) { // a is entirely before b
      ++a; // Advance a
    } else {
      // Advance b until an overlap is found
      do {
        if (a->overlaps(*b))
          res.insert(*a & *b);
        ++b;
      } while (b != rhs.end() && b->start() < a->end());
      if (b == rhs.end()) break;
    }
  }
  return res;
}

ExtentSet& ExtentSet::operator|=(const ExtentSet& rhs) {
  // No need to invalidate m_totalSizeCache (insert will do it)
  for (const Extent& x : rhs)
    insert(x);
  return *this;
}
