#pragma once

#include <dataflow/edge.hpp>

#include <atomic>
#include <mutex>
#include <memory>
#include <utility>
#include <vector>
#include <cassert>

namespace dataflow {

struct graph;
struct single_graph;

struct node {

  friend graph;
  friend single_graph;

  std::string label;
  std::string name;
  std::string graphviz_node_property;

  node (graph & g)
  : g_(g) {
    count_ = 0;
  }

  virtual void fire () = 0;
  virtual ~node () noexcept = default;

  bool find_next (node const & n) {
    for (edge * e : next_) {
      assert(e);
      if (e->next() == n) { return true; }
    }
    return false;
  }

  bool find_prev (node const & n) {
    for (edge * e : prev_) {
      assert(e);
      if (e->prev() == n) { return true; }
    }
    return false;
  }

  bool find_data (node const & n) {
    for (node * n : data_) {
      assert(n);
      if (n->data() == n) { return true; }
    }
    return false;
  }

  virtual void * data () noexcept = 0;

  bool operator== (node const & other) const noexcept {
    return this == &other;
  }

  bool operator!= (node const & other) const noexcept {
    return !(*this == other);
  }

  graph & g () noexcept {
    return g_;
  }

protected:
  graph & g_;
  std::atomic_uint count_;

  std::vector<edge *> next_;
  std::vector<edge *> prev_;
  std::vector<node *> data_;

};

}
