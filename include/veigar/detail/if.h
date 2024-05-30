/*
 * Copyright (c) winsoft666.
 * All rights reserved.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */
#ifndef VEIGAR_DETAIL_IF_H_
#define VEIGAR_DETAIL_IF_H_
#pragma once

#include "veigar/detail/invoke.h"

namespace veigar {
namespace detail {

template <typename C, typename T, typename F>
using if_ = invoke<std::conditional<C::value, T, F>>;
}
}  // namespace veigar

#endif // !VEIGAR_DETAIL_IF_H_
