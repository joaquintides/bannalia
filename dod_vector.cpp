/* SOA container (sketch) for encapsulated Data-Oriented Design.
 *
 * Copyright 2015 Joaquin M Lopez Munoz.
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
  volatile decltype(f())        res; /* to avoid optimizing f() away */
        
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
 
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
#include <boost/iterator/iterator_facade.hpp>
 
namespace dod{
     
template<typename T,int Tag=0>
struct member
{
  using type=T;
  static const int tag=Tag;
};

template<typename... Members>class access;
 
template<>
class access<>
{
  struct innaccessible{};
 
protected:
  std::ptrdiff_t off;

public:
  access():off(0){};
  access(const access& a,std::ptrdiff_t n):off(a.off+n){}

  void get(innaccessible);
};
 
template<typename Member0,typename... Members>
class access<Member0,Members...>:
  public access<Members...>
{
  using super=access<Members...>;
  using type=typename Member0::type;
 
  type* p;
 
public:
  template<typename... Args>
  access(type* p,Args&&... args):super(std::forward<Args>(args)...),p(p){}
  access(const access& a,std::ptrdiff_t n):super(a,n),p(a.p){}
 
  using super::get;
 
  type&       get(Member0){return p[off];}
  const type& get(Member0)const{return p[off];}
 
protected:
  using super::off;
 
private:
  template<typename> friend class pointer;
 
  bool equal(const access& x)const{return p+off==x.p+x.off;}
  void increment(){++off;}
  void decrement(){--off;}
  void advance(std::ptrdiff_t n){off+=n;}
  std::ptrdiff_t distance_to(const access& x)const{return (p+off)-(x.p+x.off);}
};

template<typename T> class pointer;
 
template<template <typename> class Class,typename Access>
class pointer<Class<Access>>:
  public boost::iterator_facade<
    pointer<Class<Access>>,
    Class<Access>,
    boost::random_access_traversal_tag,
    Class<Access>
  >
{
public:
  pointer(const Access& a):a(a){}
   
  Class<Access> operator[](std::ptrdiff_t n)const
  {
    return Class<Access>(Access(a,n));
  }
 
private:
  friend class boost::iterator_core_access;
   
  Class<Access> dereference()const{return Class<Access>(a);}
  bool equal(const pointer& x)const{return a.equal(x.a);}
  void increment(){a.increment();}
  void decrement(){a.decrement();}
  void advance(std::ptrdiff_t n){a.advance(n);}
  std::ptrdiff_t distance_to(const pointer& x)const{return a.distance_to(x.a);}
 
  Access a;
};
 
template<template <typename> class Class,typename Access>
pointer<Class<Access>> make_pointer(const Access& a)
{
  return pointer<Class<Access>>(a);
}

template<typename Access>
class vector_base;

template<>
class vector_base<access<>>
{
protected:
  access<> data(){return {};}
  void emplace_back(){}
};

template<typename Member0,typename... Members>
class vector_base<access<Member0,Members...>>:
  protected vector_base<access<Members...>>
{
  using super=vector_base<access<Members...>>;
  using type=typename Member0::type;
  using impl=std::vector<type>;
  using size_type=typename impl::size_type;
  impl v;
  
protected:
  access<Member0,Members...> data(){return {v.data(),super::data()};}
  size_type size()const{return v.size();}

  template<typename Arg0,typename... Args>
  void emplace_back(Arg0&& arg0,Args&&... args){
    v.emplace_back(std::forward<Arg0>(arg0));
    try{
      super::emplace_back(std::forward<Args>(args)...);
    }
    catch(...){
      v.pop_back();
      throw;
    }
  }
};
  
template<typename T> class vector;
 
template<template <typename> class Class,typename Access> 
class vector<Class<Access>>:protected vector_base<Access>
{
  using super=vector_base<Access>;
  
public:
  using iterator=pointer<Class<Access>>;
  
  iterator begin(){return super::data();}
  iterator end(){return this->begin()+super::size();}
  using super::emplace_back;
};

} // namespace dod
 
#include <iostream>
#include <vector>
 
using namespace dod;
 
static int render_output=0;
 
void do_render(int x,int y,char c)
{
  render_output+=x+y+c;
}
 
class plain_particle
{  
  char  color;
  int   x;
  int   y;
  int   dx;
  int   dy;
public:
 
  static const int max_x=200;
  static const int max_y=100;
     
  plain_particle(char color_,int x_,int y_,int dx_,int dy_):
    color(color_),x(x_),y(y_),dx(dx_),dy(dy_)
  {}
 
  void render()const
  {
    do_render(x,y,color);
  }
 
  void move()
  {
    x+=dx;
    if(x<0){
      x*=-1;
      dx*=-1;
    }
    else if(x>max_x){
      x=2*max_x-x;
      dx*=-1;      
    }
     
    y+=dy;
    if(y<0){
      y*=-1;
      dx*=-1;
    }
    else if(y>max_y){
      y=2*max_y-y;
      dy*=-1;      
    }
  }
};
 
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
    do_render(get(x()),get(y()),get(color()));
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
 
template<typename Iterator>
int render(Iterator first,Iterator last)
{
  render_output=0;
  while(first!=last){
    first->render();
    ++first;
  }
  return render_output;
}

template<typename F>
double measure(F f,std::size_t n){return (measure(f)/n)*10E6;}
 
int main()
{
  using color=member<char,0>;
  using x=member<int,0>;
  using y=member<int,1>;
  using dx=member<int,2>;
  using dy=member<int,3>;
 
  std::size_t n0=10000,n1=10000000,fn=10;
    
  std::cout<<"render:"<<std::endl;
  std::cout<<"n;oop;dod"<<std::endl;
    
  for(std::size_t n=n0;n<=n1;n*=fn){
    using access=dod::access<color,x,y,dx,dy>;

    std::vector<plain_particle>   pp_;
    dod::vector<particle<access>> p_;
 
    for(std::size_t i=0;i<n;++i){
      char carg=i%5;
      int  xarg=i,yarg=2*i,dxarg=i%20,dyarg=i%10;
      pp_.push_back(plain_particle(carg,xarg,yarg,dxarg,dyarg));
      p_.emplace_back(carg,xarg,yarg,dxarg,dyarg);
    }

    std::cout<<n<<";";
    std::cout<<measure([&](){return render(pp_.begin(),pp_.end());},n)<<";";
    std::cout<<measure([&](){return render(p_.begin(),p_.end());},n)<<"\n";
  }
}
