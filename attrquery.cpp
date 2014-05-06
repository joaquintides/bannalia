/* Multiattribute querying with Boost.MultiIndex.
 *
 * Copyright 2008 Joaquín M López Muñoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */

#include <boost/config.hpp>

#if defined(BOOST_MSVC)
#pragma warning(disable:4100 4512)
#endif

#include <boost/mpl/find.hpp>
#include <boost/mpl/fold.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/plus.hpp>
#include <boost/mpl/sort.hpp>
#include <boost/mpl/times.hpp>
#include <boost/mpl/vector_c.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/facilities/intercept.hpp>
#include <boost/preprocessor/repetition/enum.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>

using namespace boost::multi_index;

struct element
{
  element(int x1,int x2,int x3,int x4):x1(x1),x2(x2),x3(x3),x4(x4){}
  int x1,x2,x3,x4;
};

struct key1:member<element,int,&element::x1>,boost::mpl::int_<1>{};
struct key2:member<element,int,&element::x2>,boost::mpl::int_<2>{};
struct key3:member<element,int,&element::x3>,boost::mpl::int_<3>{};
struct key4:member<element,int,&element::x4>,boost::mpl::int_<4>{};

template<int>struct cover;

typedef multi_index_container<
  element,
  indexed_by<
    ordered_non_unique<
      tag<cover<1>,cover<13>,cover<123>,cover<1234> >,
      composite_key<element,key1,key3,key2,key4>
    >,
    ordered_non_unique<
      tag<cover<4>,cover<14>,cover<134> >,
      composite_key<element,key4,key1,key3>
    >,
    ordered_non_unique<
      tag<cover<2>,cover<12> >,
      composite_key<element,key2,key1>
    >,
    ordered_non_unique<
      tag<cover<24>,cover<124> >,
      composite_key<element,key4,key2,key1>
    >,
    ordered_non_unique<
      tag<cover<3>,cover<23> >,
      composite_key<element,key3,key2>
    >,
    ordered_non_unique<
      tag<cover<34>,cover<234> >,
      composite_key<element,key4,key3,key2>
    >
  >
> container;

template<int N>
struct field
{
  field(int x):x(x){}
  int x;
};

template<typename FieldSequence>
struct cover_number:
  boost::mpl::fold<
    typename boost::mpl::sort<FieldSequence>::type,
    boost::mpl::int_<0>,
    boost::mpl::plus<
      boost::mpl::times<boost::mpl::_1,boost::mpl::int_<10> >,
      boost::mpl::_2
    >
  >::type
{};

template<
  typename CompositeKey,typename FieldSequence,
  typename Pos,typename Tuple
>
int get(const Tuple& t)
{
  typedef typename boost::tuples::element<
    Pos::value,
    typename CompositeKey::key_extractor_tuple
  >::type key_at_pos;
  const int M=
    boost::mpl::distance<
      typename boost::mpl::begin<FieldSequence>::type,
      typename boost::mpl::find<
        FieldSequence,
        field<key_at_pos::value>
      >::type
    >::value;
  return t.template get<M>();
}

#define FIND_PARAM_MACRO(z,n,data) \
const field<BOOST_PP_CAT(N,n)>& BOOST_PP_CAT(f,n)

#define FIND_FIELD_MACRO(z,n,data) field<BOOST_PP_CAT(N,n)>

#define FIND_VALUE_MACRO(z,n,data) BOOST_PP_CAT(f,n).x

#define FIND_GET_MACRO(z,n,data) \
get<composite_key_type,fields_type,boost::mpl::int_<n> >(t)

#define DEFINE_FIND(num_fields) \
template<BOOST_PP_ENUM_PARAMS(num_fields,int N)> \
const element* find( \
  const container& c, \
  BOOST_PP_ENUM(num_fields,FIND_PARAM_MACRO,~)) \
{ \
  typedef cover< \
    cover_number< \
      boost::mpl::vector_c<int, \
        BOOST_PP_ENUM_PARAMS(num_fields,N) \
      > \
    >::value \
  >                                                tag; \
  typedef typename container::index<tag>::type     index_type; \
  typedef typename index_type::key_from_value      composite_key_type; \
  typedef boost::mpl::vector< \
    BOOST_PP_ENUM(num_fields,FIND_FIELD_MACRO,~) \
  >                                                fields_type; \
 \
  const index_type& i=c.get<tag>(); \
  boost::tuple< \
    BOOST_PP_ENUM_PARAMS(num_fields,int BOOST_PP_INTERCEPT) \
  > t=boost::make_tuple( \
    BOOST_PP_ENUM(num_fields,FIND_VALUE_MACRO,~) \
  ); \
  typename index_type::iterator it=i.find( \
    boost::make_tuple( \
      BOOST_PP_ENUM(num_fields,FIND_GET_MACRO,~) \
    ) \
  ); \
  if(it!=i.end())return &*it; \
  else return 0; \
}

DEFINE_FIND(1)
DEFINE_FIND(2)
DEFINE_FIND(3)
DEFINE_FIND(4)

#undef DEFINE_FIND
#undef FIND_GET_MACRO
#undef FIND_VALUE_MACRO
#undef FIND_FIELD_MACRO
#undef FIND_PARAM_MACRO

/* testing*/

#include <cassert>

int main()
{
  container c;
  c.insert(element(10,20,30,40));
  c.insert(element(50,60,70,80));
  c.insert(element(11,12,13,14));
  c.insert(element( 0, 1, 2, 3));

  assert(::find(c,field<1>(10))->x1==10);
  assert(!::find(c,field<2>(10)));
  assert(::find(c,field<3>(13))->x3==13);
  assert(!::find(c,field<4>(13)));

  assert(::find(c,field<1>(10),field<2>(20))->x1==10);
  assert(!::find(c,field<2>(10),field<3>(2)));
  assert(::find(c,field<3>(13),field<4>(14))->x3==13);
  assert(!::find(c,field<4>(13),field<2>(10)));

  assert(::find(c,field<1>(10),field<2>(20),field<4>(40))->x1==10);
  assert(!::find(c,field<2>(10),field<3>(2),field<1>(50)));

  assert(::find(c,field<1>(10),field<2>(20),field<4>(40),field<3>(30))->x1==10);
  assert(!::find(c,field<2>(10),field<3>(2),field<1>(50),field<4>(1)));
}
