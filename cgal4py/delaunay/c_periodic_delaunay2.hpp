// TODO: 
// - Add support for argbitrary return objects so that dual can be added
#include <vector>
#include <set>
#include <array>
#include <utility>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <limits>
#include <stdint.h>
#ifdef READTHEDOCS
#define VALID 1
#include "dummy_CGAL.hpp"
#else
#include <CGAL/config.h>
#if (CGAL_VERSION_NR < 1040900900)
#define VALID 0
#include "dummy_CGAL.hpp"
#else
#define VALID 1
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Periodic_2_Delaunay_triangulation_2.h>
#include <CGAL/Periodic_2_triangulation_traits_2.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>
#include <CGAL/Periodic_2_triangulation_vertex_base_2.h>
#include <CGAL/squared_distance_2.h>
#include <CGAL/Unique_hash_map.h>
#endif
#endif

typedef CGAL::Exact_predicates_inexact_constructions_kernel         K;
typedef CGAL::Periodic_2_triangulation_traits_2<K>                  Gt;
typedef CGAL::Periodic_2_triangulation_vertex_base_2<Gt>            Vbb;
typedef CGAL::Periodic_2_triangulation_face_base_2<Gt>              Fb;

template <typename Info_>
class PeriodicDelaunay_with_info_2
{
public:
  typedef CGAL::Triangulation_vertex_base_with_info_2<Info_, Gt, Vbb> Vb;
  typedef CGAL::Triangulation_data_structure_2<Vb, Fb>                Tds;
  typedef CGAL::Periodic_2_Delaunay_triangulation_2<Gt, Tds>          Delaunay;
  typedef Info_ Info;
  typedef typename Delaunay::Point                     Point;
  typedef typename Delaunay::Segment                   Segment;
  typedef typename Delaunay::Triangle                  Triangle;
  typedef typename Delaunay::Vertex_handle             Vertex_handle;
  typedef typename Delaunay::Edge                      Edge_handle;
  typedef typename Delaunay::Face_handle               Face_handle;
  typedef typename Delaunay::Vertex_circulator         Vertex_circulator;
  typedef typename Delaunay::Edge_circulator           Edge_circulator;
  typedef typename Delaunay::Face_circulator           Face_circulator;
  typedef typename Delaunay::Vertex_iterator           Vertex_iterator;
  typedef typename Delaunay::Face_iterator             Face_iterator;
  typedef typename Delaunay::Edge_iterator             Edge_iterator;
  typedef typename Delaunay::Locate_type               Locate_type;
  typedef typename Delaunay::Iso_rectangle             Iso_rectangle;
  typedef typename Delaunay::Covering_sheets           Covering_sheets;
  typedef typename Delaunay::Offset                    Offset;
  typedef typename Delaunay::Periodic_point            Periodic_point;
  typedef typename Delaunay::Periodic_segment          Periodic_segment;
  typedef typename Delaunay::Periodic_triangle         Periodic_triangle;
  typedef typename CGAL::Unique_hash_map<Vertex_handle,int>  Vertex_hash;
  typedef typename CGAL::Unique_hash_map<Face_handle,int>    Face_hash;
  Delaunay T;
  bool updated = false;
  PeriodicDelaunay_with_info_2(const double *domain = NULL) {
    if (domain != NULL) 
      set_domain(domain);
  }
  PeriodicDelaunay_with_info_2(double *pts, Info *val, uint32_t n,
			       const double *domain = NULL) { 
    if (domain != NULL) 
      set_domain(domain);
    insert(pts, val, n); 
  }
  bool is_valid() const { return T.is_valid(); }
  void num_sheets(int32_t* ns_out) const {
    Covering_sheets ns_dim = T.number_of_sheets();
    int i;
    for (i = 0; i < 2; i++)
      ns_out[i] = (int32_t)(ns_dim[i]);
  }
  uint32_t num_sheets_total() const {
    Covering_sheets ns_dim = T.number_of_sheets();
    uint32_t ns = 1;
    int i;
    for (i = 0; i < 2; i++) 
      ns = ns*ns_dim[i];
    return ns;
  }
  uint32_t num_finite_verts() const { return static_cast<uint32_t>(T.number_of_vertices()); }
  uint32_t num_finite_edges() const { return static_cast<uint32_t>(T.number_of_edges()); }
  uint32_t num_finite_cells() const { return static_cast<uint32_t>(T.number_of_faces()); }
  uint32_t num_infinite_verts() const { return static_cast<uint32_t>(0); }
  uint32_t num_infinite_edges() const { return static_cast<uint32_t>(0); }
  uint32_t num_infinite_cells() const { return static_cast<uint32_t>(0); }
  uint32_t num_verts() const { return (num_finite_verts() + num_infinite_verts()); }
  uint32_t num_edges() const { return (num_finite_edges() + num_infinite_edges()); }
  uint32_t num_cells() const { return (num_finite_cells() + num_infinite_cells()); }
  uint32_t num_stored_verts() const { return static_cast<uint32_t>(T.number_of_stored_vertices()); }
  uint32_t num_stored_edges() const { return static_cast<uint32_t>(T.number_of_stored_edges()); }
  uint32_t num_stored_cells() const { return static_cast<uint32_t>(T.number_of_stored_faces()); }

