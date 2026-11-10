/// Simple threading library - Low-Level interface.

#ifndef THREADS_LOW_LEVEL_H
#define THREADS_LOW_LEVEL_H

#ifdef __cplusplus
extern "C" {
#endif

uint32_t thread_id;

// Low-level thread info holder
struct tll_thread_info {
  int in_use;
  void *stack;
  uint32_t stack_size;
};

// Low-level thread saved state
struct tll_saved_thread_state {
  int present;
  struct tll_thread_info *thread; // access to the thread info
  uint32_t regs[8]; // saved registers
  uint32_t pc; // where to pick up the thread at
};

// Low-level thread manager
struct tll_thread_manager {
  void *main_stack;
  uint32_t max_thread_count;
  struct tll_thread_info *threads;
  struct tll_saved_thread_state *states;
  void *thread_stacks;
  uint32_t thread_stack_size;
};

// Yield to another thread managed by this manager
void yield(struct tll_thread_manager *manager);

#ifdef __cplusplus
}
#endif

#endif