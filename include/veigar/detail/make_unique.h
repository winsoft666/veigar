#ifndef MAKE_UNIQUE_H_FOOBAR
#define MAKE_UNIQUE_H_FOOBAR
#pragma once

#include <memory>

namespace veigar {
namespace detail {

// Default behavior is to assume C++11, overriding VEIGAR_CXX_STANDARD can use newer standards:
#if VEIGAR_CXX_STANDARD >= 14

using std::make_unique;

#else

template <typename T, typename... Ts>
std::unique_ptr<T> make_unique(Ts&&... params) {
    return std::unique_ptr<T>(new T(std::forward<Ts>(params)...));
}

#endif

}  // namespace detail
}  // namespace veigar

#endif
