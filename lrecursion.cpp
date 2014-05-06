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

// PRED := lambda n f x. n (lambda g h. h (g f)) (lambda u. x) (lambda u. u)
template<typename F> struct PREDAUX1;
template<typename F,typename G,typename H>
struct apply<apply<PREDAUX1<F>,G>,H>:apply<H,typename apply<G,F>::type>{};

template<typename X> struct PREDAUX2;
template<typename X,typename U>
struct apply<PREDAUX2<X>,U>{typedef X type;};

struct PREDAUX3; // equiv. to I
template<typename U>
struct apply<PREDAUX3,U>{typedef U type;};

struct PRED;
template<typename N,typename F,typename X> 
struct apply<apply<apply<PRED,N>,F>,X>:
  apply3<N,PREDAUX1<F>,PREDAUX2<X>,PREDAUX3>{};

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

// TRUE := lambda x y. x 
struct TRUE;
template<typename X,typename Y>
struct apply<apply<TRUE,X>,Y>{typedef X type;};

// FALSE := lambda x y. y 
struct FALSE;
template<typename X,typename Y>
struct apply<apply<FALSE,X>,Y>{typedef Y type;};

// AND := lambda p q. p q FALSE 
struct AND;
template<typename P,typename Q>
struct apply<apply<AND,P>,Q>:apply2<P,Q,FALSE>{};

// OR := lambda p q. p TRUE q 
struct OR;
template<typename P,typename Q>
struct apply<apply<OR,P>,Q>:apply2<P,TRUE,Q>{};

// NOT := lambda p. p FALSE TRUE
struct NOT;
template<typename P>
struct apply<NOT,P>:apply2<P,FALSE,TRUE>{};

// IFTHENELSE := lambda p f g. p f g 
struct IFTHENELSE;
template<typename P,typename F,typename G>
struct apply<apply<apply<IFTHENELSE,P>,F>,G>:apply2<P,F,G>{};

// ISZERO := lambda n. n (lambda x. FALSE) TRUE 
struct ISZEROAUX;
template<typename X>
struct apply<ISZEROAUX,X>{typedef FALSE type;};

struct ISZERO;
template<typename N>
struct apply<ISZERO,N>:apply2<N,ISZEROAUX,TRUE>{};

// FAC := lambda n. IFTHENELSE (ISZERO n) ONE (MULT n (FAC (PRED n) ) )
struct FAC;

struct FACIF;
template<typename N>
struct apply<FACIF,N>{typedef ONE type;};

struct FACELSE;
template<typename N>
struct apply<FACELSE,N>:
  apply2<
    MULT,N,
    typename apply<FAC,typename apply<PRED,N>::type>::type
  >
{};

template<typename N>
struct apply<FAC,N>:
  apply<
    typename apply3<
      IFTHENELSE,
      typename apply<ISZERO,N>::type,
      FACIF,
      FACELSE
    >::type,
    N
  >
{};

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

typedef apply2<PLUS,TWO,THREE>::type FIVE;
ASSERT_IS_NUM((apply<FAC,THREE>::type),6);
ASSERT_IS_NUM((apply<FAC,FIVE>::type),120);

int main(){}
