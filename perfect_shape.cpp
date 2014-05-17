/* Genetic algorithm to calculate the shape of the glass that keeps
 * the beer coolest in average while drinking.
 *
 * Copyright 2014 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */

#include <algorithm>
#include <array>
#include <boost/units/absolute.hpp>
#include <boost/units/cmath.hpp>
#include <boost/units/io.hpp>
#include <boost/units/systems/si.hpp>
#include <boost/units/systems/temperature/celsius.hpp>
#include <random>
#include <vector>
#include <iosfwd>

using namespace boost::units;
using namespace boost::units::si;
using boost::units::celsius::degree;
using boost::units::celsius::degrees;

const auto centimeter=        1.E-2 * meters;
const auto centimeters=       centimeter;
const auto cubic_centimeter=  1.E-6 * cubic_meter; 
const auto cubic_centimeters= cubic_centimeter; 

const auto pi=          3.1415926536;
const auto cv=          4.1796 * joules/cubic_centimeter/kelvin;
const auto h_closed=    29.8 * watts/square_metre/kelvin;
const auto h_open=      33.3 * watts/square_metre/kelvin;
const auto volume_init= 1000.0 * cubic_centimeters;
const auto phi=         volume_init/(180.0 * seconds);
const auto temp_env=    40.0 * absolute<celsius::temperature>();
const auto temp_init=   5.0 * absolute<celsius::temperature>();

class profile
{
public:
  static const quantity<length> delta_x;
  static const int num_segments=100;

private:
  typedef std::array<quantity<length>,num_segments+1> rep_type;

public:
  typedef rep_type::const_iterator iterator;
  typedef rep_type::const_iterator const_iterator;

  profile(
    quantity<length> y_begin,double slope_begin,
    quantity<length> y_end,double slope_end)
  {
    /* http://www3.nd.edu/~jjwteach/441/PdfNotes/lecture9.pdf */

    auto yy_begin=slope_begin*double(num_segments)*delta_x,
         yy_end=slope_end*double(num_segments)*delta_x;
    for(int n=0;n<=num_segments;++n){
      double x=double(n)/double(num_segments),
             x2=x*x,
             x3=x2*x,
             alpha_0=  2.0*x3 -3.0*x2        +1,
             alpha_1= -2.0*x3 +3.0*x2,
             beta_0=       x3 -2.0*x2   +x,
             beta_1=       x3     -x2;
      auto y=
        y_begin*alpha_0+y_end*alpha_1+
        yy_begin*beta_0+yy_end*beta_1;
      if(y<0.0 * meters)y=0.0 * meters;
      f[n]=y;
    }
    init();
  }

  template<typename InputIterator>
  profile(InputIterator first)
  {
    for(auto& y:f)y=*first++;
    init();
  }

  const_iterator begin()const{return f.begin();}
  const_iterator end()const{return f.end();}

  quantity<length> operator[](int n)const{return f[n];}

  quantity<area> area_open()const{return area_open_at(num_segments);}

  quantity<area> area_open_at(int n)const
  {
    return pi*f[n]*f[n];
  }

  quantity<area> area_closed()const{return area_closed_;}

  quantity<area> area_closed_at(int n)const
  {
    quantity<area> a;
    for(int m=0;m<n;++m)a+=delta_area_closed(m);
    return a;
  }

  quantity<volume> volume_at(int n)const
  {
    quantity<volume> v;
    for(int m=0;m<n;++m)v+=delta_volume(m);
    return v;
  }

  quantity<absolute<celsius::temperature>> avg_temp()const{return avg_temp_;}

private:
  void init()
  {
    normalize();
    area_closed_=total_area_closed();
    avg_temp_=calc_avg_temp();
  }

  void normalize()
  {
    double k=sqrt(
      quantity<dimensionless>(volume_init/volume_at(num_segments)));
    for(int n=0;n<=num_segments;++n)f[n]*=k;
  }

  quantity<volume> delta_volume(int n)const
  {
    /* http://mathworld.wolfram.com/ConicalFrustum.html */

    auto y0=f[n],y1=f[n+1];
    return pi*delta_x/3.0*(y0*(y0+y1)+y1*y1);
  }

  quantity<area> delta_area_closed(int n)const
  {
    /* http://mathworld.wolfram.com/ConicalFrustum.html */

    quantity<area> a;
    auto y0=f[n],y1=f[n+1];
    if(n==0)a=pi*y0*y0; /* add the glass base */
    return a+pi*(y0+y1)*sqrt((y1-y0)*(y1-y0)+delta_x*delta_x);
  }

  quantity<area> total_area_closed()const{return area_closed_at(num_segments);}

