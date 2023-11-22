//
// MessagePack for C++ deserializing routine
//
// Copyright (C) 2016 KONDO Takatoshi
//
//    Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//    http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef MSGPACK_V2_UNPACK_HPP
#define MSGPACK_V2_UNPACK_HPP

#if MSGPACK_DEFAULT_API_VERSION >= 2

#include "veigar/msgpack/unpack_decl.hpp"
#include "veigar/msgpack/parse.hpp"
#include "veigar/msgpack/create_object_visitor.hpp"

namespace veigar_msgpack {

/// @cond
MSGPACK_API_VERSION_NAMESPACE(v2) {
/// @endcond


struct zone_push_finalizer {
    zone_push_finalizer(veigar_msgpack::zone& z):m_z(&z) {}
    void set_zone(veigar_msgpack::zone& z) { m_z = &z; }
    void operator()(char* buffer) {
        m_z->push_finalizer(&detail::decr_count, buffer);
    }
    veigar_msgpack::zone* m_z;
};

class unpacker : public parser<unpacker, zone_push_finalizer>,
                 public detail::create_object_visitor {
    typedef parser<unpacker, zone_push_finalizer> parser_t;
public:
    unpacker(unpack_reference_func f = &unpacker::default_reference_func,
             void* user_data = MSGPACK_NULLPTR,
             std::size_t initial_buffer_size = MSGPACK_UNPACKER_INIT_BUFFER_SIZE,
             unpack_limit const& limit = unpack_limit())
        :parser_t(m_finalizer, initial_buffer_size),
         detail::create_object_visitor(f, user_data, limit),
         m_z(new veigar_msgpack::zone),
         m_finalizer(*m_z) {
        set_zone(*m_z);
        set_referenced(false);
    }

    detail::create_object_visitor& visitor() { return *this; }
    /// Unpack one veigar_msgpack::object.
    /**
     *
     * @param result The object that contains unpacked data.
     * @param referenced If the unpacked object contains reference of the buffer,
     *                   then set as true, otherwise false.
     *
     * @return If one veigar_msgpack::object is unpacked, then return true, if veigar_msgpack::object is incomplete
     *         and additional data is required, then return false. If data format is invalid, throw
     *         veigar_msgpack::parse_error.
     *
     * See:
     * https://github.com/msgpack/msgpack-c/wiki/v1_1_cpp_unpacker#msgpack-controls-a-buffer
     */
    bool next(veigar_msgpack::object_handle& result, bool& referenced);

    /// Unpack one veigar_msgpack::object.
    /**
     *
     * @param result The object that contains unpacked data.
     *
     * @return If one veigar_msgpack::object is unpacked, then return true, if veigar_msgpack::object is incomplete
     *         and additional data is required, then return false. If data format is invalid, throw
     *         veigar_msgpack::parse_error.
     *
     * See:
     * https://github.com/msgpack/msgpack-c/wiki/v1_1_cpp_unpacker#msgpack-controls-a-buffer
     */
    bool next(veigar_msgpack::object_handle& result);
    veigar_msgpack::zone* release_zone();
    void reset_zone();
    bool flush_zone();
private:
    static bool default_reference_func(veigar_msgpack::type::object_type /*type*/, std::size_t /*len*/, void*) {
        return true;
    }
    veigar_msgpack::unique_ptr<veigar_msgpack::zone> m_z;
    zone_push_finalizer m_finalizer;
};

inline bool unpacker::next(veigar_msgpack::object_handle& result, bool& referenced) {
    bool ret = parser_t::next();
    if (ret) {
        referenced = detail::create_object_visitor::referenced();
        result.zone().reset( release_zone() );
        result.set(data());
        reset();
    }
    else {
        result.zone().reset();
        result.set(veigar_msgpack::object());
    }
    return ret;
}

inline bool unpacker::next(veigar_msgpack::object_handle& result) {
    bool referenced;
    return next(result, referenced);
}

inline veigar_msgpack::zone* unpacker::release_zone()
{
    if(!flush_zone()) {
        return MSGPACK_NULLPTR;
    }

    veigar_msgpack::zone* r =  new veigar_msgpack::zone;
    veigar_msgpack::zone* old = m_z.release();
    m_z.reset(r);
    set_zone(*m_z);
    m_finalizer.set_zone(*m_z);

    return old;
}

inline void unpacker::reset_zone()
{
    m_z->clear();
}

inline bool unpacker::flush_zone()
{
    if(referenced()) {
        try {
            m_z->push_finalizer(&detail::decr_count, get_raw_buffer());
        } catch (...) {
            return false;
        }
        set_referenced(false);

        detail::incr_count(get_raw_buffer());
    }

    return true;
}

inline veigar_msgpack::object_handle unpack(
    const char* data, std::size_t len, std::size_t& off, bool& referenced,
    unpack_reference_func f, void* user_data,
    unpack_limit const& limit
)
{
    veigar_msgpack::object obj;
    veigar_msgpack::unique_ptr<veigar_msgpack::zone> z(new veigar_msgpack::zone);
    referenced = false;
    std::size_t noff = off;
    parse_return ret = detail::unpack_imp(
        data, len, noff, *z, obj, referenced, f, user_data, limit);

    switch(ret) {
    case PARSE_SUCCESS:
        off = noff;
        return veigar_msgpack::object_handle(obj, veigar_msgpack::move(z));
    case PARSE_EXTRA_BYTES:
        off = noff;
        return veigar_msgpack::object_handle(obj, veigar_msgpack::move(z));
    default:
        break;
    }
    return veigar_msgpack::object_handle();
}

inline veigar_msgpack::object_handle unpack(
    const char* data, std::size_t len, std::size_t& off,
    unpack_reference_func f, void* user_data,
    unpack_limit const& limit)
{
    bool referenced;
    return veigar_msgpack::v2::unpack(data, len, off, referenced, f, user_data, limit);
}

inline veigar_msgpack::object_handle unpack(
    const char* data, std::size_t len, bool& referenced,
    unpack_reference_func f, void* user_data,
    unpack_limit const& limit)
{
    std::size_t off = 0;
    return veigar_msgpack::v2::unpack(data, len, off, referenced, f, user_data, limit);
}

inline veigar_msgpack::object_handle unpack(
    const char* data, std::size_t len,
    unpack_reference_func f, void* user_data,
    unpack_limit const& limit)
{
    bool referenced;
    std::size_t off = 0;
    return veigar_msgpack::v2::unpack(data, len, off, referenced, f, user_data, limit);
}

inline void unpack(
    veigar_msgpack::object_handle& result,
    const char* data, std::size_t len, std::size_t& off, bool& referenced,
    unpack_reference_func f, void* user_data,
    unpack_limit const& limit)
{
    veigar_msgpack::object obj;
    veigar_msgpack::unique_ptr<veigar_msgpack::zone> z(new veigar_msgpack::zone);
    referenced = false;
    std::size_t noff = off;
    parse_return ret = detail::unpack_imp(
        data, len, noff, *z, obj, referenced, f, user_data, limit);

    switch(ret) {
    case PARSE_SUCCESS:
        off = noff;
        result.set(obj);
        result.zone() = veigar_msgpack::move(z);
        return;
    case PARSE_EXTRA_BYTES:
        off = noff;
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
    unpack_reference_func f, void* user_data,
            unpack_limit const& limit)
{
    bool referenced;
    veigar_msgpack::v2::unpack(result, data, len, off, referenced, f, user_data, limit);
}

inline void unpack(
    veigar_msgpack::object_handle& result,
    const char* data, std::size_t len, bool& referenced,
    unpack_reference_func f, void* user_data,
    unpack_limit const& limit)
{
    std::size_t off = 0;
    veigar_msgpack::v2::unpack(result, data, len, off, referenced, f, user_data, limit);
}

inline void unpack(
    veigar_msgpack::object_handle& result,
    const char* data, std::size_t len,
    unpack_reference_func f, void* user_data,
    unpack_limit const& limit)
{
    bool referenced;
    std::size_t off = 0;
    veigar_msgpack::v2::unpack(result, data, len, off, referenced, f, user_data, limit);
}


inline veigar_msgpack::object unpack(
    veigar_msgpack::zone& z,
    const char* data, std::size_t len, std::size_t& off, bool& referenced,
    unpack_reference_func f, void* user_data,
    unpack_limit const& limit)
{
    veigar_msgpack::object obj;
    std::size_t noff = off;
    referenced = false;
    parse_return ret = detail::unpack_imp(
        data, len, noff, z, obj, referenced, f, user_data, limit);

    switch(ret) {
    case PARSE_SUCCESS:
        off = noff;
        return obj;
    case PARSE_EXTRA_BYTES:
        off = noff;
        return obj;
    default:
        break;
    }
    return obj;
}

inline veigar_msgpack::object unpack(
    veigar_msgpack::zone& z,
    const char* data, std::size_t len, std::size_t& off,
    unpack_reference_func f, void* user_data,
    unpack_limit const& limit)
{
    bool referenced;
    return veigar_msgpack::v2::unpack(z, data, len, off, referenced, f, user_data, limit);
}

inline veigar_msgpack::object unpack(
    veigar_msgpack::zone& z,
    const char* data, std::size_t len, bool& referenced,
    unpack_reference_func f, void* user_data,
    unpack_limit const& limit)
{
    std::size_t off = 0;
    return veigar_msgpack::v2::unpack(z, data, len, off, referenced, f, user_data, limit);
}

inline veigar_msgpack::object unpack(
    veigar_msgpack::zone& z,
    const char* data, std::size_t len,
    unpack_reference_func f, void* user_data,
    unpack_limit const& limit)
{
    bool referenced;
    std::size_t off = 0;
    return veigar_msgpack::v2::unpack(z, data, len, off, referenced, f, user_data, limit);
}

namespace detail {

inline parse_return
unpack_imp(const char* data, std::size_t len, std::size_t& off,
           veigar_msgpack::zone& result_zone, veigar_msgpack::object& result, bool& referenced,
           unpack_reference_func f = MSGPACK_NULLPTR, void* user_data = MSGPACK_NULLPTR,
           unpack_limit const& limit = unpack_limit())
{
    create_object_visitor v(f, user_data, limit);
    v.set_zone(result_zone);
    referenced = false;
    v.set_referenced(referenced);
    parse_return ret = parse_imp(data, len, off, v);
    referenced = v.referenced();
    result = v.data();
    return ret;
}

} // namespace detail


/// @cond
}  // MSGPACK_API_VERSION_NAMESPACE(v2)
/// @endcond

}  // namespace veigar_msgpack

#endif // MSGPACK_DEFAULT_API_VERSION >= 2

#endif // MSGPACK_V2_UNPACK_HPP
