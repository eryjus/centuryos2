;;===================================================================================================================
;;
;;  kMemSetB.s -- Set a block of memory to the specified value by bytes
;;
;;        Copyright (c)  2017-2021 -- Adam Clark
;;        Licensed under "THE BEER-WARE LICENSE"
;;        See License.md for details.
;;
;;  Set a block of memory to the specified value.  This function operates with bytes being passed into the function,
;;  so cnt contains the number of bytes to fill.
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
;;  void kMemSetB(void *buf, uint8_t byt, size_t cnt);
;;
;; ------------------------------------------------------------------------------------------------------------------
;;
;;     Date      Tracker  Version  Pgmr  Description
;;  -----------  -------  -------  ----  ---------------------------------------------------------------------------
;;  2021-May-11  Initial  v0.0.9a  ADCL  Copied this file from kMemSetW and updated
;;
;;===================================================================================================================


;;
;; -- Expose labels to fucntions that the linker can pick up
;;    ------------------------------------------------------
    global  kMemSetB


;;
;; -- This is the beginning of the code segment for this file
;;    -------------------------------------------------------
    section .text
    bits    64


;;
;; -- Clear or set a block of memory to the specified value
;;    -----------------------------------------------------
kMemSetB:
    mov     rcx,rdx                     ;; get the number of bytes to set
    xor     rax,rsi                     ;; clear rax
    cld                                 ;; make sure we are incrementing
    rep     stosb                       ;; store the byte
    ret
