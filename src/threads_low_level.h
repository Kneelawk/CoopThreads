/// Simple threading library - Low-Level interface.

#ifndef THREADS_LOW_LEVEL_H
#define THREADS_LOW_LEVEL_H

#include <stdint.h>
#include <stdlib.h>
#include <setjmp.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (thread_func)(void *input);

// normally it would be terrible idea to store thread information in a global variable,
// but the Teensy4.1 is single-core and we are the only ones switching and managing threads
extern volatile size_t current_thread_id;

struct tll_saved_thread_state {
  jmp_buf jmp;
  thread_func *start;
  void *thread_input;
  bool present;
};

// Low-level thread manager
struct tll_thread_manager {
  struct tll_saved_thread_state *saves;
  void *thread_stacks;
  bool *stacks_in_use;
  size_t max_thread_count;
  size_t stack_size_per_thread;
};

// Initializes the thread manager construct
// thread_stacks is not managed by the thread manager
void tll_init_manager(struct tll_thread_manager *manager, size_t max_thread_count, void *thread_stacks, size_t stack_size_per_thread);

// De-initializes a thread manager by deallocating its allocated memory
void tll_deinit_manager(struct tll_thread_manager *manager);

// Schedules a thread to be launchd as soon as possible.
int tll_schedule_thread_now(struct tll_thread_manager *manager, thread_func *func, void *input);

// Yield to another thread managed by this manager
void tll_yield(struct tll_thread_manager *manager);

#ifdef __cplusplus
}
#endif

#endif
