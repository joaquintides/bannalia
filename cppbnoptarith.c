/* Cpp bignum arithmetic operations with memoization.
 *
 * Copyright 2008 Joaquín M López Muñoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */

#include <boost/preprocessor/arithmetic/add.hpp>
#include <boost/preprocessor/arithmetic/dec.hpp>
#include <boost/preprocessor/arithmetic/div.hpp>
#include <boost/preprocessor/arithmetic/inc.hpp>
#include <boost/preprocessor/arithmetic/mod.hpp>
#include <boost/preprocessor/arithmetic/mul.hpp>
#include <boost/preprocessor/arithmetic/sub.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/comparison/equal.hpp>
#include <boost/preprocessor/comparison/greater.hpp>
#include <boost/preprocessor/comparison/greater_equal.hpp>
#include <boost/preprocessor/comparison/less.hpp>
#include <boost/preprocessor/comparison/not_equal.hpp>
#include <boost/preprocessor/control/if.hpp>
#include <boost/preprocessor/facilities/empty.hpp>
#include <boost/preprocessor/facilities/identity.hpp>
#include <boost/preprocessor/logical/and.hpp>
#include <boost/preprocessor/logical/not.hpp>
#include <boost/preprocessor/logical/or.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/selection/min.hpp>
#include <boost/preprocessor/seq/elem.hpp>
#include <boost/preprocessor/seq/first_n.hpp>
#include <boost/preprocessor/seq/fold_left.hpp>
#include <boost/preprocessor/seq/replace.hpp>
#include <boost/preprocessor/seq/rest_n.hpp>
#include <boost/preprocessor/seq/reverse.hpp>
#include <boost/preprocessor/seq/seq.hpp>
#include <boost/preprocessor/seq/size.hpp>
#include <boost/preprocessor/seq/pop_back.hpp>
#include <boost/preprocessor/seq/rest_n.hpp>

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

/* BN_LESS copied from cppbncmp.c */

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

#define BN_ADD(bn1,bn2) \
BOOST_PP_IF( \
  BOOST_PP_GREATER_EQUAL( \
    BOOST_PP_SEQ_SIZE(bn1), \
    BOOST_PP_SEQ_SIZE(bn2)), \
  BN_ADD_IMPL(bn1,bn2), \
  BN_ADD_IMPL(bn2,bn1))

#define BN_ADD_IMPL(bn1,bn2) \
BN_ADD_POSTPROCESS( \
  BOOST_PP_SEQ_FOLD_LEFT( \
    BN_ADD_OP, \
    (0)(BOOST_PP_SEQ_REVERSE(bn2))((~)), \
    BOOST_PP_SEQ_REVERSE(bn1)))

#define BN_ADD_POSTPROCESS(state) \
BOOST_PP_IF( \
  BOOST_PP_NOT_EQUAL(BN_ADD_STATE_CARRY(state),0), \
  BOOST_PP_IDENTITY((BN_ADD_STATE_CARRY(state))), \
  BOOST_PP_EMPTY)() \
BOOST_PP_SEQ_POP_BACK(BN_ADD_STATE_RESULT(state))

#define BN_ADD_OP(s,state,x) \
BN_ADD_OP_IMPL( \
  x, \
  BN_ADD_STATE_Y(state), \
  BN_ADD_STATE_CARRY(state), \
  BN_ADD_STATE_RBN2(state), \
  BN_ADD_STATE_RESULT(state))

#define BN_ADD_OP_IMPL(x,y,carry,rbn2,result) \
(BN_ADD_MSD(x,y,carry)) \
(BOOST_PP_SEQ_TAIL(rbn2(0))) \
((BN_ADD_LSD(x,y,carry))result)

