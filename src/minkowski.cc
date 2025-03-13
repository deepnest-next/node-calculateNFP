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

#include <boost/polygon/polygon.hpp>

// Include Node.js headers only when building for Node.js
#ifdef USE_NODE_API
#include <napi.h>
#endif

// Use a different macro for Rust integration
#ifdef USE_RUST
// No need to include any Rust-specific headers here
// The extern "C" functions are enough for FFI
#endif

#undef min
#undef max

typedef boost::polygon::point_data<int> point;
typedef boost::polygon::polygon_set_data<int> polygon_set;
typedef boost::polygon::polygon_with_holes_data<int> polygon;
typedef std::pair<point, point> edge;
using namespace boost::polygon::operators;

// C-style data structures for cross-language compatibility
extern "C" {
    struct PointXY {
        double x;
        double y;
    };

    struct PolygonHole {
        PointXY* points;
        int num_points;
    };

    struct PolygonData {
        PointXY* points;
        int num_points;
        PolygonHole* holes;
        int num_holes;
    };

    struct NFPResult {
        PolygonData* polygons;
        int num_polygons;
    };
    
    // Forward declaration of free_nfp_result
    void free_nfp_result(NFPResult* result);
}

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

// Core function for NFP calculation with C-compatible interface
extern "C" NFPResult* calculate_nfp_raw(
    const PointXY* a_points, int a_length,
    const PointXY** a_holes, const int* a_hole_lengths, int a_num_holes,
    const PointXY* b_points, int b_length,
    const PointXY** b_holes, const int* b_hole_lengths, int b_num_holes
) {
    polygon_set a, b, c;
    std::vector<polygon> polys;
    std::vector<point> pts;
    
    // Calculate input scale based on the geometry bounds
    double Amaxx = 0, Aminx = 0, Amaxy = 0, Aminy = 0;
    double Bmaxx = 0, Bminx = 0, Bmaxy = 0, Bminy = 0;
    
    for (int i = 0; i < a_length; i++) {
        Amaxx = (std::max)(Amaxx, a_points[i].x);
        Aminx = (std::min)(Aminx, a_points[i].x);
        Amaxy = (std::max)(Amaxy, a_points[i].y);
        Aminy = (std::min)(Aminy, a_points[i].y);
    }
    
    for (int i = 0; i < b_length; i++) {
        Bmaxx = (std::max)(Bmaxx, b_points[i].x);
        Bminx = (std::min)(Bminx, b_points[i].x);
        Bmaxy = (std::max)(Bmaxy, b_points[i].y);
        Bminy = (std::min)(Bminy, b_points[i].y);
    }
    
    double Cmaxx = Amaxx + Bmaxx;
    double Cminx = Aminx + Bminx;
    double Cmaxy = Amaxy + Bmaxy;
    double Cminy = Aminy + Bminy;
    
    double maxxAbs = (std::max)(Cmaxx, std::fabs(Cminx));
    double maxyAbs = (std::max)(Cmaxy, std::fabs(Cminy));
    
    double maxda = (std::max)(maxxAbs, maxyAbs);
    int maxi = std::numeric_limits<int>::max();
    
    if (maxda < 1) {
        maxda = 1;
    }
    
    double inputscale = (0.1f * (double)(maxi)) / maxda;
    
    // Store first point of B for shift reference
    double xshift = b_points[0].x;
    double yshift = b_points[0].y;

    // Process polygon A
    for (int i = 0; i < a_length; i++) {
        int x = (int)(inputscale * a_points[i].x);
        int y = (int)(inputscale * a_points[i].y);
        pts.push_back(point(x, y));
    }
    
    polygon poly;
    boost::polygon::set_points(poly, pts.begin(), pts.end());
    a += poly;
    
    // Process holes in A
    for (int i = 0; i < a_num_holes; i++) {
        pts.clear();
        for (int j = 0; j < a_hole_lengths[i]; j++) {
            int x = (int)(inputscale * a_holes[i][j].x);
            int y = (int)(inputscale * a_holes[i][j].y);
            pts.push_back(point(x, y));
        }
        boost::polygon::set_points(poly, pts.begin(), pts.end());
        a -= poly;
    }
    
    // Process polygon B (negated for NFP)
    pts.clear();
    for (int i = 0; i < b_length; i++) {
        int x = -(int)(inputscale * b_points[i].x);
        int y = -(int)(inputscale * b_points[i].y);
        pts.push_back(point(x, y));
    }
    
    boost::polygon::set_points(poly, pts.begin(), pts.end());
    b += poly;
    
    // Process holes in B
    for (int i = 0; i < b_num_holes; i++) {
        pts.clear();
        for (int j = 0; j < b_hole_lengths[i]; j++) {
            int x = -(int)(inputscale * b_holes[i][j].x);
            int y = -(int)(inputscale * b_holes[i][j].y);
            pts.push_back(point(x, y));
        }
        boost::polygon::set_points(poly, pts.begin(), pts.end());
        b -= poly;
    }
    
    // Calculate NFP
    convolve_two_polygon_sets(c, a, b);
    polys.clear();
    c.get(polys);
    
    // Allocate and fully initialize result structure
    NFPResult* result = nullptr;
    try {
        result = new NFPResult();
        // Initialize all fields
        result->polygons = nullptr;
        result->num_polygons = 0;
        
        size_t num_polygons = polys.size();
        if (num_polygons == 0) {
            return result; // Return early for empty result
        }
        
        // Allocate polygons array
        result->polygons = new PolygonData[num_polygons]();  // Zero-initializes all memory
        result->num_polygons = static_cast<int>(num_polygons);
        
        // Process each polygon
        for (size_t i = 0; i < num_polygons; ++i) {
            // Get points from this polygon
            std::vector<point> points;
            for (auto itr = polys[i].begin(); itr != polys[i].end(); ++itr) {
                points.push_back(*itr);
            }
            
            // Store number of points
            size_t num_points = points.size();
            if (num_points == 0) {
                continue; // Skip empty polygons
            }
            
            // Allocate and populate points array
            result->polygons[i].points = new PointXY[num_points]();  // Zero-initialize
            result->polygons[i].num_points = static_cast<int>(num_points);
            
            for (size_t j = 0; j < num_points; ++j) {
                result->polygons[i].points[j].x = ((double)(points[j].get(boost::polygon::HORIZONTAL))) / inputscale + xshift;
                result->polygons[i].points[j].y = ((double)(points[j].get(boost::polygon::VERTICAL))) / inputscale + yshift;
            }
            
            // Process holes
            std::vector<std::vector<point>> holes;
            for (auto itrh = begin_holes(polys[i]); itrh != end_holes(polys[i]); ++itrh) {
                std::vector<point> hole_points;
                for (auto itr2 = (*itrh).begin(); itr2 != (*itrh).end(); ++itr2) {
                    hole_points.push_back(*itr2);
                }
                if (!hole_points.empty()) { // Only add non-empty holes
                    holes.push_back(hole_points);
                }
            }
            
            // Store hole count
            size_t num_holes = holes.size();
            result->polygons[i].num_holes = static_cast<int>(num_holes);
            
            // Skip if no holes
            if (num_holes == 0) {
                result->polygons[i].holes = nullptr;
                continue;
            }
            
            // Allocate holes array
            result->polygons[i].holes = new PolygonHole[num_holes]();  // Zero-initialize
            
            // Process each hole
            for (size_t h = 0; h < num_holes; ++h) {
                size_t num_hole_points = holes[h].size();
                result->polygons[i].holes[h].num_points = static_cast<int>(num_hole_points);
                
                // Skip empty holes
                if (num_hole_points == 0) {
                    result->polygons[i].holes[h].points = nullptr;
                    continue;
                }
                
                // Allocate and populate hole points
                result->polygons[i].holes[h].points = new PointXY[num_hole_points]();  // Zero-initialize
                
                for (size_t p = 0; p < num_hole_points; ++p) {
                    result->polygons[i].holes[h].points[p].x = 
                        ((double)(holes[h][p].get(boost::polygon::HORIZONTAL))) / inputscale + xshift;
                    result->polygons[i].holes[h].points[p].y = 
                        ((double)(holes[h][p].get(boost::polygon::VERTICAL))) / inputscale + yshift;
                }
            }
        }
    } catch (const std::exception&) {
        // Clean up on any exception
        if (result) {
            free_nfp_result(result); // Now this will be found
            result = nullptr;
        }
    }
    
    return result;
}

