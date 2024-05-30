/*
 * Copyright (c) winsoft666.
 * All rights reserved.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */
#ifndef VEIGAR_DETAIL_FUNC_TOOLS_H_
#define VEIGAR_DETAIL_FUNC_TOOLS_H_
#pragma once

#include "veigar/detail/invoke.h"
#include "veigar/detail/all.h"
#include "veigar/detail/any.h"

namespace veigar {
namespace detail {

enum class enabled {};

template <typename... C>
using enable_if = invoke<std::enable_if<all<C...>::value, enabled>>;

template <typename... C>
using disable_if = invoke<std::enable_if<!any<C...>::value, enabled>>;

}  // namespace detail
}  // namespace veigar

#endif  // !VEIGAR_DETAIL_FUNC_TOOLS_H_
