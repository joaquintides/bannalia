/* Minimal permutation cover generator
 *
 * Copyright 2008 Joaquín M López Muñoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */

#include <algorithm>
#include <boost/config.hpp>
#include <boost/function_output_iterator.hpp>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

#if defined(BOOST_NO_STDC_NAMESPACE)
namespace std{
using ::exit;
}
#endif

typedef std::vector<bool> subset;
typedef std::size_t       uint;
typedef std::vector<uint> tuple;

void assign(subset& x,const tuple& t)
{
  for(uint i=x.size();i--;)x[i]=false;
  for(tuple::const_iterator it=t.begin(),it_end=t.end();it!=it_end;++it){
    x[*it]=true;
  }
}

uint upper_bound(const tuple& t)
{
  if(t.empty())return 0;
  else return *std::max_element(t.begin(),t.end())+1;
}

uint outside_union(const subset&x,const subset& y)
{
  for(uint i=x.size();i--;)if(!x[i]&&!y[i])return i;
  return 0; // unreachable
}

void disjoint(const subset& x,subset& y)
{
  const uint n=x.size();
  uint       sum=0;
  for(uint i=0;i<n;++i){
    if(x[i])++sum;
    else if(sum!=0){
      y[i]=true;
      --sum;
    }
  }
  if(sum!=0){
    for(uint i=0;sum!=0;++i){
      if(!x[i]&&!y[i]){
        y[i]=true;
        --sum;
      }
    }
  }
}

uint extend(const subset& x,uint m)
{
  const uint n=x.size();
  uint       k=m;
  for(uint i=n;i--;){
    if(x[i]){
      if(i<=2*k)break;
      else --k;
    }
  }
  subset y(x.begin(),x.begin()+2*k+1);
  subset z(y.size());
  disjoint(y,z);
  for(uint i=0;i<z.size();++i){
    if(!x[i]&&!z[i]){
      return i;
    }
  }
  return 0; //unreachable
}

bool in_extend_range(const subset& y)
{
  const uint n=y.size();
  const uint m=(n-1)/2;
  uint       sum=0;
  uint       j=0;
  for(uint i=0;i<=m;++i){
    for(;j<=2*i;++j)if(y[j])++sum;
    if(sum==i+1)return true;
  }
  return false;
}

template<typename OutputIterator>
void tuple_cover_impl(uint m,uint n,const tuple& t,OutputIterator out)
{
  if(m==n/2)*out++=t;
  else{
    subset x(n);
    assign(x,t);
    uint a=extend(x,m);
    tuple t1(t);t1.push_back(a);
    tuple_cover_impl(m+1,n,t1,out);
    for(uint i=upper_bound(t);i<n;++i){
      x[i]=true;
      if(!in_extend_range(x)){
        tuple t1(t);t1.push_back(i);
        tuple_cover_impl(m+1,n,t1,out);
      }
      x[i]=false;
    }
  }
}

template<typename OutputIterator>
void tuple_cover(uint n,OutputIterator out)
{
  tuple_cover_impl(0,n,tuple(),out);
}

template<typename OutputIterator>
void permutation_cover(uint n,OutputIterator out)
{
  typedef std::vector<tuple> tuple_vector;

  uint n1=(n%2==0)?n-1:n;
  uint m1=n1/2;
  tuple_vector tv;
  tuple_cover(n1,std::back_inserter<tuple_vector>(tv));
  for(tuple_vector::iterator it=tv.begin(),it_end=tv.end();it!=it_end;++it){
    subset x(n1);
    assign(x,*it);
    subset xd(n1);
    disjoint(x,xd);
    subset y(n1);
    for(tuple_vector::iterator it2=tv.begin();it2!=it_end;++it2){
      assign(y,*it2);
      if(xd==y){
        uint a=outside_union(x,xd);
        tuple s=*it;
        s.push_back(a);
        s.insert(s.end(),it2->rbegin(),it2->rend());
        if(n%2==0){
          s.push_back(n1);
          *out++=s;
          s.pop_back();
          s.insert(s.begin(),n1);
          *out++=s;
        }
        else{
          *out++=s;
        }
      }
    }
  }
}

struct print_tuple
{
  void operator()(const tuple& t)const
  {
    bool              first=true;
    std::cout<<"(";
    for(tuple::const_iterator it=t.begin(),it_end=t.end();it!=it_end;++it){
      std::cout<<char('a'+*it);
    }
    std::cout<<")\n";
  }
};

int main(int argc,char *argv[])
{
  int n=0;
  if(argc>=2){
    std::istringstream istr(argv[1]);
    istr>>n;
  }
  else for(;;){
    std::cout<<"N: ";
    std::string str;
    std::getline(std::cin,str);
    if(str.empty())std::exit(EXIT_SUCCESS);
    std::istringstream istr(str);
    istr>>n;
    if(n!=0)break;
  }

  typedef boost::function_output_iterator<print_tuple> tuple_printer;

  permutation_cover(n,tuple_printer());
}