// Function to safely free the NFP result
extern "C" void free_nfp_result(NFPResult* result) {
    // Guard against null pointer
    if (!result) {
        return;
    }
    
    // Free polygon data
    if (result->polygons) {
        for (int i = 0; i < result->num_polygons; ++i) {
            // First free points for each polygon
            if (result->polygons[i].points) {
                delete[] result->polygons[i].points;
                result->polygons[i].points = nullptr;
            }
            
            // Then handle holes
            if (result->polygons[i].holes) {
                // Free each hole's points
                for (int h = 0; h < result->polygons[i].num_holes; ++h) {
                    if (result->polygons[i].holes[h].points) {
                        delete[] result->polygons[i].holes[h].points;
                        result->polygons[i].holes[h].points = nullptr;
                    }
                }
                // Free the holes array
                delete[] result->polygons[i].holes;
                result->polygons[i].holes = nullptr;
            }
        }
        // Free the polygons array
        delete[] result->polygons;
        result->polygons = nullptr;
    }
    
    // Finally delete the result itself
    delete result;
}

// Only include Node.js specific code when building for Node.js
#ifdef USE_NODE_API
double inputscale; // kept for backward compatibility

Napi::Value CalculateNFP(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    Napi::Object group = info[0].As<Napi::Object>();
    Napi::Array A = group.Get("A").As<Napi::Array>();
    Napi::Array B = group.Get("B").As<Napi::Array>();
    
    // Convert Node.js arrays to C-style arrays
    unsigned int a_length = A.Length();
    PointXY* a_points = new PointXY[a_length];
    
    for (unsigned int i = 0; i < a_length; i++) {
        Napi::Object obj = A.Get(i).As<Napi::Object>();
        a_points[i].x = obj.Get("x").As<Napi::Number>().DoubleValue();
        a_points[i].y = obj.Get("y").As<Napi::Number>().DoubleValue();
    }
    
    // Process A holes if any
    int a_num_holes = 0;
    PointXY** a_holes = nullptr;
    int* a_hole_lengths = nullptr;
    
    if (A.Has("children")) {
        Napi::Array holes = A.Get("children").As<Napi::Array>();
        a_num_holes = holes.Length();
        
        if (a_num_holes > 0) {
            a_holes = new PointXY*[a_num_holes];
            a_hole_lengths = new int[a_num_holes];
            
            for (int i = 0; i < a_num_holes; i++) {
                Napi::Array hole = holes.Get(i).As<Napi::Array>();
                int hole_length = hole.Length();
                a_hole_lengths[i] = hole_length;
                a_holes[i] = new PointXY[hole_length];
                
                for (int j = 0; j < hole_length; j++) {
                    Napi::Object obj = hole.Get(j).As<Napi::Object>();
                    a_holes[i][j].x = obj.Get("x").As<Napi::Number>().DoubleValue();
                    a_holes[i][j].y = obj.Get("y").As<Napi::Number>().DoubleValue();
                }
            }
        }
    }
    
    // Process B polygon
    unsigned int b_length = B.Length();
    PointXY* b_points = new PointXY[b_length];
    
    for (unsigned int i = 0; i < b_length; i++) {
        Napi::Object obj = B.Get(i).As<Napi::Object>();
        b_points[i].x = obj.Get("x").As<Napi::Number>().DoubleValue();
        b_points[i].y = obj.Get("y").As<Napi::Number>().DoubleValue();
    }
    
    // Process B holes if any
    int b_num_holes = 0;
    PointXY** b_holes = nullptr;
    int* b_hole_lengths = nullptr;
    
    if (B.Has("children")) {
        Napi::Array holes = B.Get("children").As<Napi::Array>();
        b_num_holes = holes.Length();
        
        if (b_num_holes > 0) {
            b_holes = new PointXY*[b_num_holes];
            b_hole_lengths = new int[b_num_holes];
            
            for (int i = 0; i < b_num_holes; i++) {
                Napi::Array hole = holes.Get(i).As<Napi::Array>();
                int hole_length = hole.Length();
                b_hole_lengths[i] = hole_length;
                b_holes[i] = new PointXY[hole_length];
                
                for (int j = 0; j < hole_length; j++) {
                    Napi::Object obj = hole.Get(j).As<Napi::Object>();
                    b_holes[i][j].x = obj.Get("x").As<Napi::Number>().DoubleValue();
                    b_holes[i][j].y = obj.Get("y").As<Napi::Number>().DoubleValue();
                }
            }
        }
    }
    
    // Call the core function
    NFPResult* result = calculate_nfp_raw(
        a_points, a_length,
        (const PointXY**)a_holes, a_hole_lengths, a_num_holes,
        b_points, b_length,
        (const PointXY**)b_holes, b_hole_lengths, b_num_holes
    );
    
    // Convert result to Node.js object
    Napi::Array result_list = Napi::Array::New(env);
    
    if (result != nullptr && result->num_polygons > 0 && result->polygons != nullptr) {
        for (int i = 0; i < result->num_polygons; i++) {
            Napi::Array pointlist = Napi::Array::New(env);
            
            if (result->polygons[i].points != nullptr && result->polygons[i].num_points > 0) {
                for (int j = 0; j < result->polygons[i].num_points; j++) {
                    Napi::Object p = Napi::Object::New(env);
                    p.Set("x", result->polygons[i].points[j].x);
                    p.Set("y", result->polygons[i].points[j].y);
                    pointlist.Set(static_cast<uint32_t>(j), p); // Cast to uint32_t to fix signed/unsigned comparison
                }
            }
            
            // Process holes
            Napi::Array children = Napi::Array::New(env);
            
            // Make sure holes pointer is valid before accessing
            if (result->polygons[i].holes != nullptr && result->polygons[i].num_holes > 0) {
                for (int k = 0; k < result->polygons[i].num_holes; k++) {
                    Napi::Array child = Napi::Array::New(env);
                    
                    if (result->polygons[i].holes[k].points != nullptr && result->polygons[i].holes[k].num_points > 0) {
                        for (int z = 0; z < result->polygons[i].holes[k].num_points; z++) {
                            Napi::Object c = Napi::Object::New(env);
                            c.Set("x", result->polygons[i].holes[k].points[z].x);
                            c.Set("y", result->polygons[i].holes[k].points[z].y);
                            child.Set(static_cast<uint32_t>(z), c); // Cast to uint32_t
                        }
                    }
                    
                    children.Set(static_cast<uint32_t>(k), child); // Cast to uint32_t
                }
            }
            
            pointlist.Set("children", children);
            result_list.Set(static_cast<uint32_t>(i), pointlist); // Cast to uint32_t
        }
    }
    
    // Free allocated memory
    free_nfp_result(result);
    
    delete[] a_points;
    if (a_num_holes > 0) {
        for (int i = 0; i < a_num_holes; i++) {
            delete[] a_holes[i];
        }
        delete[] a_holes;
        delete[] a_hole_lengths;
    }
    
    delete[] b_points;
    if (b_num_holes > 0) {
        for (int i = 0; i < b_num_holes; i++) {
            delete[] b_holes[i];
        }
        delete[] b_holes;
        delete[] b_hole_lengths;
    }
    
    return result_list;
}
#endif