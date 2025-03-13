/*
Copyright 2010 Intel Corporation

Use, modification and distribution are subject to the Boost Software License,
Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt).
*/

// code modified for the Deepnest project by Jack Qiao
// https://github.com/Jack000/Deepnest/blob/master/minkowski.cc
// updated for never node version taken from:
// https://github.com/9swampy/Deepnest/tree/develop

#define BOOST_POLYGON_NO_DEPS

#include <iostream>
#include <string>
#include <sstream>
#include <limits>

#include <napi.h>
#include <boost/polygon/polygon.hpp>

#undef min
#undef max

typedef boost::polygon::point_data<int> point;
typedef boost::polygon::polygon_set_data<int> polygon_set;
typedef boost::polygon::polygon_with_holes_data<int> polygon;
typedef std::pair<point, point> edge;
using namespace boost::polygon::operators;

void convolve_two_segments(std::vector<point>& figure, const edge& a, const edge& b) {
  using namespace boost::polygon;
  figure.clear();
  figure.push_back(point(a.first));
  figure.push_back(point(a.first));
  figure.push_back(point(a.second));
  figure.push_back(point(a.second));
  convolve(figure[0], b.second);
  convolve(figure[1], b.first);
  convolve(figure[2], b.first);
  convolve(figure[3], b.second);
}

template <typename itrT1, typename itrT2>
void convolve_two_point_sequences(polygon_set& result, itrT1 ab, itrT1 ae, itrT2 bb, itrT2 be) {
  using namespace boost::polygon;
  if(ab == ae || bb == be)
    return;
  point first_a = *ab;
  point prev_a = *ab;
  std::vector<point> vec;
  polygon poly;
  ++ab;
  for( ; ab != ae; ++ab) {
    point first_b = *bb;
    point prev_b = *bb;
    itrT2 tmpb = bb;
    ++tmpb;
    for( ; tmpb != be; ++tmpb) {
      convolve_two_segments(vec, std::make_pair(prev_b, *tmpb), std::make_pair(prev_a, *ab));
      set_points(poly, vec.begin(), vec.end());
      result.insert(poly);
      prev_b = *tmpb;
    }
    prev_a = *ab;
  }
}

template <typename itrT>
void convolve_point_sequence_with_polygons(polygon_set& result, itrT b, itrT e, const std::vector<polygon>& polygons) {
  using namespace boost::polygon;
  for(std::size_t i = 0; i < polygons.size(); ++i) {
    convolve_two_point_sequences(result, b, e, begin_points(polygons[i]), end_points(polygons[i]));
    for(polygon_with_holes_traits<polygon>::iterator_holes_type itrh = begin_holes(polygons[i]);
        itrh != end_holes(polygons[i]); ++itrh) {
      convolve_two_point_sequences(result, b, e, begin_points(*itrh), end_points(*itrh));
    }
  }
}

void convolve_two_polygon_sets(polygon_set& result, const polygon_set& a, const polygon_set& b) {
  using namespace boost::polygon;
  result.clear();
  std::vector<polygon> a_polygons;
  std::vector<polygon> b_polygons;
  a.get(a_polygons);
  b.get(b_polygons);
  for(std::size_t ai = 0; ai < a_polygons.size(); ++ai) {
    convolve_point_sequence_with_polygons(result, begin_points(a_polygons[ai]), 
                                          end_points(a_polygons[ai]), b_polygons);
    for(polygon_with_holes_traits<polygon>::iterator_holes_type itrh = begin_holes(a_polygons[ai]);
        itrh != end_holes(a_polygons[ai]); ++itrh) {
      convolve_point_sequence_with_polygons(result, begin_points(*itrh), 
                                            end_points(*itrh), b_polygons);
    }
    for(std::size_t bi = 0; bi < b_polygons.size(); ++bi) {
      polygon tmp_poly = a_polygons[ai];
      result.insert(convolve(tmp_poly, *(begin_points(b_polygons[bi]))));
      tmp_poly = b_polygons[bi];
      result.insert(convolve(tmp_poly, *(begin_points(a_polygons[ai]))));
    }
  }
}

double inputscale;

