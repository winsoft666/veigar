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
#include "string_helper.h"
#include <algorithm>
#include <cctype>
#include <cstdarg>
#include <cwctype>
#include <iterator>
#include <sstream>
#include <cassert>
#ifdef VEIGAR_OS_WINDOWS
#ifndef _INC_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif  // !WIN32_LEAN_AND_MEAN
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif  // !_WINSOCKAPI_
#include <Windows.h>
#endif  // !_INC_WINDOWS
#include <strsafe.h>
#endif

#if defined(__GNUC__)
#define VA_COPY(a, b) (va_copy(a, b))
#else
#define VA_COPY(a, b) (a = b)
#endif

namespace veigar {

// format a string
bool StringHelper::StringPrintfV(const char* format, va_list argList, std::string& output) noexcept {
    if (!format)
        return false;

    bool ret = false;
#ifdef VEIGAR_OS_WINDOWS
    char* pMsgBuffer = NULL;
    size_t iMsgBufCount = 0;

    HRESULT hr = STRSAFE_E_INSUFFICIENT_BUFFER;
    try {
        while (hr == STRSAFE_E_INSUFFICIENT_BUFFER) {
            iMsgBufCount += 1024;
            if (pMsgBuffer) {
                free(pMsgBuffer);
                pMsgBuffer = NULL;
            }

            pMsgBuffer = (char*)malloc(iMsgBufCount * sizeof(char));
            if (!pMsgBuffer) {
                break;
            }
            hr = StringCchVPrintfA(pMsgBuffer, iMsgBufCount, format, argList);
        }

        if (hr == S_OK && pMsgBuffer) {
            output.assign(pMsgBuffer);
        }

        if (pMsgBuffer) {
            free(pMsgBuffer);
            pMsgBuffer = NULL;
        }

        ret = (hr == S_OK);
    } catch (std::exception& e) {
        (e);
        ret = false;
    }

    return ret;
#else
    char* msgBuf = nullptr;
    size_t msgBufSize = 1024;

    try {
        do {
            if (msgBuf) {
                free(msgBuf);
                msgBuf = nullptr;
            }
            msgBuf = (char*)malloc(msgBufSize * sizeof(char));
            if (!msgBuf) {
                break;
            }
            memset(msgBuf, 0, msgBufSize * sizeof(char));

            va_list va_copy;
            VA_COPY(va_copy, argList);
            const int err = vsnprintf(msgBuf, msgBufSize, format, va_copy);
            if (err >= 0 && err < msgBufSize) {
                ret = true;
                break;
            }

            msgBufSize *= 2;
        } while (true);

        if (result && msgBuf) {
            output.assign(msgBuf);
        }

        if (msgBuf) {
            free(msgBuf);
            msgBuf = nullptr;
        }
    } catch (std::exception& e) {
        (e);
        ret = false;
    }

    return ret;
#endif
}

bool StringHelper::StringPrintfV(const wchar_t* format, va_list argList, std::wstring& output) noexcept {
    if (!format)
        return false;

    bool ret = false;
#ifdef VEIGAR_OS_WINDOWS
    wchar_t* pMsgBuffer = NULL;
    size_t iMsgBufCount = 0;

    HRESULT hr = STRSAFE_E_INSUFFICIENT_BUFFER;
    try {
        while (hr == STRSAFE_E_INSUFFICIENT_BUFFER) {
            iMsgBufCount += 1024;
            if (pMsgBuffer) {
                free(pMsgBuffer);
                pMsgBuffer = NULL;
            }

            pMsgBuffer = (wchar_t*)malloc(iMsgBufCount * sizeof(wchar_t));
            if (!pMsgBuffer) {
                break;
            }
            hr = StringCchVPrintfW(pMsgBuffer, iMsgBufCount, format, argList);
        }

        if (hr == S_OK && pMsgBuffer) {
            output.assign(pMsgBuffer);
        }

        if (pMsgBuffer) {
            free(pMsgBuffer);
            pMsgBuffer = NULL;
        }

        ret = (hr == S_OK);
    } catch (std::exception& e) {
        (e);
        ret = false;
    }

    return ret;
#else
    wchar_t* msgBuf = nullptr;
    size_t msgBufSize = 1024;

    try {
        do {
            if (msgBuf) {
                free(msgBuf);
                msgBuf = nullptr;
            }
            msgBuf = (wchar_t*)malloc(msgBufSize * sizeof(wchar_t));
            if (!msgBuf) {
                break;
            }
            memset(msgBuf, 0, msgBufSize * sizeof(wchar_t));

            va_list va_copy;
            VA_COPY(va_copy, argList);
            const int err = vswprintf(msgBuf, msgBufSize, format, va_copy);
            if (err >= 0 && err < msgBufSize) {
                ret = true;
                break;
            }

            msgBufSize *= 2;
        } while (true);

        if (result && msgBuf) {
            output.assign(msgBuf);
        }

        if (msgBuf) {
            free(msgBuf);
            msgBuf = nullptr;
        }
    } catch (std::exception& e) {
        (e);
        ret = false;
    }

    return ret;
#endif
}

std::string StringHelper::StringPrintf(const char* format, ...) noexcept {
    std::string output;
    try {
        va_list args;
        va_start(args, format);

        StringPrintfV(format, args, output);
        va_end(args);
    } catch (std::exception& e) {
        (e);
        output.clear();
    }

    return output;
}

std::wstring StringHelper::StringPrintf(const wchar_t* format, ...) noexcept {
    std::wstring output;
    try {
        va_list args;
        va_start(args, format);
        StringPrintfV(format, args, output);
        va_end(args);
    } catch (std::exception& e) {
        (e);
        output.clear();
    }
    return output;
}

std::string StringHelper::StringPrintfV(const char* format, va_list argList) noexcept {
    std::string output;
    StringPrintfV(format, argList, output);
    return output;
}

std::wstring StringHelper::StringPrintfV(const wchar_t* format, va_list argList) noexcept {
    std::wstring output;
    StringPrintfV(format, argList, output);
    return output;
}
}  // namespace veigar