#define BN_ADD_STATE_CARRY(state) BOOST_PP_SEQ_ELEM(0,state)
#define BN_ADD_STATE_RBN2(state) BOOST_PP_SEQ_ELEM(1,state)
#define BN_ADD_STATE_Y(state) \
BOOST_PP_SEQ_HEAD(BN_ADD_STATE_RBN2(state))
#define BN_ADD_STATE_RESULT(state) BOOST_PP_SEQ_ELEM(2,state)

#define BN_ADD_MSD(x,y,carry) \
BOOST_PP_DIV(BOOST_PP_ADD(BOOST_PP_ADD(x,y),carry),10)

#define BN_ADD_LSD(x,y,carry) \
BOOST_PP_MOD(BOOST_PP_ADD(BOOST_PP_ADD(x,y),carry),10)

#define BN_SUB(bn1,bn2) \
BOOST_PP_IF( \
  BOOST_PP_LESS(BOOST_PP_SEQ_SIZE(bn1),BOOST_PP_SEQ_SIZE(bn2)), \
  BN_SUB_CASE1, \
  BN_SUB_CASE2)(bn1,bn2)

#define BN_SUB_CASE1(bn1,bn2) (0)

#define BN_SUB_CASE2(bn1,bn2) \
BN_SUB_POSTPROCESS( \
  BOOST_PP_SEQ_FOLD_LEFT( \
    BN_SUB_OP, \
    (0)(0)(BOOST_PP_SEQ_REVERSE(bn2))((~)), \
    BOOST_PP_SEQ_REVERSE(bn1)))

#define BN_SUB_POSTPROCESS(state) \
BOOST_PP_IF( \
  BN_SUB_STATE_BORROW(state), \
  BN_SUB_POSTPROCESS_CASE1, \
  BN_SUB_POSTPROCESS_CASE2)(state)

#define BN_SUB_POSTPROCESS_CASE1(state) (0)

#define BN_SUB_POSTPROCESS_CASE2(state) \
BOOST_PP_SEQ_REST_N( \
  BOOST_PP_DEC(BN_SUB_STATE_PREFIX(state)), \
  BOOST_PP_SEQ_POP_BACK(BN_SUB_STATE_RESULT(state)))

#define BN_SUB_OP(s,state,x) \
BN_SUB_OP_IMPL( \
  x, \
  BN_SUB_STATE_Y(state), \
  BN_SUB_STATE_BORROW(state), \
  BN_SUB_STATE_PREFIX(state), \
  BN_SUB_STATE_RBN2(state), \
  BN_SUB_STATE_RESULT(state))

#define BN_SUB_OP_IMPL(x,y,borrow,prefix,rbn2,result) \
(BN_SUB_MSD(x,y,borrow)) \
(BOOST_PP_IF(BN_SUB_LSD(x,y,borrow),1,BOOST_PP_INC(prefix))) \
(BOOST_PP_SEQ_TAIL(rbn2(0))) \
((BN_SUB_LSD(x,y,borrow))result)

#define BN_SUB_STATE_BORROW(state) BOOST_PP_SEQ_ELEM(0,state)
#define BN_SUB_STATE_PREFIX(state) BOOST_PP_SEQ_ELEM(1,state)
#define BN_SUB_STATE_RBN2(state) BOOST_PP_SEQ_ELEM(2,state)
#define BN_SUB_STATE_Y(state) BOOST_PP_SEQ_HEAD(BN_SUB_STATE_RBN2(state))
#define BN_SUB_STATE_RESULT(state) BOOST_PP_SEQ_ELEM(3,state)

#define BN_SUB_MSD(x,y,borrow) \
BOOST_PP_SUB( \
  1, \
  BOOST_PP_DIV( \
    BOOST_PP_SUB(BOOST_PP_ADD(10,x),BOOST_PP_ADD(y,borrow)), \
    10))

#define BN_SUB_LSD(x,y,borrow) \
BOOST_PP_MOD( \
  BOOST_PP_SUB(BOOST_PP_ADD(10,x),BOOST_PP_ADD(y,borrow)), \
  10)

