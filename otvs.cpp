/* Optimum TV scheduling solver.
 *
 * Copyright 2007 Joaquín M López Muñoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */

#include <cmath>
#include <iterator>
#include <iostream>
#include <fstream>
#include <list>
#include <vector>
#include <string>
#include <sstream>

template<typename Float>
struct otvs_entry
{
  otvs_entry(Float total_audience,Float competence):
    total_audience(total_audience),
    competence(competence),
    profitability(total_audience/competence),
    schedule()
  {}

  Float total_audience;
  Float competence;
  Float profitability;
  Float schedule;
};

template<typename Float>
struct greater_profitability_otvs_entry_ptr
{
  bool operator()(const otvs_entry<Float>* x,const otvs_entry<Float>* y)const
  {
    return x->profitability>y->profitability;
  }
};

template<
  typename InputIterator1,
  typename InputIterator2,
  typename Float,
  typename OutputIterator
>
void optimum_tv_scheduling(
  InputIterator1 total_audience,      /* total audience */
  InputIterator1 total_audience_last,
  InputIterator2 competence,          /* competence investment */
  Float            investment,
  OutputIterator   schedule)          /* resulting schedule */
{
  using std::sqrt;

  typedef std::list<otvs_entry<Float> >   slots_type;
  typedef std::vector<otvs_entry<Float>*> sorted_slots_type;

  slots_type        slots;
  sorted_slots_type sorted_slots;

  /* store the slots and sort them by profitability */

  for(;total_audience!=total_audience_last;++total_audience,++competence)
  {
    slots.push_back(otvs_entry<Float>(*total_audience,*competence));
    sorted_slots.push_back(&slots.back());
  }
  std::sort(
    sorted_slots.begin(),sorted_slots.end(),
    greater_profitability_otvs_entry_ptr<Float>());

  /* calculate numerator and denominator of k until the cutoff slot */

  Float                                knum=investment;
  Float                                kden=Float();
  typename sorted_slots_type::iterator it_cutoff=sorted_slots.begin();
  for(typename sorted_slots_type::iterator it_end=sorted_slots.end();
      it_cutoff!=it_end;++it_cutoff){
    Float num=(*it_cutoff)->competence;
    Float den=sqrt((*it_cutoff)->total_audience*(*it_cutoff)->competence);
    if((knum+num)/(kden+den)<=num/den)break;
    knum+=num;
    kden+=den;
  }

  /* calculate scheduling */

  Float k=knum/kden;
  for(typename sorted_slots_type::iterator it=sorted_slots.begin();
      it!=it_cutoff;++it){
    (*it)->schedule=
      k*sqrt((*it)->total_audience*(*it)->competence)-(*it)->competence;
  }

  /* output */

  for(typename slots_type::iterator it=slots.begin(),it_end=slots.end();
      it!=it_end;++it){
    *schedule++=it->schedule;
  }
}

int main(int argc,char **argv)
{
  std::string in;
  if(argc>1)in=argv[1];
  else{
    std::cout<<"input file: ";
    std::getline(std::cin,in);
  }

  std::ifstream ifs(in.c_str());
  if(!ifs){
    std::cout<<"can't open "<<in<<"\n";
    return 1;
  }
  
  std::vector<double> total_audience,competence;
  while(ifs){
    std::string line;
    std::getline (ifs,line);
    std::stringstream iss(line);
    double t=0.0,c=0.0;
    iss>>t>>c;
    if(!iss.fail()){
      total_audience.push_back(t);
      competence.push_back(c);
    }
  }

  double investment=0.0;
  {
    std::string arg;
    if(argc>2)arg=argv[2];
    else{
      std::cout<<"investment: ";
      std::getline(std::cin,arg);
    }
    std::stringstream iss(arg);
    iss>>investment;
  }

  optimum_tv_scheduling(
    total_audience.begin(),total_audience.end(),competence.begin(),investment,
    std::ostream_iterator<double>(std::cout,"\n"));

  return 0;
}
