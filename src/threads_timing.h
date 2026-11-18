// Simple threading library - Timing interface

#ifndef THREADS_TIMING
#define THREADS_TIMING

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Describes a point in time from the processor's perspective
typedef struct {
  uint32_t clock_cycles;
  uint32_t milliseconds;
} thread_time_t;

// Describes a duration from the processor's perspective
typedef struct {
  uint32_t clock_cycles;
  uint32_t milliseconds;
} thread_duration_t;

// Gets the current time
thread_time_t thread_time_now();

// Gets a future point in time in milliseconds
thread_duration_t thread_duration_millis(uint32_t millis);

// Gets a future point in time in microseconds
thread_duration_t thread_duration_micros(uint32_t micros);

// Gets a future point in time in nanoseconds
thread_duration_t thread_duration_nanos(uint32_t nanos);

// Compares one thread time to another
// TODO: does this need to be inline?
int thread_delay_cmp(thread_time_t a_base, thread_duration_t a_wait, thread_time_t b_base, thread_duration_t b_wait);

// Waits for the given delay
//
// Used when there are no threads that need to be executed earlier.
//
// `base` is used to keep track of when the delay was first started so the wait function doesn't run into roll-over issues.
void thread_delay_await(thread_time_t base, thread_duration_t wait);

#ifdef __cplusplus
}
#endif

#endif
