#include "threads_low_level.h"

size_t current_thread_id;

void tll_init_manager(struct tll_thread_manager *manager, size_t max_thread_count, void *thread_stacks, size_t stack_size_per_thread) {
  // assumes manager is uninitialized but not null

  // setup thread count
  manager->max_thread_count = max_thread_count;

  // setup stack space
  manager->thread_stacks = thread_stacks;
  manager->stack_size_per_thread = stack_size_per_thread;

  // allocate thread state storage
  manager->states = malloc(sizeof(struct tll_saved_thread_state) * (max_thread_count + 1));
  manager->save_jmps = malloc(sizeof(jmp_buf) * (max_thread_count + 1));
  manager->save_jmps_present = calloc(max_thread_count + 1, sizeof(bool));

  // assumes there is no threading capability outside this,
  // so this is automatically initialized on the main thread
  current_thread_id = 0; // zero is the main thread, all threads are their index + 1

  // initialize threads
  for (size_t i = 0; i < max_thread_count; i++) {
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
  // setjmp/longjmp might work in theory but they don't seem to be implemented on the teensy
  if (setjmp(manager->save_jmps[current_thread_id])) {
    // we're returing from a yield
    manager->save_jmps_present[current_thread_id] = false;
    return;
  }
  manager->save_jmps_present[current_thread_id] = true;

  // hacky next thread thingy
  size_t next_thread_id = current_thread_id;
  for (size_t i = 0; i < manager->max_thread_count; i++) {
    next_thread_id = (next_thread_id + 1) % manager->max_thread_count;

    if (manager->save_jmps_present[next_thread_id]) {
      // next thread found, now let's jump to it
      longjmp(manager->save_jmps[next_thread_id], 1);
    }
  }
}