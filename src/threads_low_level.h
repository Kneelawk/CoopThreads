/// Simple threading library - Low-Level interface.

#ifndef THREADS_LOW_LEVEL_H
#define THREADS_LOW_LEVEL_H

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

// normally it would be terrible idea to store thread information in a global variable,
// but the Teensy4.1 is single-core and we are the only ones switching and managing threads
extern uint32_t current_thread_id;

// Low-level thread info holder
struct tll_thread_info {
  int in_use;
  void *stack; // null for unmanaged stack
  uint32_t stack_size;
};

// Low-level thread saved state
struct tll_saved_thread_state {
  int present;
  uint32_t regs[8]; // saved registers
  uint32_t pc; // where to pick up the thread at
  uint32_t stack; // current position of the stack pointer
};

// Low-level thread manager
struct tll_thread_manager {
  uint32_t max_thread_count;
  struct tll_saved_thread_state main_state;
  struct tll_thread_info *threads;
  struct tll_saved_thread_state *states;
  void *thread_stacks;
  uint32_t stack_size_per_thread;
};

// Initializes the thread manager construct
// thread_stacks is not managed by the thread manager
void tll_init_manager(struct tll_thread_manager *manager, uint32_t max_thread_count, void *thread_stacks, uint32_t stack_size_per_thread);

// De-initializes a thread manager by deallocating its allocated memory
void tll_deinit_manager(struct tll_thread_manager *manager);

// Yield to another thread managed by this manager
void tll_yield(struct tll_thread_manager *manager);

#ifdef __cplusplus
}
#endif

#endif