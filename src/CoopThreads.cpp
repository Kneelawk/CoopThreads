#include <functional>
#include "CoopThreads.h"

// Used for actually invoking the std::function when within the new thread
void coopThreadsInvoke(void *fn_ptr) {
  // call the function
  auto *fn = static_cast<std::function<void()>*>(fn_ptr);
  (*fn)();

  // memory ownership was passed to this thread, so we delete here
  delete fn;
}
