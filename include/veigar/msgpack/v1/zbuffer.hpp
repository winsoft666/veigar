#ifndef __VEIGAR_MSGPACK_V1_ZBUFFER_HPP // Add by msgpack.py
#define __VEIGAR_MSGPACK_V1_ZBUFFER_HPP
//
// MessagePack for C++ deflate buffer implementation
//
// Copyright (C) 2010-2016 FURUHASHI Sadayuki and KONDO Takatoshi
//
//    Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//    http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef MSGPACK_V1_ZBUFFER_HPP
#define MSGPACK_V1_ZBUFFER_HPP

#include "veigar/msgpack/v1/zbuffer_decl.hpp"
#include "veigar/msgpack/assert.hpp"

#include <stdexcept>
#include <zlib.h>

namespace veigar_msgpack {

/// @cond
MSGPACK_API_VERSION_NAMESPACE(v1) {
/// @endcond

class zbuffer {
public:
    zbuffer(int level = Z_DEFAULT_COMPRESSION,
            size_t init_size = MSGPACK_ZBUFFER_INIT_SIZE)
        : m_data(MSGPACK_NULLPTR), m_init_size(init_size)
    {
        m_stream.zalloc = Z_NULL;
        m_stream.zfree = Z_NULL;
        m_stream.opaque = Z_NULL;
        m_stream.next_out = Z_NULL;
        m_stream.avail_out = 0;
        if(deflateInit(&m_stream, level) != Z_OK) {
            throw std::bad_alloc();
        }
    }

    ~zbuffer()
    {
        deflateEnd(&m_stream);
        ::free(m_data);
    }

public:
    void write(const char* buf, size_t len)
    {
        MSGPACK_ASSERT(buf || len == 0);
        if (!buf) return;

        m_stream.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(buf));
        m_stream.avail_in = static_cast<uInt>(len);

        while(m_stream.avail_in > 0) {
            if(m_stream.avail_out < MSGPACK_ZBUFFER_RESERVE_SIZE) {
                if(!expand()) {
                    throw std::bad_alloc();
                }
            }

            if(deflate(&m_stream, Z_NO_FLUSH) != Z_OK) {
                throw std::bad_alloc();
            }
        }
    }

    char* flush()
    {
        while(true) {
            switch(deflate(&m_stream, Z_FINISH)) {
            case Z_STREAM_END:
                return m_data;
            case Z_OK:
            case Z_BUF_ERROR:
                if(!expand()) {
                    throw std::bad_alloc();
                }
                break;
            default:
                throw std::bad_alloc();
            }
        }
    }

    char* data()
    {
        return m_data;
    }

    const char* data() const
    {
        return m_data;
    }

    size_t size() const
    {
        return static_cast<size_t>(reinterpret_cast<char*>(m_stream.next_out) - m_data);
    }

    void reset()
    {
        if(deflateReset(&m_stream) != Z_OK) {
            throw std::bad_alloc();
        }
        reset_buffer();
    }

    void reset_buffer()
    {
        m_stream.avail_out += static_cast<uInt>(reinterpret_cast<char*>(m_stream.next_out) - m_data);
        m_stream.next_out = reinterpret_cast<Bytef*>(m_data);
    }

    char* release_buffer()
    {
        char* tmp = m_data;
        m_data = MSGPACK_NULLPTR;
        m_stream.next_out = MSGPACK_NULLPTR;
        m_stream.avail_out = 0;
        return tmp;
    }

private:
    bool expand()
    {
        size_t used = static_cast<size_t>(reinterpret_cast<char*>(m_stream.next_out) - m_data);
        size_t csize = used + m_stream.avail_out;
        size_t nsize = (csize == 0) ? m_init_size : csize * 2;

        char* tmp = static_cast<char*>(::realloc(m_data, nsize));
        if(tmp == MSGPACK_NULLPTR) {
            return false;
        }

        m_data = tmp;
        m_stream.next_out  = reinterpret_cast<Bytef*>(tmp + used);
        m_stream.avail_out = static_cast<uInt>(nsize - used);

        return true;
    }
#if defined(MSGPACK_USE_CPP03)
private:
    zbuffer(const zbuffer&);
    zbuffer& operator=(const zbuffer&);
#else  // defined(MSGPACK_USE_CPP03)
    zbuffer(const zbuffer&) = delete;
    zbuffer& operator=(const zbuffer&) = delete;
#endif // defined(MSGPACK_USE_CPP03)

private:
    z_stream m_stream;
    char* m_data;
    size_t m_init_size;
};

/// @cond
}  // MSGPACK_API_VERSION_NAMESPACE(v1)
/// @endcond

}  // namespace veigar_msgpack

#endif // MSGPACK_V1_ZBUFFER_HPP

#endif // !__VEIGAR_MSGPACK_V1_ZBUFFER_HPP