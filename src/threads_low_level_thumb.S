// ARM-based context-save/context-restore functions

// TODO: arduino arch filter

  .syntax unified
  .align 2
  .thumb

  .global retrieve_lr
  .thumb_func
retrieve_lr:
  