  bool is_equal(const PeriodicDelaunay_with_info_2<Info> other) const {
    // Verts
    if (num_verts() != other.num_verts()) return false;
    if (num_finite_verts() != other.num_finite_verts()) return false;
    if (num_infinite_verts() != other.num_infinite_verts()) return false;
    // Cells
    if (num_cells() != other.num_cells()) return false;
    if (num_finite_cells() != other.num_finite_cells()) return false;
    if (num_infinite_cells() != other.num_infinite_cells()) return false;
    // Edges
    if (num_edges() != other.num_edges()) return false;
    if (num_finite_edges() != other.num_finite_edges()) return false;
    if (num_infinite_edges() != other.num_infinite_edges()) return false;
    return true;
  }

  class Vertex;
  class Edge;
  class Cell;

  void set_domain(const double *domain) {
    Iso_rectangle dr(domain[0], domain[1], domain[2], domain[3]);
    T.set_domain(dr);
  }
  void insert(double *pts, Info *val, uint32_t n)
  {
    if (n == 0) 
      return;
    updated = true;
    uint32_t i, j;
    std::vector< std::pair<Point,Info> > points;
    for (i = 0; i < n; i++) {
      j = 2*i;
      points.push_back( std::make_pair( Point(pts[j],pts[j+1]), val[i]) );
    }
    T.insert( points.begin(),points.end() );
  }
  void remove(Vertex v) { updated = true; T.remove(v._x); }
  void clear() { updated = true; T.clear(); }

  Vertex move(Vertex v, double *pos) {
    updated = true;
    Point p = Point(pos[0], pos[1]);
    return Vertex(T.move_point(v._x, p));
  }
  Vertex move_if_no_collision(Vertex v, double *pos) {
    updated = true;
    Point p = Point(pos[0], pos[1]);
    return Vertex(T.move_if_no_collision(v._x, p));
  }

  Vertex get_vertex(Info index) const {
    Vertex_iterator it = T.vertices_begin();
    for ( ; it != T.vertices_end(); it++) {
      if (it->info() == index)
    	return Vertex(T.get_original_vertex(it));
    }
    return Vertex();
  }

  Cell locate(double* pos, int& lt, int& li) const {
    Point p = Point(pos[0], pos[1]);
    Locate_type lt_out = Locate_type(0);
    Cell out = Cell(T.locate(p, lt_out, li));
    lt = (int)lt_out;
    return out;
  }
  Cell locate(double* pos, int& lt, int& li, Cell c) const {
    Point p = Point(pos[0], pos[1]);
    Locate_type lt_out = Locate_type(0);
    Cell out = Cell(T.locate(p, lt_out, li, c._x));
    lt = (int)lt_out;
    return out;
  }

  bool has_offset(Vertex v) const {
    Offset o = T.get_offset(v._x);
    if ((o.x() == 1) or (o.y() == 1)) 
      return true;
    else
      return false;
  }
  bool has_offset(Edge e) const {
    for (int i = 0; i < 2; i++) {
      if (has_offset(e.vertex(i))) {
	return true;
      }
    }
    return false;
  }
  bool has_offset(Cell c) const {
    for (int i = 0; i < 3; i++) {
      if (has_offset(c.vertex(i))) {
	return true;
      }
    }
    return false;
  }
  void point(Vertex v, double* pos) const {
    Point p = T.point(v._x);
    pos[0] = p.x();
    pos[1] = p.y();
  }
  void point(Edge e, int i, double* pos) const {
    Point p = T.segment(e._x).vertex(i%2);
    pos[0] = p.x();
    pos[1] = p.y();
  }
  void point(Cell c, int i, double* pos) const {
    Point p = T.triangle(c._x).vertex(i%3);
    pos[0] = p.x();
    pos[1] = p.y();
  }
  void periodic_point(Vertex v, double* pos) const {
    Point p = v._x->point();
    pos[0] = p.x();
    pos[1] = p.y();
  }
  void periodic_point(Edge e, int i, double* pos) const {
    Point p = T.periodic_segment(e._x)[i%2].first;
    pos[0] = p.x();
    pos[1] = p.y();
  }
  void periodic_point(Cell c, int i, double* pos) const {
    Point p = T.periodic_triangle(c._x)[i%3].first;
    pos[0] = p.x();
    pos[1] = p.y();
  }
  void periodic_offset(Vertex v, int32_t* off) const {
    Offset o = T.get_offset(v._x);
    // Offset o = v._x->offset(); // stored vertex offset (unset unless copied)
    off[0] = o.x();
    off[1] = o.y();
  }
  void periodic_offset(Edge e, int i, int32_t* off) const {
    Offset o = T.periodic_segment(e._x)[i%2].second;
    off[0] = o.x();
    off[1] = o.y();
  }
  void periodic_offset(Cell c, int i, int32_t* off) const {
    // Offset o = T.int_to_off(c._x->offset(i)); // just cell offset
    // Offset o = T.get_offset(c.vertex(i)._x); // just vertex offset
    // Offset o = T.get_offset(c._x, i); // combined cell and vertex offset
    Offset o = T.periodic_triangle(c._x)[i%3].second;
    off[0] = o.x();
    off[1] = o.y();
  }


  template <typename Wrap, typename Wrap_handle>
  class wrap_insert_iterator
  {
  protected:
    std::vector<Wrap>* container;

  public:
    explicit wrap_insert_iterator (std::vector<Wrap>& x) : container(&x) {}
    wrap_insert_iterator& operator= (Wrap_handle value) {
      Wrap value_wrap = Wrap();
      value_wrap._x = value;
      container->push_back(value_wrap);
      return *this;
    }
    wrap_insert_iterator& operator* ()
    { return *this; }
    wrap_insert_iterator& operator++ ()
    { return *this; }
    wrap_insert_iterator operator++ (int)
    { return *this; }
  };

