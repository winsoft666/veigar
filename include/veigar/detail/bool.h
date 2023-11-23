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
