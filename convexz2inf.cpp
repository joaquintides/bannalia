/* Wavefront hull calculator for Z^2 with L-inf metric.
 *
 * Copyright 2008 Joaquín M López Muñoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */

#include <boost/multi_array.hpp>
#include <boost/tuple/tuple.hpp>
#include <climits>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>

struct point
{
  point(int x,int y):x(x),y(y){}
  int x,y;
};

struct rhomb
{
  rhomb(int pp,int pm,int mp,int mm):pp(pp),pm(pm),mp(mp),mm(mm){}
  int pp,pm,mp,mm; /* directions (+1,+1),(+1,-1),(-1,+1),(-1,-1) */
};

template<typename Sequence>
std::pair<point,rhomb> read_points(std::istream& is,Sequence& s)
{
  rhomb rh(INT_MIN,INT_MIN,INT_MIN,INT_MIN);
  int   xmax=0;
  s.clear();
  int y=0;
  for(std::string str;std::getline(is,str);++y){
    int line_xmax=0;
    for(int x=0;(x=str.find_first_of('*',x))!=std::string::npos;++x){
      s.push_back(point(x,y));
      line_xmax=x;
      if( x+y>rh.pp)rh.pp= x+y;
      if( x-y>rh.pm)rh.pm= x-y;
      if(-x+y>rh.mp)rh.mp=-x+y;
      if(-x-y>rh.mm)rh.mm=-x-y;
    }
    if(line_xmax>xmax)xmax=line_xmax;
  }
  return std::make_pair(point(xmax+1,y),rh);
}

inline int distZ2inf(const point& p0,const point& p1)
{
  int a=std::abs(p1.x-p0.x);
  int b=std::abs(p1.y-p0.y);
  return a>=b?a:b;
}

template<typename ForwardIterator>
int distZ2inf(const point& p,ForwardIterator first,ForwardIterator last)
{
  int res=INT_MAX;
  while(first!=last)
  {
    int d=distZ2inf(p,*first++);
    if(d<res)res=d;
  }
  return res;
}

template<typename MultiArray,typename Sequence>
void mark(
  MultiArray& m,const Sequence& pts,
  const point& p,int dx,int dy)
{
  for(int k=p.x<0||p.y<0||p.x>=m.shape()[0]||p.y>=m.shape()[1]?1:0,
          k_end=distZ2inf(p,pts.begin(),pts.end());k<k_end;++k){
    int x=p.x-k*dx;
    int y=p.y-k*dy;
    if(x>=0&&y>=0&&x<m.shape()[0]&&y<m.shape()[1])m[x][y]=true;
  }
}

int main()
{
  std::vector<point>         pts;
  point                      dim(0,0);
  rhomb                      rh(0,0,0,0);
  boost::tuples::tie(dim,rh)=read_points(std::cin,pts);
  boost::multi_array<bool,2> m(boost::extents[dim.x][dim.y]);

  for(point pp(rh.pp-dim.y,dim.y);pp.x<dim.x;){
    mark(m,pts,pp,1,1);
    ++pp.x;
    mark(m,pts,pp,1,1);
    --pp.y;
  }
  mark(m,pts,point(dim.x,rh.pp-dim.x),1,1);

  for(point pm(rh.pm-1,-1);pm.x<dim.x;){
    mark(m,pts,pm,1,-1);
    ++pm.x;
    mark(m,pts,pm,1,-1);
    ++pm.y;
  }
  mark(m,pts,point(dim.x,dim.x-rh.pm),1,-1);

  for(point mp(-1,rh.mp-1);mp.y<dim.y;){
    mark(m,pts,mp,-1,1);
    ++mp.y;
    mark(m,pts,mp,-1,1);
    ++mp.x;
  }
  mark(m,pts,point(dim.y-rh.mp,dim.y),-1,1);

  for(point mm(-1,1-rh.mm);mm.y>=0;){
    mark(m,pts,mm,-1,-1);
    --mm.y;
    mark(m,pts,mm,-1,-1);
    ++mm.x;
  }
  mark(m,pts,point(-rh.mm,0),-1,-1);

  for(int y=0;y<dim.y;++y){
    for(int x=0;x<dim.x;++x){
      if(x+y>rh.pp||x-y>rh.pm||-x+y>rh.mp||-x-y>rh.mm||m[x][y])std::cout<<".";
      else       std::cout<<"*";
    }
    std::cout<<"\n";
  }
}
