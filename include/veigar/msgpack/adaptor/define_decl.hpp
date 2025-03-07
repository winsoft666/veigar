#ifndef __VEIGAR_MSGPACK_ADAPTOR_DEFINE_DECL_HPP // Add by msgpack.py
#define __VEIGAR_MSGPACK_ADAPTOR_DEFINE_DECL_HPP
//
// MessagePack for C++ static resolution routine
//
// Copyright (C) 2016 KONDO Takatoshi
//
//    Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//    http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef MSGPACK_DEFINE_DECL_HPP
#define MSGPACK_DEFINE_DECL_HPP

#if defined(MSGPACK_NO_BOOST)

// MSGPACK_PP_VARIADICS is defined in msgpack/preprocessor/config/config.hpp
// http://www.boost.org/libs/preprocessor/doc/ref/variadics.html
// However, supporting compiler detection is not complete. msgpack-c requires
// variadic macro arguments support. So MSGPACK_PP_VARIADICS is defined here explicitly.
#if !defined(MSGPACK_PP_VARIADICS)
#define MSGPACK_PP_VARIADICS
#endif

#include <veigar/msgpack/preprocessor.hpp>

#define MSGPACK_BASE_ARRAY(base) (*const_cast<base *>(static_cast<base const*>(this)))
#define MSGPACK_NVP(name, value) (name) (value)

#define MSGPACK_DEFINE_MAP_EACH_PROC(r, data, elem) \
    MSGPACK_PP_IF( \
        MSGPACK_PP_IS_BEGIN_PARENS(elem), \
        elem, \
        (MSGPACK_PP_STRINGIZE(elem))(elem) \
    )

#define MSGPACK_DEFINE_MAP_IMPL(...) \
    MSGPACK_PP_SEQ_TO_TUPLE( \
        MSGPACK_PP_SEQ_FOR_EACH( \
            MSGPACK_DEFINE_MAP_EACH_PROC, \
            0, \
            MSGPACK_PP_VARIADIC_TO_SEQ(__VA_ARGS__) \
        ) \
    )

#define MSGPACK_DEFINE_MAP(...) \
    template <typename Packer> \
    void msgpack_pack(Packer& msgpack_pk) const \
    { \
        veigar_msgpack::type::make_define_map \
            MSGPACK_DEFINE_MAP_IMPL(__VA_ARGS__) \
            .msgpack_pack(msgpack_pk); \
    } \
    void msgpack_unpack(veigar_msgpack::object const& msgpack_o) \
    { \
        veigar_msgpack::type::make_define_map \
            MSGPACK_DEFINE_MAP_IMPL(__VA_ARGS__) \
            .msgpack_unpack(msgpack_o); \
    }\
    template <typename MSGPACK_OBJECT> \
    void msgpack_object(MSGPACK_OBJECT* msgpack_o, veigar_msgpack::zone& msgpack_z) const \
    { \
        veigar_msgpack::type::make_define_map \
            MSGPACK_DEFINE_MAP_IMPL(__VA_ARGS__) \
            .msgpack_object(msgpack_o, msgpack_z); \
    }

#define MSGPACK_BASE_MAP(base) \
    (MSGPACK_PP_STRINGIZE(base))(*const_cast<base *>(static_cast<base const*>(this)))

#else  // defined(MSGPACK_NO_BOOST)

// BOOST_PP_VARIADICS is defined in boost/preprocessor/config/config.hpp
// http://www.boost.org/libs/preprocessor/doc/ref/variadics.html
// However, supporting compiler detection is not complete. msgpack-c requires
// variadic macro arguments support. So BOOST_PP_VARIADICS is defined here explicitly.
#if !defined(BOOST_PP_VARIADICS)
#define BOOST_PP_VARIADICS
#endif

#include <boost/preprocessor.hpp>

#define MSGPACK_BASE_ARRAY(base) (*const_cast<base *>(static_cast<base const*>(this)))
#define MSGPACK_NVP(name, value) (name) (value)

#define MSGPACK_DEFINE_MAP_EACH_PROC(r, data, elem) \
    BOOST_PP_IF( \
        BOOST_PP_IS_BEGIN_PARENS(elem), \
        elem, \
        (BOOST_PP_STRINGIZE(elem))(elem) \
    )

#define MSGPACK_DEFINE_MAP_IMPL(...) \
    BOOST_PP_SEQ_TO_TUPLE( \
        BOOST_PP_SEQ_FOR_EACH( \
            MSGPACK_DEFINE_MAP_EACH_PROC, \
            0, \
            BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__) \
        ) \
    )

