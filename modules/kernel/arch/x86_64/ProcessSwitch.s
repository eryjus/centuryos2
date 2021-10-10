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
    extern  ProcessDoReady
    extern  ProcessUpdateTimeUsed
;;    extern  _SchCheckPostpone
    extern  sch_ProcessReady

    extern  scheduler


;;
;; -- Some local equates for use with access structure elements
;;    ---------------------------------------------------------
PROC_TOS_PROCESS_SWAP   EQU     0
PROC_VIRT_ADDR_SPACE    EQU     8
PROC_STATUS             EQU     16      ;; dword size
PROC_PRIORITY           EQU     20      ;; dword size
PROC_QUANTUM_LEFT       EQU     24


;;
;; -- some local equates for accessing the structure offsets
;;    ------------------------------------------------------
SCH_CHG_PENDING         EQU     16    ;; byte size
SCH_LOCK_COUNT          EQU     32
SCH_POSTPONE_COUNT      EQU     40


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
;;
;;    CRITICAL: the scheduler lock must be held before calling this function.  If it is not,
;;    undesireable results will occur.
;;    --------------------------------------------------------------------------------------
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
        mov     byte [rax],1

        pop     rax
        ret

;;
;; -- From here we take on the task change
;;    ------------------------------------
.cont:
        push    rbx                         ;; save rbx
        push    rcx                         ;; save rcx
        push    rdx                         ;; save rdx
        push    rsi                         ;; save rsi
        push    rdi                         ;; save rdi
        push    rbp                         ;; save rbp
        push    r8                          ;; save r8
        push    r9                          ;; save r9
        push    r10                         ;; save r10
        push    r11                         ;; save r11
        push    r12                         ;; save r12
        push    r13                         ;; save r13
        push    r14                         ;; save r14
        push    r15                         ;; save r15

        call    ProcessUpdateTimeUsed

        pop     r15                         ;; restore r15
        pop     r14                         ;; restore r14
        pop     r13                         ;; restore r13
        pop     r12                         ;; restore r12
        pop     r11                         ;; restore r11
        pop     r10                         ;; restore r10
        pop     r9                          ;; restore r9
        pop     r8                          ;; restore r8
        pop     rbp                         ;; restore rbp
        pop     rdi                         ;; restore rdi
        pop     rsi                         ;; restore rsi
        pop     rdx                         ;; restore rdx
        pop     rcx                         ;; restore rcx
        pop     rbx                         ;; restore rbx

        push    rbx                         ;; save rbx
        push    rcx                         ;; save rcx
        push    rdx                         ;; save rdx
        push    rsi                         ;; save rsi
        push    rdi                         ;; save rdi
        push    rbp                         ;; save rbp
        push    r8                          ;; save r8
        push    r9                          ;; save r9
        push    r10                         ;; save r10
        push    r11                         ;; save r11
        push    r12                         ;; save r12
        push    r13                         ;; save r13
        push    r14                         ;; save r14
        push    r15                         ;; save r15


;;
;; -- Get the current task structure
;;    ------------------------------
        mov     r14,[gs:8]                  ;; get the address of the current process
        mov     r15,rdi                     ;; save the target process to a preserved register

        cmp     dword [r14+PROC_STATUS],PROC_STS_RUNNING    ;; is this the current running process
        jne     .saveStack                  ;; if not RUNNING, do not make the process ready

        mov     rsi,r14                     ;; get the current process to make it ready

        push    r14
        call    sch_ProcessReady
        pop     r14

.saveStack:
        mov     [r14+PROC_TOS_PROCESS_SWAP],rsp ;; save the top of the current stack


;;
;; -- next, we get the next task and prepare to switch to that
;;    --------------------------------------------------------
        mov     [gs:8],r15                  ;; this is now the current task

        mov     rsp,[r15+PROC_TOS_PROCESS_SWAP]  ;; get the stop of the next process stack
        mov     dword [r15+PROC_STATUS],PROC_STS_RUNNING    ;; set the new process to be running
        xor     rax,rax
        mov     eax,dword [r15+PROC_PRIORITY]   ;; get the priority, which becomes the next quantum
        add     qword [r15+PROC_QUANTUM_LEFT],rax   ;; add it to the amount left to overcome "overdrawn" procs

;;
;; -- Here we redecorate the TSS
;;    --------------------------
        mov     rax,[r15+PROC_VIRT_ADDR_SPACE]  ;; get the paing tables address
        mov     rcx,cr3                     ;; get the current paging tables

        cmp     rax,rcx                     ;; are they the same?
        je      .noVASchg                   ;; no need to perform a TLB flush

        mov     cr3,rax                     ;; replace the paging tables

.noVASchg:
        pop     r15                         ;; restore r15
        pop     r14                         ;; restore r14
        pop     r13                         ;; restore r13
        pop     r12                         ;; restore r12
        pop     r11                         ;; restore r11
        pop     r10                         ;; restore r10
        pop     r9                          ;; restore r9
        pop     r8                          ;; restore r8
        pop     rbp                         ;; restore rbp
        pop     rdi                         ;; restore rdi
        pop     rsi                         ;; restore rsi
        pop     rdx                         ;; restore rdx
        pop     rcx                         ;; restore rcx
        pop     rbx                         ;; restore rbx
        pop     rax                         ;; restore rax (from the very first push)

        ret

