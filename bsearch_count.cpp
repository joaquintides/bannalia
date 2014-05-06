/* Complexity of lexicographical comparison in binary search.
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

void calculate(char base,unsigned int nmax=2000000)
{
  typedef instrumented_less_string less_type;

  std::cout<<"base: "<<int(base)<<"\n";

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

    less_type::reset();
    for(const auto& x:v){
      std::lower_bound(v.begin(),v.end(),x,less_type());
    }
    std::cout<<l<<";"<<
      double(less_type::comp_count)/less_type::call_count<<"\n";
    if(v.size()>=nmax)break;
  }
}

int main()
{
  for(char i=2;i<=10;++i)calculate(i);
}