  class All_verts_iter {
  public:
    Vertex_iterator _x = Vertex_iterator();
    All_verts_iter() { _x = Vertex_iterator(); }
    All_verts_iter(Vertex_iterator x) { _x = x; }
    All_verts_iter& operator*() { return *this; }
    All_verts_iter& operator++() {
      _x++;
      return *this;
    }
    All_verts_iter& operator--() {
      _x--;
      return *this;
    }
    bool operator==(All_verts_iter other) const { return (_x == other._x); }
    bool operator!=(All_verts_iter other) const { return (_x != other._x); }
  };
  All_verts_iter all_verts_begin() const { return All_verts_iter(T.vertices_begin()); }
  All_verts_iter all_verts_end() const { return All_verts_iter(T.vertices_end()); }

  class Vertex {
  public:
    Vertex_handle _x = Vertex_handle();
    Vertex() { _x = Vertex_handle(); }
    Vertex(Vertex_handle x) { _x = x; }
    Vertex(All_verts_iter x) { _x = static_cast<Vertex_handle>(x._x); }
    bool operator==(Vertex other) const { return (_x == other._x); }
    bool operator!=(Vertex other) const { return (_x != other._x); }
    void point(double* out) const {
      Point p = _x->point();
      out[0] = p.x();
      out[1] = p.y();
    }
    void offset(int32_t* out) const {
      Offset o = _x->offset();
      out[0] = o.x();
      out[1] = o.y();
    }
    Info info() const { return _x->info(); }
    Cell cell() const { return Cell(_x->face()); }
    void set_cell(Cell c) { _x->set_face(c._x); }
    void set_point(double* x) {
      Point p = Point(x[0], x[1]);
      _x->set_point(p);
    }
    void set_offset(int32_t* x) {
      Offset o = Offset(x[0], x[1]);
      _x->set_offset(o);
    }
  };

  class All_edges_iter {
  public:
    Edge_iterator _x = Edge_iterator();
    All_edges_iter() { _x = Edge_iterator(); }
    All_edges_iter(Edge_iterator x) { _x = x; }
    All_edges_iter& operator*() { return *this; }
    All_edges_iter& operator++() {
      _x++;
      return *this;
    }
    All_edges_iter& operator--() {
      _x--;
      return *this;
    }
    bool operator==(All_edges_iter other) const { return (_x == other._x); }
    bool operator!=(All_edges_iter other) const { return (_x != other._x); }
  };
  All_edges_iter all_edges_begin() const { return All_edges_iter(T.edges_begin()); }
  All_edges_iter all_edges_end() const { return All_edges_iter(T.edges_end()); }

  class Edge {
  public:
    Edge_handle _x = Edge_handle();
    Edge() {}
    Edge(Edge_handle x) { _x = x; }
    Edge(Edge_iterator x) { _x = Edge_handle(x->first, x->second); } 
    Edge(Edge_circulator x) { _x = Edge_handle(x->first, x->second); }
    Edge(All_edges_iter x) { _x = Edge_handle(x._x->first, x._x->second); }
    Edge(Cell x, int i) { _x = Edge_handle(x._x, i); }
    Cell cell() const { return Cell(_x.first); }
    int ind() const { return _x.second; }
    Vertex_handle _v1() const { return _x.first->vertex((_x.second+2)%3); }
    Vertex_handle _v2() const { return _x.first->vertex((_x.second+1)%3); }
    Vertex v1() const { return Vertex(_v1()); }
    Vertex v2() const { return Vertex(_v2()); }
    Vertex vertex(int i) const { 
      if ((i%2) == 0)
	return v1();
      else
	return v2();
    }
    bool operator==(Edge other) { 
      Vertex_handle x1 = _v1(), x2 = _v2();
      Vertex_handle o1 = other._v1(), o2 = other._v2();
      if ((x1 == o1) && (x2 == o2))
    	return true;
      else if ((x1 == o2) && (x2 == o1))
    	return true;
      else
    	return false;
    }
    bool operator!=(Edge other) { 
      Vertex_handle x1 = _v1(), x2 = _v2();
      Vertex_handle o1 = other._v1(), o2 = other._v2();
      if (x1 == o1) {
    	if (x2 != o2)
    	  return true;
    	else
    	  return false;
      } else if (x1 == o2) {
    	if (x2 != o1)
    	  return true;
    	else
    	  return false;
      } else
    	return true;
    }
  };


  // Cell constructs
  class All_cells_iter {
  public:
    Face_iterator _x = Face_iterator();
    All_cells_iter() { _x = Face_iterator(); }
    All_cells_iter(Face_iterator x) { _x = x; }
    All_cells_iter& operator*() { return *this; }
    All_cells_iter& operator++() {
      _x++;
      return *this;
    }
    All_cells_iter& operator--() {
      _x--;
      return *this;
    }
    bool operator==(All_cells_iter other) const { return (_x == other._x); }
    bool operator!=(All_cells_iter other) const { return (_x != other._x); }
  };
  All_cells_iter all_cells_begin() const { return All_cells_iter(T.faces_begin()); }
  All_cells_iter all_cells_end() const { return All_cells_iter(T.faces_end()); }

