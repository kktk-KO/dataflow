#pragma once

#include <atomic>
#include <memory>

namespace dataflow {

struct node;

struct edge {

  edge (node & prev, node & next) noexcept
  : prev_(prev), next_(next) {
    lock_ = false;
  }

  node & next () noexcept {
    return next_;
  }

  node & prev () noexcept {
    return prev_;
  }

  bool lock () noexcept {
    bool f = lock_;
    lock_ = true;
    return f;
  }

  void unlock () noexcept {
    lock_ = false;
  }

  bool is_locked () const noexcept {
    return lock_;
  }
private:
  std::atomic_bool lock_;
  node & prev_;
  node & next_;
};

}