#define MSGPACK_DEFINE_MAP(...) \
    template <typename Packer> \
    void msgpack_pack(Packer& msgpack_pk) const \
    { \
        veigar_msgpack::type::make_define_map \
            MSGPACK_DEFINE_MAP_IMPL(__VA_ARGS__) \
            .msgpack_pack(msgpack_pk); \
    } \
    void msgpack_unpack(veigar_msgpack::object const& msgpack_o) \
    { \
        veigar_msgpack::type::make_define_map \
            MSGPACK_DEFINE_MAP_IMPL(__VA_ARGS__) \
            .msgpack_unpack(msgpack_o); \
    }\
    template <typename MSGPACK_OBJECT> \
    void msgpack_object(MSGPACK_OBJECT* msgpack_o, veigar_msgpack::zone& msgpack_z) const \
    { \
        veigar_msgpack::type::make_define_map \
            MSGPACK_DEFINE_MAP_IMPL(__VA_ARGS__) \
            .msgpack_object(msgpack_o, msgpack_z); \
    }

#define MSGPACK_BASE_MAP(base) \
    (BOOST_PP_STRINGIZE(base))(*const_cast<base *>(static_cast<base const*>(this)))

#endif // defined(MSGPACK_NO_BOOST)

#include "veigar/msgpack/versioning.hpp"

// for MSGPACK_ADD_ENUM
#include "veigar/msgpack/adaptor/int.hpp"

#define MSGPACK_DEFINE_ARRAY(...) \
    template <typename Packer> \
    void msgpack_pack(Packer& msgpack_pk) const \
    { \
        veigar_msgpack::type::make_define_array(__VA_ARGS__).msgpack_pack(msgpack_pk); \
    } \
    void msgpack_unpack(veigar_msgpack::object const& msgpack_o) \
    { \
        veigar_msgpack::type::make_define_array(__VA_ARGS__).msgpack_unpack(msgpack_o); \
    }\
    template <typename MSGPACK_OBJECT> \
    void msgpack_object(MSGPACK_OBJECT* msgpack_o, veigar_msgpack::zone& msgpack_z) const \
    { \
        veigar_msgpack::type::make_define_array(__VA_ARGS__).msgpack_object(msgpack_o, msgpack_z); \
    }

// MSGPACK_ADD_ENUM must be used in the global namespace.
#define MSGPACK_ADD_ENUM(enum_name) \
  namespace veigar_msgpack { \
  /** @cond */ \
  MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) { \
  /** @endcond */ \
  namespace adaptor { \
    template<> \
    struct convert<enum_name> { \
      veigar_msgpack::object const& operator()(veigar_msgpack::object const& msgpack_o, enum_name& msgpack_v) const { \
        veigar_msgpack::underlying_type<enum_name>::type tmp; \
        veigar_msgpack::operator>>(msgpack_o, tmp);                   \
        msgpack_v = static_cast<enum_name>(tmp);   \
        return msgpack_o; \
      } \
    }; \
    template<> \
    struct object<enum_name> { \
      void operator()(veigar_msgpack::object& msgpack_o, const enum_name& msgpack_v) const { \
        veigar_msgpack::underlying_type<enum_name>::type tmp = static_cast<veigar_msgpack::underlying_type<enum_name>::type>(msgpack_v); \
        veigar_msgpack::operator<<(msgpack_o, tmp);                                    \
      } \
    }; \
    template<> \
    struct object_with_zone<enum_name> { \
      void operator()(veigar_msgpack::object::with_zone& msgpack_o, const enum_name& msgpack_v) const {  \
        veigar_msgpack::underlying_type<enum_name>::type tmp = static_cast<veigar_msgpack::underlying_type<enum_name>::type>(msgpack_v); \
        veigar_msgpack::operator<<(msgpack_o, tmp);                                    \
      } \
    }; \
    template <> \
    struct pack<enum_name> { \
      template <typename Stream> \
      veigar_msgpack::packer<Stream>& operator()(veigar_msgpack::packer<Stream>& msgpack_o, const enum_name& msgpack_v) const { \
          return veigar_msgpack::operator<<(msgpack_o, static_cast<veigar_msgpack::underlying_type<enum_name>::type>(msgpack_v)); \
      } \
    }; \
  } \
  /** @cond */ \
  } \
  /** @endcond */ \
  }

#if defined(MSGPACK_USE_DEFINE_MAP)
#define MSGPACK_DEFINE MSGPACK_DEFINE_MAP
#define MSGPACK_BASE MSGPACK_BASE_MAP
#else  // defined(MSGPACK_USE_DEFINE_MAP)
#define MSGPACK_DEFINE MSGPACK_DEFINE_ARRAY
#define MSGPACK_BASE MSGPACK_BASE_ARRAY
#endif // defined(MSGPACK_USE_DEFINE_MAP)


#include "veigar/msgpack/v1/adaptor/define_decl.hpp"
#include "veigar/msgpack/v2/adaptor/define_decl.hpp"
#include "veigar/msgpack/v3/adaptor/define_decl.hpp"

#endif // MSGPACK_DEFINE_DECL_HPP

#endif // !__VEIGAR_MSGPACK_ADAPTOR_DEFINE_DECL_HPP