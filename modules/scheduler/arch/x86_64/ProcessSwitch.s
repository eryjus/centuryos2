;;===================================================================================================================
;;
;;  ProcessSwitch.s -- Execute a task switch at the lowest level
;;
;;  This function will perform a task switch.
;;
;;  The following represent the stack structure based on the esp/frame set up at entry:
;;  +-----------+------------------------------------+
;;  |    RSP    |  Point of reference                |
;;  +===========+====================================+
;;  | rsp + 08  |  The target Process_t structure    |
;;  +-----------+------------------------------------+
;;  |   rsp     |  Return RIP                        |
;;  +-----------+------------------------------------+
;;
;;  There are a couple of call-outs here for the changes.
;;  1) I was saving everything and it was overkill.  There is no need for this complexity
;;  2) eax,ecx, and edx are considered to be argument/return registers for the arm abi.  Any interesting value
;;     would have been saved by the caller so no need to worry about them.
;;  3) The virtual address space register does not need to be saved; it is static once identified and therefore
;;     it is already saved in the structure.
;;
;; ------------------------------------------------------------------------------------------------------------------
;;
;;  Function Prototype:
;;  extern "C" void ProcessSwitch(Process_t *nextProcess);
;;
;; ------------------------------------------------------------------------------------------------------------------
;;
;;     Date      Tracker  Version  Pgmr  Description
;;  -----------  -------  -------  ----  ---------------------------------------------------------------------------
;;  2021-May-25  Initial  v0.0.9b  ADCL  Initial version -- COpied from Century
;;
;;===================================================================================================================


;;
;; -- Now, expose our function to everyone
;;    ------------------------------------
    global  ProcessSwitch


;;
;; -- Some global variables that are referenced
;;    -----------------------------------------
    extern  scheduler
    extern  ProcessDoReady
    extern  ProcessUpdateTimeUsed


;;
;; -- Some local equates for use with access structure elements
;;    ---------------------------------------------------------
PROC_TOS_PROCESS_SWAP   EQU     0
PROC_TOS_KERNEL         EQU     8
PROC_TOS_INTERRUPTED    EQU     16
PROC_VIRT_ADDR_SPACE    EQU     24
PROC_STATUS             EQU     32
PROC_PRIORITY           EQU     40
PROC_QUANTUM_LEFT       EQU     48


;;
;; -- some local equates for accessing the structure offsets
;;    ------------------------------------------------------
SCH_CHG_PENDING         EQU     0x18
SCH_LOCK_COUNT          EQU     0x28
SCH_POSTPONE_COUNT      EQU     0x30


;;
;; -- Some additional constants for use when managing process status
;;    --------------------------------------------------------------
PROC_STS_RUNNING        EQU     1
PROC_STS_READY          EQU     2


;;
;; -- This is the beginning of the code segment for this file
;;    -------------------------------------------------------
section     .text
bits        64


;;
;; -- Execute a process switch
;;    ------------------------
ProcessSwitch:
;;
;; -- before we do too much, do we need to postpone?
;;    ----------------------------------------------
        push    rax

        mov     rax,scheduler
        add     rax,SCH_POSTPONE_COUNT
        cmp     qword [rax],0
        je      .cont

        mov     rax,scheduler
        add     rax,SCH_CHG_PENDING
        mov     qword [rax],1

        pop     rax
        ret

;;
;; -- From here we take on the task change
;;    ------------------------------------
.cont:
        push    rbx                         ;; save rbx
        push    rbp                         ;; save rbp
        push    r12                         ;; save r12
        push    r13                         ;; save r13
        push    r14                         ;; save r14
        push    r15                         ;; save r15


;;
;; -- Get the current task structure
;;    ------------------------------
        mov     rsi,[gs:8]                  ;; get the address of the current process

        cmp     dword [rsi+PROC_STATUS],PROC_STS_RUNNING    ;; is this the current running process
        jne     .saveStack

        push    rsi                                         ;; make the process ready
        call    ProcessDoReady
        add     rsp,8

.saveStack:
        call    ProcessUpdateTimeUsed

        mov     [rsi+PROC_TOS_PROCESS_SWAP],rsp ;; save the top of the current stack


;;
;; -- next, we get the next task and prepare to switch to that
;;    --------------------------------------------------------
        mov     rdi,[rsp+((4+1)*8)]         ;; get the new task's structure
        mov     [gs:8],edi                  ;; this is now the currnet task

        mov     rsp,[rdi+PROC_TOS_PROCESS_SWAP]  ;; get the stop of the next process stack
        mov     qword [rdi+PROC_STATUS],PROC_STS_RUNNING    ;; set the new process to be running
        mov     rax,qword [rdi+PROC_PRIORITY]   ;; get the priority, which becomes the next quantum
        add     qword [rdi+PROC_QUANTUM_LEFT],rax   ;; add it to the amount left to overcome "overdrawn" procs

;;
;; -- Here we redecorate the TSS
;;    --------------------------
        mov     rcx,[rdi+PROC_TOS_KERNEL]   ;; get the TOS for the kernel
        mov     rax,[gs:8]                  ;; get the cpu struct address
        mov     [rax+60],rcx                ;; and set the new kernel stack

        mov     rax,[rdi+PROC_VIRT_ADDR_SPACE]  ;; get the paing tables address
        mov     rcx,cr3                     ;; get the current paging tables

        cmp     rax,rcx                     ;; are they the same?
        je      .noVASchg                   ;; no need to perform a TLB flush

        mov     cr3,rax                     ;; replace the paging tables

.noVASchg:
        pop     r15                         ;; restore r15
        pop     r14                         ;; restore r14
        pop     r13                         ;; restore r13
        pop     r12                         ;; restore r12
        pop     rbp                         ;; restore rbp
        pop     rbx                         ;; restore rbx

        ret

