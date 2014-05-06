/* Adaptive strategy for multitrait selection
 * with evaluation constraints.
 *
 * Copyright 2008 Joaquín M López Muñoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */

#include <boost/config.hpp>
#if defined(BOOST_MSVC)
#pragma warning(disable:4512)
#endif

#include <algorithm>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/array.hpp>
#include <boost/random/normal_distribution.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/shared_ptr.hpp>
#include <iostream>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

class candidate
{
public:
  static const int num_traits=5;

  candidate()
  {
    for(int i=0;i<num_traits;++i)traits[i]=rnd_normal();
    score=std::accumulate(traits.begin(),traits.end(),0.0);
  }

  boost::array<double,num_traits> traits;
  double                          score;

  double partial_score(int n)const
  {
    return std::accumulate(traits.begin(),traits.begin()+n,0.0);
  }

private:
  typedef boost::variate_generator<
    boost::mt19937,
    boost::normal_distribution<> >  rnd_normal_t;
  static rnd_normal_t               rnd_normal;
};

candidate::rnd_normal_t candidate::rnd_normal(
  boost::mt19937(0),boost::normal_distribution<>(0.0));

template<int num_candidates>
class strategy
{
public:
  typedef boost::array<candidate,num_candidates> candidates_t;
  typedef typename candidates_t::const_iterator  const_candidate_iterator;

  virtual ~strategy(){}
  virtual std::string name()const=0;
  virtual const_candidate_iterator select(const candidates_t& candidates)=0;
};

template<int num_candidates>
class simple_strategy:public strategy<num_candidates>
{
  typedef strategy<num_candidates> super;

public:
  typedef typename super::candidates_t             candidates_t;
  typedef typename super::const_candidate_iterator const_candidate_iterator;

  simple_strategy(int t):t(t),n(num_candidates/t){}

  virtual std::string name()const
  {
    std::stringstream ss;
    ss<<"simple_strategy("<<t<<")";
    return ss.str();
  }

  virtual const_candidate_iterator select(const candidates_t& candidates)
  {
    using namespace boost::lambda;

    return std::max_element(
       candidates.begin(),candidates.begin()+n,
       bind(&candidate::partial_score,_1,t) < 
       bind(&candidate::partial_score,_2,t));
  }

private:
  int t,n;
};

template<int num_candidates>
class two_stage_strategy:public strategy<num_candidates>
{
  typedef strategy<num_candidates> super;

public:
  typedef typename super::candidates_t             candidates_t;
  typedef typename super::const_candidate_iterator const_candidate_iterator;

  static bool maximal(int t1,int n1,int t2)
  {
    int n2=std::min(n1-1,(num_candidates-t1*n1)/(t2-t1));
    return t1*(n1+1)+(t2-t1)*n2>num_candidates;
  }

  two_stage_strategy(int t1,int n1,int t2):
  t1(t1),n1(n1),t2(t2),n2(std::min(n1-1,(num_candidates-t1*n1)/(t2-t1))){}

  virtual std::string name()const
  {
    std::stringstream ss;
    ss<<"two_stage_strategy("<<t1<<","<<n1<<","<<t2<<","<<n2<<")";
    return ss.str();
  }

  virtual const_candidate_iterator select(const candidates_t& candidates)
  {
    using namespace boost::lambda;

    boost::array<const_candidate_iterator,num_candidates> sorted_view;
    for(int t=0;t<n1;++t)sorted_view[t]=candidates.begin()+t;

    std::sort(
      sorted_view.begin(),sorted_view.begin()+n1,
      bind(&candidate::partial_score,*_1,t1) >
      bind(&candidate::partial_score,*_2,t1));


    return *std::max_element(
      sorted_view.begin(),sorted_view.begin()+n2,
      bind(&candidate::partial_score,*_1,t2) <
      bind(&candidate::partial_score,*_2,t2));
  }

private:
  int t1,n1,t2,n2;
};

