/*
 * Copyright (c) winsoft666.
 * All rights reserved.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */
#ifndef VEIGAR_DETAIL_INVOKE_H_
#define VEIGAR_DETAIL_INVOKE_H_
#pragma once

namespace veigar {
namespace detail {

template <typename T>
using invoke = typename T::type;

}
}  // namespace veigar

#endif // !VEIGAR_DETAIL_INVOKE_H_
