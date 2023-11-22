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
#include "log.h"
#include "os_platform.h"
#include <memory>
#ifdef VEIGAR_OS_WINDOWS
#ifndef _INC_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif  // !WIN32_LEAN_AND_MEAN
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif  // !_WINSOCKAPI_
#include <Windows.h>
#endif
#include <strsafe.h>
#else
#include <string>
#include <stdarg.h>
#endif
#include <assert.h>
#include "string_helper.h"

namespace veigar {
void log(const wchar_t* format, ...) noexcept {
    std::wstring output;
    va_list args;
    va_start(args, format);
    const bool ret = StringHelper::StringPrintfV(format, args, output);
    va_end(args);

    assert(ret);
    if (ret) {
#ifdef VEIGAR_OS_WINDOWS
        OutputDebugStringW(output.c_str());
#else
        printf("%ls", output.c_str());
#endif
    }
}

void log(const char* format, ...) noexcept {
    std::string output;
    va_list args;
    va_start(args, format);
    const bool ret = StringHelper::StringPrintfV(format, args, output);
    va_end(args);

    assert(ret);
    if (ret) {
#ifdef VEIGAR_OS_WINDOWS
        OutputDebugStringA(output.c_str());
#else
        printf("%s", output.c_str());
#endif
    }
}
}  // namespace veigar