template<int num_candidates>
class adaptive_strategy:public strategy<num_candidates>
{
  typedef strategy<num_candidates> super;

public:
  typedef typename super::candidates_t             candidates_t;
  typedef typename super::const_candidate_iterator const_candidate_iterator;

private:
  struct select_entry
  {
    select_entry(){}
    select_entry(const_candidate_iterator it):
      it(it),score(0.0),evaluated_traits(0){}

    const_candidate_iterator it;
    double                   score;
    int                      evaluated_traits;
  };

public:
  adaptive_strategy(){}

  virtual std::string name()const{return "adaptive_strategy";}

  virtual const_candidate_iterator select(const candidates_t& candidates)
  {
    boost::array<select_entry,num_candidates> view;
    for(int t=0;t<num_candidates;++t)view[t]=select_entry(candidates.begin()+t);

    for(int n=0;n<num_candidates;++n){
      int i=0;while(view[i].evaluated_traits==candidate::num_traits)++i;
      
      double old_score=view[i].score;
      view[i].score=view[i].it->partial_score(++view[i].evaluated_traits);
      if(view[i].score>old_score){
        int j=0;while(view[j].score>view[i].score)++j;
        std::rotate(view.begin()+j,view.begin()+i,view.begin()+(i+1));
      }
      else{
        int j=i+1;while(j<num_candidates&&view[j].score>view[i].score)++j;
        std::rotate(view.begin()+i,view.begin()+(i+1),view.begin()+j);
      }
    }
    return view[0].it;
  }
};

int main()
{
  using namespace boost::lambda;

  static const int num_candidates=60; // divisible by 1,...,candidate::num_traits
  static const int num_traits=candidate::num_traits;
  static const int N=20000;

  typedef strategy<num_candidates>             strategy_t;
  typedef strategy_t::candidates_t             candidates_t;
  typedef strategy_t::const_candidate_iterator const_candidate_iterator;
  
  std::vector<boost::shared_ptr<strategy_t> > strategies;
  strategies.push_back(boost::shared_ptr<strategy_t>(
    new simple_strategy<num_candidates>(5)));
  strategies.push_back(boost::shared_ptr<strategy_t>(
    new two_stage_strategy<num_candidates>(1,28,5)));
  strategies.push_back(boost::shared_ptr<strategy_t>(
    new adaptive_strategy<num_candidates>()));

  std::vector<boost::array<int,num_candidates> > histogram(strategies.size());
  std::vector<double>                            acc_score(strategies.size()+1);

  for(int n=N;n--;){
    typedef boost::array<candidate,num_candidates>        candidates_t;
    typedef boost::array<const candidate*,num_candidates> candidates_ptr_t;

    const candidates_t candidates;
    candidates_ptr_t   sorted_candidates;
    for(std::size_t i=candidates.size();i--;){
      sorted_candidates[i]=&candidates[i];
    }
    std::sort(
      sorted_candidates.begin(),sorted_candidates.end(),
      bind(&candidate::score,*_1) > bind(&candidate::score,*_2));

    acc_score.back()+=sorted_candidates[0]->score;

    for(std::size_t t=0;t<strategies.size();++t){
      const_candidate_iterator it=strategies[t]->select(candidates);
      acc_score[t]+=it->score;

      candidates_ptr_t::iterator itp=
        std::find_if(
          sorted_candidates.begin(),sorted_candidates.end(),
          _1==&*it);
      std::size_t choice=(std::size_t)(itp-sorted_candidates.begin());
      ++(histogram[t][choice]);
    }
  }

  for(std::size_t t=0;t<strategies.size();++t){
    std::cout<<strategies[t]->name()<<"\t"<<acc_score[t]/N<<std::endl;
    for(std::size_t i=0;i<histogram[t].size();++i){
      std::cout<<histogram[t][i]<<"\t";
    }
    std::cout<<std::endl;
  }
  std::cout<<"optimum_strategy\t"<<acc_score[strategies.size()]/N<<std::endl;
}
