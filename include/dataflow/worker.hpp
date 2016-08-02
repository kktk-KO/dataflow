#pragma once

#include <thread>
#include <mutex>

namespace dataflow {

struct node;

struct worker {
  std::size_t id;
  std::thread thread;

  worker (std::size_t id)
  : id(id) {
  }
};

}
