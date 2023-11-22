//
// MessagePack for C++ deserializing routine
//
// Copyright (C) 2018 KONDO Takatoshi
//
//    Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//    http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef MSGPACK_V3_UNPACK_HPP
#define MSGPACK_V3_UNPACK_HPP

#include "veigar/msgpack/unpack_decl.hpp"
#include "veigar/msgpack/parse.hpp"
#include "veigar/msgpack/create_object_visitor.hpp"

namespace veigar_msgpack {

/// @cond
MSGPACK_API_VERSION_NAMESPACE(v3) {
/// @endcond

inline veigar_msgpack::object_handle unpack(
    const char* data, std::size_t len, std::size_t& off, bool& referenced,
    veigar_msgpack::unpack_reference_func f, void* user_data,
    veigar_msgpack::unpack_limit const& limit
)
{
    veigar_msgpack::object obj;
    veigar_msgpack::unique_ptr<veigar_msgpack::zone> z(new veigar_msgpack::zone);
    referenced = false;
    parse_return ret = detail::unpack_imp(
        data, len, off, *z, obj, referenced, f, user_data, limit);

    switch(ret) {
    case PARSE_SUCCESS:
        return veigar_msgpack::object_handle(obj, veigar_msgpack::move(z));
    case PARSE_EXTRA_BYTES:
        return veigar_msgpack::object_handle(obj, veigar_msgpack::move(z));
    default:
        break;
    }
    return veigar_msgpack::object_handle();
}

inline veigar_msgpack::object_handle unpack(
    const char* data, std::size_t len, std::size_t& off,
    veigar_msgpack::unpack_reference_func f, void* user_data,
    veigar_msgpack::unpack_limit const& limit)
{
    bool referenced;
    return veigar_msgpack::v3::unpack(data, len, off, referenced, f, user_data, limit);
}

inline veigar_msgpack::object_handle unpack(
    const char* data, std::size_t len, bool& referenced,
    veigar_msgpack::unpack_reference_func f, void* user_data,
    veigar_msgpack::unpack_limit const& limit)
{
    std::size_t off = 0;
    return veigar_msgpack::v3::unpack(data, len, off, referenced, f, user_data, limit);
}

inline veigar_msgpack::object_handle unpack(
    const char* data, std::size_t len,
    veigar_msgpack::unpack_reference_func f, void* user_data,
    veigar_msgpack::unpack_limit const& limit)
{
    bool referenced;
    std::size_t off = 0;
    return veigar_msgpack::v3::unpack(data, len, off, referenced, f, user_data, limit);
}

inline void unpack(
    veigar_msgpack::object_handle& result,
    const char* data, std::size_t len, std::size_t& off, bool& referenced,
    veigar_msgpack::unpack_reference_func f, void* user_data,
    veigar_msgpack::unpack_limit const& limit)
{
    veigar_msgpack::object obj;
    veigar_msgpack::unique_ptr<veigar_msgpack::zone> z(new veigar_msgpack::zone);
    referenced = false;
    parse_return ret = detail::unpack_imp(
        data, len, off, *z, obj, referenced, f, user_data, limit);

    switch(ret) {
    case PARSE_SUCCESS:
        result.set(obj);
        result.zone() = veigar_msgpack::move(z);
        return;
    case PARSE_EXTRA_BYTES:
        result.set(obj);
        result.zone() = veigar_msgpack::move(z);
        return;
    default:
        return;
    }
}

inline void unpack(
    veigar_msgpack::object_handle& result,
    const char* data, std::size_t len, std::size_t& off,
    veigar_msgpack::v3::unpack_reference_func f, void* user_data,
            veigar_msgpack::unpack_limit const& limit)
{
    bool referenced;
    veigar_msgpack::v3::unpack(result, data, len, off, referenced, f, user_data, limit);
}

inline void unpack(
    veigar_msgpack::object_handle& result,
    const char* data, std::size_t len, bool& referenced,
    veigar_msgpack::unpack_reference_func f, void* user_data,
    veigar_msgpack::unpack_limit const& limit)
{
    std::size_t off = 0;
    veigar_msgpack::v3::unpack(result, data, len, off, referenced, f, user_data, limit);
}

inline void unpack(
    veigar_msgpack::object_handle& result,
    const char* data, std::size_t len,
    veigar_msgpack::unpack_reference_func f, void* user_data,
    veigar_msgpack::unpack_limit const& limit)
{
    bool referenced;
    std::size_t off = 0;
    veigar_msgpack::v3::unpack(result, data, len, off, referenced, f, user_data, limit);
}


inline veigar_msgpack::object unpack(
    veigar_msgpack::zone& z,
    const char* data, std::size_t len, std::size_t& off, bool& referenced,
    veigar_msgpack::unpack_reference_func f, void* user_data,
    veigar_msgpack::unpack_limit const& limit)
{
    veigar_msgpack::object obj;
    referenced = false;
    parse_return ret = detail::unpack_imp(
        data, len, off, z, obj, referenced, f, user_data, limit);

    switch(ret) {
    case PARSE_SUCCESS:
        return obj;
    case PARSE_EXTRA_BYTES:
        return obj;
    default:
        break;
    }
    return obj;
}

inline veigar_msgpack::object unpack(
    veigar_msgpack::zone& z,
    const char* data, std::size_t len, std::size_t& off,
    veigar_msgpack::unpack_reference_func f, void* user_data,
    veigar_msgpack::unpack_limit const& limit)
{
    bool referenced;
    return veigar_msgpack::v3::unpack(z, data, len, off, referenced, f, user_data, limit);
}

inline veigar_msgpack::object unpack(
    veigar_msgpack::zone& z,
    const char* data, std::size_t len, bool& referenced,
    veigar_msgpack::unpack_reference_func f, void* user_data,
    veigar_msgpack::unpack_limit const& limit)
{
    std::size_t off = 0;
    return veigar_msgpack::v3::unpack(z, data, len, off, referenced, f, user_data, limit);
}

inline veigar_msgpack::object unpack(
    veigar_msgpack::zone& z,
    const char* data, std::size_t len,
    veigar_msgpack::unpack_reference_func f, void* user_data,
    veigar_msgpack::unpack_limit const& limit)
{
    bool referenced;
    std::size_t off = 0;
    return veigar_msgpack::v3::unpack(z, data, len, off, referenced, f, user_data, limit);
}

/// @cond
}  // MSGPACK_API_VERSION_NAMESPACE(v3)
/// @endcond

}  // namespace veigar_msgpack


#endif // MSGPACK_V3_UNPACK_HPP
