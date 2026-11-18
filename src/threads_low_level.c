#include "threads_low_level.h"

// Threads Low-Level subroutine for performing a setjmp but with a custom stack
//
// Note: it is not safe to return if this returns a non-zero value
//
// source is in threads_low_level_thumb.S
int tll_setjmp_stack(jmp_buf jump_to, uint32_t new_sp, uint32_t new_sl);

// multithreading design based on: https://stackoverflow.com/a/15554890 (minus using the main thread as the scheduler)

volatile size_t current_thread_id;

// Internal. Launch next thread.
void tll_launch_next(struct tll_thread_manager *manager) {
  // hacky real-time next thread thingy

  const size_t max_thread_id = manager->max_thread_count + 1;

  // use the current thread as a default
  thread_time_t base = manager->saves[current_thread_id].schedule_time;
  thread_duration_t delay = manager->saves[current_thread_id].schedule_delay;
  size_t new_thread_id = current_thread_id;

  // go through every thread to find the one scheduled soonest
  for (size_t i = 0; i < max_thread_id; i++) {
    struct tll_saved_thread_state *save = &manager->saves[i];
    if (save->present && thread_delay_cmp(base, delay, save->schedule_time, save->schedule_delay) >= 0) {
      base = save->schedule_time;
      delay = save->schedule_delay;
      new_thread_id = i;
    }
  }

  // for debugging purposes, make it clear that we are not in a thread during this delay
  current_thread_id = -1;

  // wait for soonest thread
  thread_delay_await(base, delay);

  // launch new thread
  current_thread_id = new_thread_id;
  longjmp(manager->saves[new_thread_id].jmp, 1);
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
      to_save = &manager->saves[i];
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
  to_save->schedule_time = thread_time_now();
  to_save->schedule_delay = (thread_duration_t) {0};
  to_save->present = true;

  // get storage pointer
  // free_thread_id is the index of our stack + 1, meaning this variable
  // points to the high end of this thread's stack, as stacks usually grow downwards
  void *new_stack = manager->thread_stacks + (free_thread_id * manager->stack_size_per_thread);
  void *new_stack_limit = manager->thread_stacks + ((free_thread_id - 1) * manager->stack_size_per_thread);
  // we're using it now
  manager->stacks_in_use[free_thread_id] = true;

  // push storage manager pointer on the new stack so we can access it post setjmp
  new_stack -= sizeof(void *);
  *((void **) new_stack) = manager;

  if (tll_setjmp_stack(to_save->jmp, (uint32_t) new_stack, (uint32_t) new_stack_limit)) {
    // we're now in the thread on the new stack, we likely don't have access to the rest of this method's variables

    // pop thread manager pointer off new stack
    struct tll_thread_manager *ns_manager;
    asm volatile("POP {%0}" : "=r" (ns_manager));

    // retrieve new thread state
    struct tll_saved_thread_state *ns_save = &ns_manager->saves[current_thread_id];
    thread_func *ns_func = ns_save->start;
    void *ns_input = ns_save->thread_input;

    // setup save state for thread running
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

  // we're not in the thread and are back in our own stack, so it is safe to return here
  return 0;
}

void tll_yield(struct tll_thread_manager *manager) {
  // yield to arduino first
  yield();

  // prepare current thread for re-launch as soon as possible
  manager->saves[current_thread_id].schedule_time = thread_time_now();
  manager->saves[current_thread_id].schedule_delay = (thread_duration_t) {0};
  manager->saves[current_thread_id].present = true;

  // setup the return state
  if (setjmp(manager->saves[current_thread_id].jmp)) {
    // we're returing from a yield
    manager->saves[current_thread_id].present = false;
    return;
  }

  tll_launch_next(manager);
}

// Delays (and yields) for roughly the given number of milliseconds
void tll_delay_millis(struct tll_thread_manager *manager, uint32_t millis) {
  // prepare current thread for re-launch after delay
  manager->saves[current_thread_id].schedule_time = thread_time_now();
  manager->saves[current_thread_id].schedule_delay = thread_duration_millis(millis);
  manager->saves[current_thread_id].present = true;

  // setup the return state
  if (setjmp(manager->saves[current_thread_id].jmp)) {
    // we're returing from a yield
    manager->saves[current_thread_id].present = false;
    return;
  }

  tll_launch_next(manager);
}

// Delays (and yields) for roughly the given number of microseconds
void tll_delay_micros(struct tll_thread_manager *manager, uint32_t micros) {
  // prepare current thread for re-launch after delay
  manager->saves[current_thread_id].schedule_time = thread_time_now();
  manager->saves[current_thread_id].schedule_delay = thread_duration_micros(micros);
  manager->saves[current_thread_id].present = true;

  // setup the return state
  if (setjmp(manager->saves[current_thread_id].jmp)) {
    // we're returing from a yield
    manager->saves[current_thread_id].present = false;
    return;
  }

  tll_launch_next(manager);
}

// Delays (and yields) for roughly the given number of nanosecond
//
// Note: this method will likely not be very accurate for delays less than 200ns
void tll_delay_nanos(struct tll_thread_manager *manager, uint32_t nanos) {
  // prepare current thread for re-launch after delay
  manager->saves[current_thread_id].schedule_time = thread_time_now();
  manager->saves[current_thread_id].schedule_delay = thread_duration_nanos(nanos);
  manager->saves[current_thread_id].present = true;

  // setup the return state
  if (setjmp(manager->saves[current_thread_id].jmp)) {
    // we're returing from a yield
    manager->saves[current_thread_id].present = false;
    return;
  }

  tll_launch_next(manager);
}
