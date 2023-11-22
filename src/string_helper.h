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

#ifndef ASHE_STRING_HELPER_HPP__
#define ASHE_STRING_HELPER_HPP__
#include "os_platform.h"
#include <string>
#include <vector>

namespace veigar {
class StringHelper {
   public:
    // format a string
    static bool StringPrintfV(const char* format, va_list argList, std::string& output) noexcept;
    static bool StringPrintfV(const wchar_t* format, va_list argList, std::wstring& output) noexcept;

    static std::string StringPrintf(const char* format, ...) noexcept;
    static std::wstring StringPrintf(const wchar_t* format, ...) noexcept;

    static std::string StringPrintfV(const char* format, va_list argList) noexcept;
    static std::wstring StringPrintfV(const wchar_t* format, va_list argList) noexcept;
};
}  // namespace veigar
#endif  // !ASHE_STRING_HELPER_HPP__