  class Cell {
  public:
    Face_handle _x = Face_handle();
    Cell() { _x = Face_handle(); }
    Cell(Face_handle x) { _x = x; }
    Cell(All_cells_iter x) { _x = static_cast<Face_handle>(x._x); }
    Cell(Vertex v1, Vertex v2, Vertex v3) { _x = Face_handle(v1._x, v2._x, v3._x); }
    Cell(Vertex v1, Vertex v2, Vertex v3, Cell c1, Cell c2, Cell c3) {
      _x = Face_handle(v1._x, v2._x, v3._x, c1._x, c2._x, c3._x); }
    bool operator==(Cell other) const { return (_x == other._x); }
    bool operator!=(Cell other) const { return (_x != other._x); }

    Vertex vertex(int i) const { return Vertex(_x->vertex(i)); }
    bool has_vertex(Vertex v) const { return _x->has_vertex(v._x); }
    bool has_vertex(Vertex v, int *i) const { return _x->has_vertex(v._x, *i); }
    int ind(Vertex v) const { return _x->index(v._x); }

    Cell neighbor(int i) const { return Cell(_x->neighbor(i)); }
    bool has_neighbor(Cell c) const { return _x->has_neighbor(c._x); }
    bool has_neighbor(Cell c, int *i) const { return _x->has_neighbor(c._x, *i); }
    int ind(Cell c) const { return _x->index(c._x); }

    void set_vertex(int i, Vertex v) { _x->set_vertex(i, v._x); }
    void set_vertices() { _x->set_vertices(); }
    void set_vertices(Vertex v1, Vertex v2, Vertex v3) {
      _x->set_vertices(v1._x, v2._x, v3._x); }
    void set_neighbor(int i, Cell c) { _x->set_neighbor(i, c._x); }
    void set_neighbors() { _x->set_neighbors(); }
    void set_neighbors(Cell c1, Cell c2, Cell c3) {
      _x->set_neighbors(c1._x, c2._x, c3._x); }

    void reorient() { _x->reorient(); }
    void ccw_permute() { _x->ccw_permute(); }
    void cw_permute() { _x->cw_permute(); }

    int dimension() const { return _x->dimension(); }
  };


  // Check if construct is incident to the infinite vertex
  bool is_infinite(Vertex x) const { return false; }
  bool is_infinite(Edge x) const { return false; }
  bool is_infinite(Cell x) const { return false; }
  bool is_infinite(All_verts_iter x) const { return false; }
  bool is_infinite(All_edges_iter x) const { return false; }
  bool is_infinite(All_cells_iter x) const { return false; }

  bool is_edge(Vertex x1, Vertex x2) const { return T.is_edge(x1._x, x2._x); }
  bool is_edge(Vertex x1, Vertex x2, Cell& c, int& i) const { return T.is_edge(x1._x, x2._x, c._x, i); }
  bool is_cell(Vertex x1, Vertex x2, Vertex x3) const { return T.is_face(x1._x, x2._x, x3._x); }
  bool is_cell(Vertex x1, Vertex x2, Vertex x3, Cell& c) const { return T.is_face(x1._x, x2._x, x3._x, c._x); }

  // Constructs incident to a vertex
  std::vector<Vertex> incident_vertices(Vertex x) const {
    std::vector<Vertex> out;
    Vertex_circulator vc = T.adjacent_vertices(x._x), done(vc);
    if (vc == 0)
      return out;
    do {
      out.push_back(Vertex(static_cast<Vertex_handle>(vc)));
    } while (++vc != done);
    return out;
  }
  std::vector<Edge> incident_edges(Vertex x) const {
    std::vector<Edge> out;
    Edge_circulator ec = T.incident_edges(x._x), done(ec);
    if (ec == 0)
      return out;
    do {
      out.push_back(Edge(ec));
    } while (++ec != done);
    return out;
  }
  std::vector<Cell> incident_cells(Vertex x) const {
    std::vector<Cell> out;
    Face_circulator fc = T.incident_faces(x._x), done(fc);
    if (fc == 0)
      return out;
    do {
      out.push_back(Cell(static_cast<Face_handle>(fc)));
    } while (++fc != done);
    return out;
  }

  // Constructs incident to an edge
  std::vector<Vertex> incident_vertices(Edge x) const {
    std::vector<Vertex> out;
    out.push_back(x.v1());
    out.push_back(x.v2());
    return out;
  }
  std::vector<Edge> incident_edges(Edge x) const {
    uint32_t i;
    std::vector<Edge> out1, out2, out;
    out1 = incident_edges(x.v1());
    out2 = incident_edges(x.v2());
    for (i = 0; i < out1.size(); i++) {
      if (out1[i] != x)
  	out.push_back(out1[i]);
    }
    for (i = 0; i < out2.size(); i++) {
      if (out2[i] != x)
  	out.push_back(out2[i]);
    }
    return out;
  }
  std::vector<Cell> incident_cells(Edge x) const {
    uint32_t i;
    std::vector<Cell> out1, out2;
    Vertex v1 = x.v1(), v2 = x.v2();
    out1 = incident_cells(v1);
    out2 = incident_cells(v2);
    for (i = 0; i < out2.size(); i++) {
      if (!out2[i].has_vertex(v1))
  	out1.push_back(out2[i]);
    }
    return out1;
  }

  // Constructs incident to a cell
  std::vector<Vertex> incident_vertices(Cell x) const {
    uint32_t i;
    std::vector<Vertex> out;
    for (i = 0; i < 3; i++) 
      out.push_back(x.vertex(i));
    return out;
  }
  std::vector<Edge> incident_edges(Cell x) const {
    // uint32_t i1, i2;
    std::vector<Edge> out;
    for (int i = 0; i < 3; i++)
      out.push_back(Edge(x, i));
    return out;
  }
  std::vector<Cell> incident_cells(Cell x) const {
    uint32_t i;
    std::vector<Cell> out;
    for (i = 0; i < 3; i++)
      out.push_back(x.neighbor(i));
    return out;
  }

