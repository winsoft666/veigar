/*
 * Copyright (c) winsoft666.
 * All rights reserved.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include "uuid.h"
#ifdef VEIGAR_OS_WINDOWS
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif  // !_WINSOCKAPI_
#include <combaseapi.h>
#else
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#endif
#include "string_helper.h"

namespace veigar {
#ifndef VEIGAR_OS_WINDOWS
uint32_t UUID::Rand32() {
    return ((rand() & 0x3) << 30) | ((rand() & 0x7fff) << 15) |
           (rand() & 0x7fff);
}

std::string UUID::GenUuid4() {
    return StringHelper::StringPrintf(
        "%08x%04x%04x%04x%04x%08x",
        Rand32(),           // Generates a 32-bit Hex number
        Rand32() & 0xffff,  // Generates a 16-bit Hex number
        ((Rand32() & 0x0fff) |
         0x4000),  // Generates a 16-bit Hex number of the form 4xxx (4 indicates
                   // the UUID version)
        (Rand32() & 0x3fff) +
            0x8000,  // Generates a 16-bit Hex number in the range [0x8000, 0xbfff]
        Rand32() & 0xffff,
        Rand32());
}
#endif

std::string UUID::Create() {
#ifdef VEIGAR_OS_WINDOWS
    GUID guid;
    if (S_OK != CoCreateGuid(&guid))
        return std::string();

    return StringHelper::StringPrintf(
        "%08x%04x%04x%02x%02x%02x%02x%02x%02x%02x%02x",
        guid.Data1,
        guid.Data2,
        guid.Data3,
        guid.Data4[0],
        guid.Data4[1],
        guid.Data4[2],
        guid.Data4[3],
        guid.Data4[4],
        guid.Data4[5],
        guid.Data4[6],
        guid.Data4[7]);
#else
    return GenUuid4();
#endif
}
}  // namespace veigar