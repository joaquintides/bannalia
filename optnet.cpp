/* Optimum network solver.
 *
 * Copyright 2007 Joaquín M López Muñoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */

#include <boost/config.hpp>
#if defined(BOOST_MSVC)
#pragma warning(disable:4267)
#endif

#include <algorithm>
#include <boost/algorithm/minmax_element.hpp>
#include <boost/array.hpp>
#include <boost/graph/kruskal_min_spanning_tree.hpp>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/function_output_iterator.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/tuple/tuple.hpp>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <iostream>
#include <vector>

struct point
{
  double x,y;
};

inline double dist2(const point& p0,const point& p1)
{
  return (p1.x-p0.x)*(p1.x-p0.x)+(p1.y-p0.y)*(p1.y-p0.y);
}

inline double dist(const point& p0,const point& p1)
{
  return std::sqrt(dist2(p0,p1));
}

namespace minimum_spanning_tree_detail{

struct complete_graph
{
  typedef std::size_t                                 vertex_descriptor;
  typedef std::pair<std::size_t,std::size_t>          edge_descriptor;
  typedef void                                        adjacency_iterator;
  typedef void                                        out_edge_iterator;
  typedef void                                        in_edge_iterator;
  typedef boost::counting_iterator<vertex_descriptor> vertex_iterator;
  class edge_iterator:public boost::iterator_facade<
    edge_iterator,
    edge_descriptor,
    std::forward_iterator_tag,
    edge_descriptor
  >
  {
  public:
    edge_iterator(){}
    edge_iterator(std::size_t N):N(N),value(0,1){}
    edge_iterator(
      std::size_t N,
      vertex_descriptor u,vertex_descriptor v):
      N(N),value(u,v){}
  private:
    friend class boost::iterator_core_access;
    edge_descriptor dereference()const{return value;}
    bool equal(const edge_iterator& y)const
    {
      return value.first==y.value.first&&
             value.second==y.value.second;
    }
    void increment()
    {
      if(++value.second==N){
        ++value.first;
        value.second=value.first+1;
      }
    }
    std::size_t     N;
    edge_descriptor value;
  };
  typedef boost::undirected_tag                       directed_category;
  typedef boost::disallow_parallel_edge_tag           edge_parallel_category;
  struct traversal_category:
    boost::vertex_list_graph_tag,
    boost::edge_list_graph_tag
  {};
  typedef std::size_t                                 vertices_size_type;
  typedef std::size_t                                 edges_size_type;
  typedef void                                        degree_size_type;
  typedef void                                        vertex_property_type;
  typedef void                                        graph_tag;
  vertex_descriptor null_vertex(){return N;}

  complete_graph(std::size_t N):N(N){}
  std::size_t N;
};

inline std::pair<
  complete_graph::vertex_iterator,
  complete_graph::vertex_iterator
>
vertices(const complete_graph& g)
{
  return std::make_pair(
    complete_graph::vertex_iterator(0),
    complete_graph::vertex_iterator(g.N));
};

inline std::size_t num_vertices(const complete_graph& g){return g.N;};

inline std::pair<
  complete_graph::edge_iterator,
  complete_graph::edge_iterator>
edges(const complete_graph& g)
{
  return std::make_pair(
    complete_graph::edge_iterator(g.N),
    complete_graph::edge_iterator(g.N,g.N-1,g.N));
};

inline std::size_t num_edges(const complete_graph& g){return g.N*(g.N-1)/2;};

inline boost::identity_property_map get(
  boost::vertex_index_t,const complete_graph&)
{
  return boost::identity_property_map();
}

struct distance_map
{
  typedef double                           value_type;
  typedef double                           reference;
  typedef std::size_t                      key_type;
  typedef boost::readable_property_map_tag category;

  typedef std::vector<point>               node_sequence;

  distance_map(const node_sequence& nodes):nodes(&nodes){}

