/* Performance measurements of a fast collection type for polymorphic objects.
 *
 * Copyright 2014-2015 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */
  
#include <algorithm>
#include <array>
#include <chrono>
#include <numeric> 

std::chrono::high_resolution_clock::time_point measure_start,measure_pause;
    
template<typename F>
double measure(F f)
{
  using namespace std::chrono;
    
  static const int              num_trials=10;
  static const milliseconds     min_time_per_trial(200);
  std::array<double,num_trials> trials;
  static decltype(f())          res; /* to avoid optimizing f() away */
    
  for(int i=0;i<num_trials;++i){
    int                               runs=0;
    high_resolution_clock::time_point t2;
    
    measure_start=high_resolution_clock::now();
    do{
      res=f();
      ++runs;
      t2=high_resolution_clock::now();
    }while(t2-measure_start<min_time_per_trial);
    trials[i]=duration_cast<duration<double>>(t2-measure_start).count()/runs;
  }
  (void)res; /* var not used warn */
    
  std::sort(trials.begin(),trials.end());
  return std::accumulate(
    trials.begin()+2,trials.end()-2,0.0)/(trials.size()-4);
}
    
void pause_timing()
{
  measure_pause=std::chrono::high_resolution_clock::now();
}
    
void resume_timing()
{
  measure_start+=std::chrono::high_resolution_clock::now()-measure_pause;
}

#include <cstddef>
#include <map>
#include <memory>
#include <random>
#include <typeindex>
#include <type_traits>
#include <vector>

template<class Base>
class poly_collection_segment_base
{
public:
  virtual ~poly_collection_segment_base(){};

  void insert(const Base& x)
  {
    this->insert_(x);
  }

  template<typename F>
  void for_each(F& f)
  {
    std::size_t s=this->element_size_();
    for(auto it=this->begin_(),end=it+this->size_()*s;it!=end;it+=s){
      f(*reinterpret_cast<Base*>(it));
    }
  }
  
  template<typename F>
  void for_each(F& f)const
  {
    std::size_t s=this->element_size_();
    for(auto it=this->begin_(),end=it+this->size_()*s;it!=end;it+=s){
      f(*reinterpret_cast<const Base*>(it));
    }
  }

  void shuffle()
  {
    this->shuffle_();
  }

private:  
  virtual void insert_(const Base& x)=0;
  virtual char* begin_()=0;
  virtual const char* begin_()const=0;
  virtual std::size_t size_()const=0;
  virtual std::size_t element_size_()const=0;
  virtual void shuffle_()=0;
};

template<class Derived,class Base>
class poly_collection_segment:
  public poly_collection_segment_base<Base>
{
private:
  virtual void insert_(const Base& x)
  {
    store.push_back(static_cast<const Derived&>(x));
  }

  virtual char* begin_()
  {
    return reinterpret_cast<char*>(
      static_cast<Base*>(const_cast<Derived*>(store.data())));
  }

  virtual const char* begin_()const
  {
    return reinterpret_cast<const char*>(
      static_cast<const Base*>(store.data()));
  }

  virtual std::size_t size_()const{return store.size();}
  virtual std::size_t element_size_()const{return sizeof(Derived);}

  virtual void shuffle_()
  {
    std::shuffle(store.begin(),store.end(),std::mt19937(1));
  }

  std::vector<Derived> store;
};

template<class Base>
class poly_collection
{
public:
  template<class Derived>
  void insert(
    const Derived& x,
    typename std::enable_if<std::is_base_of<Base,Derived>::value>::type* =0)
  {
    auto& pchunk=chunks[typeid(x)];
    if(!pchunk)pchunk.reset(new poly_collection_segment<Derived,Base>());
    pchunk->insert(x);
  }
 
  template<typename F>
  F for_each(F f)
  {
    for(const auto& p:chunks)p.second->for_each(f);
    return std::move(f);
  }

  template<typename F>
  F for_each(F f)const
  {
    for(const auto& p:chunks)
      const_cast<const segment&>(*p.second).for_each(f);
    return std::move(f);
  }

  void shuffle()
  {
    for(const auto& p:chunks)p.second->shuffle();
  }

private:
  typedef poly_collection_segment_base<Base> segment;
  typedef std::unique_ptr<segment>           pointer;

  std::map<std::type_index,pointer> chunks;
};

#include <functional>
#include <iostream>

struct base
{
  virtual int f(int x)const=0;
};

struct derived1:base
{
  derived1(int n):n(n){}
  virtual int f(int x)const{return x*n;}
  
  int n;
};

struct derived2:base
{
  derived2(int n):n(n){}
  virtual int f(int x)const{return x+n;}
  
  int unused,n;
};

struct derived3:base
{
  derived3(int n):n(n){}
  virtual int f(int x)const{return x-n;}
  
  int unused,n;
};

template<typename Base>
class vector_ptr
{
public:
  template<class Derived>
  void insert(
    const Derived& x,
    typename std::enable_if<std::is_base_of<Base,Derived>::value>::type* =0)
  {
    store.push_back(pointer(new Derived(x)));
  }

  template<typename F>
  F for_each(F f)
  {
    std::for_each(
      store.begin(),store.end(),
      [&f](pointer& p){f(*p);});
    return std::move(f);
  }
  
  template<typename F>
  F for_each(F f)const
  {
    std::for_each(
      store.begin(),store.end(),
      [&f](const pointer& p){f(const_cast<const Base&>(*p));});
    return std::move(f);
  }

  void shuffle()
  {
    std::shuffle(store.begin(),store.end(),std::mt19937(1));
  }
  
private:
  typedef std::unique_ptr<Base> pointer;

  std::vector<pointer> store;
};

template<typename Collection>
void fill(Collection& c,unsigned int n)
{
  n/=3;
  while(n--){
    c.insert(derived1(n));
    c.insert(derived2(n));
    c.insert(derived3(n));
  }
}

template<typename Collection>
struct run_for_each
{
  typedef int result_type;
  
  result_type operator()(const Collection& c)const
  {
    int res=0;
    c.for_each([&res](const base& x){
      res+=x.f(1);
    });
    return res;
  }
}; 

template<template<typename> class Tester,typename Collection>
double measure_test(unsigned int n)
{
  Collection c;
  fill(c,n);
  c.shuffle();
  double t=measure(std::bind(Tester<Collection>(),std::cref(c)));
  return (t/n)*10E6;
}

int main()
{
  typedef vector_ptr<base>      collection_t1;
  typedef poly_collection<base> collection_t2;
  
  unsigned int n0=1000,n1=11000000,dn=1000;
  double       fdn=1.1;
  
  std::cout<<"for_each:"<<std::endl;
  std::cout<<"vector_ptr;poly_collection"<<std::endl;
  
  for(unsigned int n=n0;n<=n1;n+=dn,dn=(unsigned int)(dn*fdn)){
    std::cout<<
      n<<";"<<
      measure_test<run_for_each,collection_t1>(n)<<";"<<
      measure_test<run_for_each,collection_t2>(n)<<std::endl;
  }
}
