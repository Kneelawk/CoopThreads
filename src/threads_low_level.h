/// Simple threading library - Low-Level interface.

#ifndef THREADS_LOW_LEVEL_H
#define THREADS_LOW_LEVEL_H

#include <stdint.h>
#include <stdlib.h>
#include <setjmp.h>

#ifdef __cplusplus
extern "C" {
#endif

// normally it would be terrible idea to store thread information in a global variable,
// but the Teensy4.1 is single-core and we are the only ones switching and managing threads
extern size_t current_thread_id;

// Low-level thread saved state
struct tll_saved_thread_state {
  uint32_t regs[8]; // saved registers
  int saved;
};

// Low-level thread manager
struct tll_thread_manager {
  struct tll_saved_thread_state *states;
  jmp_buf *save_jmps;
  bool *save_jmps_present;
  size_t max_thread_count;
  void *thread_stacks;
  size_t stack_size_per_thread;
};

// Initializes the thread manager construct
// thread_stacks is not managed by the thread manager
void tll_init_manager(struct tll_thread_manager *manager, size_t max_thread_count, void *thread_stacks, size_t stack_size_per_thread);

// De-initializes a thread manager by deallocating its allocated memory
void tll_deinit_manager(struct tll_thread_manager *manager);

// Yield to another thread managed by this manager
void tll_yield(struct tll_thread_manager *manager);

// Yield to another thread managed by this manager
void tll_yield2(struct tll_thread_manager *manager);

#ifdef __cplusplus
}
#endif

#endif