#define BN_CACHED_DIGIT_MUL(bn,y,cache) \
BOOST_PP_IF( \
  BOOST_PP_EQUAL( \
    1, \
    BOOST_PP_SEQ_SIZE(BOOST_PP_SEQ_ELEM(y,cache))), \
  BN_CACHED_DIGIT_MUL_CASE1, \
  BN_CACHED_DIGIT_MUL_CASE2)(bn,y,cache)

#define BN_CACHED_DIGIT_MUL_CASE1(bn,y,cache) \
BN_CACHED_DIGIT_MUL_CASE1_IMPL(BN_DIGIT_MUL(bn,y),y,cache)

#define BN_CACHED_DIGIT_MUL_CASE1_IMPL(bny,y,cache) \
(bny)(BOOST_PP_SEQ_REPLACE(cache,y,(~)bny))

#define BN_CACHED_DIGIT_MUL_CASE2(bn,y,cache) \
(BOOST_PP_SEQ_TAIL(BOOST_PP_SEQ_ELEM(y,cache)))(cache)

#define BN_DIGIT_MUL(bn,y) \
BN_DIGIT_MUL_POSTPROCESS( \
  BOOST_PP_SEQ_FOLD_LEFT( \
    BN_DIGIT_MUL_OP, \
    (y)(0)((~)), \
    BOOST_PP_SEQ_REVERSE(bn)))

#define BN_DIGIT_MUL_POSTPROCESS(state) \
BOOST_PP_IF( \
  BOOST_PP_NOT_EQUAL(BN_DIGIT_MUL_STATE_CARRY(state),0), \
  BOOST_PP_IDENTITY((BN_DIGIT_MUL_STATE_CARRY(state))), \
  BOOST_PP_EMPTY)() \
BOOST_PP_SEQ_POP_BACK(BN_DIGIT_MUL_STATE_RESULT(state))

#define BN_DIGIT_MUL_OP(s,state,x) \
BN_DIGIT_MUL_OP_IMPL( \
  x, \
  BN_DIGIT_MUL_STATE_Y(state), \
  BN_DIGIT_MUL_STATE_CARRY(state), \
  BN_DIGIT_MUL_STATE_RESULT(state))

#define BN_DIGIT_MUL_OP_IMPL(x,y,carry,result) \
(y) \
(BN_DIGIT_MUL_MSD(x,y,carry)) \
((BN_DIGIT_MUL_LSD(x,y,carry))result)

#define BN_DIGIT_MUL_STATE_Y(state) BOOST_PP_SEQ_ELEM(0,state)
#define BN_DIGIT_MUL_STATE_CARRY(state) BOOST_PP_SEQ_ELEM(1,state)
#define BN_DIGIT_MUL_STATE_RESULT(state) BOOST_PP_SEQ_ELEM(2,state)

#define BN_DIGIT_MUL_MSD(x,y,carry) \
BOOST_PP_DIV(BOOST_PP_ADD(BOOST_PP_MUL(x,y),carry),10)

#define BN_DIGIT_MUL_LSD(x,y,carry) \
BOOST_PP_MOD(BOOST_PP_ADD(BOOST_PP_MUL(x,y),carry),10)

#define BN_MUL(bn1,bn2) \
BN_MUL_STATE_RESULT( \
  BOOST_PP_SEQ_FOLD_LEFT( \
    BN_MUL_OP, \
    ((~))(bn2) \
    (((~)(0))((~)bn2)((~))((~))((~))((~))((~))((~))((~))((~))) \
    ((0)), \
    BOOST_PP_SEQ_REVERSE(bn1)))

#define BN_MUL_OP(s,state,x) \
BN_MUL_OP_IMPL( \
  x, \
  BN_MUL_STATE_SHIFT(state), \
  BN_MUL_STATE_BN2(state), \
  BN_MUL_STATE_CACHE(state), \
  BN_MUL_STATE_RESULT(state))

