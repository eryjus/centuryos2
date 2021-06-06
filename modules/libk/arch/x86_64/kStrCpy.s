;;===================================================================================================================
;;
;;  kStrCpy.s -- copy a string to a buffer
;;
;;        Copyright (c)  2017-2020 -- Adam Clark
;;        Licensed under "THE BEER-WARE LICENSE"
;;        See License.md for details.
;;
;;  copy a string into a buffer.  The buffer must be guaranteed to be big enough to hold the string
;;
;;  Prototype:
;;  void kStrCpy(char *dest, char *src);
;;
;; -----------------------------------------------------------------------------------------------------------------
;;
;;    Date      Tracker  Version  Pgmr  Description
;; -----------  -------  -------  ----  ---------------------------------------------------------------------------
;; 2021-May-12  Initial  v0.0.9b  ADCL  Initial version -- Copied from CenturyOS
;;
;;===================================================================================================================

;;
;; -- Expose labels to fucntions that the linker can pick up
;;    ------------------------------------------------------
    global      kStrCpy


;;
;; -- This is the beginning of the code segment for this file
;;    -------------------------------------------------------
    section     .text
    bits        64


;;
;; -- Copy a string to a new location
;;    -------------------------------
kStrCpy:
    cld
    push        rsi                         ;; save this register
    push        rdi                         ;; and this one

.loop:
    mov        al,[rsi]                     ;; get the character
    mov        [rdi],al                     ;; and set the new character

    cmp        al,0                         ;; is the string over?
    je        .out                          ;; if so, leave

    inc        rsi                          ;; move to the next character
    inc        rdi                          ;; and the next loc in the buffer

    jmp        .loop                        ;; and do it again

.out:
    pop        rdi                          ;; restore the register
    pop        rsi                          ;; and this one
    ret