  Vertex nearest_vertex(double* pos) const {
    Point p = Point(pos[0], pos[1]);
    Vertex out = Vertex(T.nearest_vertex(p));
    return out;
  }

  int mirror_index(Cell x, int i) const { return T.mirror_index(x._x, i); }
  Vertex mirror_vertex(Cell x, int i) const { return Vertex(T.mirror_vertex(x._x, i)); }

  void circumcenter(Cell x, double* out) const {
    Point p = T.circumcenter(x._x);
    out[0] = p.x();
    out[1] = p.y();
  }

  double dual_area(const Vertex v) const {
    Face_circulator fstart = T.incident_faces(v._x);
    Face_circulator fcit = fstart;
    std::vector<Point> pts;
    pts.push_back(T.circumcenter(static_cast<Face_handle>(fstart)));
    fcit++;
    for ( ; fcit != fstart; fcit++) {
      Point dual = T.circumcenter(fcit);
      pts.push_back(dual);
    }
    pts.push_back(T.circumcenter(fstart));
    double vol = 0.0;
    Point orig = T.point(v._x);
    // Point orig = v._x->point();
    for (uint32_t i=0 ; i<pts.size()-1 ; i++) {
      vol = vol + Triangle(orig,pts[i],pts[i+1]).area();
    }

    return vol;
  }

  void dual_areas(double* vols) const {
    Vertex_iterator it = T.vertices_begin();
    for ( ; it != T.vertices_end(); it++) {
      vols[it->info()] = dual_area(Vertex(it));
    }
  }

  double length(const Edge e) const {
    Segment s = T.segment(e._x);
    double out = std::sqrt(s.squared_length());
    // Vertex_handle v1 = e._v1(), v2 = e._v2();
    // Point p1 = T.point(v1); // v1->point();
    // Point p2 = T.point(v2); // v2->point();
    // double out = std::sqrt(static_cast<double>(CGAL::squared_distance(p1, p2)));
    return out;
  }

  bool flip(Cell x, int i) { 
    T.flippable(x._x, i); 
    return true;
  }
  bool flip(Edge x) {
    T.flippable(x.cell()._x, x.ind());
    return true;
  }
  // for completeness with 3D case
  void flip_flippable(Cell x, int i) { 
    T.flippable(x._x, i); 
  }
  void flip_flippable(Edge x) { 
    T.flippable(x.cell()._x, x.ind()); 
  }

  std::vector<Edge> get_boundary_of_conflicts(double* pos, Cell start) const {
    std::vector<Edge> out;
    Point p = Point(pos[0], pos[1]);
    T.get_boundary_of_conflicts(p, wrap_insert_iterator<Edge,Edge_handle>(out), 
  				start._x);
    return out;
  }
  std::vector<Cell> get_conflicts(double* pos, Cell start) const {
    std::vector<Cell> out;
    Point p = Point(pos[0], pos[1]);
    T.get_conflicts(p, wrap_insert_iterator<Cell,Face_handle>(out), start._x);
    return out;
  }
  std::pair<std::vector<Cell>,std::vector<Edge>>
    get_conflicts_and_boundary(double* pos, Cell start) const {
    std::pair<std::vector<Cell>,std::vector<Edge>> out;
    std::vector<Cell> fit;
    std::vector<Edge> eit;
    Point p = Point(pos[0], pos[1]);
    T.get_conflicts_and_boundary(p, 
  				 wrap_insert_iterator<Cell,Face_handle>(fit), 
  				 wrap_insert_iterator<Edge,Edge_handle>(eit), 
  				 start._x);
    out = std::make_pair(fit, eit);
    return out;
  }

  int oriented_side(Cell f, const double* pos) const {
    if (std::isinf(pos[0]) || std::isinf(pos[1])) {
      return 1;
    } else {
      Point p = Point(pos[0], pos[1]);
      return (int)T.oriented_side(f._x, p);
    }
  }

  int side_of_oriented_circle(Cell f, const double* pos) const {
    if (std::isinf(pos[0]) || std::isinf(pos[1])) {
      return 1;
    } else {
      Point p = Point(pos[0], pos[1]);
      return (int)T.side_of_oriented_circle(f._x, p);
    }
  }

  void write_to_file(const char* filename) const
  {
    std::ofstream os(filename, std::ios::binary);
    if (!os) std::cerr << "Error cannot create file: " << filename << std::endl;
    else {
      std::streambuf* oldCoutStreamBuf = std::cout.rdbuf();
      std::ostringstream newCoutStream;
      std::cout.rdbuf( newCoutStream.rdbuf() );
      T.save(os);
      std::cout.rdbuf( oldCoutStreamBuf );
      os.close();
    }
  }

  void read_from_file(const char* filename)
  {
    std::ifstream is(filename, std::ios::binary);
    if (!is) std::cerr << "Error cannot open file: " << filename << std::endl;
    else {
      updated = true;
      std::streambuf* oldCoutStreamBuf = std::cout.rdbuf();
      std::ostringstream newCoutStream;
      std::cout.rdbuf( newCoutStream.rdbuf() );
      T.load(is);
      std::cout.rdbuf( oldCoutStreamBuf );
      is.close();
    }
  }

