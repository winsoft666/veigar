namespace veigar {
namespace detail {
template <typename F>
bool CallDispatcher::bind(std::string const& name, F func) noexcept {
    if (name.empty() || isFuncNameExist(name)) {
        return false;
    }

    return bind(name,
                func,
                typename detail::func_kind_info<F>::result_kind(),
                typename detail::func_kind_info<F>::args_kind());
}

template <typename F>
bool CallDispatcher::bind(std::string const& name, F func, detail::tags::void_result const&, detail::tags::zero_arg const&) noexcept {
    using args_type = typename func_traits<F>::args_type;
    if (name.empty() || isFuncNameExist(name)) {
        return false;
    }

    funcs_.insert(
        std::make_pair(name, [func, name](veigar_msgpack::object const& args) {
            constexpr int args_count = std::tuple_size<args_type>::value;
            assert(args_count == args.via.array.size);
            func();
            return detail::make_unique<veigar_msgpack::object_handle>();
        }));
    return true;
}

template <typename F>
bool CallDispatcher::bind(std::string const& name, F func, detail::tags::void_result const&, detail::tags::nonzero_arg const&) noexcept {
    using detail::func_traits;
    using args_type = typename func_traits<F>::args_type;

    if (name.empty() || isFuncNameExist(name)) {
        return false;
    }

    funcs_.insert(
        std::make_pair(name,
                       [func, name](veigar_msgpack::object const& args) {
                           constexpr int args_count = std::tuple_size<args_type>::value;
                           assert(args_count == args.via.array.size);

                           args_type args_real;
                           // note: dispatcher will catch this type_error exception.
                           args.convert(args_real);
                           detail::call(func, args_real);

                           return detail::make_unique<veigar_msgpack::object_handle>();
                       }));
    return true;
}

template <typename F>
bool CallDispatcher::bind(std::string const& name, F func, detail::tags::nonvoid_result const&, detail::tags::zero_arg const&) noexcept {
    using detail::func_traits;
    using args_type = typename func_traits<F>::args_type;

    if (name.empty() || isFuncNameExist(name)) {
        return false;
    }

    funcs_.insert(
        std::make_pair(name,
                       [func, name](veigar_msgpack::object const& args) {
                           constexpr int args_count = std::tuple_size<args_type>::value;
                           assert(args_count == args.via.array.size);

                           auto z = detail::make_unique<veigar_msgpack::zone>();
                           auto result = veigar_msgpack::object(func(), *z);

                           return detail::make_unique<veigar_msgpack::object_handle>(result, std::move(z));
                       }));
    return true;
}

template <typename F>
bool CallDispatcher::bind(std::string const& name, F func, detail::tags::nonvoid_result const&, detail::tags::nonzero_arg const&) noexcept {
    using detail::func_traits;
    using args_type = typename func_traits<F>::args_type;

    if (name.empty() || isFuncNameExist(name)) {
        return false;
    }

    funcs_.insert(std::make_pair(name,
                                 [func, name](veigar_msgpack::object const& args) {
                                     constexpr int args_count = std::tuple_size<args_type>::value;
                                     assert(args_count == args.via.array.size);

                                     args_type args_real;
                                     // note: dispatcher will catch this type_error exception.
                                     args.convert(args_real);

                                     auto z = detail::make_unique<veigar_msgpack::zone>();
                                     auto result = veigar_msgpack::object(detail::call(func, args_real), *z);

                                     return detail::make_unique<veigar_msgpack::object_handle>(result, std::move(z));
                                 }));

    return true;
}
}  // namespace detail
}  // namespace veigar
