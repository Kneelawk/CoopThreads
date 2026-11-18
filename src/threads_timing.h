// Simple threading library - Timing interface

#ifndef THREADS_TIMING
#define THREADS_TIMING

#include <stdint.h>

// Arduino API
#include <core_pins.h>

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
static inline __attribute__((always_inline, unused)) thread_time_t thread_time_now() {
  return (thread_time_t) {
    .clock_cycles = ARM_DWT_CYCCNT,
    .milliseconds = systick_millis_count,
  };
}

// Gets a future point in time in milliseconds
static inline __attribute__((always_inline, unused)) thread_duration_t thread_duration_millis(uint32_t millis) {
  return (thread_duration_t) {
    .clock_cycles = 0,
    .milliseconds = millis,
  };
}

// Gets a future point in time in microseconds
static inline __attribute__((always_inline, unused)) thread_duration_t thread_duration_micros(uint32_t micros) {
  return (thread_duration_t) {
    // Copied from Teensy4 core_pins.h
    .clock_cycles = F_CPU_ACTUAL / 1000000 * micros,
    .milliseconds = 0,
  };
}

// Gets a future point in time in nanoseconds
static inline __attribute__((always_inline, unused)) thread_duration_t thread_duration_nanos(uint32_t nanos) {
  return (thread_duration_t) {
    // Coped from Teensy4 core_pins.h
    .clock_cycles = ((F_CPU_ACTUAL>>16) * nanos) / (1000000000UL>>16),
    .milliseconds = 0,
  };
}

// Compares one thread time to another
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
