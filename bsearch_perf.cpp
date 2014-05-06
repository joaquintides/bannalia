/* Measuring performance of binary search algorithms with and without optimized
 * lexicographical comparison.
 *
 * Copyright 2014 Joaquin M Lopez Munoz.
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

#include <functional>
#include <iterator>
#include <string>

template<typename Comp,typename T>
struct comp_binder
{
  comp_binder(Comp cmp,const T& x):cmp(cmp),x(x){}

  template<typename Q> bool less(const Q& y){return cmp(y,x);}
  template<typename Q> bool greater(const Q& y){return cmp(x,y);}

  Comp     cmp;
  const T& x;
};

template<typename Comp,typename T>
comp_binder<Comp,T> comp_bind(Comp cmp,const T& x)
{
  return comp_binder<Comp,T>(cmp,x);
}

template<typename T>
struct prefix_less:std::less<T>
{
};

template<typename InputIterator,typename Distance,typename Category>
void join_advance(InputIterator& it1,InputIterator& it2,Distance n,Category)
{
  while(n--){++it1;++it2;}
}

template<typename InputIterator,typename Distance>
void join_advance(
  InputIterator& it1,InputIterator& it2,Distance n,
  std::bidirectional_iterator_tag)
{
  if(n>=0)while(n--){++it1;++it2;}
  else    while(n++){--it1;--it2;}
}
	
template<typename InputIterator,typename Distance>
void join_advance(
  InputIterator& it1,InputIterator& it2,Distance n,
  std::random_access_iterator_tag)
{
  it1+=n;
  it2+=n;
}

template<typename InputIterator,typename Distance>
void join_advance(InputIterator& it1,InputIterator& it2,Distance n)
{
  join_advance(
    it1,it2,n,
    typename std::iterator_traits<InputIterator>::iterator_category());
}

template<typename T>
struct prefix_less_value_traits
{
  typedef typename T::value_type value_type;
  static bool lt(const value_type& x,const value_type& y){return x<y;}
};

template<typename CharT,typename Traits,typename Allocator>
struct prefix_less_value_traits<std::basic_string<CharT,Traits,Allocator>>
{
  static bool lt(CharT x,CharT y){return Traits::lt(x,y);}
};

template<typename T>
struct comp_binder<prefix_less<T>,T>
{
  comp_binder(prefix_less<T> cmp,const T& x):
    cmp(cmp),x(x),x_begin(x.begin()),x_size(x.size()),
    pref_left(0),pref_right(0)
  {}

  template<typename Q> bool less(const Q& y){return cmp(y,x);}
  template<typename Q> bool greater(const Q& y){return cmp(x,y);}

  typedef prefix_less_value_traits<T> value_traits;

  bool less(const T& y)
  {
    /* std::min not used, see
     * http://randomascii.wordpress.com/2013/11/24/stdmin-causing-three-times-slowdown-on-vc/
     */

    auto n=pref_left<pref_right?pref_left:pref_right;
    auto m=x_size<y.size()?x_size:y.size();
    auto it1=x_begin;
    auto it2=y.begin();
    join_advance(it1,it2,n);
    for(;n!=m;++n,++it1,++it2){
      if(value_traits::lt(*it2,*it1))return (pref_left=n,true);
      if(value_traits::lt(*it1,*it2))return (pref_right=n,false);
    }
    return n!=x_size?(pref_left=n,true):(pref_right=n,false);
  }

  bool greater(const T& y)
  {
    auto n=pref_left<pref_right?pref_left:pref_right;
    auto m=x_size<y.size()?x_size:y.size();
    auto it1=x_begin;
    auto it2=y.begin();
    join_advance(it1,it2,n);
    for(;n!=m;++n,++it1,++it2){
      if(value_traits::lt(*it1,*it2))return (pref_right=n,true);
      if(value_traits::lt(*it2,*it1))return (pref_left=n,false);
    }
    return n!=y.size()?(pref_right=n,true):(pref_left=n,false);
  }

  prefix_less<T>             cmp;
  const T&                   x;
  typename T::const_iterator x_begin;
  typename T::size_type      x_size,pref_left,pref_right;
};

template <typename ForwardIterator,typename T,typename Compare>
ForwardIterator lower_bound(
  ForwardIterator first,ForwardIterator last,const T& x,Compare comp)
{
  ForwardIterator it;
  typename std::iterator_traits<ForwardIterator>::difference_type count,step;
  count=std::distance(first,last);
  auto cbind=comp_bind(comp,x);
  while(count>0){
    it=first;
    step=count/2;
    std::advance(it,step);
    if(cbind.less(*it)){
      first=++it;
      count-=step+1;
    }
    else count=step;
  }
  return first;
}

template<typename ForwardIterator,typename T,typename Compare>
bool binary_search(
  ForwardIterator first, ForwardIterator last,const T& x,Compare comp)
{
  ForwardIterator it;
  typename std::iterator_traits<ForwardIterator>::difference_type count,step;
  count=std::distance(first,last);
  auto cbind=comp_bind(comp,x);
  while(count>0){
    it=first;
    step=count/2;
    std::advance(it,step);
    if(cbind.less(*it)){
      first=++it;
      count-=step+1;
    }
    else count=step;
  }
  return (first!=last&&!cbind.greater(*first));
}

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <set>
#include <vector>

template<typename Comp>
struct run_lower_bound{
  typedef unsigned int result_type;

  template<typename Sequence>
  result_type operator()(const Sequence& s)const
  {
    Comp comp;
    unsigned int res=0;
    for(const auto& x:s){
      if(*(::lower_bound(s.begin(),s.end(),x,comp))==x)++res;
    }
    return res;
  }
};