  template <typename I>
  I serialize(I &n, I &m, int32_t &d, double* domain, int32_t* cover,
	      double* vert_pos, Info* vert_info, 
	      I* faces, I* neighbors, int32_t* offsets) const
  {
    I idx_inf = std::numeric_limits<I>::max();
    Delaunay T1 = T;
    T1.convert_to_1_sheeted_covering();
    int j;

    // Header
    Iso_rectangle dom_rect = T1.domain();
    Covering_sheets ns_dim = T.number_of_sheets();
    n = static_cast<I>(T1.number_of_vertices());
    m = static_cast<I>(T1.number_of_faces());
    d = static_cast<I>(T1.dimension());
    domain[0] = dom_rect.xmin();
    domain[1] = dom_rect.ymin();
    domain[2] = dom_rect.xmax();
    domain[3] = dom_rect.ymax();
    for (j = 0; j < d; j++)
      cover[j] = (int32_t)(ns_dim[j]);
    int dim = (d == -1 ? 1 :  d + 1);
    if ((n == 0) || (m == 0)) {
      return idx_inf;
    }

    Face_hash F;
    Vertex_hash V;
    Vertex_handle vit;
    int inum;

    // vertices
    inum = 0;
    for (Vertex_iterator vit = T1.vertices_begin(); vit != T1.vertices_end(); vit++) {
      vert_pos[d*inum + 0] = static_cast<double>(vit->point().x());
      vert_pos[d*inum + 1] = static_cast<double>(vit->point().y());
      vert_info[inum] = vit->info();
      V[vit] = inum++;
    }
      
    // vertices of the faces
    inum = 0;
    for (Face_iterator ib = T1.faces_begin();
	 ib != T1.faces_end(); ++ib) {
      for (j = 0; j < dim ; ++j) {
	vit = ib->vertex(j);
	faces[dim*inum + j] = V[vit];
      }
      F[ib] = inum++;
    }
  
    // neighbor pointers of the faces
    inum = 0;
    for (Face_iterator ib = T1.faces_begin();
	 ib != T1.faces_end(); ++ib) {
      for (j = 0; j < d+1; ++j){
	neighbors[(d+1)*inum + j] = F[ib->neighbor(j)];
      }
      inum++;
    }

    // offsets of face vertices
    inum = 0;
    for (Face_iterator ib = T1.faces_begin();
	 ib != T1.faces_end(); ++ib) {
      for (j = 0; j < d+1; ++j){
	offsets[(d+1)*inum + j] = ib->offset(j);
      }
      inum++;
    }

    return idx_inf;
  }

  template <typename I>
  Info serialize_idxinfo(I &n, I &m, int32_t &d, double* domain, int32_t* cover,
			 Info* faces, I* neighbors, int32_t *offsets) const
  {
    Info idx_inf = std::numeric_limits<Info>::max();
    int j;
    Delaunay T1 = T;
    T1.convert_to_1_sheeted_covering();

    // Header
    Iso_rectangle dom_rect = T1.domain();
    Covering_sheets ns_dim = T.number_of_sheets();
    n = static_cast<I>(T1.number_of_vertices());
    m = static_cast<I>(T1.number_of_faces());
    d = static_cast<I>(T1.dimension());
    domain[0] = dom_rect.xmin();
    domain[1] = dom_rect.ymin();
    domain[2] = dom_rect.xmax();
    domain[3] = dom_rect.ymax();
    for (j = 0; j < d; j++)
      cover[j] = (int32_t)(ns_dim[j]);
    int dim = (d == -1 ? 1 :  d + 1);
    if ((n == 0) || (m == 0)) {
      return idx_inf;
    }

    Face_hash F;
    Vertex_handle vit;
    int inum;

    // vertices of the faces
    inum = 0;
    for (Face_iterator ib = T1.faces_begin();
	 ib != T1.faces_end(); ++ib) {
      for (j = 0; j < dim ; ++j) {
	vit = ib->vertex(j);
	faces[dim*inum + j] = vit->info();
      }
      F[ib] = inum++;
    }
  
    // neighbor pointers of the faces
    inum = 0;
    for (Face_iterator ib = T1.faces_begin();
	 ib != T1.faces_end(); ++ib) {
      for (j = 0; j < d+1; ++j){
	neighbors[(d+1)*inum + j] = F[ib->neighbor(j)];
      }
      inum++;
    }

    // offsets of face vertices
    inum = 0;
    for (Face_iterator ib = T1.faces_begin();
	 ib != T1.faces_end(); ++ib) {
      for (j = 0; j < d+1; ++j){
	offsets[(d+1)*inum + j] = ib->offset(j);
      }
      inum++;
    }

    return idx_inf;
  }

