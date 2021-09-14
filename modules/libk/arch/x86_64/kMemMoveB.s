;;===================================================================================================================
;;
;;  kMemMoveB.s -- Copy a block of memory to the specified location
;;
;;        Copyright (c)  2017-2020 -- Adam Clark
;;        Licensed under "THE BEER-WARE LICENSE"
;;        See License.md for details.
;;
;;  Copy a block of memory to the specified location
;;
;;  On entry, the stack has the following structure:
;;  +-----------+------------------------------------+
;;  |   reg     |  description of the contents       |
;;  +-----------+-----------+------------------------------------+
;;  |   rdx     |  Number of bytes to set            |
;;  +-----------+------------------------------------+
;;  |   rsi     |  The byte to set in the memory     |
;;  +-----------+------------------------------------+
;;  |   rdi     |  The memory location to set        |
;;  +-----------+------------------------------------+
;;  |   rsp     |  Return RIP                        |
;;  +-----------+------------------------------------+
;;
;;  Prototype:
;;  void kMemMove(void *tgt, void *src, size_t cnt);
;;
;; ------------------------------------------------------------------------------------------------------------------
;;
;;     Date      Tracker  Version  Pgmr  Description
;;  -----------  -------  -------  ----  ---------------------------------------------------------------------------
;;  2021-Jun-18  Initial  v0.0.9d  ADCL  Copied this file from kMemMove and updated
;;
;;===================================================================================================================


;;
;; -- Expose labels to fucntions that the linker can pick up
;;    ------------------------------------------------------
    global  kMemMoveB


;;
;; -- This is the beginning of the code segment for this file
;;    -------------------------------------------------------
    section .text
    bits    64


;;
;; -- Copy a block of memory to a new location
;;    ----------------------------------------
kMemMoveB:
    mov     rcx,rdx                     ;; get the number of bytes to set
    xor     rax,rax                     ;; clear rax
    cld                                 ;; make sure we are incrementing
    rep     movsb                                               ;; copy the bytes
    ret
