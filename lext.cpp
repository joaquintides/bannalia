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

template <int N> struct int_;

struct succ;
template<int N>
struct apply<succ,int_<N> >{typedef int_<N+1> type;};

struct pred;
template<int N>
struct apply<pred,int_<N> >{typedef int_<N-1> type;};

struct plus;
template<int M,int N>
struct apply<apply<plus,int_<M> >,int_<N> >{typedef int_<M+N> type;};

struct mult;
template<int M,int N>
struct apply<apply<mult,int_<M> >,int_<N> >{typedef int_<M*N> type;};


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

struct iszero;
template<int N>
struct apply<iszero,int_<N> >{typedef FALSE type;};
template<>
struct apply<iszero,int_<0> >{typedef TRUE type;};

// FAC := lambda n. IFTHENELSE (ISZERO n) ONE (MULT n (FAC (PRED n) ) )
struct FAC;

struct FACIF;
template<typename N>
struct apply<FACIF,N>{typedef int_<1> type;};

struct FACELSE;
template<typename N>
struct apply<FACELSE,N>:
  apply2<
    mult,N,
    typename apply<FAC,typename apply<pred,N>::type>::type
  >
{};

template<typename N>
struct apply<FAC,N>:
  apply<
    typename apply3<
      IFTHENELSE,
      typename apply<iszero,N>::type,
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

BOOST_MPL_ASSERT((is_same<apply<FAC,int_<3> >::type,int_<6> >));
BOOST_MPL_ASSERT((is_same<apply<FAC,int_<5> >::type,int_<120> >));

int main(){}
