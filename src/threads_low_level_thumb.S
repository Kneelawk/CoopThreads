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


  // Threads Low-Level subroutine for performing a setjmp but with a custom stack
  //
  // Note: it is not safe to return if this returns a non-zero value
  //
  // Arguments:
  // r0: pointer to buffer to store jmp registers into
  // r1: new stack pointer
  // r2: new stack limit
  // Returns:
  // r0: setjmp result
  .global tll_setjmp_stack
  .thumb_func
tll_setjmp_stack:
  // save old r4, r5, & r6 on old stack
  PUSH {r4, r5, r6} 
  // save old sp, sl, & lr in saved registers
  MOV r4, sp
  MOV r5, sl
  MOV r6, lr
  // apply sp & sl
  MOV sp, r1
  MOV sl, r2
  // call setjmp
  BL setjmp
  // we're back from setjmp, either we've just setup our jmp buf, or we've been jumped to
  CBZ r0, _tll_setjmp_stack_continue
  // we've been jumped to, so just return now with the new stack
  BX r6

_tll_setjmp_stack_continue:
  // we've just setup our jump buf, so reset everything to how it was when we were called
  MOV sl, r5
  MOV sp, r4
  POP {r4, r5, r6}
  BX r6