Napi::Value CalculateNFP(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  Napi::Object group = info[0].As<Napi::Object>();
  Napi::Array A = group.Get("A").As<Napi::Array>();
  Napi::Array B = group.Get("B").As<Napi::Array>();
  
  polygon_set a, b, c;
  std::vector<polygon> polys;
  std::vector<point> pts;
  
  // get maximum bounds for scaling factor
  unsigned int len = A.Length();
  double Amaxx = 0;
  double Aminx = 0;
  double Amaxy = 0;
  double Aminy = 0;
  for (unsigned int i = 0; i < len; i++) {
    Napi::Object obj = A.Get(i).As<Napi::Object>();
    Amaxx = (std::max)(Amaxx, obj.Get("x").As<Napi::Number>().DoubleValue());
    Aminx = (std::min)(Aminx, obj.Get("x").As<Napi::Number>().DoubleValue());
    Amaxy = (std::max)(Amaxy, obj.Get("y").As<Napi::Number>().DoubleValue());
    Aminy = (std::min)(Aminy, obj.Get("y").As<Napi::Number>().DoubleValue());
  }
  
  len = B.Length();
  double Bmaxx = 0;
  double Bminx = 0;
  double Bmaxy = 0;
  double Bminy = 0;
  for (unsigned int i = 0; i < len; i++) {
    Napi::Object obj = B.Get(i).As<Napi::Object>();
    Bmaxx = (std::max)(Bmaxx, obj.Get("x").As<Napi::Number>().DoubleValue());
    Bminx = (std::min)(Bminx, obj.Get("x").As<Napi::Number>().DoubleValue());
    Bmaxy = (std::max)(Bmaxy, obj.Get("y").As<Napi::Number>().DoubleValue());
    Bminy = (std::min)(Bminy, obj.Get("y").As<Napi::Number>().DoubleValue());
  }
  
  double Cmaxx = Amaxx + Bmaxx;
  double Cminx = Aminx + Bminx;
  double Cmaxy = Amaxy + Bmaxy;
  double Cminy = Aminy + Bminy;
  
  double maxxAbs = (std::max)(Cmaxx, std::fabs(Cminx));
  double maxyAbs = (std::max)(Cmaxy, std::fabs(Cminy));
  
  double maxda = (std::max)(maxxAbs, maxyAbs);
  int maxi = std::numeric_limits<int>::max();
  
  if(maxda < 1){
    maxda = 1;
  }
  
  // why 0.1? dunno. it doesn't screw up with 0.1
  inputscale = (0.1f * (double)(maxi)) / maxda;
  
  //double scale = 1000;
  len = A.Length();
  
  for (unsigned int i = 0; i < len; i++) {
    Napi::Object obj = A.Get(i).As<Napi::Object>();
    int x = (int)(inputscale * obj.Get("x").As<Napi::Number>().DoubleValue());
    int y = (int)(inputscale * obj.Get("y").As<Napi::Number>().DoubleValue());
        
    pts.push_back(point(x, y));
  }
  
  polygon poly;
  boost::polygon::set_points(poly, pts.begin(), pts.end());
  a+=poly;
  
  // subtract holes from a here...
  if (A.Has("children")) {
    Napi::Array holes = A.Get("children").As<Napi::Array>();
    len = holes.Length();
    
    for(unsigned int i=0; i<len; i++){
      Napi::Array hole = holes.Get(i).As<Napi::Array>();
      pts.clear();
      unsigned int hlen = hole.Length();
      for(unsigned int j=0; j<hlen; j++){
        Napi::Object obj = hole.Get(j).As<Napi::Object>();
        int x = (int)(inputscale * obj.Get("x").As<Napi::Number>().DoubleValue());
        int y = (int)(inputscale * obj.Get("y").As<Napi::Number>().DoubleValue());
        pts.push_back(point(x, y));
      }
      boost::polygon::set_points(poly, pts.begin(), pts.end());
      a -= poly;
    }
  }
  
  //and then load points B
  pts.clear();
  len = B.Length();
  
  //javascript nfps are referenced with respect to the first point
  double xshift = 0;
  double yshift = 0;
  
  for (unsigned int i = 0; i < len; i++) {
    Napi::Object obj = B.Get(i).As<Napi::Object>();
    int x = -(int)(inputscale * obj.Get("x").As<Napi::Number>().DoubleValue());
    int y = -(int)(inputscale * obj.Get("y").As<Napi::Number>().DoubleValue());
    pts.push_back(point(x, y));
    
    if(i==0){
      xshift = obj.Get("x").As<Napi::Number>().DoubleValue();
      yshift = obj.Get("y").As<Napi::Number>().DoubleValue();
    }
  }
  
  boost::polygon::set_points(poly, pts.begin(), pts.end());
  b+=poly;
  
  polys.clear();
  
  convolve_two_polygon_sets(c, a, b);
  c.get(polys);
  
  Napi::Array result_list = Napi::Array::New(env);
  
  for(unsigned int i = 0; i < polys.size(); ++i ){
      
    Napi::Array pointlist = Napi::Array::New(env);
    int j = 0;
        
    for(polygon_traits<polygon>::iterator_type itr = polys[i].begin(); itr != polys[i].end(); ++itr) {
       Napi::Object p = Napi::Object::New(env);
       p.Set("x", ((double)(*itr).get(boost::polygon::HORIZONTAL)) / inputscale + xshift);
       p.Set("y", ((double)(*itr).get(boost::polygon::VERTICAL)) / inputscale + yshift);
       
       pointlist[j] = p;
       j++;
    }
    
    // holes
    Napi::Array children = Napi::Array::New(env);
    int k = 0;
    for(polygon_with_holes_traits<polygon>::iterator_holes_type itrh = begin_holes(polys[i]); itrh != end_holes(polys[i]); ++itrh){
      Napi::Array child = Napi::Array::New(env);
      int z = 0;
      for(polygon_traits<polygon>::iterator_type itr2 = (*itrh).begin(); itr2 != (*itrh).end(); ++itr2) {
        Napi::Object c = Napi::Object::New(env);
        c.Set("x", ((double)(*itr2).get(boost::polygon::HORIZONTAL)) / inputscale + xshift);
        c.Set("y", ((double)(*itr2).get(boost::polygon::VERTICAL)) / inputscale + yshift);
        
        child[z] = c;
        z++;
      }
      children[k] = child;
      k++;
    }
    
    pointlist.Set("children", children);
    
    result_list[i] = pointlist;
  }
  
  return result_list;  
}