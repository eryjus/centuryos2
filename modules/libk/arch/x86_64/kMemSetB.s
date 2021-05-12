;;===================================================================================================================
;;
;;  kMemSetB.s -- Set a block of memory to the specified value by bytes
;;
;;        Copyright (c)  2017-2020 -- Adam Clark
;;        Licensed under "THE BEER-WARE LICENSE"
;;        See License.md for details.
;;
;;  Set a block of memory to the specified value.  This function operates with bytes being passed into the function,
;;  so cnt contains the number of bytes to fill.
;;
;;  On entry, the stack has the following structure:
;;  +-----------+-----------+------------------------------------+
;;  |  via rbp  |  via esp  |  description of the contents       |
;;  +-----------+-----------+------------------------------------+
;;  | rbp + 32  | rsp + 24  |  Number of bytes to set            |
;;  +-----------+-----------+------------------------------------+
;;  | rbp + 24  | rsp + 16  |  The byte to set in the memory     |
;;  +-----------+-----------+------------------------------------+
;;  | rbp + 16  | rsp + 08  |  The memory location to set        |
;;  +-----------+-----------+------------------------------------+
;;  | rbp + 08  |   rsp     |  Return RIP                        |
;;  +-----------+-----------+------------------------------------+
;;  |   rbp     | rsp - 08  |  RBP                               |
;;  +-----------+-----------+------------------------------------+
;;  | rbp - 08  | rsp - 16  |  RAX                               |
;;  +-----------+-----------+------------------------------------+
;;  | rbp - 16  | rsp - 24  |  RCX                               |
;;  +-----------+-----------+------------------------------------+
;;  | rbp - 24  | rsp - 32  |  RDI                               |
;;  +-----------+-----------+------------------------------------+
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
    push    rbp                         ;; create a frame
    mov     rbp,rsp                     ;; ... create a frame
    push    rax                         ;; save rax
    push    rcx                         ;; save rcx
    push    rdi                         ;; save rdi

    mov     rax,[rbp+16]                ;; get the memory location to set
    mov     rdi,rax                     ;; and put it in rdi
    mov     rcx,[rbp+32]                ;; get the number of bytes to set
    mov     al,[rbp+24]                 ;; get the byte to set
    cld                                 ;; make sure we are incrementing
    rep     stosb                       ;; store the byte

    pop     rdi                         ;; restore rdi
    pop     rcx                         ;; restore rcx
    pop     rax                         ;; restore rax
    pop     rbp                         ;; restore previous frame
    ret
