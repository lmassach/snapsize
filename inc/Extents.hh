/** Utilities to exploit the fiemap ioctl, see
 * <https://www.kernel.org/doc/Documentation/filesystems/fiemap.txt>.
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
#include <linux/types.h>
#include <set>

/// Simple class to represent an extent. Zero-length extents are all
/// represented as starting at zero.
class Extent {
public:
  //////////////////////////// Constructors ////////////////////////////

  /// Default constructor (sets start and length at zero)
  Extent() : Extent(0, 0) {}

  /// Normal constructor requiring start and length of the extent
  Extent(__u64 start, __u64 length) : m_start(length ? start : 0), m_length(length) {}

  /// Static constructor-method taking start and one-past-end positions
  static inline Extent FromTo(__u64 start, __u64 end) { return Extent(start, end - start); }

  /////////////////////////////// Getters //////////////////////////////

  /// Returns the starting position of the extent
  inline __u64 start() const { return m_start; }

  /// Returns the length of the extent
  inline __u64 length() const { return m_length; }

  /// Returns the position that is one position past the end of the extent
  inline __u64 end() const { return m_start + m_length; }

  ///////////////////////////// Comparison /////////////////////////////

  friend bool operator<(const Extent& lhs, const Extent& rhs) {
    return lhs.m_start < rhs.m_start || (lhs.m_start == rhs.m_start && lhs.m_length < rhs.m_length);
  }

  friend bool operator>(const Extent& lhs, const Extent& rhs) { return rhs < lhs; }
  friend bool operator<=(const Extent& lhs, const Extent& rhs) { return !(lhs > rhs); }
  friend bool operator>=(const Extent& lhs, const Extent& rhs) { return !(lhs < rhs); }

  friend bool operator==(const Extent& lhs, const Extent& rhs) {
    return lhs.m_start == rhs.m_start && lhs.m_length == rhs.m_length;
  }

  friend bool operator!=(const Extent& lhs, const Extent& rhs) { return !(lhs == rhs); }

  ////////////////////////////// Relations /////////////////////////////

  /// Returns true if `other` is contained or equal to this extent
  bool contains(const Extent& other) const;

  /// Returns true if `other` contains or is equal to this extent
  inline bool isContained(const Extent& other) const { return other.contains(*this); }

  /// Returns true if the extents have any position in common
  bool overlaps(const Extent& other) const;

  /// Returns true if the extents can be joined, i.e. if they overlap or
  /// one starts where the other ends
  bool joins(const Extent& other) const;

  ////////////////////////////// Operators /////////////////////////////

  /// In-place intersection. Returns Extent(0, 0) if the extents do not overlap.
  Extent& operator&=(const Extent& rhs);

  /// Intersection. Returns Extent(0, 0) if the extents do not overlap.
  friend Extent operator&(Extent lhs, const Extent& rhs) { lhs &= rhs; return lhs; }

  /// In-place union/join. Throws std::domain_error if the extents are not contiguous.
  Extent& operator|=(const Extent& rhs);

  /// Union/join. Throws std::domain_error if the extents are not contiguous.
  friend Extent operator|(Extent lhs, const Extent& rhs) { lhs |= rhs; return lhs; }

private:
  __u64 m_start, m_length;
};


/// Specialized class containing a set of extents. Overlapping or
/// contiguous extents are automatically coalesced, minimizing memory
/// usage. Extents are sorted by starting position.
class ExtentSet {
public:
  ////////////////////////////// Typedefs //////////////////////////////

  typedef std::set<Extent>::iterator iterator; // iterator and const_interator are the same for sets
  typedef std::set<Extent>::const_reverse_iterator const_reverse_iterator;

  ////////////////////////////// Modifiers /////////////////////////////

  void insert(Extent x);

  inline void clear() { m_set.clear(); m_totalSizeCache = 0; }

  /// Inserts all the Extents from the given file
  void insertFromFile(const char* path);

  /// Inserts all the Extents from all files in path (recursively)
  void insertFromDir(const char* path, bool stopOnError = false);

  ////////////////////////////// Capacity //////////////////////////////

  inline bool empty() const { return m_set.empty(); }
  inline operator bool() const { return m_set.empty(); }
  inline std::size_t size() const { return m_set.size(); }

  ////////////////////////////// Accessors /////////////////////////////

  /// Returns a const reference to the first element. Throws std::out_of_range if empty.
  const Extent& first() const;

  /// Returns a const reference to the last element. Throws std::out_of_range if empty.
  const Extent& last() const;

  ////////////////////////////// Iterators /////////////////////////////

  inline iterator begin() const { return m_set.begin(); }
  inline iterator end() const { return m_set.end(); }
  inline const_reverse_iterator rbegin() const { return m_set.rbegin(); }
  inline const_reverse_iterator rend() const { return m_set.rend(); }

  ///////////////////////////// Statistics /////////////////////////////

  __u64 totalLength() const;

  ////////////////////////////// Operators /////////////////////////////

  /// In-place intersection
  ExtentSet& operator&=(const ExtentSet& rhs);

  /// Intersection
  friend ExtentSet operator&(const ExtentSet& lhs, const ExtentSet& rhs);

  /// In-place union/join
  ExtentSet& operator|=(const ExtentSet& rhs);

  /// Union/join
  friend ExtentSet operator|(ExtentSet lhs, const ExtentSet& rhs) { lhs |= rhs; return lhs; }

private:
  std::set<Extent> m_set;
  mutable __u64 m_totalSizeCache = 0;
};
