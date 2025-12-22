/** Utility class to exploit C++ destructors for file descriptors.
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
#include <string>
#include <utility>

/// Auto-closing file descriptor.
class UniqueFileDescriptor {
public:
  /// Default constructor, initializes an invalid descriptor and an empty path
  UniqueFileDescriptor() : m_fd(-1) {}

  /// Normal constructor, calls libc's open with the given arguments.
  /// Throws std::runtime_error if the return value of open is invalid.
  UniqueFileDescriptor(const char* file, int flag);

  /// Destructor. Calls libc's close if the descriptor is valid.
  inline ~UniqueFileDescriptor() { close(); }

  /// Deleted copy constructor (would break the auto-closing logic).
  UniqueFileDescriptor(const UniqueFileDescriptor&) = delete;

  /// Deleted copy assignment (would break the auto-closing logic).
  UniqueFileDescriptor& operator=(const UniqueFileDescriptor&) = delete;

  /// Move constructor (invalidates the argument).
  UniqueFileDescriptor(UniqueFileDescriptor&& x)
  : m_fd(std::exchange(x.m_fd, -1)), m_path(std::exchange(x.m_path, nullptr)) {}

  /// Move assignment (closes this and invalidates the argument).
  UniqueFileDescriptor& operator=(UniqueFileDescriptor&& x);

  /// Swap operation
  friend void swap(UniqueFileDescriptor& lhs, UniqueFileDescriptor& rhs);

  ////////////////////////////// Getters ///////////////////////////////

  /// Returns the descriptor
  inline int get() const { return m_fd; }

  /// Returns the descriptor
  inline operator int() const { return m_fd; }

  /// Returns true if the descriptor is valid (i.e. nonnegative)
  inline operator bool() const { return m_fd > -1; }

  /// Returns the file path passed to libc's open, or an empty string
  inline const std::string& getPath() const { return m_path; }

private:
  /// Closes the file descriptor (if valid) and sets m_fd and m_path to -1 and nullptr
  void close();

  int m_fd;
  std::string m_path;
};
