//
// MessagePack for C++ static resolution routine
//
// Copyright (C) 2008-2016 FURUHASHI Sadayuki and KONDO Takatoshi
//
//    Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//    http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef MSGPACK_V1_CPP11_MSGPACK_TUPLE_HPP
#define MSGPACK_V1_CPP11_MSGPACK_TUPLE_HPP

#include "veigar/msgpack/v1/adaptor/detail/cpp11_msgpack_tuple_decl.hpp"
#include "veigar/msgpack/adaptor/adaptor_base.hpp"
#include "veigar/msgpack/pack.hpp"

namespace veigar_msgpack {

/// @cond
MSGPACK_API_VERSION_NAMESPACE(v1) {
/// @endcond

namespace type {

template <class... Args>
inline tuple<Args...> make_tuple(Args&&... args) {
    return tuple<Args...>(std::forward<Args>(args)...);
}

template<class... Args>
inline tuple<Args&&...> forward_as_tuple (Args&&... args) noexcept {
    return tuple<Args&&...>(std::forward<Args>(args)...);
}

template <class... Tuples>
inline auto tuple_cat(Tuples&&... args) ->
    decltype(
        std::tuple_cat(std::forward<typename std::remove_reference<Tuples>::type::base>(args)...)
    ) {
    return std::tuple_cat(std::forward<typename std::remove_reference<Tuples>::type::base>(args)...);
}

template <class... Args>
inline tuple<Args&...> tie(Args&... args) {
    return tuple<Args&...>(args...);
}
} // namespace type

// --- Pack from tuple to packer stream ---
template <typename Stream, typename Tuple, std::size_t N>
struct MsgpackTuplePacker {
    static void pack(
        veigar_msgpack::packer<Stream>& o,
        const Tuple& v) {
        MsgpackTuplePacker<Stream, Tuple, N-1>::pack(o, v);
        o.pack(v.template get<N-1>());
    }
};

template <typename Stream, typename Tuple>
struct MsgpackTuplePacker<Stream, Tuple, 1> {
    static void pack (
        veigar_msgpack::packer<Stream>& o,
        const Tuple& v) {
        o.pack(v.template get<0>());
    }
};

template <typename Stream, typename Tuple>
struct MsgpackTuplePacker<Stream, Tuple, 0> {
    static void pack (
        veigar_msgpack::packer<Stream>&,
        const Tuple&) {
    }
};

namespace adaptor {

template <typename... Args>
struct pack<veigar_msgpack::type::tuple<Args...>> {
    template <typename Stream>
    veigar_msgpack::packer<Stream>& operator()(
        veigar_msgpack::packer<Stream>& o,
        const veigar_msgpack::type::tuple<Args...>& v) const {
        o.pack_array(sizeof...(Args));
        MsgpackTuplePacker<Stream, decltype(v), sizeof...(Args)>::pack(o, v);
        return o;
    }
};

} // namespace adaptor

// --- Convert from tuple to object ---

template <typename T, typename... Args>
struct MsgpackTupleAsImpl {
    static veigar_msgpack::type::tuple<T, Args...> as(veigar_msgpack::object const& o) {
        return veigar_msgpack::type::tuple_cat(
            veigar_msgpack::type::make_tuple(o.via.array.ptr[o.via.array.size - sizeof...(Args) - 1].as<T>()),
            MsgpackTupleAs<Args...>::as(o));
    }
};

template <typename... Args>
struct MsgpackTupleAs {
    static veigar_msgpack::type::tuple<Args...> as(veigar_msgpack::object const& o) {
        return MsgpackTupleAsImpl<Args...>::as(o);
    }
};

template <>
struct MsgpackTupleAs<> {
    static veigar_msgpack::type::tuple<> as (veigar_msgpack::object const&) {
        return veigar_msgpack::type::tuple<>();
    }
};

template <typename Tuple, std::size_t N>
struct MsgpackTupleConverter {
    static void convert(
        veigar_msgpack::object const& o,
        Tuple& v) {
        MsgpackTupleConverter<Tuple, N-1>::convert(o, v);
        if (o.via.array.size >= N)
            o.via.array.ptr[N-1].convert<typename std::remove_reference<decltype(v.template get<N-1>())>::type>(v.template get<N-1>());
    }
};

template <typename Tuple>
struct MsgpackTupleConverter<Tuple, 1> {
    static void convert (
        veigar_msgpack::object const& o,
        Tuple& v) {
        o.via.array.ptr[0].convert<typename std::remove_reference<decltype(v.template get<0>())>::type>(v.template get<0>());
    }
};

template <typename Tuple>
struct MsgpackTupleConverter<Tuple, 0> {
    static void convert (
        veigar_msgpack::object const&,
        Tuple&) {
    }
};

namespace adaptor {

template <typename... Args>
struct as<veigar_msgpack::type::tuple<Args...>, typename std::enable_if<veigar_msgpack::any_of<veigar_msgpack::has_as, Args...>::value>::type>  {
    veigar_msgpack::type::tuple<Args...> operator()(
        veigar_msgpack::object const& o) const {
        if (o.type != veigar_msgpack::type::ARRAY) { throw veigar_msgpack::type_error(); }
        return MsgpackTupleAs<Args...>::as(o);
    }
};

template <typename... Args>
struct convert<veigar_msgpack::type::tuple<Args...>> {
    veigar_msgpack::object const& operator()(
        veigar_msgpack::object const& o,
        veigar_msgpack::type::tuple<Args...>& v) const {
        if(o.type != veigar_msgpack::type::ARRAY) { throw veigar_msgpack::type_error(); }
        MsgpackTupleConverter<decltype(v), sizeof...(Args)>::convert(o, v);
        return o;
    }
};

} // namespace adaptor

// --- Convert from tuple to object with zone ---
template <typename Tuple, std::size_t N>
struct MsgpackTupleToObjectWithZone {
    static void convert(
        veigar_msgpack::object::with_zone& o,
        const Tuple& v) {
        MsgpackTupleToObjectWithZone<Tuple, N-1>::convert(o, v);
        o.via.array.ptr[N-1] = veigar_msgpack::object(v.template get<N-1>(), o.zone);
    }
};

template <typename Tuple>
struct MsgpackTupleToObjectWithZone<Tuple, 1> {
    static void convert (
        veigar_msgpack::object::with_zone& o,
        const Tuple& v) {
        o.via.array.ptr[0] = veigar_msgpack::object(v.template get<0>(), o.zone);
    }
};

template <typename Tuple>
struct MsgpackTupleToObjectWithZone<Tuple, 0> {
    static void convert (
        veigar_msgpack::object::with_zone&,
        const Tuple&) {
    }
};

namespace adaptor {

template <typename... Args>
    struct object_with_zone<veigar_msgpack::type::tuple<Args...>> {
    void operator()(
        veigar_msgpack::object::with_zone& o,
        veigar_msgpack::type::tuple<Args...> const& v) const {
        o.type = veigar_msgpack::type::ARRAY;
        o.via.array.ptr = static_cast<veigar_msgpack::object*>(o.zone.allocate_align(sizeof(veigar_msgpack::object)*sizeof...(Args), MSGPACK_ZONE_ALIGNOF(veigar_msgpack::object)));
        o.via.array.size = sizeof...(Args);
        MsgpackTupleToObjectWithZone<decltype(v), sizeof...(Args)>::convert(o, v);
    }
};

}  // namespace adaptor

/// @cond
}  // MSGPACK_API_VERSION_NAMESPACE(v1)
///@endcond

}  // namespace veigar_msgpack

#endif // MSGPACK_CPP11_MSGPACK_TUPLE_HPP
