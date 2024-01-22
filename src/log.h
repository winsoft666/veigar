/*
 * Copyright (c) winsoft666.
 * All rights reserved.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */
#ifndef VEIGAR_LOG_H_
#define VEIGAR_LOG_H_
#pragma once

namespace veigar {
void log(const wchar_t* format, ...) noexcept;
void log(const char* format, ...) noexcept;
}  // namespace veigar

#endif  // !VEIGAR_LOG_H_