template<typename Comp>
struct run_binary_search{
  typedef unsigned int result_type;

  template<typename Sequence>
  result_type operator()(const Sequence& s)const
  {
    Comp comp;
    unsigned int res=0;
    for(const auto& x:s){
      if(::binary_search(s.begin(),s.end(),x,comp))++res;
    }
    return res;
  }
};

template<typename Sequence>
void profile(const char* name,const Sequence& s)
{
  typedef typename Sequence ::value_type value_type;

  /* pretest */

  auto res1=run_lower_bound<std::less<value_type>>()(s);
  auto res2=run_lower_bound<prefix_less<value_type>>()(s);
  auto res3=run_binary_search<std::less<value_type>>()(s);
  auto res4=run_binary_search<prefix_less<value_type>>()(s);
  if(!(res1==res2&&res3==res4)){
    std::cerr<<"prefix_less implementation bug\n";
    std::exit(EXIT_FAILURE);
  }

  std::cout<<name<<";"<<s.size();

  double t=measure(
    std::bind(run_lower_bound<std::less<value_type>>(),std::cref(s)));
  std::cout<<";"<<(t/s.size())*10E6;

  t=measure(
    std::bind(run_lower_bound<prefix_less<value_type>>(),std::cref(s)));
  std::cout<<";"<<(t/s.size())*10E6;

  t=measure(
    std::bind(run_binary_search<std::less<value_type>>(),std::cref(s)));
  std::cout<<";"<<(t/s.size())*10E6;

  t=measure(
    std::bind(run_binary_search<prefix_less<value_type>>(),std::cref(s)));
  std::cout<<";"<<(t/s.size())*10E6<<std::endl;
}

template<typename Value>
struct value_conv
{
  static Value from_string(const std::string& str)
  {
    return Value(str.begin(),str.end());
  }
};

template<>
struct value_conv<std::vector<std::string>>
{
  static std::vector<std::string> from_string(const std::string& str)
  {
    std::vector<std::string> res;
    for(char ch:str)res.push_back(std::string(1,ch));
    return res;
  }
};

struct udchar
{
  udchar(char c):c(c){}
  void operator++(){++c;}
  bool operator==(const udchar& x)const{return c==x.c;}
  bool operator<(const udchar& x)const{return c<x.c;}
  
  char c;
};

template<typename Value>
std::vector<Value> strings(char base,int length,int decimate=0)
{
  typedef typename Value::size_type  size_type;
  typedef value_conv<Value>          value_conv;

  std::vector<Value> v;
  std::string        str(size_type(length),'0');
  int                d=0;
  for(bool done=false;!done;){
    if(d==0)v.push_back(value_conv::from_string(str));
    d=(d==0)?decimate:d-1;
    done=true;
    for(auto it=str.rbegin(),it_end=str.rend();it!=it_end;++it){
      auto& c=*it;
      if(c<'0'+base-1){
        ++c;
        done=false;
        break;
      }
      else c='0';
    }
  }
  return v;
}

template<typename Value>
std::vector<Value> sorted_quijote()
{
  typedef value_conv<Value> value_conv;

  std::set<Value> s;
  std::ifstream ifs("quijote.txt");
  while(ifs){
    std::string str;
    ifs>>str;
    s.insert(value_conv::from_string(str));
  }
  return std::vector<Value>(s.begin(),s.end());
}

int main()
{
  std::cout<<"name;size;lower_bound;pref lower_bound;"
            "binary_search;pref binary_search\n";

  profile("S(2,4)", strings<std::string>(2,4));
  profile("S(2,10)",strings<std::string>(2,10));
  profile("S(2,20)",strings<std::string>(2,20));
  profile("S(10,4)",strings<std::string>(10,4));
  profile("S(10,6)",strings<std::string>(10,6));
  profile("S(20,4)",strings<std::string>(20,4));
  profile("Quijote",sorted_quijote<std::string>());

  profile("S(2,4)(wstr)", strings<std::wstring>(2,4));
  profile("S(2,10)(wstr)",strings<std::wstring>(2,10));
  profile("S(2,20)(wstr)",strings<std::wstring>(2,20));
  profile("S(10,4)(wstr)",strings<std::wstring>(10,4));
  profile("S(10,6)(wstr)",strings<std::wstring>(10,6));
  profile("S(20,4)(wstr)",strings<std::wstring>(20,4));
  profile("Quijote(wstr)",sorted_quijote<std::wstring>());

  profile("S(2,4)(vec<udch>)", strings<std::vector<udchar>>(2,4));
  profile("S(2,10)(vec<udch>)",strings<std::vector<udchar>>(2,10));
  profile("S(2,20)(vec<udch>)",strings<std::vector<udchar>>(2,20));
  profile("S(10,4)(vec<udch>)",strings<std::vector<udchar>>(10,4));
  profile("S(10,6)(vec<udch>)",strings<std::vector<udchar>>(10,6));
  profile("S(20,4)(vec<udch>)",strings<std::vector<udchar>>(20,4));
  profile("Quijote(vec<udch>)",sorted_quijote<std::vector<udchar>>());

  profile("S(2,4)(vec<str>)", strings<std::vector<std::string>>(2,4));
  profile("S(2,10)(vec<str>)",strings<std::vector<std::string>>(2,10));
  profile("S(2,20)(vec<str>)",strings<std::vector<std::string>>(2,20));
  profile("S(10,4)(vec<str>)",strings<std::vector<std::string>>(10,4));
  profile("S(10,6)(vec<str>)",strings<std::vector<std::string>>(10,6));
  profile("S(20,4)(vec<str>)",strings<std::vector<std::string>>(20,4));
  profile("Quijote(vec<str>)",sorted_quijote<std::vector<std::string>>());
}
