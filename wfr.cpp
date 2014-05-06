/* Word frequency calculator.
 *
 * Copyright 2008 Joaquín M López Muñoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/tokenizer.hpp>
#include <iostream>
#include <iomanip>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

int main()
{
  typedef boost::tokenizer<
    boost::char_separator<char>,std::istream_iterator<char>
  > text_tokenizer;
  
  typedef std::pair<std::size_t,std::string> table_value;
  
  typedef boost::multi_index_container<
    table_value,
    boost::multi_index::indexed_by<
      boost::multi_index::ordered_non_unique<
        boost::multi_index::member<table_value,std::size_t,&table_value::first>,
        std::greater<std::size_t>
      >,      
      boost::multi_index::hashed_unique<
        boost::multi_index::member<table_value,std::string,&table_value::second>
      >
    >
  > table;
  
  table t;
  std::istream_iterator<char> begin_cin(std::cin),end_cin;
  text_tokenizer tok(
    begin_cin,end_cin,boost::char_separator<char>(" \t\n.,;:!?'\"-"));
 
  std::cin.unsetf(std::ios::skipws);
    
  for(text_tokenizer::iterator iti=tok.begin();iti!=tok.end();++iti){
    table_value v(0,*iti);
    boost::algorithm::to_lower(v.second);
    table::iterator it=t.insert(v).first;
    t.modify_key(it,++boost::lambda::_1);
  }
  
  table::iterator it=t.begin(),ite=t.end();
  for(std::size_t n=1000;it!=ite&&n--;++it){
    std::cout<<it->second<<"\t"<<it->first<<"\t"<<it->second.size()<<std::endl;
  }
}
