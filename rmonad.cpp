/* Resource monad in C++ template metaprogramming.
 *
 * Copyright 2008 Joaquín M López Muñoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */

#include <boost/mpl/apply.hpp>
#include <boost/mpl/divides.hpp>
#include <boost/mpl/equal_to.hpp>
#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/minus.hpp>
#include <boost/mpl/modulus.hpp>
#include <boost/mpl/negate.hpp>
#include <boost/mpl/next_prior.hpp>
#include <boost/mpl/pair.hpp>
#include <boost/mpl/plus.hpp>
#include <boost/mpl/quote.hpp>
#include <boost/mpl/times.hpp>
#include <iostream>

using namespace boost::mpl;

template<typename M,typename F>
struct mbind;

template<typename M,typename T>
struct mreturn;

template<typename T>struct left;
template<typename T>struct right;

template<typename C>
struct RMonad
{
  typedef C      value_type;
};

template<typename C1,typename FC2>
struct mbind<RMonad<C1>,FC2>
{
  struct C
  {
    template<typename R>
    struct apply
    {
      template<typename _>
      struct impl;

      template<typename R1,typename V>
      struct impl<pair<R1,left<V> > >
      {
        typedef typename boost::mpl::apply<
          typename boost::mpl::apply<FC2,V>::type::value_type,
          R1
        >::type type;
      };

      template<typename R1,typename PC1>
      struct impl<pair<R1,right<PC1> > >
      {
        typedef pair<
          R1,
          right<typename mbind<PC1,FC2>::type>
        > type;
      };

      typedef typename impl<
        typename boost::mpl::apply<C1,R>::type
      >::type type;
    };
  };

  typedef RMonad<C> type;
};

template<typename _,typename V>
struct mreturn<RMonad<_>,V>
{
  struct C
  {
    template<typename R>
    struct apply
    {
      typedef pair<R,left<V> > type;
    };
  };

  typedef RMonad<C> type;
};

template<typename V>
struct step
{
  struct C
  {
    template<typename R>
    struct apply:
      if_c<
        R::value!=0,
        pair<int_<R::value-1>,left<V> >,
        pair<R,right<RMonad<C> > >
      >
    {};
  };

  typedef RMonad<C> type;
};

template<typename F>
struct lift1
{
  struct type
  {
    template<typename RA1>
    struct apply
    {
      struct FC
      {
        template<typename A1>
        struct apply
        {
          typedef typename step<
            typename boost::mpl::apply<F,A1>::type
          >::type type;
        };
      };

      typedef typename mbind<RA1,FC>::type type;
    };
  };
};

template<typename F>
struct lift2
{
  struct type
  {
    template<typename RA1,typename RA2>
    struct apply
    {
      struct FC1
      {
        template<typename A1>
        struct apply
        {
          struct FC2
          {
            template<typename A2>
            struct apply
            {
              typedef typename step<
                typename boost::mpl::apply<F,A1,A2>::type
              >::type type;
            };
          };

          typedef typename mbind<RA2,FC2>::type type;
        };
      };

      typedef typename mbind<RA1,FC1>::type type;
    };
  };
};

#define LIFT1(f)                            \
template<typename C>                        \
struct f<RMonad<C> >                        \
{                                           \
  struct F                                  \
  {                                         \
    template<typename T>                    \
    struct apply                            \
    {                                       \
      typedef typename f<T>::type type;     \
    };                                      \
  };                                        \
                                            \
  typedef typename apply<                   \
    typename lift1<F>::type,                \
    RMonad<C>                               \
  >::type type;                             \
};

#define LIFT2(f)                            \
template<typename C1,typename C2>           \
struct f<RMonad<C1>,RMonad<C2> >            \
{                                           \
  struct F                                  \
  {                                         \
    template<typename T1,typename T2>       \
    struct apply                            \
    {                                       \
      typedef typename f<T1,T2>::type type; \
    };                                      \
  };                                        \
                                            \
  typedef typename apply<                   \
    typename lift2<F>::type,                \
    RMonad<C1>,RMonad<C2>                   \
  >::type type;                             \
};
  
namespace boost{
namespace mpl{

LIFT1(negate)
LIFT1(next)
LIFT1(prior)
LIFT2(equal_to)
LIFT2(plus)
LIFT2(minus)
LIFT2(times)
LIFT2(divides)
LIFT2(modulus)

template<typename C,typename T1,typename T2>
struct if_<RMonad<C>,T1,T2>
{
  struct impl
  {
    template<typename CC>
    struct apply:if_<CC,T1,T2>{};
  };

  typedef typename mbind<RMonad<C>,impl>::type type;
};

template<typename C,typename T1,typename T2>
struct eval_if<RMonad<C>,T1,T2>
{
  struct impl
  {
    template<typename CC>
    struct apply:eval_if<CC,T1,T2>{};
  };

  typedef typename mbind<RMonad<C>,impl>::type type;
};

} // namespace mpl
} // namespace boost

template<int N>
struct from_int
{
  typedef typename mreturn<RMonad<void>,int_<N> >::type type;
};

template<typename X>
struct fact
{
  struct else_:
    times<
      X,
      typename fact<typename prior<X>::type>::type
    >
  {};

  typedef typename eval_if<
    typename equal_to<X,from_int<0>::type>::type,
    from_int<1>,
    else_
  >::type type;
};

template<int N,typename RM>
struct run
{
  template<typename _>
  struct impl:int_<0>
  {
    enum{steps=-1};
  };

  template<typename R,typename V>
  struct impl<pair<R,left<V> > >:V
  {
    enum{steps=N-R::value};
  };

  typedef impl<
    typename apply<typename RM::value_type,int_<N> >::type
  > type;
};

int main()
{
  typedef negate<
    plus<
      from_int<2>::type,
      from_int<2>::type
    >::type
  >::type                               m1;
  typedef run<20,m1>::type              r1;
  typedef fact<from_int<3>::type>::type m2;
  typedef run<20,m2>::type              r2;

  std::cout<<"-(2+2)="<<r1::value<<" ("<<r1::steps<<" steps)"<<std::endl;
  std::cout<<"fact(3)="<<r2::value<<" ("<<r2::steps<<" steps)"<<std::endl;
}