  const node_sequence* nodes;
};

inline double get(
  const distance_map& m,
  const std::pair<std::size_t,std::size_t>& e)
{
  return dist((*(m.nodes))[e.first],(*(m.nodes))[e.second]);
}

struct distance_accumulator
{
  typedef std::vector<point> node_sequence;

  distance_accumulator(double& acc,const node_sequence& nodes):
    acc(&acc),nodes(&nodes){}
  void operator()(const std::pair<std::size_t,std::size_t>& e)const
  {
    *acc+=dist((*nodes)[e.first],(*nodes)[e.second]);
  }

  double*              acc;
  const node_sequence* nodes;
};

} /* namespace minimum_spanning_tree_detail */

template<typename OutputIterator>
void minimum_spanning_tree(
  const std::vector<point> nodes,OutputIterator tree_edges)
{
  using namespace minimum_spanning_tree_detail;
  boost::kruskal_minimum_spanning_tree(
    complete_graph(nodes.size()),tree_edges,
    boost::weight_map(distance_map(nodes)));
}

double minimum_spanning_tree_length(const std::vector<point> nodes)
{
  using namespace minimum_spanning_tree_detail;
  double res=0.0;
  boost::kruskal_minimum_spanning_tree(
    complete_graph(nodes.size()),
    boost::function_output_iterator<distance_accumulator>
      (distance_accumulator(res,nodes)),
    boost::weight_map(distance_map(nodes)));
  return res;
}

class optimum_network_solver
{
public:
  template<typename InputIterator>
  optimum_network_solver(
    InputIterator cities_first,InputIterator cities_last,
    std::size_t num_nodes,
    double min_distance,
    double mutation_rate,
    std::size_t population):
    cities(cities_first,cities_last),
    num_nodes(num_nodes),
    min_distance(min_distance),
    mutation_rate(mutation_rate)
  {
    using boost::lambda::_1;
    using boost::lambda::_2;

    std::vector<point>::const_iterator pxmin,pxmax;
    boost::tuples::tie(pxmin,pxmax)=boost::minmax_element(
      cities.begin(),cities.end(),
      &_1->*(&point::x)<&_2->*(&point::x));
    xmin=pxmin->x;
    xmax=pxmax->x;

    std::vector<point>::const_iterator pymin,pymax;
    boost::tuples::tie(pymin,pymax)=boost::minmax_element(
      cities.begin(),cities.end(),
      &_1->*(&point::y)<&_2->*(&point::y));
    ymin=pymin->y;
    ymax=pymax->y;

    boost::uniform_real<>           rnd_type;
    boost::uniform_int<std::size_t> rnd_city(0,cities.size()-1);
    boost::uniform_real<>           rnd_theta(0,2.0*3.14159265);
    boost::uniform_real<>           rnd_mix;
    boost::uniform_real<>           rnd_x(xmin,xmax);
    boost::uniform_real<>           rnd_y(ymin,ymax);
    pool.reserve(population);
    for(std::size_t m=0;m<population;++m){
      mesh msh;
      msh.nodes.reserve(num_nodes);
      for(std::size_t n=0;n<num_nodes;++n){
        point p;
        if(rnd_type(rnd_gen)<cities.size()/num_nodes){
          const  point& city=cities[rnd_city(rnd_gen)];
          double rho=min_distance*0.995;
          double theta=rnd_theta(rnd_gen);
          p.x=city.x+rho*std::cos(theta);
          p.y=city.y+rho*std::sin(theta);
        }
        else{
          const  point& city1=cities[rnd_city(rnd_gen)];
          const  point& city2=cities[rnd_city(rnd_gen)];
          double mix=rnd_mix(rnd_gen);
          p.x=mix*city1.x+(1.0-mix)*city2.x;
          p.y=mix*city1.y+(1.0-mix)*city2.y;
        }
        msh.nodes.push_back(p);
      }
      calculate_length(msh);
      pool.push_back(msh);
    }

    sort_by_length();
  }