  quantity<absolute<celsius::temperature>> calc_avg_temp()const
  {
    auto sum=0.0 * degrees*cubic_meters;
    auto v=volume_init;
    auto temp=temp_init;
    auto a_closed=area_closed_;
    for(int n=num_segments;n--;){
      auto delta_v=delta_volume(n);
      quantity<celsius::temperature> delta_temp=
        (1.0/cv/phi)*
        (h_closed*a_closed+h_open*area_open_at(n+1))*
        (temp_env-temp)/v*delta_v;
      sum+=(temp+delta_temp/2.0-temp_env)*delta_v;
      temp+=delta_temp;
      v-=delta_v;
      a_closed-=delta_area_closed(n);
    }
    return temp_env+quantity<celsius::temperature>(sum/volume_init);
  }
  
  rep_type                                 f;
  quantity<area>                           area_closed_;
  quantity<absolute<celsius::temperature>> avg_temp_;
};

const quantity<length> profile::delta_x= 0.5 * centimeters;

bool operator<(const profile& a,const profile& b)
{
  return a.avg_temp()<b.avg_temp();
}

void profile_csv(std::ostream& os,const profile& p)
{
  for(const auto& y:p){
    os<<quantity<dimensionless>(y/(1.0*centimeter)).value()<<"\n";
  }
}

void revolution_dat(std::ostream& os,const profile& p)
{
  static const int num_sectors=20;

  for(int x=0;x<=profile::num_segments;++x){
    auto z=quantity<dimensionless>(
      double(x)*profile::delta_x/(1.0*centimeter)).value();
    auto r=quantity<dimensionless>(p[x]/(1.0*centimeter)).value();
    for(int s=0;s<=num_sectors;++s){
      double theta=2.0*pi*s/num_sectors;
      os<<theta<<"\t"<<z<<"\t"<<r<<"\n";
    }
    os<<"\n";
  }
}

static const double mutation_rate=       0.01;
static const double min_mutating_factor= 0.0;
static const double max_mutating_factor= 2.0;

template<typename URNG>
profile breed(const profile& a,const profile& b,URNG& g)
{
  std::array<
    quantity<length>,
    profile::num_segments+1>       c;
  std::uniform_int_distribution<>  cut(0,profile::num_segments+1);
  std::uniform_real_distribution<> mut;
  std::uniform_real_distribution<> change(
                                     min_mutating_factor,max_mutating_factor);
  int cut1=cut(g),cut2=cut(g);
  if(cut1>cut2)std::swap(cut1,cut2);

  std::transform(a.begin(),a.begin()+cut1,c.begin(),
    [&](quantity<length> y){
      if(mut(g)<mutation_rate)y*=change(g);
      return y;
    }
  );

  int n=0,dc=cut2-cut1;
  std::transform(
    a.begin()+cut1,a.begin()+cut2,
    b.begin()+cut1,c.begin()+cut1,
    [&](quantity<length> y1,quantity<length> y2){
      auto y=y1*double(dc-n)/double(dc)+y2*double(n)/double(dc);
      if(mut(g)<mutation_rate)y*=change(g);
      ++n;
      return y;
    }
  );

  std::transform(b.begin()+cut2,b.end(),c.begin()+cut2,
    [&](quantity<length> y){
      if(mut(g)<mutation_rate)y*=change(g);
      return y;
    }
  );

  return profile(c.begin());
}

#include <iostream>
#include <fstream>

static const int pool_size=          40000;
static const int num_gens=           1000;
static const int surviving_fraction= 4;

int main()
{
  std::mt19937                     rnd(1);
  std::vector<profile>             pool;
  std::uniform_real_distribution<> y(0.0,10.0);
  std::uniform_real_distribution<> slope(-1.0,1.0);
  for(int n=pool_size;n--;){
    pool.push_back(
      profile(
        quantity<length>(y(rnd) * centimeters),slope(rnd),
        quantity<length>(y(rnd) * centimeters),slope(rnd)));
  }

  std::uniform_int_distribution<> surviving(0,pool.size()/surviving_fraction);
  for(int n=0;n<num_gens;++n){
    std::sort(pool.begin(),pool.end());
    std::cout<<pool[0].area_closed()<<";"<<pool[0].avg_temp()<<"\n";
    for(std::size_t m=pool.size()/surviving_fraction+1,m_end=pool.size();
        m<m_end;++m){
      pool[m]=breed(pool[surviving(rnd)],pool[surviving(rnd)],rnd);
    }
  }

  std::ofstream ofs1("profile.csv",std::ios::trunc);
  profile_csv(ofs1,pool[0]);
  std::ofstream ofs2("glass.dat",std::ios::trunc);
  revolution_dat(ofs2,pool[0]);
}
