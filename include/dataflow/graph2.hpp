#pragma once

#include <dataflow/edge.hpp>
#include <dataflow/node.hpp>
#include <dataflow/node_impl.hpp>
#include <dataflow/queue.hpp>
#include <dataflow/worker.hpp>

#include <atomic>
#include <iostream>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <vector>

namespace dataflow {

struct graph {

  std::string graphviz_post;

  graph (std::size_t num_worker = 4)
  : queue_(num_worker) {
    for (std::size_t i = 0; i < num_worker; ++i) {
      worker_.emplace_back(i);
    }
  }

  std::size_t num_worker () const noexcept {
    return worker_.size();
  }

  void run () {
    stop_ = false;
    for (auto & worker : worker_) {
      worker.thread = std::thread([&] () { work(worker.id); });
    }
  }

  void run (node & n) {
    fire(n);
    run();
  }

  void fire (node & n) {
    queue_[0].push(&n);
  }


  void stop () {
    stop_ = true;

    for (auto & worker : worker_) {
      if (worker.thread.joinable()) {
        worker.thread.join();
      }
    }
  }

  template <class T, class F, class ... Args>
  deduce_node_type<F, T> & add_node (std::string const & name, F && f, Args && ... args) {
    auto * n = make_node<T>(*this, std::forward<F>(f), std::forward<Args>(args) ...);
    n->name = name;
    nodes_.emplace(n);
    if (name.size() > 0) { // TODO dupulicate check
      nodes_map_[name] = n;
    }
    return *n;
  }

  template <class F, class ... Args>
  deduce_node_type<F, void> & add_node (std::string const & name, F && f, Args && ... args) {
    auto * n = make_node(*this, std::forward<F>(f), std::forward<Args>(args) ...);
    n->name = name;
    nodes_.emplace(n);
    if (name.size() > 0) { // TODO dupulicate check
      nodes_map_[name] = n;
    }

    return *n;
  }

  void connect (node & from, node & to) {
    assert(!from.find_next(to));  
    assert(!to.find_prev(from));
    edge * e = new edge(from, to);

    from.next_.push_back(e);
    to.prev_.push_back(e);
    edges_.emplace(e);
  }

  void connect_data (node & from, node & to) {
    assert(!from.find_data(to));  
    from.data_.push_back(&to);
  }

  void print (std::ostream & ost = std::cout, std::string post = "") {
    ost << "@" << std::endl;
    ost << "digraph { " << std::endl;
    ost << "  node [" << std::endl;
    ost << "    shape = box" << std::endl;
    ost << "  ];" << std::endl;

    for (node * n : nodes_) {
      ost << "  \"";
      if (n->name.size() > 0) {
        ost << n->name;
      } else {
        ost << n;
      }
      ost << "\" [" << std::endl;
      if (n->label.size() > 0) {
        ost << "  label = \"" << n->label << "\"" << std::endl;
      } else {
        ost << "  label = \"none\"" << std::endl;
      }
      ost << "  " << n->graphviz_node_property << std::endl;
      ost << "   ];" << std::endl;
    }

    for (node * n : nodes_) {
      for (edge * e : n->next_) {
        ost << "  \"";
        if (n->name.size() > 0) {
          ost << n->name;
        } else {
          ost << n;
        }
        ost << "\" -> \"";
        if (e->next().name.size() > 0) {
          ost << e->next().name;
        } else {
          ost << &(e->next());
        }
        ost << "\" [" << std::endl;
        ost << "    color = " << (e->is_locked() ? "red" : "green") << "" << std::endl;
        ost << "   ];" << std::endl;
      }
    }
    ost << graphviz_post << std::endl;
    ost << "}" << std::endl;
  }

  ~graph () {
    for (node * n : nodes_) {
      delete n;
    }
    for (edge * e : edges_) {
      delete e;
    }

  }

  node * find_node (std::string const & name) {
    auto it = nodes_map_.find(name);
    if (it == nodes_map_.end()) { return nullptr; }
    return it->second;
  }

private:
  std::unordered_set<node *> nodes_;
  std::unordered_set<edge *> edges_;
  std::unordered_map<std::string, node *> nodes_map_;

  std::vector<queue<node *>> queue_;
  std::vector<worker> worker_;
  std::atomic_bool stop_;

  void work (std::size_t id) {
    while (!stop_) {
      node * n = nullptr;
      bool flag = false;
      for (std::size_t i = 0; i < num_worker(); ++i) {
        std::size_t index = (i + id) % num_worker();
        if (queue_[index].try_pop(n)) {
          flag = true;
          fire_(id, *n);
          break;
        }
      }

      if (!flag) {
        std::this_thread::sleep_for(std::chrono::milliseconds(128));
      }
    }
  }

  void fire_ (std::size_t id, node & n) {
    for (edge * prev : n.prev_) {
      prev->unlock();
    }

    n.count_ = 0;

    n.fire();

    int i = id;
    for (edge * next : n.next_) {
      if (!next->lock()) {
        unsigned n = next->next().count_.fetch_add(1);
        if (n + 1 == next->next().prev_.size()) {
          queue_[i % num_worker()].push(&next->next());
          ++i;
        }
      }
    }
  }

};

node & operator>> (node & left, node & right) {
  left.g().connect(left, right);
  return right;
}

node & operator<< (node & left, node & right) {
  left.g().connect_data(right, left);
  return right;
}

}
