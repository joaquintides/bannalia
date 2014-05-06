/* Monads and slots.
 *
 * Copyright 2014 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */
      
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

/* slot stuff */

/* all slots within this namespace */
    
namespace slots{
        
template<typename Instance>
struct slot
{
  const Instance& final()const{return static_cast<const Instance&>(*this);}
    
  template<typename T,typename ...Args>
  auto operator()(T&& t,Args&&... args)const
  {
    return t(final(),std::forward<Args>(args)...);
  }
};
    
}

/* Defining slot instances this way can result in duplicate symbol link
 * problems when different translation units are involved. For the sake of
 * simplicity we won't address the problem here.
 */

#define SLOT(name) \
namespace slots{ \
struct name##_s:slot<name##_s>{}; \
name##_s name; \
} 

/* a slot-based replication of std::vector */
    
#include <vector>
#include <iostream>
    
/* Define slot names for std::vector interface.
 * If another class defines homonym slots in the same translation unit we
 * have a problem: a duplicate-proof slot definition protocol is needed.
 */
    
SLOT(assign)
SLOT(get_allocator)
SLOT(begin)
SLOT(end)
SLOT(rbegin)
SLOT(rend)
SLOT(cbegin)
SLOT(cend)
SLOT(crbegin)
SLOT(crend)
SLOT(size)
SLOT(max_size)
SLOT(resize)
SLOT(capacity)
SLOT(empty)
SLOT(reserve)
SLOT(shrink_to_fit)
SLOT(at)
SLOT(front)
SLOT(back)
SLOT(data)
SLOT(emplace_back)
SLOT(push_back)
SLOT(pop_back)
SLOT(emplace)
SLOT(insert)
SLOT(erase)
SLOT(swap)
SLOT(clear)
    
namespace slot_std{
        
using namespace slots;
      
template <class T, class Allocator = std::allocator<T> >
class vector {
  typedef std::vector<T,Allocator> impl_type;
  impl_type impl;
public:
  // types:
  typedef T value_type;
  typedef value_type& reference;
  typedef const value_type& const_reference;
  typedef typename impl_type::iterator iterator;
  typedef typename impl_type::const_iterator const_iterator;
  typedef typename impl_type::size_type size_type; 
  typedef typename impl_type::difference_type difference_type;
  typedef Allocator allocator_type;
  typedef typename std::allocator_traits<Allocator>::pointer pointer;
  typedef typename std::allocator_traits<Allocator>::const_pointer const_pointer;
  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
      
  // construct/copy/destroy:
  explicit vector(const Allocator& al = Allocator()):impl(al){}
  explicit vector(size_type n, const Allocator& al = Allocator()):impl(n,al){}
  vector(size_type n, const T& value, const Allocator& al = Allocator()):impl(n,value,al){}
  template <class InputIterator>
  vector(InputIterator first, InputIterator last,
    const Allocator& = Allocator()):impl(first,last){}
  vector(const vector& x):impl(x.impl){}
  vector(vector&& x):impl(std::move(x.impl)){}
  vector(const vector& x, const Allocator& al):impl(x.impl,al){}
  vector(vector&& x, const Allocator& al):impl(std::move(x.impl),al){}
  vector(std::initializer_list<T> il, const Allocator& al = Allocator()):impl(il,al){}
  ~vector(){}
  vector& operator=(const vector& x){impl=x.impl;}
  vector& operator=(vector&& x){impl=std::move(x.impl);}
  vector& operator=(std::initializer_list<T> il){impl=il;}
  template <class InputIterator>
  void operator()(assign_s,InputIterator first, InputIterator last){impl.assign(first,last);}
  void operator()(assign_s,size_type n, const T& u){impl.assign(n,u);}
  void operator()(assign_s,std::initializer_list<T> il){impl.assign(il);}
  allocator_type operator()(get_allocator_s) const noexcept{return impl.get_allocator();}
    
  // iterators:
  iterator operator()(begin_s) noexcept{return impl.begin();}
  const_iterator operator()(begin_s) const noexcept{return impl.begin();}
  iterator operator()(end_s) noexcept{return impl.end();}
  const_iterator operator()(end_s) const noexcept{return impl.end();}
  reverse_iterator operator()(rbegin_s) noexcept{return impl.rbegin();}
  const_reverse_iterator operator()(rbegin_s) const noexcept{return impl.rbegin();}
  reverse_iterator operator()(rend_s) noexcept{return impl.rend();}
  const_reverse_iterator operator()(rend_s) const noexcept{return impl.end();}
  const_iterator operator()(cbegin_s) const noexcept{return impl.cbegin();}
  const_iterator operator()(cend_s) const noexcept{return impl.cend();}
  const_reverse_iterator operator()(crbegin_s) const noexcept{return impl.crbegin();}
  const_reverse_iterator operator()(crend_s) const noexcept{return impl.crend();}
    
