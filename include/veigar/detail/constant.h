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
