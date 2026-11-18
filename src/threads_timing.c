#include "threads_timing.h"

// Compares one thread time to another
// TODO: does this need to be inline?
int thread_delay_cmp(thread_time_t a_base, thread_duration_t a_wait, thread_time_t b_base, thread_duration_t b_wait) {
  int32_t diff_millis = a_base.milliseconds + a_wait.milliseconds - b_base.milliseconds - b_wait.milliseconds;
  if (diff_millis) return diff_millis;

  return a_base.clock_cycles + a_wait.clock_cycles - b_base.clock_cycles - b_wait.clock_cycles;
}

// Waits for the given delay
//
// Used when there are no threads that need to be executed earlier.
//
// `base` is used to keep track of when the delay was first started so the wait function doesn't run into roll-over issues.
void thread_delay_await(thread_time_t base, thread_duration_t wait) {
  // wait milliseconds
  uint32_t current_millis = millis();
  if (current_millis - base.milliseconds < wait.milliseconds) {
    delay(wait.milliseconds - (current_millis - base.milliseconds));
  }
  
  // FIXME: if delay runs, this won't run, but this will probably be okay for most uses
  // wait cycles (copied from Teensy4 core_pins.h)
  while (ARM_DWT_CYCCNT - base.clock_cycles < wait.clock_cycles);
}

