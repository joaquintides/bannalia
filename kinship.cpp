/* Kinship distribution estimator.
 *
 * Copyright 2008 Joaquín M López Muñoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */

#include <boost/config.hpp>
#if defined(BOOST_MSVC)
#pragma warning(disable:4996)
#endif

#include <algorithm>
#include <boost/array.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/poisson_distribution.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/uniform_smallint.hpp>
#include <boost/random/variate_generator.hpp>
#include <iomanip>
#include <iostream>
#include <vector>
#include <sstream>
#include <string>

struct individual
{
  enum sex_t{female,male};

  individual(sex_t sex_,int mother_,int father_)
  {
    pop.push_back(impl_t(sex_,mother_,father_));
    impl=(int)pop.size();
  }

  individual(sex_t sex_,const individual& mother_,const individual& father_)
  {
    pop.push_back(impl_t(sex_,mother_.impl,father_.impl));
    impl=(int)pop.size();
  }

  individual(int i):impl(i){}

  sex_t      sex()const{return pop[impl].sex_;}
  individual mother()const{return individual(pop[impl].mother_);}
  individual father()const{return individual(pop[impl].father_);}
  bool       mated()const{return pop[impl].mated_;}

  void mate(){pop[impl].mated_=true;}

  friend bool operator==(const individual& x,const individual& y)
  {
    return x.impl==y.impl;
  }

  friend bool operator!=(const individual& x,const individual& y)
  {
    return !(x==y);
  }

  friend bool siblings(const individual& x,const individual& y)
  {
    return x!=y&&pop[x.impl].mother_==pop[y.impl].mother_;
  }

  friend int bounded_kinship(const individual& x,const individual& y,int stop)
  {
    if(stop==0)return 0;
    if(x==y)return 0;
    if(stop==2)return 2;
    if(pop[x.impl].mother_<0)return 2;
    if(pop[x.impl].mother_==pop[y.impl].mother_)return 2;
    
    if(stop==4)return 4;
    int a=bounded_kinship(x.mother(),y.mother(),stop-2);
    int b=bounded_kinship(x.mother(),y.father(),a);
    int c=bounded_kinship(x.father(),y.mother(),b);
    int d=bounded_kinship(x.father(),y.father(),c);
    return 2+d;
  }

  friend int kinship(const individual& x,const individual& y)
  {
    static int stop=0;
    static boost::array<int,10> prev;

    int res=0;
    stop=*std::max_element(prev.begin(),prev.end());
    do{
      stop+=4;
      res=bounded_kinship(x,y,stop);
    }while(res==stop);
    std::copy(prev.begin()+1,prev.end(),prev.begin());
    prev.back()=res;
    return res;
  }

  struct impl_t
  {
    impl_t(sex_t sex_,int mother_,int father_):
      sex_(sex_),mother_(mother_),father_(father_),mated_(false){}

    sex_t sex_;
    int   mother_,father_;
    bool  mated_;
  };

  static std::vector<impl_t> pop;

  int impl;
};

std::vector<individual::impl_t> individual::pop;

template<typename T>
static T get_arg(int argc,char** argv,int position,const char* msg,const T& default_v)
{
  std::string arg_str;
  if(argc>position+1)arg_str=argv[position+1];
  else{
    std::cout<<msg<<" (enter: "<<default_v<<") ";
    std::getline(std::cin,arg_str);
  }
  T x=default_v;
  std::stringstream iss(arg_str);
  iss>>x;

  return x;
}

int main(int argc,char** argv)
{
  const int N=      get_arg(argc,argv,0,"population size",1000);
  const int NUM_GEN=get_arg(argc,argv,1,"num of generations",30);

  boost::mt19937                    rnd_gen;
  boost::uniform_smallint<>         rnd_bool(0,1);
  typedef boost::variate_generator<
    boost::mt19937,
    boost::poisson_distribution<> > rnd_poisson_t;
  rnd_poisson_t                     rnd_poisson(
                                      rnd_gen,
                                      boost::poisson_distribution<>(2.0));

  /* initial generation */

  std::cout<<N*(NUM_GEN+2)<<std::endl;
  individual::pop.reserve(N*(NUM_GEN+2));

  for(int i=0;i<N;++i){
    individual x(
      rnd_bool(rnd_gen)==0?individual::female:individual::male,-i-1,-i-1);
  }

  /* loop */

  int begin_gen=0,end_gen=N;

  for(int i=0;i<NUM_GEN;++i){
    std::cout<<"generation "<<i+1<<std::endl;

    rnd_poisson=rnd_poisson_t(
      rnd_gen,
      boost::poisson_distribution<>(2.0*N/(end_gen-begin_gen)));
    int begin_male_search=begin_gen;

    for(int n=begin_gen;n<end_gen;++n){
      individual x(n);
      if(x.sex()==individual::male)continue;
      int num_children=rnd_poisson();
      if(num_children==0)continue;

      bool males_left_behind=false;
      for(int m=begin_male_search;m<end_gen;++m){
        individual y(m);
        if(y.sex()==individual::female||y.mated())continue;
        if(siblings(x,y)){
          males_left_behind=true;
          continue;
        }

        y.mate();
        for(int nc=0;nc<num_children;++nc){
          individual c(
            rnd_bool(rnd_gen)==0?individual::female:individual::male,x,y);
        }
        if(!males_left_behind)begin_male_search=m+1;
        break;
      }
    }
    begin_gen=end_gen;
    end_gen=(int)individual::pop.size();
    std::random_shuffle(
      individual::pop.begin()+begin_gen,individual::pop.begin()+end_gen);
  }

  /* kinship histogram */

  static const int     N_SAMPLES=10000;
  boost::uniform_int<> rnd_ind(begin_gen,end_gen-1);
  std::vector<int>     sum;

  std::cout<<"estimating histogram... ("<<
    N_SAMPLES<<" samples, *=1000 samples)"<<std::endl;

  for(int i=0;i<N_SAMPLES;++i){
    int k=kinship(individual(rnd_ind(rnd_gen)),individual(rnd_ind(rnd_gen)))/2;
    if(k>=(int)sum.size())sum.resize(k+1);
    ++sum[k];
    if(i%1000==0&&i!=0)std::cout<<"*";
  }

  std::cout<<"\nkinship histogram:"<<std::endl;
  for(int k=0;k<(int)sum.size();++k){
    std::cout<<"k="<<2*k<<": "<<
      std::setprecision(3)<<100.0*sum[k]/N_SAMPLES<<"%"<<std::endl;
  }
}