  void iterate()
  {
    boost::uniform_int<std::size_t> rnd(0,pool.size()/4);
    for(std::size_t m=pool.size()/4+1,m_end=pool.size();m<m_end;++m){
      breed(pool[m],pool[rnd(rnd_gen)],pool[rnd(rnd_gen)]);
    }

    sort_by_length();
  }

  template<typename OutputIterator>
  void solution(OutputIterator tree_edges)const
  {
    typedef boost::function_output_iterator<
      solution_output<OutputIterator>
    > output_iterator;
      
    minimum_spanning_tree(
      pool[0].nodes,
      output_iterator(solution_output<OutputIterator>(tree_edges,pool[0])));
  }

  double solution_length()const{return pool[0].length;}

  double min_x()const{return xmin;}
  double max_x()const{return xmax;}
  double min_y()const{return ymin;}
  double max_y()const{return ymax;}

private:
  struct mesh
  {
    std::vector<point> nodes;
    double             length;
  };

  void calculate_length(mesh& msh)
  {
    msh.length=
      cities_not_covered(msh)*(xmax-xmin)+
      minimum_spanning_tree_length(msh.nodes);
  }

  void sort_by_length()
  {
    using boost::lambda::_1;
    using boost::lambda::_2;
    std::sort(
      pool.begin(),pool.end(),
      &_1->*(&mesh::length)<&_2->*(&mesh::length));
  }

  std::size_t cities_not_covered(const mesh& msh)const
  {
    std::size_t res=0;
    for(std::size_t c=0,c_end=cities.size();c<c_end;++c){
      bool ok=false;
      for(std::size_t n=0;n<num_nodes;++n){
        if(dist(cities[c],msh.nodes[n])<=min_distance){
          ok=true;
          break;
        }
      }
      if(!ok)++res;
    }
    return res;
  }

  void breed(mesh& dst,const mesh& src0,const mesh& src1)
  {
    std::vector<point> nodes0(src0.nodes);
    std::vector<point> nodes1(src1.nodes);

    boost::uniform_int<std::size_t> rnd_city(0,cities.size()-1);
    dist2_to                        d(cities[rnd_city(rnd_gen)]);
    std::sort(nodes0.begin(),nodes0.end(),d);
    std::sort(nodes1.begin(),nodes1.end(),d);

    boost::uniform_int<std::size_t> rnd_cut(0,num_nodes);
    std::size_t                     cut=rnd_cut(rnd_gen);
    mutate_copy(nodes0.begin(),nodes0.begin()+cut,dst.nodes.begin());
    mutate_copy(nodes1.begin()+cut,nodes1.end(),dst.nodes.begin()+cut);

    calculate_length(dst);
  }

  template<typename InputIterator,typename OutputIterator>
  void mutate_copy(InputIterator first,InputIterator last,OutputIterator out)
  {
    boost::uniform_real<>           rnd_mut;
    boost::uniform_real<>           rnd_mix;
    boost::uniform_int<std::size_t> rnd_city(0,cities.size()-1);
    while(first!=last)
    {
      point p=*first++;
      if(rnd_mut(rnd_gen)<mutation_rate){
        const point& city=cities[rnd_city(rnd_gen)];
        double       mix=rnd_mix(rnd_gen);
        p.x=mix*p.x+(1.0-mix)*city.x;
        p.y=mix*p.y+(1.0-mix)*city.y;
      }
      *out++=p;
    }
  }

  struct dist2_to
  {
    dist2_to(const point& x):x(x){}
    double operator()(const point& p0,const point& p1)const
    {
      return dist2(p0,x)<dist2(p1,x);
    }
    point x;
  };

  template<typename OutputIterator>
  struct solution_output
  {
    solution_output(OutputIterator out,const mesh& msh):out(out),msh(&msh){}

    void operator()(const std::pair<std::size_t,std::size_t>& p)
    {
      *out++=std::make_pair(msh->nodes[p.first],msh->nodes[p.second]);
    }

