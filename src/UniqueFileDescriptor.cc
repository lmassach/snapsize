/** Utility class to exploit C++ destructors for file descriptors
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
#include "UniqueFileDescriptor.hh"
#include <algorithm>
#include <stdexcept>
#include <string>
#include <fcntl.h>
#include <unistd.h>
using namespace std::string_literals;

UniqueFileDescriptor::UniqueFileDescriptor(const char* file, int flag) : m_fd(-1), m_path(file) {
  m_fd = open(m_path, flag);
  if (m_fd < 0)
    throw std::runtime_error("Could not open file: "s + m_path);
}

UniqueFileDescriptor& UniqueFileDescriptor::operator=(UniqueFileDescriptor&& x) {
  close();
  m_fd = std::exchange(x.m_fd, -1);
  m_path = std::exchange(x.m_path, nullptr);
  return *this;
}

void swap(UniqueFileDescriptor& lhs, UniqueFileDescriptor& rhs) {
  std::swap(lhs.m_fd, rhs.m_fd);
  std::swap(lhs.m_path, rhs.m_path);
}

void UniqueFileDescriptor::close() {
  if (m_fd > -1)
    ::close(m_fd);
  m_fd = -1;
  m_path = nullptr;
}