#define BN_MUL_OP_IMPL(x,shift,bn2,cache,result) \
BN_MUL_OP_IMPL_RETURN( \
  BN_CACHED_DIGIT_MUL(bn2,x,cache),shift,bn2,result)

#define BN_MUL_OP_IMPL_RETURN(bn2x_cache,shift,bn2,result) \
(shift(0)) \
(bn2) \
(BOOST_PP_SEQ_ELEM(1,bn2x_cache)) \
(BN_ADD( \
   result, \
   BOOST_PP_SEQ_ELEM(0,bn2x_cache)BOOST_PP_SEQ_TAIL(shift)))

#define BN_MUL_STATE_SHIFT(state) BOOST_PP_SEQ_ELEM(0,state)
#define BN_MUL_STATE_BN2(state) BOOST_PP_SEQ_ELEM(1,state)
#define BN_MUL_STATE_CACHE(state) BOOST_PP_SEQ_ELEM(2,state)
#define BN_MUL_STATE_RESULT(state) BOOST_PP_SEQ_ELEM(3,state)

#define BN_CACHED_QDIGIT_REMAINDER(bn1,bn2,cache) \
BN_CACHED_QDIGIT_REMAINDER_IMPL( \
  BN_ZERO_SEQ( \
    BOOST_PP_IF( \
      BOOST_PP_LESS_EQUAL( \
        BOOST_PP_SEQ_SIZE(bn1), \
        BOOST_PP_SEQ_SIZE(bn2)), \
      BOOST_PP_INC( \
        BOOST_PP_SUB( \
          BOOST_PP_SEQ_SIZE(bn2), \
          BOOST_PP_SEQ_SIZE(bn1))), \
      0))bn1, \
  bn2, \
  cache)

#define BN_ZERO_SEQ(n) \
BOOST_PP_REPEAT(n,BN_ZERO_SEQ_OP,~)

#define BN_ZERO_SEQ_OP(z,n,data) (0)

#define BN_CACHED_QDIGIT_REMAINDER_IMPL(bn1,bn2,cache) \
BOOST_PP_IF( \
  BOOST_PP_LESS(BOOST_PP_SEQ_SIZE(bn2),2), \
  BN_CACHED_QDIGIT_REMAINDER_CASE1, \
  BN_CACHED_QDIGIT_REMAINDER_CASE2)(bn1,bn2,cache)

#define BN_CACHED_QDIGIT_REMAINDER_CASE1(bn1,bn2,cache) \
BN_CACHED_QDIGIT_REMAINDER_CASE1_RETURN( \
  BOOST_PP_ADD( \
    BOOST_PP_MUL(BOOST_PP_SEQ_ELEM(0,bn1),10), \
    BOOST_PP_SEQ_ELEM(1,bn1)), \
  BOOST_PP_SEQ_ELEM(0,bn2), \
  cache)

#define BN_CACHED_QDIGIT_REMAINDER_CASE1_RETURN(n1,n2,cache) \
(BOOST_PP_DIV(n1,n2))((BOOST_PP_MOD(n1,n2)))(cache)

#define BN_CACHED_QDIGIT_REMAINDER_CASE2(bn1,bn2,cache) \
BN_CACHED_QDIGIT_REMAINDER_CASE2_TEST( \
  bn1,bn2, \
  BN_DIV_RRR_DD( \
    BOOST_PP_ADD( \
      BOOST_PP_MUL(BOOST_PP_SEQ_ELEM(0,bn1),10), \
      BOOST_PP_SEQ_ELEM(1,bn1)), \
    BOOST_PP_SEQ_ELEM(2,bn1), \
    BOOST_PP_SEQ_ELEM(0,bn2), \
    BOOST_PP_SEQ_ELEM(1,bn2)), \
  cache)

#define BN_CACHED_QDIGIT_REMAINDER_CASE2_TEST(bn1,bn2,q,cache) \
BN_CACHED_QDIGIT_REMAINDER_CASE2_TEST_IMPL( \
  bn1,bn2,q,BN_CACHED_DIGIT_MUL(bn2,q,cache))

