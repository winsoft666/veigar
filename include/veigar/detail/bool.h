/*
 * Copyright (c) winsoft666.
 * All rights reserved.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */
#ifndef VEIGAR_DETAIL_BOOL_H_
#define VEIGAR_DETAIL_BOOL_H_
#pragma once

#include "veigar/detail/constant.h"

namespace veigar {
namespace detail {

template <bool B>
using bool_ = constant<bool, B>;

using true_ = bool_<true>;

using false_ = bool_<false>;

}  // namespace detail
}  // namespace veigar

#endif  // !VEIGAR_DETAIL_BOOL_H_
