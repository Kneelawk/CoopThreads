// Simple threading library - Timing interface

#ifndef THREADS_TIMING
#define THREADS_TIMING

#ifdef __cplusplus
extern "C" {
#endif

// Describes a point in time from the processor's perspective
typedef struct {
  uint32_t clock_cycle;
  uint32_t millisecond;
} thread_time_t;

// Gets the current time
// TODO: look into inlining this
thread_time_t thread_time_now();

// Gets a future point in time in milliseconds
thread_time_t thread_time_future_millis(uint32_t millis);

// Gets a future point in time in microseconds
thread_time_t thread_time_future_micros(uint32_t micros);

// Gets a future point in time in nanoseconds
// TODO: look into inlining this
thread_time_t thread_time_future_nanos(uint32_t nanos);

// Compares one thread time to another
int thread_time_cmp(thread_time_t a, thread_time_t b);

// Waits until the given time is past
// TODO: look into inlining this
void thread_time_await(thread_time_t time);

#ifdef __cplusplus
}
#endif

#endif
