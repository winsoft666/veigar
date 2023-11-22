#pragma once

#ifndef IF_H_1OW9DR7G
#define IF_H_1OW9DR7G

#include "veigar/detail/invoke.h"

namespace veigar {
namespace detail {

template <typename C, typename T, typename F>
using if_ = invoke<std::conditional<C::value, T, F>>;
}
}

#endif /* end of include guard: IF_H_1OW9DR7G */
