/* Cpp tail recursion.
 *
 * Copyright 2008 Joaquín M López Muñoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */

#include <boost/preprocessor/arithmetic/dec.hpp>
#include <boost/preprocessor/arithmetic/mul.hpp>
#include <boost/preprocessor/control/if.hpp>
#include <boost/preprocessor/control/while.hpp>
#include <boost/preprocessor/seq/elem.hpp>
#include <boost/preprocessor/seq/rest_n.hpp>
#include <boost/preprocessor/seq/seq.hpp>

#define RECURSE(body,args) \
RECURSE_RES(BOOST_PP_WHILE(RECURSE_PRED,RECURSE_OP,(body)ITERATE args))

#define RECURSE_PRED(d,state) \
BOOST_PP_SEQ_ELEM(1,state)

#define RECURSE_OP(d,state) \
(RECURSE_BODY(state))RECURSE_BODY(state)(BOOST_PP_SEQ_REST_N(2,state))

#define RECURSE_BODY(state) \
BOOST_PP_SEQ_ELEM(0,state)

#define RECURSE_RES(state) \
BOOST_PP_SEQ_ELEM(2,state)

#define STOP (0)

#define ITERATE (1)

#define FACTORIAL(n) RECURSE(FACTORIAL_BODY,(n)(1))

#define FACTORIAL_BODY(args) \
FACTORIAL_BODY_IMPL( \
  BOOST_PP_SEQ_ELEM(0,args),BOOST_PP_SEQ_ELEM(1,args))

#define FACTORIAL_BODY_IMPL(n,res) \
BOOST_PP_IF( \
  n, \
  ITERATE(BOOST_PP_DEC(n))(BOOST_PP_MUL(n,res)), \
  STOP(res))

/* testing */ 

#include <stdio.h>

int main()
{
  printf("fact(1)=%d\n",FACTORIAL(1));
  printf("fact(2)=%d\n",FACTORIAL(2));
  printf("fact(3)=%d\n",FACTORIAL(3));
  printf("fact(4)=%d\n",FACTORIAL(4));
  printf("fact(5)=%d\n",FACTORIAL(5));
}
