// ARM-based context-save/context-restore functions

// TODO: arduino arch filter

// CoopThreads takes some tips from TeensyThreads arduino library, but ultimately has a different design goal

// cursed hardcoded constants for struct access
#define TLL_SAVED_THREAD_STATE__SIZE #44

  .syntax unified
  .align 2
  .thumb

  // Save r4-r11, lr, sp into the current thread's thread state structure
  //
  // expects the address of the thread manager to be in r0, takes no other arguments, returns nothing
  .global tll_save
  .thumb_func
tll_save:
  // get thread state location
  LDR r3, =current_thread_id                  // address of current thread id variable
  MOVS r1, TLL_SAVED_THREAD_STATE__SIZE       // size of the tll_saved_thread_state struct
  LDR r0, [r0]                                // deref thread_manager arg to get states pointer
  LDR r2, [r3]                                // deref thread id addr to get thread id
  MLA r0, r1, r2, r0                          // index into thread_manager states array by thread id

  

  .global tll_yield
  .thumb_func
tll_yield:
  // we need to store everything first, which means we need to control our entire method setup/teardown
  
