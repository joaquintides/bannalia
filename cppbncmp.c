/* Cpp bignum comparison.
 *
 * Copyright 2008 Joaquín M López Muñoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/comparison/equal.hpp>
#include <boost/preprocessor/comparison/greater.hpp>
#include <boost/preprocessor/comparison/not_equal.hpp>
#include <boost/preprocessor/logical/not.hpp>
#include <boost/preprocessor/seq/elem.hpp>
#include <boost/preprocessor/seq/fold_left.hpp>
#include <boost/preprocessor/control/if.hpp>
#include <boost/preprocessor/seq/seq.hpp>
#include <boost/preprocessor/seq/size.hpp>

#define BN_TO_LITERAL(bn) \
BOOST_PP_IF( \
  BOOST_PP_GREATER(BOOST_PP_SEQ_SIZE(bn),1), \
  BN_TO_LITERAL_CASE1, \
  BN_TO_LITERAL_CASE2)(bn)

#define BN_TO_LITERAL_CASE1(bn) \
BOOST_PP_SEQ_FOLD_LEFT( \
  BN_TO_LITERAL_CASE1_OP, \
  BOOST_PP_SEQ_HEAD(bn), \
  BOOST_PP_SEQ_TAIL(bn))

#define BN_TO_LITERAL_CASE1_OP(s,state,x) BOOST_PP_CAT(state,x)

#define BN_TO_LITERAL_CASE2(bn) BOOST_PP_SEQ_HEAD(bn)

#define BN_EQUAL(bn1,bn2) \
BOOST_PP_IF( \
  BOOST_PP_NOT_EQUAL( \
    BOOST_PP_SEQ_SIZE(bn1),BOOST_PP_SEQ_SIZE(bn2)), \
  BN_EQUAL_CASE1, \
  BN_EQUAL_CASE2)(bn1,bn2)

#define BN_EQUAL_CASE1(bn1,bn2) 0

#define BN_EQUAL_CASE2(bn1,bn2) \
BN_EQUAL_STATE_RESULT( \
  BOOST_PP_SEQ_FOLD_LEFT(BN_EQUAL_CASE2_OP,(bn2)(1),bn1))

#define BN_EQUAL_CASE2_OP(s,state,x) \
BN_EQUAL_CASE2_OP_IMPL( \
  x, \
  BOOST_PP_SEQ_HEAD(BN_EQUAL_STATE_BN2(state)), \
  BN_EQUAL_STATE_BN2(state), \
  BN_EQUAL_STATE_RESULT(state))

#define BN_EQUAL_CASE2_OP_IMPL(x,y,bn2,result) \
(BOOST_PP_SEQ_TAIL(bn2(0))) \
(BOOST_PP_IF(result,BOOST_PP_EQUAL(x,y),0))

#define BN_EQUAL_STATE_BN2(state) BOOST_PP_SEQ_ELEM(0,state)
#define BN_EQUAL_STATE_RESULT(state) BOOST_PP_SEQ_ELEM(1,state)

#define BN_LESS(bn1,bn2) \
BOOST_PP_IF( \
  BOOST_PP_LESS(BOOST_PP_SEQ_SIZE(bn1),BOOST_PP_SEQ_SIZE(bn2)), \
  BN_LESS_CASE1, \
  BOOST_PP_IF( \
    BOOST_PP_LESS(BOOST_PP_SEQ_SIZE(bn2),BOOST_PP_SEQ_SIZE(bn1)), \
    BN_LESS_CASE2, \
    BN_LESS_CASE3))(bn1,bn2)

#define BN_LESS_CASE1(bn1,bn2) 1
#define BN_LESS_CASE2(bn1,bn2) 0

#define BN_LESS_CASE3(bn1,bn2) \
BOOST_PP_EQUAL( \
  BN_LESS_STATE_RESULT( \
    BOOST_PP_SEQ_FOLD_LEFT(BN_LESS_CASE3_OP,(bn2)(0),bn1)), \
  1)

#define BN_LESS_CASE3_OP(s,state,x) \
BN_LESS_CASE3_OP_IMPL( \
  x, \
  BOOST_PP_SEQ_HEAD(BN_LESS_STATE_BN2(state)), \
  BN_LESS_STATE_BN2(state), \
  BN_LESS_STATE_RESULT(state))

#define BN_LESS_CASE3_OP_IMPL(x,y,bn2,result) \
(BOOST_PP_SEQ_TAIL(bn2(0))) \
(BOOST_PP_IF( \
  BOOST_PP_EQUAL(result,0), \
  BN_LESS_CMP(x,y), \
  result))

#define BN_LESS_STATE_BN2(state) BOOST_PP_SEQ_ELEM(0,state)
#define BN_LESS_STATE_RESULT(state) BOOST_PP_SEQ_ELEM(1,state)

#define BN_LESS_CMP(x,y) \
BOOST_PP_IF( \
  BOOST_PP_LESS(x,y), \
  1, \
  BOOST_PP_IF( \
    BOOST_PP_LESS(y,x), \
    2, \
    0))

#define BN_NOT_EQUAL(bn1,bn2) BOOST_PP_NOT(BN_EQUAL(bn1,bn2))
#define BN_GREATER(bn1,bn2) BN_LESS(bn2,bn1)
#define BN_GREATER_EQUAL(bn1,bn2) BOOST_PP_NOT(BN_LESS(bn1,bn2))
#define BN_LESS_EQUAL(bn1,bn2) BOOST_PP_NOT(BN_LESS(bn2,bn1))

/* testing */

#include <boost/preprocessor/seq/for_each_product.hpp>
#include <assert.h>

#define bns \
((0))((1))((9))((1)(0))((1)(1))((3)(4)(5))((6)(7)(2))((1)(0)(2)(4))

#define TEST(r,bns) \
TEST_IMPL( \
  BOOST_PP_SEQ_ELEM(0,bns), \
  BOOST_PP_SEQ_ELEM(1,bns), \
  BN_TO_LITERAL(BOOST_PP_SEQ_ELEM(0,bns)), \
  BN_TO_LITERAL(BOOST_PP_SEQ_ELEM(1,bns)))

#define TEST_IMPL(bn1,bn2,n1,n2) \
  assert((n1==n2)==BN_EQUAL(bn1,bn2)); \
  assert((n1<n2)==BN_LESS(bn1,bn2)); \
  assert((n1>n2)==BN_GREATER(bn1,bn2)); \
  assert((n1>=n2)==BN_GREATER_EQUAL(bn1,bn2)); \
  assert((n1<=n2)==BN_LESS_EQUAL(bn1,bn2));

int main()
{
  BOOST_PP_SEQ_FOR_EACH_PRODUCT(TEST,(bns)(bns))
  return 0;
}