  template <typename I>
  I serialize_info2idx(I &n, I &m, int32_t &d,
		       double* domain, int32_t* cover,
		       I* faces, I* neighbors, int32_t* offsets,
		       Info max_info, I* idx) const
  {
    I idx_inf = std::numeric_limits<I>::max();
    int j;
    Delaunay T1 = T;
    T1.convert_to_1_sheeted_covering();

    // Header
    Iso_rectangle dom_rect = T1.domain();
    Covering_sheets ns_dim = T.number_of_sheets();
    n = static_cast<I>(T1.number_of_vertices());
    d = static_cast<I>(T1.dimension());
    domain[0] = dom_rect.xmin();
    domain[1] = dom_rect.ymin();
    domain[2] = dom_rect.xmax();
    domain[3] = dom_rect.ymax();
    for (j = 0; j < d; j++)
      cover[j] = (int32_t)(ns_dim[j]);
    int dim = (d == -1 ? 1 :  d + 1);
    if ((n == 0) || (m == 0)) {
      return idx_inf;
    }

    Face_hash F;
    Vertex_handle vit;
    int inum, inum_tot;

    // vertices of the faces
    bool *include_face = (bool*)malloc(m*sizeof(bool));
    inum = 0, inum_tot = 0;
    for (Face_iterator ib = T1.faces_begin();
	 ib != T1.faces_end(); ++ib) {
      include_face[inum_tot] = false;
      for (j = 0; j < dim ; ++j) {
	vit = ib->vertex(j);
	if (vit->info() < max_info) {
	  include_face[inum_tot] = true;
	  break;
	}
      }
      if (include_face[inum_tot]) {
	for (j = 0; j < dim ; ++j) {
	  vit = ib->vertex(j);
	  faces[dim*inum + j] = idx[vit->info()];
	}
	F[ib] = inum++;
      } else {
	F[ib] = idx_inf;
      }
      inum_tot++;
    }
    m = inum;

    // neighbor pointers of the faces
    inum = 0, inum_tot = 0;
    for (Face_iterator ib = T1.faces_begin();
	 ib != T1.faces_end(); ++ib) {
      if (include_face[inum_tot]) {
	for (j = 0; j < d+1; ++j){
	  neighbors[(d+1)*inum + j] = F[ib->neighbor(j)];
	}
	inum++;
      }
      inum_tot++;
    }

    // offsets of face vertices
    inum = 0;
    for (Face_iterator ib = T1.faces_begin();
	 ib != T1.faces_end(); ++ib) {
      if (include_face[inum_tot]) {
	for (j = 0; j < d+1; ++j){
	  offsets[(d+1)*inum + j] = ib->offset(j);
	}
	inum++;
      }
      inum_tot++;
    }
  
    free(include_face);
    return idx_inf;
  }

  template <typename I>
  void deserialize(I n, I m, int32_t d, double* domain, int32_t *cover,
		   double* vert_pos, Info* vert_info, 
		   I* faces, I* neighbors, int32_t* offsets, I idx_inf)
  {
    updated = true;

    T.clear();
  
    if (n==0) {
      return;
    }

    Iso_rectangle dom_rect(domain[0],domain[1],domain[2],domain[3]);
    T.convert_to_1_sheeted_covering();
    T.set_domain(dom_rect);
    T.tds().set_dimension(d);

    int32_t ns = cover[0]*cover[1];

    std::vector<Vertex_handle> V(n);
    std::vector<Face_handle> F(m);
    Vertex_handle v;
    I index;
    int dim = (d == -1 ? 1 : d + 1);
    I i;
    int j;

    // Create vertices
    for(i = 0; i < n; ++i) {
      V[i] = T.tds().create_vertex();
      V[i]->point() = Point(vert_pos[d*i], vert_pos[d*i + 1]);
      V[i]->info() = vert_info[i];
    }

    // First face
    i = 0;
    if (T.faces_begin() != T.faces_end()) {
      F[i] = T.faces_begin();
      for (j = 0; j < dim; ++j) {
	index = faces[dim*i + j];
	v = V[index];
	F[i]->set_vertex(j, v);
	v->set_face(F[i]);
      }
      i++;
    }

    // Creation of the faces
    for( ; i < m; ++i) {
      F[i] = T.tds().create_face() ;
      for(j = 0; j < dim ; ++j){
	index = faces[dim*i + j];
	v = V[index];
	F[i]->set_vertex(j, v);
	// The face pointer of vertices is set too often,
	// but otherwise we had to use a further map 
	v->set_face(F[i]);
      }
    }

    // Setting the neighbor pointers
    for(i = 0; i < m; ++i) {
      for(j = 0; j < d+1; ++j){
	index = neighbors[(d+1)*i + j];
	F[i]->set_neighbor(j, F[index]);
      }
    }

    // Setting the offset of vertices
    for (i = 0; i < m; ++i) {
      T.set_offsets(F[i],
		    offsets[(d+1)*i],
		    offsets[(d+1)*i + 1],
		    offsets[(d+1)*i + 2]);
    }

    // Restore to 9 sheet covering if necessary
    if (ns == 9) {
      T.convert_to_9_sheeted_covering();
    }
  }

  template <typename I>
  void deserialize_idxinfo(I n, I m, int32_t d, 
			   double* domain, int32_t *cover, double* vert_pos,
			   I* faces, I* neighbors, int32_t* offsets, I idx_inf)
  {
    updated = true;

    T.clear();
  
    if (n==0) {
      return;
    }

    Iso_rectangle dom_rect(domain[0],domain[1],domain[2],domain[3]);
    T.convert_to_1_sheeted_covering();
    T.set_domain(dom_rect);
    T.tds().set_dimension(d);

    int32_t ns = cover[0]*cover[1];

    std::vector<Vertex_handle> V(n);
    std::vector<Face_handle> F(m);
    Vertex_handle v;
    I index;
    int dim = (d == -1 ? 1 : d + 1);
    I i;
    int j;

    // Create vertices
    for(i = 0; i < n; ++i) {
      V[i] = T.tds().create_vertex();
      V[i]->point() = Point(vert_pos[d*i], vert_pos[d*i + 1]);
      V[i]->info() = (Info)(i);
    }

    // First face
    i = 0;
    if (T.faces_begin() != T.faces_end()) {
      F[i] = T.faces_begin();
      for (j = 0; j < dim; ++j) {
	index = faces[dim*i + j];
	v = V[index];
	F[i]->set_vertex(j, v);
	v->set_face(F[i]);
      }
      i++;
    }
    
    // Creation of the faces
    for( ; i < m; ++i) {
      F[i] = T.tds().create_face() ;
      for(j = 0; j < dim ; ++j){
	index = faces[dim*i + j];
	v = V[index];
	F[i]->set_vertex(j, v);
	// The face pointer of vertices is set too often,
	// but otherwise we had to use a further map 
	v->set_face(F[i]);
      }
    }

    // Setting the neighbor pointers
    for(i = 0; i < m; ++i) {
      for(j = 0; j < d+1; ++j){
	index = neighbors[(d+1)*i + j];
	F[i]->set_neighbor(j, F[index]);
      }
    }

    // Setting the offset of vertices
    for (i = 0; i < m; ++i) {
      T.set_offsets(F[i],
		    offsets[(d+1)*i],
		    offsets[(d+1)*i + 1],
		    offsets[(d+1)*i + 2]);
    }

    // Restore to 9 sheet covering if necessary
    if (ns == 9) {
      T.convert_to_9_sheeted_covering();
    }
  }