#define BN_CACHED_QDIGIT_REMAINDER_CASE2_TEST_IMPL(bn1,bn2,q,bn2q_cache) \
BOOST_PP_IF( \
  BN_LESS_WITH_ZERO(bn1,BOOST_PP_SEQ_ELEM(0,bn2q_cache)), \
  BN_CACHED_QDIGIT_REMAINDER_CASE2_TEST_IMPL_CASE1, \
  BN_CACHED_QDIGIT_REMAINDER_CASE2_TEST_IMPL_CASE2) \
(bn1,bn2,q,BOOST_PP_SEQ_ELEM(0,bn2q_cache),BOOST_PP_SEQ_ELEM(1,bn2q_cache))

#define BN_CACHED_QDIGIT_REMAINDER_CASE2_TEST_IMPL_CASE1(bn1,bn2,q,bn2q,cache) \
(BOOST_PP_DEC(q))(BN_SUB(bn1,BN_SUB(bn2q,bn2)))(cache)

#define BN_CACHED_QDIGIT_REMAINDER_CASE2_TEST_IMPL_CASE2(bn1,bn2,q,bn2q,cache) \
(q)(BN_SUB(bn1,bn2q))(cache)

#define BN_LESS_WITH_ZERO(bn1,bn2) \
BOOST_PP_IF( \
  BOOST_PP_EQUAL(BOOST_PP_SEQ_ELEM(0,bn1),0), \
  BN_LESS_WITH_ZERO_CASE1, \
  BN_LESS)(bn1,bn2)

#define BN_LESS_WITH_ZERO_CASE1(bn1,bn2) \
BN_LESS(BOOST_PP_SEQ_TAIL(bn1),bn2)

#define BN_DIV_RRR_DD(r1r2,r3,d1,d2) \
BOOST_PP_CAT( \
  BN_DIV_RRR_DD_, \
  BN_DIV_RR_D(r1r2,d1))(r1r2,r3,d1,d2)

#define BN_DIV_RR_D(r1r2,d1) \
BOOST_PP_IF( \
  BOOST_PP_EQUAL(d1,0), \
  BN_DIV_RR_D_CASE1, \
  BN_DIV_RR_D_CASE2)(r1r2,d1)

#define BN_DIV_RR_D_CASE1(r1r2,d1) 0
#define BN_DIV_RR_D_CASE2(r1r2,d1) BOOST_PP_MIN(9,BOOST_PP_DIV(r1r2,d1))

#define BN_DIV_RRR_DD_9(r1r2,r3,d1,d2) \
BOOST_PP_IF(BN_DIV_RRR_DD_TEST(9,r1r2,r3,d1,d2), \
  BN_DIV_RRR_DD_RETURN_9,BN_DIV_RRR_DD_8)(r1r2,r3,d1,d2)
#define BN_DIV_RRR_DD_8(r1r2,r3,d1,d2) \
BOOST_PP_IF(BN_DIV_RRR_DD_TEST(8,r1r2,r3,d1,d2), \
  BN_DIV_RRR_DD_RETURN_8,BN_DIV_RRR_DD_7)(r1r2,r3,d1,d2)
#define BN_DIV_RRR_DD_7(r1r2,r3,d1,d2) \
BOOST_PP_IF(BN_DIV_RRR_DD_TEST(7,r1r2,r3,d1,d2), \
  BN_DIV_RRR_DD_RETURN_7,BN_DIV_RRR_DD_6)(r1r2,r3,d1,d2)
#define BN_DIV_RRR_DD_6(r1r2,r3,d1,d2) \
BOOST_PP_IF(BN_DIV_RRR_DD_TEST(6,r1r2,r3,d1,d2), \
  BN_DIV_RRR_DD_RETURN_6,BN_DIV_RRR_DD_5)(r1r2,r3,d1,d2)
