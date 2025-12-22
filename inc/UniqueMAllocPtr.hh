/** Utility class to exploit C++ destructors for malloc and free.
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
#include <cstdlib>
#include <stdexcept>
#include <utility>

/// Auto-freeing malloc pointer.
template <class T> class UniqueMAllocPtr {
public:
  /// Default constructor, setting a null pointer without calling malloc
  UniqueMAllocPtr() : m_ptr(nullptr), m_size(0) {}

  /// Normal constructor, calling malloc and throwing std::runtime_error on failure
  UniqueMAllocPtr(std::size_t sz) : m_ptr(nullptr), m_size(sz) {
    if (!(m_ptr = (T*)std::malloc(sz)))
      throw std::runtime_error("malloc failed");
  }

  /// Destructor. Calls free if the pointer is non-null.
  inline ~UniqueMAllocPtr() { free(); }

  /// Deleted copy constructor (would break the auto-closing logic).
  UniqueMAllocPtr(const UniqueMAllocPtr<T>&) = delete;

  /// Deleted copy assignment (would break the auto-closing logic).
  UniqueMAllocPtr& operator=(const UniqueMAllocPtr<T>&) = delete;

  UniqueMAllocPtr(UniqueMAllocPtr<T>&& p)
  : m_ptr(std::exchange(p.m_ptr, nullptr)), m_size(std::exchange(p.m_size, 0)) {}

  UniqueMAllocPtr& operator=(UniqueMAllocPtr<T>&& p) {
    free();
    m_ptr = std::exchange(p.m_ptr, nullptr);
    m_size = std::exchange(p.m_size, 0);
  }

  /////////////////////////////// Getters //////////////////////////////

  inline T* get() const { return m_ptr; }
  inline operator T*() const { return m_ptr; }
  inline T* operator->() const { return m_ptr; }
  inline operator void*() const { return m_ptr; }
  inline operator bool() const { return m_ptr; }
  inline std::size_t size() const { return m_size; }

  ////////////////////////////// Modifiers /////////////////////////////

  /// Calls realloc. Throws std::runtime_error on failure. If force is
  /// false (default) and the requested size is smaller than the
  /// already-allocated size, does nothing.
  void realloc(std::size_t sz, bool force = false) {
    if (!force && sz <= m_size)
      return;
    T* ptr = (T*)std::realloc(m_ptr, sz);
    if (ptr)
      m_ptr = ptr;
    else
      throw std::runtime_error("realloc failed");
  }

  /// Calls free and set pointer to null and size to zero
  void free() {
    ::free(m_ptr);
    m_ptr = nullptr;
    m_size = 0;
  }

private:
  T* m_ptr;
  std::size_t m_size;
};
