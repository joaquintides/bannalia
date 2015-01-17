/* Performance measurements of a fast collection type for polymorphic objects
 * with devirtulization.
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

#include <boost/config.hpp>
#include <cstddef>
#include <map>
#include <memory>
#include <random>
#include <typeindex>
#include <type_traits>
#include <vector>

#ifdef BOOST_NO_CXX11_VARIADIC_TEMPLATES
#include <boost/preprocessor/facilities/intercept.hpp>
#include <boost/preprocessor/repetition/enum.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_params_with_a_default.hpp>
#include <boost/preprocessor/repetition/enum_shifted_params.hpp>
#endif

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

private:  
  virtual void insert_(const Base& x)=0;
  virtual char* begin_()=0;
  virtual const char* begin_()const=0;
  virtual std::size_t size_()const=0;
  virtual std::size_t element_size_()const=0;
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

  std::vector<Derived> store;
};

template<class Derived>
class poly_collection_static_segment
{
public:
  void insert(const Derived& x)
  {
    store.push_back(x);
  }
  
  template<typename F>
  void for_each(F& f)
  {
    std::for_each(store.begin(),store.end(),f);
  }
  
  template<typename F>
  void for_each(F& f)const
  {
    std::for_each(store.begin(),store.end(),f);
  }

private:
  std::vector<Derived> store;
};

#ifndef BOOST_NO_CXX11_VARIADIC_TEMPLATES
#define POLY_COLLECTION_TEMPL_PARAM_PACK                  class ...Derivedn
#define POLY_COLLECTION_TEMPL_PARAM_PACK_REMAINDER        class ...Derivedn
#define POLY_COLLECTION_TEMPL_EXPAND_PARAM_PACK           Derivedn...
#define POLY_COLLECTION_TEMPL_EXPAND_PARAM_PACK_REMAINDER Derivedn...
#else
#define POLY_COLLECTION_LIMIT_VARTEMPL_ARGS 5
#define POLY_COLLECTION_TEMPL_PARAM_PACK \
BOOST_PP_ENUM_PARAMS_WITH_A_DEFAULT(\
  POLY_COLLECTION_LIMIT_VARTEMPL_ARGS,class Derived,void)
#define POLY_COLLECTION_TEMPL_PARAM_PACK_REMAINDER \
BOOST_PP_ENUM_SHIFTED_PARAMS(POLY_COLLECTION_LIMIT_VARTEMPL_ARGS,class Derived)
#define POLY_COLLECTION_TEMPL_EXPAND_PARAM_PACK \
BOOST_PP_ENUM_PARAMS(POLY_COLLECTION_LIMIT_VARTEMPL_ARGS,Derived)
#define POLY_COLLECTION_TEMPL_EXPAND_PARAM_PACK_REMAINDER \
BOOST_PP_ENUM_SHIFTED_PARAMS(POLY_COLLECTION_LIMIT_VARTEMPL_ARGS,Derived)
#endif

#ifndef BOOST_NO_CXX11_VARIADIC_TEMPLATES
template<POLY_COLLECTION_TEMPL_PARAM_PACK>
class poly_collection_static_segments
#else
template<POLY_COLLECTION_TEMPL_PARAM_PACK>
class poly_collection_static_segments;

template<>
class poly_collection_static_segments<
  BOOST_PP_ENUM_PARAMS(
    POLY_COLLECTION_LIMIT_VARTEMPL_ARGS,void BOOST_PP_INTERCEPT)
>
#endif
{
  struct inaccessible;
public:
  void insert(inaccessible);
  template<typename F>
  void for_each(F& f){}
  template<typename F>
  void for_each(F f)const{}
};

#ifndef BOOST_NO_CXX11_VARIADIC_TEMPLATES
template<class Derived0,POLY_COLLECTION_TEMPL_PARAM_PACK_REMAINDER>
class poly_collection_static_segments<
  Derived0,POLY_COLLECTION_TEMPL_EXPAND_PARAM_PACK_REMAINDER>:
#else
template<class Derived0,POLY_COLLECTION_TEMPL_PARAM_PACK_REMAINDER>
class poly_collection_static_segments:
#endif
  public poly_collection_static_segment<Derived0>,
  public poly_collection_static_segments<
    POLY_COLLECTION_TEMPL_EXPAND_PARAM_PACK_REMAINDER>
{
  typedef poly_collection_static_segment<Derived0>     super1;
  typedef poly_collection_static_segments<
    POLY_COLLECTION_TEMPL_EXPAND_PARAM_PACK_REMAINDER> super2;
public:
  using super2::insert;
  using super1::insert;

  template<typename F>
  void for_each(F& f)
  {
    super1::for_each(f);
    super2::for_each(f);
  }
  
  template<typename F>
  void for_each(F& f)const
  {
    super1::for_each(f);
    super2::for_each(f);
  }
};

template<class Base,POLY_COLLECTION_TEMPL_PARAM_PACK>
class poly_collection:
  public poly_collection_static_segments<
    POLY_COLLECTION_TEMPL_EXPAND_PARAM_PACK>
{
  typedef poly_collection_static_segments<
    POLY_COLLECTION_TEMPL_EXPAND_PARAM_PACK> super; 
public:
  using super::insert;

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
    super::for_each(f);
    return std::move(f);
  }

  template<typename F>
  F for_each(F f)const
  {
    for(const auto& p:chunks)
      const_cast<const segment&>(*p.second).for_each(f);
    super::for_each(f);
    return std::move(f);
  }

private:
  typedef poly_collection_segment_base<Base> segment;
  typedef std::unique_ptr<segment>           pointer;

  std::map<std::type_index,pointer> chunks;
};

#undef POLY_COLLECTION_TEMPL_EXPAND_PARAM_PACK_REMAINDER
#undef POLY_COLLECTION_TEMPL_EXPAND_PARAM_PACK
#undef POLY_COLLECTION_TEMPL_PARAM_PACK_REMAINDER
#undef POLY_COLLECTION_TEMPL_PARAM_PACK

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

struct final_derived1:base
{
  final_derived1(int n):n(n){}
  virtual int f(int x)const final{return x*n;}
  
  int n;
};

struct final_derived2:base
{
  final_derived2(int n):n(n){}
  virtual int f(int x)const final{return x+n;}
  
  int unused,n;
};

struct final_derived3:base
{
  final_derived3(int n):n(n){}
  virtual int f(int x)const final{return x-n;}
  
  int unused,n;
};

template<typename Collection>
struct fill_derived
{
  void operator()(Collection& c,unsigned int n)const
  {
    n/=3;
    while(n--){
      c.insert(derived1(n));
      c.insert(derived2(n));
      c.insert(derived3(n));
    }
  }
};

template<typename Collection>
struct fill_final_derived
{
  void operator()(Collection& c,unsigned int n)const
  {
    n/=3;
    while(n--){
      c.insert(final_derived1(n));
      c.insert(final_derived2(n));
      c.insert(final_derived3(n));
    }
  }
};

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

template<typename Collection>
struct run_poly_for_each
{
  struct poly_lambda
  {
    poly_lambda(int& res):res(res){};

    template<typename T>
    void operator()(const T& x)const
    {
      res+=x.f(1);
    }

    int& res;
  };

  typedef int result_type;
  
  result_type operator()(const Collection& c)const
  {
    int res=0;
    c.for_each(poly_lambda(res));
    return res;
  }
}; 

template<
  template<typename> class Tester,template<typename> class Filler,
  typename Collection
>
double measure_test(unsigned int n)
{
  Collection         c;
  Filler<Collection> fill;
  fill(c,n);
  double t=measure(std::bind(Tester<Collection>(),std::cref(c)));
  return (t/n)*10E6;
}

int main()
{
  typedef poly_collection<base> collection_t1;
  typedef poly_collection<
    base,derived1>              collection_t2;
  typedef poly_collection<
    base,derived1,derived2>     collection_t3;
  typedef poly_collection<
    base,
    derived1,derived2,derived3> collection_t4;
  typedef poly_collection<
    base,
    final_derived1>             collection_t5;
  typedef poly_collection<
    base,
    final_derived1,
    final_derived2>             collection_t6;
  typedef poly_collection<
    base,
    final_derived1,
    final_derived2,
    final_derived3>             collection_t7;

  static unsigned int ns[]={1000,10000,100000,10000000};

  std::cout<<"pc<b>;pc<b,d1>;pc<b,d1,d2>;pc<b,d1,d2,d3>"
             "pc<b,fd1>;pc<b,fd1,fd2>;pc<b,fd1,fd2,fd3>"<<std::endl;
  std::cout<<"for_each:"<<std::endl;
  for(auto n:ns){
    std::cout<<
      n<<";"<<
      measure_test<run_for_each,fill_derived,collection_t1>(n)<<";"<<
      measure_test<run_for_each,fill_derived,collection_t2>(n)<<";"<<
      measure_test<run_for_each,fill_derived,collection_t3>(n)<<";"<<
      measure_test<run_for_each,fill_derived,collection_t4>(n)<<";"<<
      measure_test<run_for_each,fill_final_derived,collection_t5>(n)<<";"<<
      measure_test<run_for_each,fill_final_derived,collection_t6>(n)<<";"<<
      measure_test<run_for_each,fill_final_derived,collection_t7>(n)<<std::endl;
  }

  std::cout<<"poly_for_each:"<<std::endl;
  for(auto n:ns){
    std::cout<<
      n<<";"<<
      measure_test<run_poly_for_each,fill_derived,collection_t1>(n)<<";"<<
      measure_test<run_poly_for_each,fill_derived,collection_t2>(n)<<";"<<
      measure_test<run_poly_for_each,fill_derived,collection_t3>(n)<<";"<<
      measure_test<run_poly_for_each,fill_derived,collection_t4>(n)<<";"<<
      measure_test<run_poly_for_each,fill_final_derived,collection_t5>(n)<<";"<<
      measure_test<run_poly_for_each,fill_final_derived,collection_t6>(n)<<";"<<
      measure_test<run_poly_for_each,fill_final_derived,collection_t7>(n)<<std::endl;
  }
}