#define BN_DIV_RRR_DD_5(r1r2,r3,d1,d2) \
BOOST_PP_IF(BN_DIV_RRR_DD_TEST(5,r1r2,r3,d1,d2), \
  BN_DIV_RRR_DD_RETURN_5,BN_DIV_RRR_DD_4)(r1r2,r3,d1,d2)
#define BN_DIV_RRR_DD_4(r1r2,r3,d1,d2) \
BOOST_PP_IF(BN_DIV_RRR_DD_TEST(4,r1r2,r3,d1,d2), \
  BN_DIV_RRR_DD_RETURN_4,BN_DIV_RRR_DD_3)(r1r2,r3,d1,d2)
#define BN_DIV_RRR_DD_3(r1r2,r3,d1,d2) \
BOOST_PP_IF(BN_DIV_RRR_DD_TEST(3,r1r2,r3,d1,d2), \
  BN_DIV_RRR_DD_RETURN_3,BN_DIV_RRR_DD_2)(r1r2,r3,d1,d2)
#define BN_DIV_RRR_DD_2(r1r2,r3,d1,d2) \
BOOST_PP_IF(BN_DIV_RRR_DD_TEST(2,r1r2,r3,d1,d2), \
  BN_DIV_RRR_DD_RETURN_2,BN_DIV_RRR_DD_1)(r1r2,r3,d1,d2)
#define BN_DIV_RRR_DD_1(r1r2,r3,d1,d2) \
BOOST_PP_IF(BN_DIV_RRR_DD_TEST(1,r1r2,r3,d1,d2), \
  BN_DIV_RRR_DD_RETURN_1,BN_DIV_RRR_DD_0)(r1r2,r3,d1,d2)
#define BN_DIV_RRR_DD_0(r1r2,r3,d1,d2) 0

#define BN_DIV_RRR_DD_RETURN_9(r1r2,r3,d1,d2) 9
#define BN_DIV_RRR_DD_RETURN_8(r1r2,r3,d1,d2) 8
#define BN_DIV_RRR_DD_RETURN_7(r1r2,r3,d1,d2) 7
#define BN_DIV_RRR_DD_RETURN_6(r1r2,r3,d1,d2) 6
#define BN_DIV_RRR_DD_RETURN_5(r1r2,r3,d1,d2) 5
#define BN_DIV_RRR_DD_RETURN_4(r1r2,r3,d1,d2) 4
#define BN_DIV_RRR_DD_RETURN_3(r1r2,r3,d1,d2) 3
#define BN_DIV_RRR_DD_RETURN_2(r1r2,r3,d1,d2) 2
#define BN_DIV_RRR_DD_RETURN_1(r1r2,r3,d1,d2) 1

#define BN_DIV_RRR_DD_TEST(n,r1r2,r3,d1,d2) \
BN_GREATER_EQUAL_RRR_QQQ( \
  r1r2,r3, \
  BOOST_PP_ADD( \
    BOOST_PP_MUL(d1,n), \
    BOOST_PP_DIV(BOOST_PP_MUL(d2,n),10)), \
  BOOST_PP_MOD(BOOST_PP_MUL(d2,n),10))

#define BN_GREATER_EQUAL_RRR_QQQ(r1r2,r3,q1q2,q3) \
BOOST_PP_OR( \
  BOOST_PP_GREATER(r1r2,q1q2), \
  BOOST_PP_AND( \
    BOOST_PP_EQUAL(r1r2,q1q2), \
    BOOST_PP_GREATER_EQUAL(r3,q3)))

#define BN_DIV(bn1,bn2) \
BOOST_PP_IF( \
  BN_LESS(bn1,bn2), \
  BN_DIV_CASE1, \
  BN_DIV_CASE2)(bn1,bn2)

#define BN_DIV_CASE1(bn1,bn2) (0)

