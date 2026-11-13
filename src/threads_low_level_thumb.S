// ARM-based context-save/context-restore functions

// TODO: arduino arch filter

// CoopThreads takes some tips from TeensyThreads arduino library, but ultimately has a different design goal

// cursed hardcoded constants for struct access
//#define TLL_SAVED_THREAD_STATE__SIZE #44

  .syntax unified
  .align 2
  .thumb

  // Save r4-r11, lr, sp into the current thread's thread state structure
  //
  // expects the address of the thread manager to be in r0,
  // takes no other arguments, returns nothing, maintains r0
//  .global tll_save
//  .thumb_func
//tll_save:
  // get thread state location
//  LDR r3, =current_thread_id                  // address of current thread id variable
//  MOVS r1, TLL_SAVED_THREAD_STATE__SIZE       // size of the tll_saved_thread_state struct
//  LDR r2, [r0]                                // deref thread_manager arg to get states pointer
//  LDR ip, [r3]                                // deref thread id addr to get thread id
//  MLA r2, r1, ip, r2                          // index into thread_manager states array by thread id

  // store registers to tll_saved_thread_state struct
//  STMIA r2!, {r4-r11}                          // store saved registers

