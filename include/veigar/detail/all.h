#ifndef VEIGAR_DETAIL_ALL_H_
#define VEIGAR_DETAIL_ALL_H_
#pragma once

#include "veigar/detail/invoke.h"
#include "veigar/detail/if.h"
#include "veigar/detail/bool.h"

namespace veigar {
namespace detail {

// This type can be used to check multiple conditions.
// It will be true_type if all its arguments are true.
template <typename... T>
struct all : true_ {};

template <typename H, typename... T>
struct all<H, T...>
    : if_<H, all<T...>, false_> {};

}  // namespace detail
}  // namespace veigar

#endif // !VEIGAR_DETAIL_ALL_H_
