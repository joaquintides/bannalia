/* Experiments on slot-based C++ interfaces.
 *
 * Copyright 2013 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */
  
#include <utility>
  
/* all slots within this namespace */
  
namespace slots{
      
template<typename Instance>
struct slot
{
  const Instance& final()const{return static_cast<const Instance&>(*this);}
  
  template<typename T,typename ...Args>
  auto operator()(T&& t,Args&&... args)const
    ->decltype(t(this->final(),std::forward<Args>(args)...))
  {
    return t(final(),std::forward<Args>(args)...);
  }
};
  
}
  
/* A problem here: name is declared extern but defined nowhere. GCC seems to
 * cope fine, though. Declaring name as static would result in ODR violations
 * when a template using a slot name is instantiated in different traslation
 * units.
 */
   
#define SLOT(name) \
namespace slots{ \
struct name##_s:slot<name##_s>{}; \
extern name##_s name; \
}
  
/* slot utilities */
  
#include <iostream>
#include <typeinfo>
  
/* logger decorates T (be it a class or a slot) with some output to std::cout */
  
template <typename T>
class logger
{
  T t;
public:
  template<typename ...Args>
  logger(Args&&... args):t(std::forward<Args>(args)...){}
  
  template<typename Q,typename ...Args>
  auto operator()(Q&& q,Args&&... args)
    ->decltype(t(std::forward<Q>(q),std::forward<Args>(args)...))
  {
     std::cout<<typeid(q).name()<<std::endl;
     return t(std::forward<Q>(q),std::forward<Args>(args)...);
  }
  
  template<typename Q,typename ...Args>
  auto operator()(Q&& q,Args&&... args)const
    ->decltype(t(std::forward<Q>(q),std::forward<Args>(args)...))
  {
     std::cout<<typeid(q).name()<<std::endl;
     return t(std::forward<Q>(q),std::forward<Args>(args)...);
  }
};
  
#include <memory>
  
/* shared_ref<T> acts as T& where the underyling object is shared among
 * their references and destroyed when these cease to exist.
 */
   
template <typename T>
class shared_ref
{
  std::shared_ptr<T> p;
public:
  template<typename ...Args>
  shared_ref(Args&&... args):p(std::forward<Args>(args)...){}
  
  template<typename ...Args>
  auto operator()(Args&&... args)
    ->decltype((*p)(std::forward<Args>(args)...))
  {
     return (*p)(std::forward<Args>(args)...);
  }
};
  
/* open_ended<T> augments the slot-based interface of T by adding a
 * default overload of operator() with the semantics
 *
 *  o(s,...) -> s(t,...)
 *
 * where o is of type open_ended<T> and t is the augmented T object held by o.
 */
   
template <typename T>
class open_ended
{
  T t;
public:
  template<typename ...Args>
  open_ended(Args&&... args):t(std::forward<Args>(args)...){}
        
  template<typename Slot,typename ...Args>
  auto operator()(Slot&& s,Args&&... args)
    ->decltype(s(t,std::forward<Args>(args)...))
  {
     return s(t,std::forward<Args>(args)...);
  }
  
/* for some strange reson GCC 4.8 chokes if this const overload is present */
  
#if 0
  template<typename Slot,typename ...Args>
  auto operator()(Slot&& s,Args&&... args)const
    ->decltype(s(t,std::forward<Args>(args)...))
  {
     return s(t,std::forward<Args>(args)...);
  }
#endif
};
  
/* a slot-based replication of std::vector */
  
#include <vector>
  
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
};
  
}
  
/* testing */
  
#include <cassert>
#include <numeric>
  
template<typename Container>
static void test(Container c)
{
  using namespace slots;
  c(clear);
  for(int i=0;i<10;++i){
    push_back(c,2*i);
    c(emplace_back,3*i);
  }
    
  assert(std::accumulate(c(begin),c(end),0)==225);
}
  
template<template<class> class Decorated>
struct slot_decorator
{
  template<typename Slot>
  Decorated<Slot> operator()(const Slot& s)const{return Decorated<Slot>(s);}
};
  
template<typename Container,typename SlotDecorator>
static void test(Container c,SlotDecorator decorator)
{
  using namespace slots;
  decorator(clear)(c);
  for(int i=0;i<10;++i){
    decorator(push_back)(c,2*i);
    decorator(emplace_back)(c,3*i);
  }
    
  assert(std::accumulate(decorator(begin)(c),decorator(end)(c),0)==225);
}
  
int main()
{
  using namespace slots;
  using namespace slot_std;
   
  test(vector<int>());
  test(vector<int>(),slot_decorator<logger>());
  test(shared_ref<vector<int>>(std::make_shared<vector<int>>()));
  test(shared_ref<vector<int>>(std::make_shared<vector<int>>()),
       slot_decorator<logger>());
  test(logger<shared_ref<vector<int>>>(std::make_shared<vector<int>>()));
   
  open_ended<vector<int>> c;  
  logger<emplace_back_s>  logged_emplace_back;
  c(logged_emplace_back,100);
  assert(*(c(begin))==100);
    
  std::cout<<"all OK"<<std::endl;
}
