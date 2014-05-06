/* Monadic lifting.
 *
 * Copyright 2014 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */
    
#include <utility>
#include <functional>
#include <tuple>
#include <type_traits>

// make_index_sequence from http://stackoverflow.com/questions/19783205/

template<std::size_t ...> struct index_sequence{using type=index_sequence;};
template<typename T> using invoke = typename T :: type ;
template<typename T,typename U> struct concate;
template<std::size_t ...I,std::size_t ...J>
struct concate<index_sequence<I...>,index_sequence<J...>>:
  index_sequence<I...,(J+sizeof ...(I))...>
{};
template<std::size_t N>
struct make_index_sequence_help:
concate<
  invoke<make_index_sequence_help<N/2>>,
  invoke<make_index_sequence_help<N-N/2>>
>
{};
template<> struct make_index_sequence_help<0>:index_sequence<>{};
template<> struct make_index_sequence_help<1>:index_sequence<0>{};

template<int N> using make_index_sequence=invoke<make_index_sequence_help<N>>;

template <typename F,typename Tuple,size_t... I>
auto apply_pointer_tuple_impl(F&& f,Tuple&& t,index_sequence<I...>)
{
  return std::forward<F>(f)(*std::get<I>(std::forward<Tuple>(t))...);
}
template <typename F,typename Tuple>
auto apply_pointer_tuple(F&& f,Tuple&& t) 
{
  using index_sequence=
    typename make_index_sequence<
      std::tuple_size<typename std::decay<Tuple>::type>::value>::type;
  return apply_pointer_tuple_impl(
    std::forward<F>(f),std::forward<Tuple>(t),index_sequence());
}

struct mvoid{};

template<template<typename> class M,class T>
M<T> mreturn(T&& t)
{
  return std::forward<T>(t);
}

template<template<typename> class M,class T>
struct mwrapped{using type=T;};
template<template<typename> class M,class T>
struct mwrapped<M,M<T>>{using type=T;};
template<template<typename> class M,class T>
struct mwrapped<M,const M<T>>{using type=const T;};
template<template<typename> class M,class T>
using mwrapped_t=typename mwrapped<M,T>::type;

template<template<typename> class M,typename Context,std::size_t N>
struct mlifted_call_stage
{
  static const std::size_t I=Context::num_args-N;
  
  mlifted_call_stage(Context& c):c(c){}
  
  auto operator()(){return operator()(*std::get<I>(c.args));}
  
  template<typename T>
  auto operator()(T&& t)
  {
     std::get<I>(c.postargs)=&t;
     mlifted_call_stage<M,Context,N-1> next(c);
     return next();
  }

  template<typename T> auto operator()(const M<T>& m){return m>>=*this;}
  template<typename T> auto operator()(M<T>& m){return m>>=*this;}

  Context& c;
};

template<template<typename> class M,typename Context>
struct mlifted_call_stage_end_base
{
  mlifted_call_stage_end_base(Context& c):c(c){}
 
  static const bool void_return=
    std::is_void<
      typename std::result_of<mlifted_call_stage_end_base()>::type>::value;

  auto operator()()
  {
     return apply_pointer_tuple(c.f,c.postargs);
  }

  Context& c;
};

template<
  template<typename> class M,typename Context,
  bool VoidReturn=mlifted_call_stage_end_base<M,Context>::void_return
>
struct mlifted_call_stage_end;

template<template<typename> class M,typename Context>
struct mlifted_call_stage_end<M,Context,false>:
  mlifted_call_stage_end_base<M,Context>
{
  using super=mlifted_call_stage_end_base<M,Context>;
  
  mlifted_call_stage_end(Context& c):super(c){}
 
  auto operator()()
  {
    return mreturn<M>(super::operator()());
  }
};

template<template<typename> class M,typename Context>
struct mlifted_call_stage_end<M,Context,true>:
  mlifted_call_stage_end_base<M,Context>
{
  using super=mlifted_call_stage_end_base<M,Context>;
  
  mlifted_call_stage_end(Context& c):super(c){}
 
  auto operator()()
  {
    super::operator()();
    return mreturn<M>(mvoid());
  }
};

template<template<typename> class M,typename Context>
struct mlifted_call_stage<M,Context,0>:mlifted_call_stage_end<M,Context>
{
  using super=mlifted_call_stage_end<M,Context>;
  
  mlifted_call_stage(Context& c):super(c){}
};

template<template<typename> class M,typename F,typename ...Args>
struct mlifted_call_context
{
  static const std::size_t num_args=sizeof...(Args);
  
  mlifted_call_context(F& f,Args&... args):
    f(f),args(std::make_tuple(&args...)){}
  
  auto operator()()
  {
    mlifted_call_stage<M,mlifted_call_context,num_args> stage(*this);
    return stage();
  }
  
  F& f;
  std::tuple<
    typename std::remove_reference<Args>::type*...>               args;
  std::tuple<
    mwrapped_t<M,typename std::remove_reference<Args>::type>*...> postargs;
};

template<template<typename> class M,typename F>
struct mlifted
{
  mlifted(const F& f):f(f){}
  mlifted(F&& f):f(std::move(f)){}
  
  template<typename ...Args>
  auto operator()(Args&&... args)
  {
    mlifted_call_context<M,F,Args...> context(f,args...);
    return context();
  }
  
private:
  F f;
};

template<template<typename> class M,typename F>
mlifted<M,F> mlift(F&& f){return std::forward<F>(f);}

/* testing */

#include <algorithm>
#include <boost/optional.hpp>
#include <boost/none.hpp>
#include <iostream>
#include <vector>

template<typename T>
struct maybe:boost::optional<T>
{
  using super=boost::optional<T>;
  using super::super;
};

template<typename T,typename F>
auto operator>>=(const maybe<T>& m,F&& f){return m?f(m.get()):boost::none;}
template<typename T,typename F>
auto operator>>=(maybe<T>& m,F&& f){return m?f(m.get()):boost::none;}

std::ostream& operator<<(std::ostream& os,mvoid)
{
  return os<<"void";
}

template<typename T>
void dump(const maybe<T>& m)
{
  if(m)std::cout<<m.get()<<"\n";
  else std::cout<<"none\n";
}

template<typename T>
struct logged
{
  logged(const T& t):t(t){}
  
  T t;
};

template<typename T,typename F>
auto operator>>=(const logged<T>& m,F&& f)
{
  auto ret=f(m.t);
  std::cout<<m.t<<"->"<<ret.t<<"\n";
  return ret;
}

template<typename T,typename F>
auto operator>>=(logged<T>& m,F&& f)
{
  auto ret=f(m.t);
  std::cout<<m.t<<"->"<<ret.t<<"\n";
  return ret;
}

int foo(int x,int y){return x*y;}
void bar(int& x){x=190;}

int main()
{
  auto mfoo=mlift<maybe>(&foo);
  dump(mfoo(maybe<int>(),maybe<int>(2)));
  const maybe<int> m=2;
  dump(mfoo(m,m));
  dump(mfoo(2,maybe<int>(2)));
  dump(mfoo(2,2));
  dump(mfoo(2,maybe<int>()));


  auto mbar=mlift<maybe>(&bar);
  maybe<int> mm=2;
  mbar(mm);
  dump(mm);
  dump(mbar(mm));

  auto lfoo=mlift<logged>(&foo);
  lfoo(logged<int>(2),logged<int>(3));
  
  std::vector<logged<int> > v;
  for(int i=0;i<10;++i)v.push_back(i);
  std::accumulate(
    v.begin(),v.end(),logged<int>(0),mlift<logged>(std::plus<int>()));
}
