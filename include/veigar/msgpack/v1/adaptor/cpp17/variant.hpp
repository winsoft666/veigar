#ifndef __VEIGAR_MSGPACK_V1_ADAPTOR_CPP17_VARIANT_HPP // Add by msgpack.py
#define __VEIGAR_MSGPACK_V1_ADAPTOR_CPP17_VARIANT_HPP
//
// MessagePack for C++ static resolution routine
//
// Copyright (C) 2023 Uy Ha
//
//    Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//    http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef MSGPACK_V1_TYPE_VARIANT_HPP
#define MSGPACK_V1_TYPE_VARIANT_HPP

#if defined(MSGPACK_USE_STD_VARIANT_ADAPTOR)

#include "veigar/msgpack/cpp_version.hpp"

#if MSGPACK_CPP_VERSION >= 201703

#include "veigar/msgpack/adaptor/adaptor_base.hpp"
#include "veigar/msgpack/object.hpp"
#include "veigar/msgpack/versioning.hpp"

#include <variant>

namespace veigar_msgpack {
MSGPACK_API_VERSION_NAMESPACE(v1) {
namespace adaptor {
namespace detail {
template <
    typename Variant,
    typename T,
    typename... Ts,
    std::size_t current_index,
    std::size_t... indices
>
Variant construct_variant(
    std::size_t index,
    veigar_msgpack::object& object,
    std::index_sequence<current_index, indices...>
) {
    if constexpr(sizeof...(Ts) == 0) {
        return object.as<T>();
    }
    else {
        if (index == current_index) {
            return object.as<T>();
        }
        return construct_variant<Variant, Ts...>(
            index,
            object,
            std::index_sequence<indices...>()
        );
    }
}

struct object_variant_overload {
    object_variant_overload(veigar_msgpack::object& obj, veigar_msgpack::zone& zone)
        : obj{obj}
        , zone{zone} {}

    template<typename T>
    void operator()(T const& value) {
        obj = veigar_msgpack::object(value, zone);
    }

    veigar_msgpack::object& obj;
    veigar_msgpack::zone& zone;
};
} // namespace detail

template <typename... Ts>
struct as<std::variant<Ts...>, typename std::enable_if<(veigar_msgpack::has_as<Ts>::value && ...)>::type> {
    std::variant<Ts...> operator()(veigar_msgpack::object const& o) const {
        if (  o.type != veigar_msgpack::type::ARRAY
           || o.via.array.size != 2
           || o.via.array.ptr[0].type != veigar_msgpack::type::POSITIVE_INTEGER
           || o.via.array.ptr[0].via.u64 >= sizeof...(Ts)) {
            throw veigar_msgpack::type_error{};
        }

        return detail::construct_variant<std::variant<Ts...>, Ts...>(
            o.via.array.ptr[0].as<std::size_t>(),
            o.via.array.ptr[1],
            std::make_index_sequence<sizeof...(Ts)>()
        );
    }
};

template<typename... Ts>
struct convert<std::variant<Ts...>> {
    veigar_msgpack::object const& operator()(veigar_msgpack::object const& o, std::variant<Ts...>& v) const {
        if (  o.type != veigar_msgpack::type::ARRAY
           || o.via.array.size != 2
           || o.via.array.ptr[0].type != veigar_msgpack::type::POSITIVE_INTEGER
           || o.via.array.ptr[0].via.u64 >= sizeof...(Ts)) {
            throw veigar_msgpack::type_error{};
        }

        v =  detail::construct_variant<std::variant<Ts...>, Ts...>(
            o.via.array.ptr[0].as<std::size_t>(),
            o.via.array.ptr[1],
            std::make_index_sequence<sizeof...(Ts)>()
        );
        return o;
    }
};

template <typename... Ts>
struct pack<std::variant<Ts...>>{
    template<typename Stream>
    veigar_msgpack::packer<Stream>& operator()(
        veigar_msgpack::packer<Stream>& o,
        std::variant<Ts...> const& v
    ) const {
        o.pack_array(2);
        o.pack_uint64(v.index());
        std::visit([&o](auto const& value){o.pack(value);}, v);
        return o;
    }
};


template<typename... Ts>
struct object_with_zone<std::variant<Ts...>> {
    void operator()(
        veigar_msgpack::object::with_zone& o,
        std::variant<Ts...> const& v
    ) const {
        veigar_msgpack::object *p =
            static_cast<veigar_msgpack::object *>(
                o.zone.allocate_align(
                    sizeof(veigar_msgpack::object) * 2,
                    MSGPACK_ZONE_ALIGNOF(veigar_msgpack::object)
                )
            );

        o.type = veigar_msgpack::type::ARRAY;
        o.via.array.size = 2;
        o.via.array.ptr = p;
        o.via.array.ptr[0]= veigar_msgpack::object(v.index(), o.zone);
        std::visit(detail::object_variant_overload(o.via.array.ptr[1], o.zone), v);
    }
};
} // namespace adaptor
}
} // namespace veigar_msgpack

#endif // MSGPACK_CPP_VERSION >= 201703
#endif // defined(MSGPACK_USE_STD_VARIANT_ADAPTOR)
#endif // MSGPACK_V1_TYPE_VARIANT_HPP

#endif // !__VEIGAR_MSGPACK_V1_ADAPTOR_CPP17_VARIANT_HPP