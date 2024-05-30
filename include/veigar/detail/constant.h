/*
 * Copyright (c) winsoft666.
 * All rights reserved.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */
#ifndef VEIGAR_DETAIL_CONSTANT_H_
#define VEIGAR_DETAIL_CONSTANT_H_
#pragma once

#include <type_traits>

namespace veigar {
namespace detail {

template <typename T, T I>
struct constant : std::integral_constant<T, I> {};

}  // namespace detail
}  // namespace veigar

#endif  // !VEIGAR_DETAIL_CONSTANT_H_