    OutputIterator out;
    const mesh*    msh;
  };

  std::vector<point> cities;
  std::size_t        num_nodes;
  double             min_distance;
  double             mutation_rate;
  double             xmin,xmax,ymin,ymax;
  std::vector<mesh>  pool;
  boost::mt19937     rnd_gen;
};

namespace output_svg_detail{

struct edge_to_svg
{
  edge_to_svg(
    std::ostream& os,
    double scale,double x0,double y0,std::size_t img_height):
    os(&os),scale(scale),x0(x0),y0(y0),img_height(img_height){}
  void operator()(const std::pair<point,point>& e)
  {
    (*os)<<
      "<line "
        "x1=\""<<(e.first.x-x0)*scale<<"\" "
        "y1=\""<<img_height-(e.first.y-y0)*scale<<"\" "
        "x2=\""<<(e.second.x-x0)*scale<<"\" "
        "y2=\""<<img_height-(e.second.y-y0)*scale<<"\"/>";
  }
  std::ostream* os;
  double        scale,x0,y0;
  std::size_t   img_height;
};

} /* namespace output_svg_detail */

template<typename InputIterator>
void output_svg(
  const char* filename,
  const optimum_network_solver& solver,
  InputIterator cities_first,InputIterator cities_last,
  double img_width)
{
  using namespace output_svg_detail;

  double      width=solver.max_x()-solver.min_x();
  double      height=solver.max_y()-solver.min_y();
  double      offset=0.05*width;
  double      scale=img_width/(1.1*width);
  double      x0=solver.min_x()-offset;
  double      y0=solver.min_y()-offset;
  std::size_t img_height=
                (std::size_t)(img_width*(height+2.0*offset)/(1.1*width));

  std::ofstream ofs(filename);
  ofs<<
    "<?xml version=\"1.0\" standalone=\"no\"?>"
    "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" "
      "\"http://www.w3.org/graphics/svg/1.1/dtd/svg11.dtd\">"
    "<svg width=\""<<img_width<<"\" height =\""<<img_height<<"\" "
      "version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\">"
    "<g stroke=\"rgb(0,0,0)\" fill=\"rgb(255,0,0)\">";
  while(cities_first!=cities_last){
    point city=*cities_first++;
    ofs<<
      "<circle "
        "cx=\""<<(city.x-x0)*scale<<"\" "
        "cy=\""<<img_height-(city.y-y0)*scale<<"\" r=\"3\"/>";
  }
  solver.solution(
    boost::function_output_iterator<edge_to_svg>(
      edge_to_svg(ofs,scale,x0,y0,img_height)));
  ofs<<"</g></svg>";
};