  // capacity:
  size_type operator()(size_s) const noexcept{return impl.size();}
  size_type operator()(max_size_s) const noexcept{return impl.max_size();}
  void operator()(resize_s,size_type sz){impl.resize(sz);}
  void operator()(resize_s,size_type sz, const T& c){impl.resize(sz,c);}
  size_type operator()(capacity_s) const noexcept{return impl.capacity();}
  bool operator()(empty_s) const noexcept{return impl.empty();}
  void operator()(reserve_s,size_type n){impl.reserve(n);}
  void operator()(shrink_to_fit_s){impl.shrink_to_fit();}
    
  // element access:
  reference operator[](size_type n){return impl.operator[](n);}
  const_reference operator[](size_type n) const{return impl.operator[](n);}
  const_reference operator()(at_s,size_type n) const{return impl.at(n);}
  reference operator()(at_s,size_type n){return impl.at(n);}
  reference operator()(front_s){return impl.front();}
  const_reference operator()(front_s) const{return impl.front();}
  reference operator()(back_s){return impl.back();}
  const_reference operator()(back_s) const{return impl.back();}
    
  // data access
  T* operator()(data_s) noexcept{return impl.data();}
  const T* operator()(data_s) const noexcept{return impl.data();}
    
  // modifiers:
  template <class... Args> void operator()(emplace_back_s,Args&&... args){impl.emplace_back(std::forward<Args>(args)...);}
  void operator()(push_back_s,const T& x){impl.push_back(x);}
  void operator()(push_back_s,T&& x){impl.push_back(std::move(x));}
  void operator()(pop_back_s){impl.pop_back();}
  template <class... Args> iterator operator()(emplace_s,const_iterator position, Args&&... args){return impl.emplace(position,std::forward<Args>(args)...);}
  iterator operator()(insert_s,const_iterator position, const T& x){return impl.insert(position,x);}
  iterator operator()(insert_s,const_iterator position, T&& x){return impl.insert(position,std::move(x));}
  iterator operator()(insert_s,const_iterator position, size_type n, const T& x){return impl.insert(position,n,x);}
  template <class InputIterator>
  iterator operator()(insert_s,const_iterator position,
    InputIterator first, InputIterator last){return impl.insert(first,last);}
  iterator operator()(insert_s,const_iterator position, std::initializer_list<T> il){return impl.insert(position,il);}
  iterator operator()(erase_s,const_iterator position){return impl.erase(position);}
  iterator operator()(erase_s,const_iterator first, const_iterator last){return impl.erase(first,last);}
  void operator()(swap_s,vector& x){impl.swap(x.impl);}
  void operator()(clear_s) noexcept{impl.clear();}
    
  friend bool operator==(const vector<T,Allocator>& x, const vector<T,Allocator>& y){return x.impl==y.impl;}
  friend bool operator< (const vector<T,Allocator>& x, const vector<T,Allocator>& y){return x.impl<y.impl;}
  friend bool operator!=(const vector<T,Allocator>& x, const vector<T,Allocator>& y){return x.impl!=y.impl;}
  friend bool operator> (const vector<T,Allocator>& x, const vector<T,Allocator>& y){return x.impl>y.impl;}
  friend bool operator>=(const vector<T,Allocator>& x, const vector<T,Allocator>& y){return x.impl>=y.impl;}
  friend bool operator<=(const vector<T,Allocator>& x, const vector<T,Allocator>& y){return x.impl<=y.impl;}
      
  // specialized algorithms:
  friend void swap(vector<T,Allocator>& x, vector<T,Allocator>& y){x.swap(y);}

  // used for testing

  friend std::ostream& operator<<(std::ostream& os,const vector& x)  
  {
    os<<"{";
    if(x.impl.empty())os<<"}";
    else for(auto it=x.impl.begin(),it_end=x.impl.end();it!=it_end;){
      os<<*it;
      os<<(++it==it_end?"}":",");
    }
    return os;
  }
};

} 

/* monad stuff */

/* make_index_sequence from http://stackoverflow.com/questions/19783205/ */
  
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
  mlifted(F&& f):f(std::forward<F>(f)){}
    
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
  
/* slot-compatible maybe monad */
  
#include <boost/optional.hpp>
#include <boost/none.hpp>

template<template<typename> class M,typename T>
struct slot_monad
{
  template<typename Slot,typename ...Args>
  auto operator()(Slot&& s,Args&&... args)const
  {
    return mlift<M>(std::forward<Slot>(s))(
      static_cast<const M<T>&>(*this),std::forward<Args>(args)...);
  } 

