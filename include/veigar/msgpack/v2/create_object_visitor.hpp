#ifndef __VEIGAR_MSGPACK_V2_CREATE_OBJECT_VISITOR_HPP // Add by msgpack.py
#define __VEIGAR_MSGPACK_V2_CREATE_OBJECT_VISITOR_HPP
//
// MessagePack for C++ deserializing routine
//
// Copyright (C) 2017 KONDO Takatoshi
//
//    Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//    http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef MSGPACK_V2_CREATE_OBJECT_VISITOR_HPP
#define MSGPACK_V2_CREATE_OBJECT_VISITOR_HPP

#include "veigar/msgpack/unpack_decl.hpp"
#include "veigar/msgpack/unpack_exception.hpp"
#include "veigar/msgpack/v2/create_object_visitor_decl.hpp"
#include "veigar/msgpack/v2/null_visitor.hpp"
#include "veigar/msgpack/assert.hpp"

namespace veigar_msgpack {

/// @cond
MSGPACK_API_VERSION_NAMESPACE(v2) {
/// @endcond

namespace detail {

class create_object_visitor : public veigar_msgpack::v2::null_visitor {
public:
    create_object_visitor(unpack_reference_func f, void* user_data, unpack_limit const& limit)
        :m_func(f), m_user_data(user_data), m_limit(limit) {
        m_stack.reserve(MSGPACK_EMBED_STACK_SIZE);
        m_stack.push_back(&m_obj);
    }

#if !defined(MSGPACK_USE_CPP03)
    create_object_visitor(create_object_visitor&& other)
        :m_func(other.m_func),
         m_user_data(other.m_user_data),
         m_limit(std::move(other.m_limit)),
         m_stack(std::move(other.m_stack)),
         m_zone(other.m_zone),
         m_referenced(other.m_referenced) {
        other.m_zone = MSGPACK_NULLPTR;
        m_stack[0] = &m_obj;
    }
    create_object_visitor& operator=(create_object_visitor&& other) {
        this->~create_object_visitor();
        new (this) create_object_visitor(std::move(other));
        return *this;
    }
#endif // !defined(MSGPACK_USE_CPP03)

    void init() {
        m_stack.resize(1);
        m_obj = veigar_msgpack::object();
        m_stack[0] = &m_obj;
    }
    veigar_msgpack::object const& data() const
    {
        return m_obj;
    }
    veigar_msgpack::zone const& zone() const { return *m_zone; }
    veigar_msgpack::zone& zone() { return *m_zone; }
    void set_zone(veigar_msgpack::zone& zone) { m_zone = &zone; }
    bool referenced() const { return m_referenced; }
    void set_referenced(bool referenced) { m_referenced = referenced; }
    // visit functions
    bool visit_nil() {
        veigar_msgpack::object* obj = m_stack.back();
        obj->type = veigar_msgpack::type::NIL;
        return true;
    }
    bool visit_boolean(bool v) {
        veigar_msgpack::object* obj = m_stack.back();
        obj->type = veigar_msgpack::type::BOOLEAN;
        obj->via.boolean = v;
        return true;
    }
    bool visit_positive_integer(uint64_t v) {
        veigar_msgpack::object* obj = m_stack.back();
        obj->type = veigar_msgpack::type::POSITIVE_INTEGER;
        obj->via.u64 = v;
        return true;
    }
    bool visit_negative_integer(int64_t v) {
        veigar_msgpack::object* obj = m_stack.back();
        if(v >= 0) {
            obj->type = veigar_msgpack::type::POSITIVE_INTEGER;
            obj->via.u64 = static_cast<uint64_t>(v);
        }
        else {
            obj->type = veigar_msgpack::type::NEGATIVE_INTEGER;
            obj->via.i64 = v;
        }
        return true;
    }
    bool visit_float32(float v) {
        veigar_msgpack::object* obj = m_stack.back();
        obj->type = veigar_msgpack::type::FLOAT32;
        obj->via.f64 = v;
        return true;
    }
    bool visit_float64(double v) {
        veigar_msgpack::object* obj = m_stack.back();
        obj->type = veigar_msgpack::type::FLOAT64;
        obj->via.f64 = v;
        return true;
    }
    bool visit_str(const char* v, uint32_t size) {
        MSGPACK_ASSERT(v || size == 0);
        if (size > m_limit.str()) throw veigar_msgpack::str_size_overflow("str size overflow");
        veigar_msgpack::object* obj = m_stack.back();
        obj->type = veigar_msgpack::type::STR;
        if (m_func && m_func(obj->type, size, m_user_data)) {
            obj->via.str.ptr = v;
            obj->via.str.size = size;
            set_referenced(true);
        }
        else {
            if (v) {
                char* tmp = static_cast<char*>(zone().allocate_align(size, MSGPACK_ZONE_ALIGNOF(char)));
                std::memcpy(tmp, v, size);
                obj->via.str.ptr = tmp;
                obj->via.str.size = size;
            }
            else {
                obj->via.str.ptr = MSGPACK_NULLPTR;
                obj->via.str.size = 0;
            }
        }
        return true;
    }
    bool visit_bin(const char* v, uint32_t size) {
        MSGPACK_ASSERT(v || size == 0);
        if (size > m_limit.bin()) throw veigar_msgpack::bin_size_overflow("bin size overflow");
        veigar_msgpack::object* obj = m_stack.back();
        obj->type = veigar_msgpack::type::BIN;
        if (m_func && m_func(obj->type, size, m_user_data)) {
            obj->via.bin.ptr = v;
            obj->via.bin.size = size;
            set_referenced(true);
        }
        else {
            if (v) {
                char* tmp = static_cast<char*>(zone().allocate_align(size, MSGPACK_ZONE_ALIGNOF(char)));
                std::memcpy(tmp, v, size);
                obj->via.bin.ptr = tmp;
                obj->via.bin.size = size;
            }
            else {
                obj->via.bin.ptr = MSGPACK_NULLPTR;
                obj->via.bin.size = 0;
            }
        }
        return true;
    }
    bool visit_ext(const char* v, uint32_t size) {
        MSGPACK_ASSERT(v || size == 0);
        if (size > m_limit.ext()) throw veigar_msgpack::ext_size_overflow("ext size overflow");
        veigar_msgpack::object* obj = m_stack.back();
        obj->type = veigar_msgpack::type::EXT;
        if (m_func && m_func(obj->type, size, m_user_data)) {
            obj->via.ext.ptr = v;
            obj->via.ext.size = static_cast<uint32_t>(size - 1);
            set_referenced(true);
        }
        else {
            if (v) {
                char* tmp = static_cast<char*>(zone().allocate_align(size, MSGPACK_ZONE_ALIGNOF(char)));
                std::memcpy(tmp, v, size);
                obj->via.ext.ptr = tmp;
                obj->via.ext.size = static_cast<uint32_t>(size - 1);
            }
            else {
                obj->via.ext.ptr = MSGPACK_NULLPTR;
                obj->via.ext.size = 0;
            }
        }
        return true;
    }
    bool start_array(uint32_t num_elements) {
        if (num_elements > m_limit.array()) throw veigar_msgpack::array_size_overflow("array size overflow");
        if (m_stack.size() > m_limit.depth()) throw veigar_msgpack::depth_size_overflow("depth size overflow");
        veigar_msgpack::object* obj = m_stack.back();
        obj->type = veigar_msgpack::type::ARRAY;
        obj->via.array.size = num_elements;
        if (num_elements == 0) {
            obj->via.array.ptr = MSGPACK_NULLPTR;
        }
        else {

#if SIZE_MAX == UINT_MAX
            if (num_elements > SIZE_MAX/sizeof(veigar_msgpack::object))
                throw veigar_msgpack::array_size_overflow("array size overflow");
#endif // SIZE_MAX == UINT_MAX

            size_t size = num_elements*sizeof(veigar_msgpack::object);
            obj->via.array.ptr =
                static_cast<veigar_msgpack::object*>(m_zone->allocate_align(size, MSGPACK_ZONE_ALIGNOF(veigar_msgpack::object)));
        }
        m_stack.push_back(obj->via.array.ptr);
        return true;
    }
    bool start_array_item() {
        return true;
    }
    bool end_array_item() {
        ++m_stack.back();
        return true;
    }
    bool end_array() {
        m_stack.pop_back();
        return true;
    }
    bool start_map(uint32_t num_kv_pairs) {
        if (num_kv_pairs > m_limit.map()) throw veigar_msgpack::map_size_overflow("map size overflow");
        if (m_stack.size() > m_limit.depth()) throw veigar_msgpack::depth_size_overflow("depth size overflow");
        veigar_msgpack::object* obj = m_stack.back();
        obj->type = veigar_msgpack::type::MAP;
        obj->via.map.size = num_kv_pairs;
        if (num_kv_pairs == 0) {
            obj->via.map.ptr = MSGPACK_NULLPTR;
        }
        else {

#if SIZE_MAX == UINT_MAX
            if (num_kv_pairs > SIZE_MAX/sizeof(veigar_msgpack::object_kv))
                throw veigar_msgpack::map_size_overflow("map size overflow");
#endif // SIZE_MAX == UINT_MAX
            size_t size = num_kv_pairs*sizeof(veigar_msgpack::object_kv);
            obj->via.map.ptr =
                static_cast<veigar_msgpack::object_kv*>(m_zone->allocate_align(size, MSGPACK_ZONE_ALIGNOF(veigar_msgpack::object_kv)));
        }
        m_stack.push_back(reinterpret_cast<veigar_msgpack::object*>(obj->via.map.ptr));
        return true;
    }
    bool start_map_key() {
        return true;
    }
    bool end_map_key() {
        ++m_stack.back();
        return true;
    }
    bool start_map_value() {
        return true;
    }
    bool end_map_value() {
        ++m_stack.back();
        return true;
    }
    bool end_map() {
        m_stack.pop_back();
        return true;
    }
    void parse_error(size_t /*parsed_offset*/, size_t /*error_offset*/) {
        throw veigar_msgpack::parse_error("parse error");
    }
    void insufficient_bytes(size_t /*parsed_offset*/, size_t /*error_offset*/) {
        throw veigar_msgpack::insufficient_bytes("insufficient bytes");
    }
private:
public:
    unpack_reference_func m_func;
    void* m_user_data;
    unpack_limit m_limit;
    veigar_msgpack::object m_obj;
    std::vector<veigar_msgpack::object*> m_stack;
    veigar_msgpack::zone* m_zone;
    bool m_referenced;
};

} // detail

/// @cond
}  // MSGPACK_API_VERSION_NAMESPACE(v2)
/// @endcond

}  // namespace veigar_msgpack

#endif // MSGPACK_V2_CREATE_OBJECT_VISITOR_HPP

#endif // !__VEIGAR_MSGPACK_V2_CREATE_OBJECT_VISITOR_HPP