/*
 * Copyright (c) winsoft666.
 * All rights reserved.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */
#ifndef VEIGAR_GUID_H__
#define VEIGAR_GUID_H__
#pragma once

#include <string>
#include "os_platform.h"

namespace veigar {
class UUID {
   public:
    static std::string Create() noexcept;

   protected:
#ifndef VEIGAR_OS_WINDOWS
    static uint32_t Rand32() noexcept;
    static std::string GenUuid4() noexcept;
#endif
};
}  // namespace veigar
#endif  // !VEIGAR_GUID_H__