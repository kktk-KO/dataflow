#pragma once

#include <atomic>
#include <thread>

namespace dataflow {

struct spinlock {

  void lock () {
    while (f_.test_and_set(std::memory_order_acquire)) {
//       std::this_thread::yield();
    }
  }

  bool try_lock () {
    return f_.test_and_set(std::memory_order_acquire);
  }

  void unlock () {
    f_.clear(std::memory_order_release);
  }
private:
  std::atomic_flag f_ = ATOMIC_FLAG_INIT;  
};

}
