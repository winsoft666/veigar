#ifndef __VEIGAR_MSGPACK_PREPROCESSOR_SEQ_INSERT_HPP // Add by msgpack.py
#define __VEIGAR_MSGPACK_PREPROCESSOR_SEQ_INSERT_HPP
# /* **************************************************************************
#  *                                                                          *
#  *     (C) Copyright Paul Mensonides 2002.
#  *     Distributed under the Boost Software License, Version 1.0. (See
#  *     accompanying file LICENSE_1_0.txt or copy at
#  *     http://www.boost.org/LICENSE_1_0.txt)
#  *                                                                          *
#  ************************************************************************** */
#
# /* See http://www.boost.org for most recent version. */
#
# ifndef MSGPACK_PREPROCESSOR_SEQ_INSERT_HPP
# define MSGPACK_PREPROCESSOR_SEQ_INSERT_HPP
#
# include <veigar/msgpack/preprocessor/config/config.hpp>
# include <veigar/msgpack/preprocessor/seq/first_n.hpp>
# include <veigar/msgpack/preprocessor/seq/rest_n.hpp>
#
# /* MSGPACK_PP_SEQ_INSERT */
#
# if ~MSGPACK_PP_CONFIG_FLAGS() & MSGPACK_PP_CONFIG_EDG()
#    define MSGPACK_PP_SEQ_INSERT(seq, i, elem) MSGPACK_PP_SEQ_FIRST_N(i, seq) (elem) MSGPACK_PP_SEQ_REST_N(i, seq)
# else
#    define MSGPACK_PP_SEQ_INSERT(seq, i, elem) MSGPACK_PP_SEQ_INSERT_I(seq, i, elem)
#    define MSGPACK_PP_SEQ_INSERT_I(seq, i, elem) MSGPACK_PP_SEQ_FIRST_N(i, seq) (elem) MSGPACK_PP_SEQ_REST_N(i, seq)
# endif
#
# endif

#endif // !__VEIGAR_MSGPACK_PREPROCESSOR_SEQ_INSERT_HPP