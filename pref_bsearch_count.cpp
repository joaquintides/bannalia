/* Optimized lexicographical comparison for binary search algorithms.
 *
 * Copyright 2014 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */

#include <algorithm>
#include <iostream>
#include <math.h>
#include <string>
#include <vector>

template<typename Comp,typename Value>
struct comp_binder
{
  comp_binder(const Comp& cmp,const Value& x):cmp(cmp),x(x){}

  bool less(const Value& y){return cmp(y,x);}
  bool greater(const Value& y){return cmp(x,y);}

  const Comp&  cmp;
  const Value& x;
};

template<typename Comp,typename Value>
comp_binder<Comp,Value> comp_bind(const Comp& cmp,const Value& x)
{
  return comp_binder<Comp,Value>(cmp,x);
}

struct instrumented_less_string
{
  static unsigned int call_count,comp_count;

  static void reset()
  {
    call_count=comp_count=0;
  }

  bool operator()(const std::string& x,const std::string& y)const
  {
    ++call_count;
    std::string::size_type m=std::min(x.size(),y.size());
    for(std::string::size_type n=0;n!=m;++n){
      ++comp_count;
      if(x[n]<y[n])return true;
      if(y[n]<x[n])return false;
    }
    ++comp_count;
    return x.size()<y.size();
  }
};
unsigned int instrumented_less_string::call_count=0;
unsigned int instrumented_less_string::comp_count=0;

template<typename T>
struct instrumented_prefix_less:std::less<T>
{
  static unsigned int call_count,comp_count;

  static void reset()
  {
    call_count=comp_count=0;
  }
};
template<typename T>
unsigned int instrumented_prefix_less<T>::call_count=0;
template<typename T>
unsigned int instrumented_prefix_less<T>::comp_count=0;

template<typename T>
struct comp_binder<instrumented_prefix_less<T>,T>
{
  typedef instrumented_prefix_less<T> less_type;

  comp_binder(const less_type&,const T& x):
    x(x),x_size(x.size()),x_begin(x.begin()),pref_left(0),pref_right(0)
  {}

  bool less(const T& y)
  {
    ++less_type::call_count;
    auto n=std::min(pref_left,pref_right);
    auto m=std::min(x_size,y.size());
    auto it1=x_begin+n,it2=y.begin()+n;
    for(;n!=m&&*it1==*it2;++n,++it1,++it2){
      ++less_type::comp_count;
    }
    ++less_type::comp_count;
    return (n!=m?*it2<*it1:n<x_size)?(pref_left=n,true):(pref_right=n,false);
  }

  bool greater(const T& y)
  {
    ++less_type::call_count;
    auto n=std::min(pref_left,pref_right);
    auto m=std::min(x_size,y.size());
    auto it1=x_begin+n,it2=y.begin()+n;
    for(;n!=m&&*it1==*it2;++n,++it1,++it2){
      ++less_type::comp_count;
    }
    ++less_type::comp_count;
    return (n!=m?*it1<*it2:n<y.size())?(pref_right=n,true):(pref_left=n,false);
  }

  const T &                  x;
  typename T::size_type      x_size;
  typename T::const_iterator x_begin;
  typename T::size_type      pref_left,pref_right;
};

template<typename ForwardIterator,typename T,typename Compare>
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

template<typename Less,typename Vector>
double run_lower_bound(const Vector& v)
{
  Less::reset();
  for(const auto& x:v)::lower_bound(v.begin(),v.end(),x,Less());
  return double(Less::comp_count)/Less::call_count;
}

void calculate(char base,unsigned int nmax=2000000)
{
  std::cout<<"base: "<<int(base)<<"\n";
  std::cout<<"length;non-optimized;optimized\n";

  for(unsigned int l=1;;++l){
    std::vector<std::string> v;
    std::string str(l,'0');
    for(bool done=false;!done;){
      v.push_back(str);
      done=true;
      for(auto it=str.rbegin(),it_end=str.rend();it!=it_end;++it){
        char& c=*it;
        if(c<'0'+base-1){
          ++c;
          done=false;
          break;
        }
        else c='0';
      }
    }

    std::cout<<l<<";"<<
      run_lower_bound<instrumented_less_string>(v)<<";"<<
      run_lower_bound<instrumented_prefix_less<std::string>>(v)<<"\n";
    if(v.size()>=nmax)break;
  }
}

int main()
{
  for(char i=2;i<=10;++i)calculate(i);
}