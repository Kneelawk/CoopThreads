#include "threads_low_level.h"

uint32_t current_thread_id;

void tll_init_manager(struct tll_thread_manager *manager, uint32_t max_thread_count, void *thread_stacks, uint32_t stack_size_per_thread) {
  // assumes manager is uninitialized but not null

  // setup thread count
  manager->max_thread_count = max_thread_count;

  // setup stack space
  manager->thread_stacks = thread_stacks;
  manager->stack_size_per_thread = stack_size_per_thread;

  // allocate thread state storage
  manager->states = malloc(sizeof(struct tll_saved_thread_state) * (max_thread_count + 1));

  // assumes there is no threading capability outside this,
  // so this is automatically initialized on the main thread
  current_thread_id = 0; // zero is the main thread, all threads are their index + 1

  // initialize threads
  for (uint32_t i = 0; i < max_thread_count; i++) {
    struct tll_saved_thread_state *state = &manager->states[i];

    // saved thread state setup
    state->saved = 0;
  }
}

void tll_deinit_manager(struct tll_thread_manager *manager) {
  free(manager->states);
  manager->states = NULL;
}

void tll_yield2(struct tll_thread_manager *manager) {
  struct tll_saved_thread_state *state = &manager->states[current_thread_id];
  state->pc = 2;
  state->regs[0] = 0;
  state->regs[1] = 1;
  state->saved = 3;
}