  void info_ordered_vertices(double* pos) const {
    Info i;
    Point p;
    Vertex_handle vh1, vh2;
    for (Vertex_iterator it = T.vertices_begin(); it != T.vertices_end(); it++) {
      i = it->info();
      p = it->point();
      pos[2*i + 0] = p.x();
      pos[2*i + 1] = p.y();
    }
  }

  void vertex_info(Info* verts) const {
    int i = 0;
    for (Vertex_iterator it = T.vertices_begin(); it != T.vertices_end(); it++) {
      verts[i] = it->info();
      i++;
    }
  }

  void edge_info(Info* edges) const {
    int i = 0;
    Info i1, i2;
    for (Edge_iterator it = T.edges_begin(); it != T.edges_end(); it++) {
      i1 = it->first->vertex(T.cw(it->second))->info();
      i2 = it->first->vertex(T.ccw(it->second))->info();
      edges[2*i + 0] = i1;
      edges[2*i + 1] = i2;
      i++;
    }
  }

  bool intersect_sph_box(Point *c, double r, double *le, double *re) const {
    // x 
    if (c->x() < le[0]) {
      if ((c->x() + r) < le[0])
	return false;
    } else if (c->x() > re[0]) {
      if ((c->x() - r) > re[0])
	return false;
    }
    // y
    if (c->y() < le[1]) {
      if ((c->y() + r) < le[1])
	return false;
    } else if (c->y() > re[1]) {
      if ((c->y() - r) > re[1])
	return false;
    }
    return true;
  }

  std::vector<std::vector<Info>> outgoing_points(uint64_t nbox,
						 double *left_edges, 
						 double *right_edges) const {
    std::vector<std::vector<Info>> out;
    uint64_t b;
    for (b = 0; b < nbox; b++) 
      out.push_back(std::vector<Info>());

    Vertex_handle v;
    Point cc, p1;
    double cr;
    int i;

    for (Face_iterator it = T.faces_begin(); it != T.faces_end(); it++) {
      p1 = T.point(it->vertex(0));
      // p1 = it->vertex(0)->point();
      cc = T.circumcenter(it);
      cr = std::sqrt(static_cast<double>(CGAL::squared_distance(p1, cc)));
      for (b = 0; b < nbox; b++) {
	if (intersect_sph_box(&cc, cr, left_edges + 2*b, right_edges + 2*b))
	  for (i = 0; i < 3; i++) out[b].push_back((it->vertex(i))->info());
      }
    }
    for (b = 0; b < nbox; b++) {
      std::sort( out[b].begin(), out[b].end() ); 
      out[b].erase( std::unique( out[b].begin(), out[b].end() ), out[b].end() );
    }

    return out;
  }

  void boundary_points(double *left_edge, double *right_edge, bool periodic,
                       std::vector<Info>& lx, std::vector<Info>& ly,
                       std::vector<Info>& rx, std::vector<Info>& ry,
                       std::vector<Info>& alln) const
  {
    Vertex_handle v;
    // Info hv;
    Point cc, p1;
    double cr;
    int i;

    for (Face_iterator it = T.faces_begin(); it != T.faces_end(); it++) {
      p1 = T.point(it->vertex(0));
      // p1 = it->vertex(0)->point();
      cc = T.circumcenter(it);
      cr = std::sqrt(static_cast<double>(CGAL::squared_distance(p1, cc)));
      if ((cc.x() + cr) >= right_edge[0])
	for (i = 0; i < 3; i++) rx.push_back((it->vertex(i))->info());
      if ((cc.y() + cr) >= right_edge[1])
	for (i = 0; i < 3; i++) ry.push_back((it->vertex(i))->info());
      if ((cc.x() - cr) < left_edge[0])
	for (i = 0; i < 3; i++) lx.push_back((it->vertex(i))->info());
      if ((cc.y() - cr) < left_edge[1])
	for (i = 0; i < 3; i++) ly.push_back((it->vertex(i))->info());
    }
    std::sort( alln.begin(), alln.end() );
    std::sort( lx.begin(), lx.end() );
    std::sort( ly.begin(), ly.end() );
    std::sort( rx.begin(), rx.end() );
    std::sort( ry.begin(), ry.end() );
    alln.erase( std::unique( alln.begin(), alln.end() ), alln.end() );
    lx.erase( std::unique( lx.begin(), lx.end() ), lx.end() );
    ly.erase( std::unique( ly.begin(), ly.end() ), ly.end() );
    rx.erase( std::unique( rx.begin(), rx.end() ), rx.end() );
    ry.erase( std::unique( ry.begin(), ry.end() ), ry.end() );
    
  }
};

