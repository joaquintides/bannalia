/* C++ encapsulation for Data-Oriented Design.
 *
 * Copyright 2015 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */

#include <tuple>
#include <utility>

namespace dod{
    
template<typename T,int Tag=0>
struct member
{
  using type=T;
  static const int tag=Tag;
};

template<typename... Members>class access;

template<typename Member>
class access<Member>
{
  using type=typename Member::type;
  type* p;

public:
  access(type* p):p(p){}
  
  type&       get(Member){return *p;}
  const type& get(Member)const{return *p;}
};

template<typename Member0,typename... Members>
class access<Member0,Members...>:
  public access<Member0>,access<Members...>
{
public:
  template<typename Arg0,typename... Args>
  access(Arg0&& arg0,Args&&... args):
    access<Member0>(std::forward<Arg0>(arg0)),
    access<Members...>(std::forward<Args>(args)...)
  {}
  
  using access<Member0>::get;
  using access<Members...>::get;
};

template<typename Tuple,std::size_t Index,typename... Members>
class tuple_storage_base;

template<typename Tuple,std::size_t Index>
class tuple_storage_base<Tuple,Index>:public Tuple
{
  struct inaccessible{};
public:
  using Tuple::Tuple;
  
  void get(inaccessible);
  
  Tuple&       tuple(){return *this;}
  const Tuple& tuple()const{return *this;}
};

template<
  typename Tuple,std::size_t Index,
  typename Member0,typename... Members
>
class tuple_storage_base<Tuple,Index,Member0,Members...>:
  public tuple_storage_base<Tuple,Index+1,Members...>
{
  using super=tuple_storage_base<Tuple,Index+1,Members...>;
  using type=typename Member0::type;

public:
  using super::super;
  using super::get;
  
  type&       get(Member0)
                {return std::get<Index>(this->tuple());}
  const type& get(Member0)const
                {return std::get<Index>(this->tuple());}  
};

template<typename... Members>
class tuple_storage:
  public tuple_storage_base<
    std::tuple<typename Members::type...>,0,Members...
  >
{
  using super=tuple_storage_base<
    std::tuple<typename Members::type...>,0,Members...
  >;
  
public:
  using super::super;
};
    
} // namespace dod

#include <algorithm>
#include <iostream>

using namespace dod;

template<typename Access>
class particle:Access
{
  using Access::get;
  
  using color=member<char,0>;
  using x=member<int,0>;
  using y=member<int,1>;
  using dx=member<int,2>;
  using dy=member<int,3>;

public:

  static const int max_x=200;
  static const int max_y=100;

  particle(const Access& a):Access(a){}

  void render()const
  {
    std::cout<<"["<<get(x())<<","
      <<get(y())<<","<<int(get(color()))<<"]\n";
  }

  void move()
  {
    get(x())+=get(dx());
    if(get(x())<0){
      get(x())*=-1;
      get(dx())*=-1;
    }
    else if(get(x())>max_x){
      get(x())=2*max_x-get(x());
      get(dx())*=-1;      
    }
    
    get(y())+=get(dy());
    if(get(y())<0){
      get(y())*=-1;
      get(dy())*=-1;
    }
    else if(get(y())>max_y){
      get(y())=2*max_y-get(y());
      get(dy())*=-1;      
    }
  }
};

template<typename Access>
particle<Access> make_particle(Access&& a)
{
  return particle<Access>(std::forward<Access>(a));
}

int main()
{
  using color=member<char,0>;
  using x=member<int,0>;
  using y=member<int,1>;
  using dx=member<int,2>;
  using dy=member<int,3>;

  char color_=5;
  int  x_=20,y_=40,dx_=2,dy_=-1;
  
  auto p=make_particle(access<color,x,y>(&color_,&x_,&y_));
  auto q=make_particle(access<x,y,dx,dy>(&x_,&y_,&dx_,&dy_));
  p.render();
  q.move();
  p.render();

  using storage=tuple_storage<color,x,y,dx,dy>;
  auto r=make_particle(storage(3,100,10,10,-15));
  auto s=r;
  r.render();
  r.move();
  r.render();
  s.render(); // different data than r
}
