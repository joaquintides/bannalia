#include <boost/mpl/if.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/plus.hpp>
#include <cstddef>
#include <iostream>

template<std::size_t Id,std::size_t T>
struct var
{
  typedef typename var<Id,T-1>::type type;
};

template<typename T> struct unparens_helper;
template<typename T> struct unparens_helper<void(T)>{typedef T type;};
#define UNPARENTHESIZE(f) unparens_helper<void f>::type

#define VAR(Id) var<Id,__COUNTER__>::type
#define LET(Id,x) template<> struct var<Id,__COUNTER__>:UNPARENTHESIZE(x){};

LET(0,(boost::mpl::int_<2>))
LET(1,(boost::mpl::int_<3>))
LET(1,(boost::mpl::plus<VAR(0),VAR(1)>))

int main()
{
  std::cout<<VAR(1)::value<<std::endl;
}
