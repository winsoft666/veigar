#ifndef VEIGAR_DETAIL_ANY_H_
#define VEIGAR_DETAIL_ANY_H_
#pragma once

#include "veigar/detail/invoke.h"
#include "veigar/detail/if.h"
#include "veigar/detail/bool.h"

namespace veigar {
namespace detail {

//! \brief Evaluates to true_type if any of its arguments is true_type.
template <typename... T>
struct any : false_ {};

template <typename H, typename... T>
struct any<H, T...> : if_<H, true_, any<T...>> {};
}  // namespace detail
}  // namespace veigar

#endif  // !VEIGAR_DETAIL_ANY_H_
