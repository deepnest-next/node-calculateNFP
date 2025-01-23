#define BOOST_POLYGON_NO_DEPS

#include <iostream>
#include <string>
#include <sstream>
#include <limits>

#include <napi.h>
#include "polygon/polygon.hpp"

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
  if (ab == ae || bb == be)
    return;
  point first_a = *ab;
  point prev_a = *ab;
  std::vector<point> vec;
  polygon poly;
  ++ab;
  for (; ab != ae; ++ab) {
    point first_b = *bb;
    point prev_b = *bb;
    itrT2 tmpb = bb;
    ++tmpb;
    for (; tmpb != be; ++tmpb) {
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
  for (std::size_t i = 0; i < polygons.size(); ++i) {
    convolve_two_point_sequences(result, b, e, begin_points(polygons[i]), end_points(polygons[i]));
    for (polygon_with_holes_traits<polygon>::iterator_holes_type itrh = begin_holes(polygons[i]);
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
  for (std::size_t ai = 0; ai < a_polygons.size(); ++ai) {
    convolve_point_sequence_with_polygons(result, begin_points(a_polygons[ai]),
                                          end_points(a_polygons[ai]), b_polygons);
    for (polygon_with_holes_traits<polygon>::iterator_holes_type itrh = begin_holes(a_polygons[ai]);
         itrh != end_holes(a_polygons[ai]); ++itrh) {
      convolve_point_sequence_with_polygons(result, begin_points(*itrh),
                                            end_points(*itrh), b_polygons);
    }
    for (std::size_t bi = 0; bi < b_polygons.size(); ++bi) {
      polygon tmp_poly = a_polygons[ai];
      result.insert(convolve(tmp_poly, *(begin_points(b_polygons[bi]))));
      tmp_poly = b_polygons[bi];
      result.insert(convolve(tmp_poly, *(begin_points(a_polygons[ai]))));
    }
  }
}

double inputscale;

Napi::Value calculateNFP(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    // Verifica che ci siano abbastanza argomenti
    if (info.Length() < 1 || !info[0].IsObject()) {
        Napi::TypeError::New(env, "Un oggetto e' richiesto").ThrowAsJavaScriptException();
        return env.Null();
    }

    Napi::Object group = info[0].As<Napi::Object>();
    Napi::Object A = group.Get("A").As<Napi::Object>();
    Napi::Array A_points = A.Get("points").As<Napi::Array>();
    Napi::Array B = group.Get("B").As<Napi::Array>();

    // Verifica se il poligono A ha buchi
    bool hasHoles = group.Get("hasHoles").ToBoolean();
    Napi::Array holes;
    if (hasHoles) {
        holes = A.Get("children").As<Napi::Array>();
    } else {
        holes = Napi::Array::New(env); // Array vuoto se non ci sono buchi
    }

    polygon_set a, b, c;
    std::vector<polygon> polys;
    std::vector<point> pts;

    // Calcola inputscale
    double maxda = 1.0; // Valore predefinito
    inputscale = (0.1 * std::numeric_limits<int>::max()) / maxda;

    // Carica i punti di A
    for (unsigned int i = 0; i < A_points.Length(); i++) {
        Napi::Object obj = A_points.Get(i).As<Napi::Object>();
        int x = static_cast<int>(inputscale * obj.Get("x").ToNumber().DoubleValue());
        int y = static_cast<int>(inputscale * obj.Get("y").ToNumber().DoubleValue());
        pts.push_back(point(x, y));
    }

    polygon poly;
    boost::polygon::set_points(poly, pts.begin(), pts.end());
    a += poly;

    // Carica i buchi di A (se presenti)
    for (unsigned int i = 0; i < holes.Length(); i++) {
        Napi::Array hole = holes.Get(i).As<Napi::Array>();
        pts.clear();
        for (unsigned int j = 0; j < hole.Length(); j++) {
            Napi::Object obj = hole.Get(j).As<Napi::Object>();
            int x = static_cast<int>(inputscale * obj.Get("x").ToNumber().DoubleValue());
            int y = static_cast<int>(inputscale * obj.Get("y").ToNumber().DoubleValue());
            pts.push_back(point(x, y));
        }
        boost::polygon::set_points(poly, pts.begin(), pts.end());
        a -= poly; // Sottrae il buco dal poligono principale
    }

    // Carica i punti di B
    pts.clear();
    double xshift = 0, yshift = 0;
    for (unsigned int i = 0; i < B.Length(); i++) {
        Napi::Object obj = B.Get(i).As<Napi::Object>();
        int x = -static_cast<int>(inputscale * obj.Get("x").ToNumber().DoubleValue());
        int y = -static_cast<int>(inputscale * obj.Get("y").ToNumber().DoubleValue());
        pts.push_back(point(x, y));

        if (i == 0) {
            xshift = obj.Get("x").ToNumber().DoubleValue();
            yshift = obj.Get("y").ToNumber().DoubleValue();
        }
    }

    boost::polygon::set_points(poly, pts.begin(), pts.end());
    b += poly;

    // Calcola la convoluzione
    polys.clear();
    convolve_two_polygon_sets(c, a, b);
    c.get(polys);

    // Costruisci il risultato
    Napi::Array result_list = Napi::Array::New(env, polys.size());
    for (unsigned int i = 0; i < polys.size(); ++i) {
        Napi::Array pointlist = Napi::Array::New(env);
        int j = 0;

        for (auto itr = polys[i].begin(); itr != polys[i].end(); ++itr) {
            Napi::Object p = Napi::Object::New(env);
            p.Set("x", Napi::Number::New(env, ((double)(*itr).x()) / inputscale + xshift));
            p.Set("y", Napi::Number::New(env, ((double)(*itr).y()) / inputscale + yshift));
            pointlist.Set(j, p);
            j++;
        }

        // Aggiungi i buchi (se presenti)
        Napi::Array children = Napi::Array::New(env);
        int k = 0;
        for (auto itrh = polys[i].begin_holes(); itrh != polys[i].end_holes(); ++itrh) {
            Napi::Array child = Napi::Array::New(env);
            int z = 0;
            for (auto itr2 = (*itrh).begin(); itr2 != (*itrh).end(); ++itr2) {
                Napi::Object c = Napi::Object::New(env);
                c.Set("x", Napi::Number::New(env, ((double)(*itr2).x()) / inputscale + xshift));
                c.Set("y", Napi::Number::New(env, ((double)(*itr2).y()) / inputscale + yshift));
                child.Set(z, c);
                z++;
            }
            children.Set(k, child);
            k++;
        }

        pointlist.Set("children", children);
        result_list.Set(i, pointlist);
    }

    return result_list;
}