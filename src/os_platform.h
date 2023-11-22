/*******************************************************************************
*    Veigar: Cross platform RPC library using shared memory.
*    ---------------------------------------------------------------------------
*    Copyright (C) 2023 winsoft666 <winsoft666@outlook.com>.
*
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/
#ifndef VEIGAR_PLATFORM_H_
#define VEIGAR_PLATFORM_H_

// detect platform

#if defined(WIN64) || defined(_WIN64) || defined(__WIN64__) ||                    \
    defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__) || \
    defined(WINCE) || defined(_WIN32_WCE)
#define VEIGAR_OS_WINDOWS
#elif defined(__linux__) || defined(linux) || defined(__linux)
#define VEIGAR_OS_LINUX
#elif defined(__APPLE__)
#define VEIGAR_OS_MACOS 1
#elif defined(__unix__) || defined(unix) || defined(__unix)
#define VEIGAR_OS_UNIX 1
#endif
#endif  // !VEIGAR_PLATFORM_H_
