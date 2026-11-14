#include "threads_low_level.h"

// Threads Low-Level subroutine for performing a setjmp but with a custom stack
//
// Note: it is not safe to return if this returns a non-zero value
int tll_setjmp_stack(jmp_buf jump_to, uint32_t new_sp, uint32_t new_sl);

// multithreading design based on: https://stackoverflow.com/a/15554890

volatile size_t current_thread_id;

// Internal. Launch next thread.
void tll_launch_next(struct tll_thread_manager *manager) {
  // hacky next thread thingy
  size_t next_thread_id = current_thread_id;
  const size_t max_thread_id = manager->max_thread_count + 1;
  for (size_t i = 0; i < max_thread_id; i++) {
    next_thread_id = (next_thread_id + 1) % max_thread_id;

    if (manager->saves[next_thread_id].present) {
      // next thread found, now let's jump to it
      current_thread_id = next_thread_id;
      longjmp(manager->saves[next_thread_id].jmp, 1);
    }
  }
}

void tll_init_manager(struct tll_thread_manager *manager, size_t max_thread_count, void *thread_stacks, size_t stack_size_per_thread) {
  // assumes manager is uninitialized but not null

  // setup thread count
  manager->max_thread_count = max_thread_count;

  // setup stack space
  manager->thread_stacks = thread_stacks;
  manager->stack_size_per_thread = stack_size_per_thread;

  // allocate thread state storage
  manager->saves = calloc(max_thread_count + 1, sizeof(struct tll_saved_thread_state));

  // allocate stack management variables
  manager->stacks_in_use = calloc(max_thread_count + 1, sizeof(bool));

  // assumes there is no threading capability outside this,
  // so this is automatically initialized on the main thread
  current_thread_id = 0; // zero is the main thread, all threads are their index + 1
  manager->stacks_in_use[0] = true;
}

void tll_deinit_manager(struct tll_thread_manager *manager) {
  free(manager->saves);
  free(manager->stacks_in_use);
  manager->saves = NULL;
  manager->stacks_in_use = NULL;
}

int tll_schedule_thread_now(struct tll_thread_manager *manager, thread_func *func, void *input) {
  // find free thread stack
  struct tll_saved_thread_state *to_save = NULL;
  size_t free_thread_id = 0;
  const size_t max_thread_id = manager->max_thread_count + 1;
  for (size_t i = 0; i < max_thread_id; i++) {
    if (!manager->stacks_in_use[i]) {
      free_thread_id = i;
      to_save = &manager->saves[i];;
    }
  }

  if (!to_save) {
    // no free threads
    return 2;
  }

  if (!free_thread_id) {
    // main stack should never be free
    return 3;
  }

  // store thread stuff because we won't have access to it post setjmp
  to_save->start = func;
  to_save->thread_input = input;
  to_save->present = true;

  // get storage pointer
  // free_thread_id is the index of our stack + 1, meaning this variable
  // points to the high end of this thread's stack, as stacks usually grow downwards
  void *new_stack = manager->thread_stacks + (free_thread_id * manager->stack_size_per_thread);
  void *new_stack_limit = manager->thread_stacks + ((free_thread_id - 1) * manager->stack_size_per_thread);
  // we're using it now
  manager->stacks_in_use[free_thread_id] = true;

  // store storage manager pointer on the new stack so we can access it post setjmp
  new_stack -= sizeof(void *);
  *((void **) new_stack) = manager;

  if (tll_setjmp_stack(to_save->jmp, (uint32_t) new_stack, (uint32_t) new_stack_limit)) {
    // we're now in the thread, we likely don't have access to the rest of this method's variables
    struct tll_thread_manager *ns_manager;
    asm volatile("POP {%0}" : "=r" (ns_manager));

    // retrieve new thread state
    struct tll_saved_thread_state *ns_save = &ns_manager->saves[current_thread_id];
    thread_func *ns_func = ns_save->start;
    void *ns_input = ns_save->thread_input;
    ns_save->start = NULL;
    ns_save->thread_input = NULL;
    ns_save->present = false;

    // launch the thread
    (ns_func)(ns_input);

    // the thread exited, so ig that means our stack is no longer in use
    ns_manager->stacks_in_use[current_thread_id] = false;

    // the thread finished, jump to another thread
    while (true) {
      // make sure we can never return
      tll_launch_next(ns_manager);
      // if we returned, then it means there are no other threads active, this is an error
    }
  }

  return 0;
}

void tll_yield(struct tll_thread_manager *manager) {
  if (setjmp(manager->saves[current_thread_id].jmp)) {
    // we're returing from a yield
    manager->saves[current_thread_id].present = false;
    return;
  }
  manager->saves[current_thread_id].present = true;

  tll_launch_next(manager);
}