int main()
{
  boost::array<point,47> cities={{
    /* Albacete 01°51'21W 38°59'44.1N      */ {599082.94,4316909.05},
    /* Alicante 00°28'47W 38°20'54.3N      */ {720239.17,4247480.14},
    /* Almería 02°27'56W 36°49'36.0N       */ {547660.68,4075777.46},
    /* Ávila 04°41'51W 40°39'20.6N         */ {356494.18,4501925.33},
    /* Badajoz 06°58'03W 38°52'55.2N       */ {155818.50,4311169.06},
    /* Barcelona 02°10'36E 41°23'02.5N     */ {932903.59,4594336.86},
    /* Bilbao 02°55'26W 43°15'26.0N        */ {506177.75,4789382.34},
    /* Burgos 03°42'16W 42°20'25.3N        */ {441971.37,4687807.53},
    /* Cáceres 06°22'15W 39°28'22.8N       */ {210034.58,4374693.91},
    /* Cádiz 06°17'52W 36°31'54.7N         */ {204739.79,4048003.58},
    /* Castellón 00°12'15W 39°59'10.0N     */ {738716.52,4429960.00},
    /* Ciudad Real 03°55'52W 38°59'11.7N   */ {419357.48,4315699.99},
    /* Córdoba 04°46'48W 37°52'46.1N       */ {343459.07,4193935.64},
    /* La Coruña 08°23'27W 43°22'12.5N     */ {63211.75,4816051.65},
    /* Cuenca 02°07'54W 40°04'35.1N        */ {574038.47,4436599.99},
    /* Gerona 02°49'27E 41°58'55.1N        */ {982567.64,4664210.46},
    /* Granada 03°36'00W 37°10'34.9N       */ {446737.65,4114605.50},
    /* Guadalajara 03°09'45W 40°38'04.1N   */ {486258.28,4498193.99},
    /* Huelva 06°57'00W 37°15'35.9N        */ {149670.03,4131030.37},
    /* Huesca 00°24'32W 42°08'25.8N        */ {714124.36,4668625.70},
    /* Jaén 03°47'24W 37°45'54.4N          */ {430418.91,4180047.92},
    /* León 05°34'01W 42°35'56.9N          */ {289411.60,4719495.55},
    /* Lérida 00°37'36E 41°37'02.5N        */ {802168.61,4613649.66},
    /* Logroño 02°26'45W 42°27'59.2N       */ {545558.13,4701715.88},
    /* Lugo 07°33'28W 43°00'33.7N          */ {128531.85,4771943.58},
    /* Madrid 03°41'15W 40°24'30.0N        */ {441666.03,4473306.70},
    /* Málaga 04°24'50W 36°43'12.9N        */ {373734.67,4064771.13},
    /* Murcia 01°07'42W 37°59'03.5N        */ {664369.51,4205726.32},
    /* Orense 07°51'48W 42°20'11.2N        */ {99313.76,4698599.69},
    /* Oviedo 05°50'36W 43°21'44.2N        */ {269605.27,4804973.23},
    /* Palencia 04°32'04W 42°00'27.8N      */ {372934.71,4651772.44},
    /* Pamplona 01°38'30W 42°49'10.9N      */ {611039.50,4741687.21},
    /* Pontevedra 08°38'51W 42°26'01.4N    */ {35409.25,4713409.38},
    /* Salamanca 05°40'00W 40°57'39.5N     */ {275585.48,4537849.45},
    /* San Sebastián 01°58'54W 43°19'01.3N */ {582574.92,4796524.62},
    /* Santander 03°48'19W 43°27'47.7N     */ {434858.27,4812574.92},
    /* Segovia 04°07'33W 40°57'00.4N       */ {405243.46,4533829.14},
    /* Sevilla 05°59'33W 37°23'10.0N       */ {235059.57,4141909.48},
    /* Soria 02°28'00W 41°46'06.3N         */ {544329.26,4624201.75},
    /* Tarragona 01°22'13E 41°15'12.3N     */ {866182.48,4576107.59},
    /* Teruel 01°06'33W 40°20'38.7N        */ {660592.71,4467664.05},
    /* Toledo 04°01'27W 39°51'25.6N        */ {412394.65,4412399.97},
    /* Valencia 00°22'33W 39°28'30.7N      */ {725718.73,4372799.01},
    /* Valladolid 04°43'24W 41°39'08.0N    */ {356500.58,4612598.53},
    /* Vitoria 02°40'18W 42°50'50.8N       */ {526828.03,4743926.26},
    /* Zamora 05°45'16W 41°29'56.1N        */ {270091.65,4597807.23},
    /* Zaragoza 00°52'47W 41°39'24.2N      */ {676541.72,4613835.31}
  }};
  optimum_network_solver solver(
    cities.begin(),cities.end(),92,50000.0,0.01,5000);
  for(int i=0;;++i){
    std::rename("optnet.svg","optnet_bak.svg");
    output_svg("optnet.svg",solver,cities.begin(),cities.end(),500);
    std::remove("optnet_bak.svg");
    std::cout<<"iteration: "<<i<<"\t"
             <<"total length: "<<solver.solution_length()<<"\n";
    solver.iterate();
  }
}