  template<typename Slot,typename ...Args>
  auto operator()(Slot&& s,Args&&... args)
  {
    return mlift<M>(std::forward<Slot>(s))(
      static_cast<M<T>&>(*this),std::forward<Args>(args)...);
  } 
};

template<typename T>
struct maybe:boost::optional<T>,slot_monad<maybe,T>
{
  using super=boost::optional<T>;
  using super::super;
};

template<typename T,typename F>
auto operator>>=(const maybe<T>& m,F&& f){return m?f(m.get()):boost::none;}
template<typename T,typename F>
auto operator>>=(maybe<T>& m,F&& f){return m?f(m.get()):boost::none;}

template<typename T>
std::ostream& operator<<(std::ostream& os,const maybe<T>& m)
{
  if(m)os<<m.get();
  else os<<"none";
  return os;
}

/* slot-compatible list monad */

#include <list>

template<typename T>
struct mlist:std::list<T>,slot_monad<mlist,T>
{
  using super=std::list<T>;
  using super::super;
  mlist(const T& t){super::push_back(t);}
  mlist(T&& t){super::emplace_back(std::move(t));}
};

template<typename T,typename F>
auto operator>>=(const mlist<T>& m,F&& f)
{
   decltype(f(std::declval<T>())) ret={};
   for(const auto& t:m)ret.splice(ret.end(),f(t));
   return std::move(ret);
}

template<typename T,typename F>
auto operator>>=(mlist<T>& m,F&& f)
{
   decltype(f(std::declval<T>())) ret={};
   for(auto& t:m)ret.splice(ret.end(),f(t));
   return std::move(ret);
}

template<typename T>
std::ostream& operator<<(std::ostream& os,const mlist<T>& m)
{
  os<<"{";
  if(m.empty())os<<"}";
  else for(auto it=m.begin(),it_end=m.end();it!=it_end;){
    os<<*it;
    os<<(++it==it_end?"}":",");
  }
  return os;
}

/* testing */

#include <algorithm>
#include <iostream>

struct accumulate
{
   template<class InputIterator, class T>
   T operator()(InputIterator first,InputIterator last,T init)const
   {
     return std::accumulate(first,last,init);
   }
   
  template<class InputIterator,class T,class BinaryOperation>
  T operator()(
    InputIterator first,InputIterator last,T init,
    BinaryOperation binary_op)const
  {
    return std::accumulate(first,last,init,binary_op);
  }  
};

int main()
{
  using namespace slot_std;
  
  {
    maybe<vector<int>> v=vector<int>{};
    for(int i=0;i<100;++i)v(push_back,i);
    std::cout<<mlift<maybe>(accumulate())(v(begin),v(end),0)<<"\n";
  }
  {
    maybe<vector<int>> v=boost::none;
    for(int i=0;i<100;++i)v(push_back,i);
    std::cout<<mlift<maybe>(accumulate())(v(begin),v(end),0)<<"\n";
  }
  {
    maybe<vector<int>> v=vector<int>{};
    for(int i=0;i<100;++i)v(maybe<push_back_s>{},i);
    std::cout<<mlift<maybe>(accumulate())(v(begin),v(end),0)<<"\n";
  }
  {
    mlist<vector<int>> ml={{0,1},{1,2}};
    std::cout<<ml(size)<<"\n";
    ml(push_back,1);
    std::cout<<ml<<"\n";
    ml([](vector<int>& v){v(push_back,v(at,0));});
    std::cout<<ml<<"\n";
  }
  {
    mlist<maybe<vector<int>>> ml={vector<int>{0,1},vector<int>{2,3},boost::none};
    std::cout<<ml(size)<<"\n";
    ml(push_back,1);
    std::cout<<ml<<"\n";
    ml([](maybe<vector<int>>& v){v(push_back,v(at,0));});
    std::cout<<ml<<"\n";
    ml(push_back,mlist<int>{5,6,7,8});
    std::cout<<ml<<"\n";
  }
  {
    using namespace std::placeholders;
    mlist<maybe<vector<int>>> ml={vector<int>{0,1},vector<int>{2,3},boost::none};
    mlist<std::function<maybe<mvoid>(maybe<vector<int>>&)>> commands=
    {
      pop_back,
      std::bind(push_back,_1,0)
    };
    commands(ml);
    std::cout<<ml<<"\n";
  }
  {
    vector<int> v;
    mlift<mlist>(push_back)(v,mlist<int>{0,1,2,3,4});
    std::cout<<v<<"\n";
  }
}
