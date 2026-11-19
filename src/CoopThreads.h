#include "core_pins.h"
#include <utility>
#ifndef COOP_THREADS_H
#define COOP_THREADS_H

#include <Arduino.h>

#include <functional>

#include "threads_low_level.h"

// Used for actually invoking the std::function when within the new thread
void coopThreadsInvoke(void *fn_ptr);

// Thread manager supertype
//
// This is used in libraries that make use of CoopThreads so that they can
// accept a thread manager without having to care how much stack space is
// reserved.
class ThreadManager {
public:
  // thread manager constructor
  ThreadManager() {}

  // thread manager destructor
  virtual ~ThreadManager() {}

  // Spawns a new thread
  virtual bool spawn(std::function<void()> fn) = 0;

  // Spawns a new thread
  template<typename... Args>
  bool spawn(std::function<void(Args...)> f, Args... args) {
    // capture args as context
    std::function<void()> capture = [=]() mutable {
      f(std::forward<Args>(args)...);
    };

    return spawn(capture);
  }

  // Spawns a new thread
  template<typename... Args>
  bool spawn(void (*f)(Args...), Args... args) {
    // capture args as context
    std::function<void()> capture = [=]() mutable {
      f(std::forward<Args>(args)...);
    };

    return spawn(capture);
  }

  // Yields to another thread, allowing it to execute if scheduled for now
  virtual void yield() = 0;

  // Delays the current thread by the given number of milliseconds,
  // yielding to other threads while this thread waits
  virtual void delay(uint32_t msec) = 0;

  // Delays the current thread by the given number of microseconds,
  // yielding to other threads while this thread waits
  virtual void delayMicroseconds(uint32_t usec) = 0;

  // Delays the current thread by the given number of nanoseconds,
  // yielding to other threads while this thread waits
  virtual void delayNanoseconds(uint32_t nsec) = 0;
};

// Cooperative multithreading thread manager
template <size_t stack_space>
class CoopThreads : public ThreadManager {
  tll_thread_manager threads;
  uint8_t thread_stacks[stack_space];

public:
  // Creates a new coop thread manager
  CoopThreads(const size_t custom_thread_count) {
    tll_init_manager(&threads, custom_thread_count, thread_stacks, stack_space / custom_thread_count);
  }

  // Deinitializes the thread manager
  virtual ~CoopThreads() override {
    tll_deinit_manager(&threads);
  }

  virtual bool spawn(std::function<void()> fn) override {
    std::function<void()> *fn_ptr = new std::function<void()>(fn);

    return !tll_schedule_thread_now(&threads, coopThreadsInvoke, fn_ptr);
  }

  // Spawns a new thread
  template<typename... Args>
  bool spawn(std::function<void(Args...)> f, Args... args) {
    // capture args as context
    std::function<void()> capture = [=]() mutable {
      f(std::forward<Args>(args)...);
    };

    return spawn(capture);
  }

  // Spawns a new thread
  template<typename... Args>
  bool spawn(void (*f)(Args...), Args... args) {
    // capture args as context
    std::function<void()> capture = [=]() mutable {
      f(std::forward<Args>(args)...);
    };

    return spawn(capture);
  }

  virtual void yield() override {
    tll_yield(&threads);
  }

  virtual void delay(uint32_t msec) override {
    tll_delay_millis(&threads, msec);
  }

  virtual void delayMicroseconds(uint32_t usec) override {
    tll_delay_micros(&threads, usec);
  }

  virtual void delayNanoseconds(uint32_t nsec) override {
    if (nsec < 200) {
      ::delayNanoseconds(nsec);
    } else {
      tll_delay_nanos(&threads, nsec);
    }
  }
};

#endif
