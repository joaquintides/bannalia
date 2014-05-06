/* Profiling stable_vector.
 *
 * Copyright 2008 Joaquín M López Muñoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */

#include <boost/config.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/mpl/apply.hpp>
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <deque>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "stable_vector.hpp"

#if defined(BOOST_NO_STDC_NAMESPACE)
namespace std{
using ::clock;
using ::clock_t;
using ::exit;
}
#endif

class timer
{
public:
  timer(){restart();}

  void restart(){t=std::clock();}

  void time(const char* str,int n)
  {
    std::cout<<str<<": "
             <<(double)(std::clock()-t)/CLOCKS_PER_SEC*1000000/n
             <<" us/op\n";
  }

private:
  std::clock_t t;
};

struct bigobj
{
  bigobj(int i):i(i),str(boost::lexical_cast<std::string>(i)){}

  operator int()const{return i;}

  int         i;
  std::string str;
};

template<typename Container>
void subtest(int n)
{
  typedef typename Container::value_type value_type;

  Container c;

  timer t;
  for(int i=0;i<n;++i)c.push_back(value_type(i));
  t.time("  push_back",n);

  t.restart();
  for(int i=0;i<n/10000;++i)c.insert(c.begin()+c.size()/2,value_type(i));
  t.time("  insert",n/10000);

  t.restart();
  int s=0;
  for(int j=0;j<100;++j){
    for(int i=0;i<n;++i)s+=c[i];
  }
  t.time("  operator[]",100*n);
};

template<typename ContainerSpecifier>
void test()
{
  const int N=10000000;

  std::cout<<"int"<<std::endl;
  subtest<typename boost::mpl::apply1<ContainerSpecifier,int>::type>(N);

  std::cout<<"bigobj"<<std::endl;
  subtest<typename boost::mpl::apply1<ContainerSpecifier,bigobj>::type>(N/10);
}

struct test_case
{
  const char* name;
  void (*test)();
};

test_case test_table[]=
{
  {
    "std::vector",
    &test<std::vector<boost::mpl::_> >
  },
  {
    "std::deque",
    &test<std::deque<boost::mpl::_> >
  },
  {
    "stable_vector",
    &test<stable_vector<boost::mpl::_> >
  }
};

const int num_test_cases=sizeof(test_table)/sizeof(test_case);

int main()
{
  try{
    for(int i=0;i<num_test_cases;++i){
      std::cout<<i+1<<". "<<test_table[i].name<<"\n";
    }
    int option=-1;
    for(;;){
      std::cout<<"select option, enter to exit: ";
      std::string str;
      std::getline(std::cin,str);
      if(str.empty())std::exit(EXIT_SUCCESS);
      std::istringstream istr(str);
      istr>>option;
      if(option>=1&&option<=num_test_cases){
        --option; /* pass from 1-based menu to 0-based test_table */
        break;
      }
    }

    test_table[option].test();
  }
  catch(const std::exception& e){
    std::cout<<"error: "<<e.what()<<"\n";
  }

  return 0;
}

