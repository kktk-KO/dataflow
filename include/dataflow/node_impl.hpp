#pragma once

#include <dataflow/node.hpp>
#include <dataflow/meta.hpp>

#include <memory>
#include <tuple>

namespace dataflow {

struct graph;

template <class F, class T, class ... Args>
struct node_impl : public node {

  F handler;
  T value;

  template <class ... Ts>
  node_impl (graph & g, F && handler, Ts && ... ts)
  : node(g),
    handler(std::forward<F>(handler)),
    value(std::forward<Ts>(ts) ...) {
  }

  void fire () override {
    if (data_.size() != sizeof...(Args)) {
      throw "Invalid Data node size.";
    }

    fire_impl(make_index_sequence<sizeof...(Args)>());
  }

  void * data () noexcept override {
    return &value;
  }


private:

  using arg_tuple = std::tuple<Args ...>;

  void fire_impl (index_sequence<>) {
    handler(value);
  }

  template <std::size_t ... Is>
  void fire_impl (index_sequence<Is ...>) {
    handler(value, *reinterpret_cast<typename std::tuple_element<Is, arg_tuple>::type *>(data_[Is]->data()) ...);
  }

};

template <class F, class ... Args>
struct node_impl<F, void, Args ...> : public node {

  F handler;

  node_impl (graph & g, F && handler)
  : node(g),
    handler(std::forward<F>(handler)) {
  }  

  void fire () override {
    if (data_.size() != sizeof...(Args)) {
      throw "Invalid Data node size.";
    }

    fire_impl(make_index_sequence<sizeof...(Args)>());
  }

  void * data () noexcept override {
    return nullptr;
  }

private:

  using arg_tuple = std::tuple<Args ...>;

  void fire_impl (index_sequence<>) {
    handler();
  }

  template <std::size_t ... Is>
  void fire_impl (index_sequence<Is ...>) {
    handler(*reinterpret_cast<typename std::tuple_element<Is, arg_tuple>::type *>(data_[Is]->data()) ...);
  }

};

namespace impl {
  template <class F, class T>
  struct Deducenode_type {
    using type = node_impl<F, T>;
  };

  template <class F, class T, class ... Args>
  struct Deducenode_type<F, T(Args ...)> {
    using type = node_impl<F, T, Args...>;
  };

};

template <class F, class T>
using deduce_node_type = typename impl::Deducenode_type<F, T>::type;

template <class T, class F, class ... Args>
deduce_node_type<F, T> * make_node (graph & g, F && f, Args && ... args) {
  using node_type = deduce_node_type<F, T>;
  return new node_type(g, std::forward<F>(f), std::forward<Args>(args) ...);
}

template <class F, class ... Args>
deduce_node_type<F, void> * make_node (graph & g, F && f, Args && ... args) {
  using node_type = deduce_node_type<F, void>;
  return new node_type(g, std::forward<F>(f), std::forward<Args>(args) ...);
}

}
