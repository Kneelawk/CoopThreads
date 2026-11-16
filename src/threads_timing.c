#include "threads_timing.h"

// Arduino API

#include <core_pins.h>

// timing impls

thread_time_t thread_time_now() {
  return {
    .clock_cycles = ARM_DWT_CYCCNT,
    .milliseconds = systick_millis_count,
  };
}
