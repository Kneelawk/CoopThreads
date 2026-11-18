#include "core_pins.h"
#include <utility>
#ifndef COOP_THREADS_H
#define COOP_THREADS_H

#include <Arduino.h>

#include <functional>

#include "threads_low_level.h"

// Used for actually invoking the std::function when within the new thread
void coopThreadsInvoke(void *fn_ptr);

// Cooperative multithreading thread manager
template <size_t stack_space>
class CoopThreads {
  tll_thread_manager threads;
  uint8_t thread_stacks[stack_space];

public:
  // Creates a new coop thread manager
  CoopThreads(const size_t custom_thread_count) {
    tll_init_manager(&threads, custom_thread_count, thread_stacks, stack_space / custom_thread_count);
  }

  // Deinitializes the thread manager
  virtual ~CoopThreads() {
    tll_deinit_manager(&threads);
  }

  // Spawns a new thread
  template<typename... Args>
  bool spawn(std::function<void(Args...)> f, Args... args) {
    // capture args as context
    std::function<void()> capture = [=]() mutable {
      f(std::forward<Args>(args)...);
    };
    std::function<void()> *fn_ptr = new std::function<void()>(capture);

    return tll_schedule_thread_now(&threads, coopThreadsInvoke, fn_ptr);
  }

  // Spawns a new thread
  template<typename... Args>
  bool spawn(void (*f)(Args...), Args... args) {
    // capture args as context
    std::function<void()> capture = [=]() mutable {
      f(std::forward<Args>(args)...);
    };
    std::function<void()> *fn_ptr = new std::function<void()>(capture);

    return tll_schedule_thread_now(&threads, coopThreadsInvoke, fn_ptr);
  }

  void yield() {
    tll_yield(&threads);
  }

  void delay(uint32_t msec) {
    tll_delay_millis(&threads, msec);
  }

  void delayMicroseconds(uint32_t usec) {
    tll_delay_micros(&threads, usec);
  }

  void delayNanoseconds(uint32_t nsec) {
    if (nsec < 200) {
      ::delayNanoseconds(nsec);
    } else {
      tll_delay_nanos(&threads, nsec);
    }
  }
};

#endif
