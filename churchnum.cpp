/* Embedding lambda calculus into metaC++.
 *
 * Copyright 2007 Joaquín M López Muñoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */

template<typename F,typename G>
struct apply{typedef apply type;};
template<typename F,typename G,typename H>
struct apply2{typedef typename apply<typename apply<F,G>::type,H>::type type;};
template<typename F,typename G,typename H,typename I>
struct apply3{typedef typename apply<typename apply2<F,G,H>::type,I>::type type;};

// 0 := lambda f x. x 
struct ZERO;
template<typename F,typename X>
struct apply<apply<ZERO,F>,X>{typedef X type;};

// 1 := lambda f x. f x 
struct ONE;
template<typename F,typename X>
struct apply<apply<ONE,F>,X>:apply<F,X>{};

// 2 := lambda f x. f (f x) 
struct TWO;
template<typename F,typename X>
struct apply<apply<TWO,F>,X>:apply<F,typename apply<F,X>::type>{};

// 3 := lambda f x. f (f (f x))
struct THREE;
template<typename F,typename X>
struct apply<apply<THREE,F>,X>:
  apply<F,typename apply<F,typename apply<F,X>::type>::type>{};

// SUCC := lambda n f x. f (n f x)
struct SUCC;
template<typename N,typename F,typename X> 
struct apply<apply<apply<SUCC,N>,F>,X>:
  apply<F,typename apply2<N,F,X>::type>{};

// PLUS := lambda m n f x. n f (m f x)
struct PLUS;
template<typename M,typename N,typename F,typename X>
struct apply<apply<apply<apply<PLUS,M>,N>,F>,X>:
  apply2<N,F,typename apply2<M,F,X>::type>{};

// MULT := lambda m n. m (PLUS n) 0
struct MULT;
template<typename M,typename N>
struct apply<apply<MULT,M>,N>:
  apply2<M,typename apply<PLUS,N>::type,ZERO>{};

// testing
#include <boost/type_traits/is_same.hpp>
#include <boost/mpl/assert.hpp>

using namespace boost;

template <int N>struct int_{};
struct succ;
template<int N> struct apply<succ,int_<N> >{typedef int_<N+1> type;};

template<typename T> struct unparens_helper;
template<typename T> struct unparens_helper<void(T)>{typedef T type;};
#define UNPARENTHESIZE(f) unparens_helper<void f>::type

#define ASSERT_IS_NUM(f,n)                                             \
BOOST_MPL_ASSERT((                                                     \
  is_same<                                                             \
    apply<apply<UNPARENTHESIZE(f),succ>::type,int_<0> >::type, \
    int_<n> >))

ASSERT_IS_NUM((ZERO),0);
ASSERT_IS_NUM((ONE),1);
ASSERT_IS_NUM((TWO),2);
ASSERT_IS_NUM((THREE),3);
ASSERT_IS_NUM((apply<SUCC,TWO>::type),3);
ASSERT_IS_NUM((apply<SUCC,TWO>::type),3);
ASSERT_IS_NUM((apply<apply<PLUS,TWO>::type,THREE>::type),5);
typedef apply<apply<MULT,THREE>::type,THREE>::type NINE;
ASSERT_IS_NUM((NINE),9);
typedef apply<apply<MULT,NINE>::type,NINE>::type EIGHTYONE;
ASSERT_IS_NUM((EIGHTYONE),81);
ASSERT_IS_NUM((apply<apply<PLUS,EIGHTYONE>::type,TWO>::type),83);

int main(){}
