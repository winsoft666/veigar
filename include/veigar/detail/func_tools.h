#pragma once

#ifndef FUNC_TOOLS_H_9FNRD4G2
#define FUNC_TOOLS_H_9FNRD4G2

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

#endif