#define BN_DIV_CASE2(bn1,bn2) \
BN_DIV_POSTPROCESS( \
  BOOST_PP_SEQ_TAIL(BN_DIV_STATE_RESULT(\
    BOOST_PP_SEQ_FOLD_LEFT( \
      BN_DIV_OP, \
      ( \
        (0) \
        BOOST_PP_SEQ_FIRST_N( \
         BOOST_PP_DEC(BOOST_PP_SEQ_SIZE(bn2)),bn1)) \
      (bn2) \
      (((~)(0))((~)bn2)((~))((~))((~))((~))((~))((~))((~))((~))) \
      ((~)), \
      BOOST_PP_SEQ_REST_N( \
        BOOST_PP_DEC(BOOST_PP_SEQ_SIZE(bn2)),bn1)))))

#define BN_DIV_POSTPROCESS(result) \
BOOST_PP_IF( \
  BOOST_PP_EQUAL(0,BOOST_PP_SEQ_ELEM(0,result)), \
  BOOST_PP_SEQ_TAIL, \
  BN_DIV_POSTPROCESS_CASE2)(result)

#define BN_DIV_POSTPROCESS_CASE2(result) result

#define BN_DIV_OP(s,state,x) \
BN_DIV_OP_IMPL( \
  BN_CACHED_QDIGIT_REMAINDER( \
    BN_DIV_STATE_LEAD(state)(x), \
    BN_DIV_STATE_BN2(state), \
    BN_DIV_STATE_CACHE(state)), \
  BN_DIV_STATE_BN2(state), \
  BN_DIV_STATE_RESULT(state))

#define BN_DIV_OP_IMPL(qr_cache,bn2,result) \
(BOOST_PP_SEQ_ELEM(1,qr_cache)) \
(bn2) \
(BOOST_PP_SEQ_ELEM(2,qr_cache)) \
(result(BOOST_PP_SEQ_ELEM(0,qr_cache)))

#define BN_DIV_STATE_LEAD(state) BOOST_PP_SEQ_ELEM(0,state)
#define BN_DIV_STATE_BN2(state) BOOST_PP_SEQ_ELEM(1,state)
#define BN_DIV_STATE_CACHE(state) BOOST_PP_SEQ_ELEM(2,state)
#define BN_DIV_STATE_RESULT(state) BOOST_PP_SEQ_ELEM(3,state)

/* testing */

#include <boost/preprocessor/seq/for_each_product.hpp>
#include <assert.h>

#define SAFE_DIV(bn1,bn2) \
BOOST_PP_IF(BN_LESS(bn2,(1)),SAFE_DIV_CASE1,BN_DIV)(bn1,bn2)

#define SAFE_DIV_CASE1(bn1,bn2) (0)

#define bns \
((0))((1))((9))((1)(0))((1)(1))((3)(4)(5))((6)(7)(2))((1)(0)(2)(4))

#define TEST(r,bns) \
TEST_IMPL( \
  BOOST_PP_SEQ_ELEM(0,bns), \
  BOOST_PP_SEQ_ELEM(1,bns), \
  BN_TO_LITERAL(BOOST_PP_SEQ_ELEM(0,bns)), \
  BN_TO_LITERAL(BOOST_PP_SEQ_ELEM(1,bns)))

#define TEST_IMPL(bn1,bn2,n1,n2) \
  assert((n1+n2)==BN_TO_LITERAL(BN_ADD(bn1,bn2))); \
  assert((n1*n2)==BN_TO_LITERAL(BN_MUL(bn1,bn2))); \
  assert((n1>=n2?n1-n2:0)==BN_TO_LITERAL(BN_SUB(bn1,bn2))); \
  assert((n1/(n2?n2:n1+1))==BN_TO_LITERAL(SAFE_DIV(bn1,bn2)));

int main()
{
  BOOST_PP_SEQ_FOR_EACH_PRODUCT(TEST,(bns)(bns))
  return 0;
}
