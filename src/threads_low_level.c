#include "threads_low_level.h"

uint32_t current_thread_id;

void tll_init_manager(struct tll_thread_manager *manager, uint32_t max_thread_count, void *thread_stacks, uint32_t stack_size_per_thread) {
  // assumes manager is uninitialized but not null

  // setup thread count
  manager->max_thread_count = max_thread_count;

  // setup stack space
  manager->thread_stacks = thread_stacks;
  manager->stack_size_per_thread = stack_size_per_thread;

  // allocate thread info storage
  manager->threads = malloc(sizeof(struct tll_thread_info) * max_thread_count);
  manager->states = malloc(sizeof(struct tll_saved_thread_state) * max_thread_count);

  // initialize threads
  for (uint32_t i = 0; i < max_thread_count; i++) {
    struct tll_thread_info *thread = &manager->threads[i];
    struct tll_saved_thread_state *state = &manager->states[i];

    // thread info setup
    thread->in_use = 0;
    thread->thread_id = i;
    thread->stack = thread_stacks + (i * stack_size_per_thread);
    thread->stack_size = stack_size_per_thread;

    // saved thread state setup
    state->present = 0;
    state->thread = thread;
  }
}

void tll_deinit_manager(struct tll_thread_manager *manager) {
  free(manager->states);
  free(manager->threads);
  manager->states = NULL;
  manager->threads = NULL;
}

void tll_yield(struct tll_thread_manager *manager) {
  
}