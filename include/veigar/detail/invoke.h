#ifndef INVOKE_H_0CWMPLUE
#define INVOKE_H_0CWMPLUE
#pragma once

namespace veigar {
namespace detail {

template <typename T>
using invoke = typename T::type;

}
}  // namespace veigar

#endif /* end of include guard: INVOKE_H_0CWMPLUE */
