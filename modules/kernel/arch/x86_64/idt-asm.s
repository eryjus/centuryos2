;;===================================================================================================================
;;
;;  idt-asm.cc -- assembly-level interfacing to the IDT
;;
;;        Copyright (c)  2017-2021 -- Adam Clark
;;        Licensed under "THE BEER-WARE LICENSE"
;;        See License.md for details.
;;
;;  -----------------------------------------------------------------------------------------------------------------
;;
;;     Date      Tracker  Version  Pgmr  Description
;;  -----------  -------  -------  ----  ---------------------------------------------------------------------------
;;  2021-Jan-13  Initial  v0.0.2   ADCL  Initial version
;;
;;===================================================================================================================


        global  InternalTarget

        extern  internalTable
        extern  serviceTable
        extern  vectorTable
        extern  krn_SpinLock
        extern  krn_SpinUnlock


MAX_HANDLERS    equ         1024

        cpu     x64
        section .text


;;
;; -- This is a macro to mimic the pusha instruction which is not available in x64
;;    ----------------------------------------------------------------------------
%macro  PUSHA 0
        push    rax
        push    rbx
        push    rcx
        push    rdx
        push    rbp
        push    rsi
        push    rdi
        push    r8
        push    r9
        push    r10
        push    r11
        push    r12
        push    r13
        push    r14
        push    r15
%endmacro



;;
;; -- This is a macro to mimic the popa instrucion, which is not available in x64
;;    ---------------------------------------------------------------------------
%macro  POPA 0
        pop     r15
        pop     r14
        pop     r13
        pop     r12
        pop     r11
        pop     r10
        pop     r9
        pop     r8
        pop     rdi
        pop     rsi
        pop     rbp
        pop     rdx
        pop     rcx
        pop     rbx
        pop     rax
%endmacro


;;
;; -- These are macros to facilitate getting an interrupt exception entry point built
;;    -------------------------------------------------------------------------------
%macro INT_ERROR 1
        global  int%1
int%1:
        push    qword %1                ;; the interrupt number
        jmp     IntCommonTarget
%endmacro

%macro INT_NO_ERROR 1
        global  int%1
int%1:
        push    qword 0                 ;; a placeholder error code
        push    qword %1                ;; the interrupt number
        jmp     IntCommonTarget
%endmacro


;;
;; -- This is a macro to facilitate getting an ISR entry point built
;;    --------------------------------------------------------------
%macro ISR_ENTRY 1
        global  isr%1                   ;; make the label available to other modules

isr%1:
        push    qword 0                 ;; since no error was pushed, align the stack
        push    qword %1                ;; push the interrupt number
        push    rax
        push    rbx

        mov     rax,[rsp+16]
        mov     rbx,rax
        shl     rax,5                   ;; 32 bytes in the structure; offset the service
        shl     rbx,4                   ;; add another 16 bytes to the structure;
        add     rax,rbx                 ;; total of 48 bytes.

        mov     rbx,vectorTable
        lea     rbx,[rbx+rax]           ;; load the table address

        call    CommonTarget            ;; jump to the common handler (below)
        jmp     ExitPoint
%endmacro


;;
;; -- Set up all the entry points for each interrupt vector
;;    -----------------------------------------------------
INT_NO_ERROR 0
INT_NO_ERROR 1
INT_NO_ERROR 2
INT_NO_ERROR 3
INT_NO_ERROR 4
INT_NO_ERROR 5
INT_NO_ERROR 6
INT_NO_ERROR 7
INT_ERROR 8
INT_NO_ERROR 9
INT_ERROR 10
INT_ERROR 11
INT_ERROR 12
INT_ERROR 13
INT_ERROR 14
INT_NO_ERROR 15
INT_NO_ERROR 16
INT_ERROR 17
INT_NO_ERROR 18
INT_NO_ERROR 19
INT_NO_ERROR 20
INT_ERROR 21
INT_NO_ERROR 22
INT_NO_ERROR 23
INT_NO_ERROR 24
INT_NO_ERROR 25
INT_NO_ERROR 26
INT_NO_ERROR 27
INT_NO_ERROR 28
INT_NO_ERROR 29
INT_NO_ERROR 30
INT_NO_ERROR 31
ISR_ENTRY 32
ISR_ENTRY 33
ISR_ENTRY 34
ISR_ENTRY 35
ISR_ENTRY 36
ISR_ENTRY 37
ISR_ENTRY 38
ISR_ENTRY 39
ISR_ENTRY 40
ISR_ENTRY 41
ISR_ENTRY 42
ISR_ENTRY 43
ISR_ENTRY 44
ISR_ENTRY 45
ISR_ENTRY 46
ISR_ENTRY 47
ISR_ENTRY 48
ISR_ENTRY 49
ISR_ENTRY 50
ISR_ENTRY 51
ISR_ENTRY 52
ISR_ENTRY 53
ISR_ENTRY 54
ISR_ENTRY 55
ISR_ENTRY 56
ISR_ENTRY 57
ISR_ENTRY 58
ISR_ENTRY 59
ISR_ENTRY 60
ISR_ENTRY 61
ISR_ENTRY 62
ISR_ENTRY 63
ISR_ENTRY 64
ISR_ENTRY 65
ISR_ENTRY 66
ISR_ENTRY 67
ISR_ENTRY 68
ISR_ENTRY 69
ISR_ENTRY 70
ISR_ENTRY 71
ISR_ENTRY 72
ISR_ENTRY 73
ISR_ENTRY 74
ISR_ENTRY 75
ISR_ENTRY 76
ISR_ENTRY 77
ISR_ENTRY 78
ISR_ENTRY 79
ISR_ENTRY 80
ISR_ENTRY 81
ISR_ENTRY 82
ISR_ENTRY 83
ISR_ENTRY 84
ISR_ENTRY 85
ISR_ENTRY 86
ISR_ENTRY 87
ISR_ENTRY 88
ISR_ENTRY 89
ISR_ENTRY 90
ISR_ENTRY 91
ISR_ENTRY 92
ISR_ENTRY 93
ISR_ENTRY 94
ISR_ENTRY 95
ISR_ENTRY 96
ISR_ENTRY 97
ISR_ENTRY 98
ISR_ENTRY 99
ISR_ENTRY 100
ISR_ENTRY 101
ISR_ENTRY 102
ISR_ENTRY 103
ISR_ENTRY 104
ISR_ENTRY 105
ISR_ENTRY 106
ISR_ENTRY 107
ISR_ENTRY 108
ISR_ENTRY 109
ISR_ENTRY 110
ISR_ENTRY 111
ISR_ENTRY 112
ISR_ENTRY 113
ISR_ENTRY 114
ISR_ENTRY 115
ISR_ENTRY 116
ISR_ENTRY 117
ISR_ENTRY 118
ISR_ENTRY 119
ISR_ENTRY 120
ISR_ENTRY 121
ISR_ENTRY 122
ISR_ENTRY 123
ISR_ENTRY 124
ISR_ENTRY 125
ISR_ENTRY 126
ISR_ENTRY 127
ISR_ENTRY 128
ISR_ENTRY 129
ISR_ENTRY 130
ISR_ENTRY 131
ISR_ENTRY 132
ISR_ENTRY 133
ISR_ENTRY 134
ISR_ENTRY 135
ISR_ENTRY 136
ISR_ENTRY 137
ISR_ENTRY 138
ISR_ENTRY 139
ISR_ENTRY 140
ISR_ENTRY 141
ISR_ENTRY 142
ISR_ENTRY 143
ISR_ENTRY 144
ISR_ENTRY 145
ISR_ENTRY 146
ISR_ENTRY 147
ISR_ENTRY 148
ISR_ENTRY 149
ISR_ENTRY 150
ISR_ENTRY 151
ISR_ENTRY 152
ISR_ENTRY 153
ISR_ENTRY 154
ISR_ENTRY 155
ISR_ENTRY 156
ISR_ENTRY 157
ISR_ENTRY 158
ISR_ENTRY 159
ISR_ENTRY 160
ISR_ENTRY 161
ISR_ENTRY 162
ISR_ENTRY 163
ISR_ENTRY 164
ISR_ENTRY 165
ISR_ENTRY 166
ISR_ENTRY 167
ISR_ENTRY 168
ISR_ENTRY 169
ISR_ENTRY 170
ISR_ENTRY 171
ISR_ENTRY 172
ISR_ENTRY 173
ISR_ENTRY 174
ISR_ENTRY 175
ISR_ENTRY 176
ISR_ENTRY 177
ISR_ENTRY 178
ISR_ENTRY 179
ISR_ENTRY 180
ISR_ENTRY 181
ISR_ENTRY 182
ISR_ENTRY 183
ISR_ENTRY 184
ISR_ENTRY 185
ISR_ENTRY 186
ISR_ENTRY 187
ISR_ENTRY 188
ISR_ENTRY 189
ISR_ENTRY 190
ISR_ENTRY 191
ISR_ENTRY 192
ISR_ENTRY 193
ISR_ENTRY 194
ISR_ENTRY 195
ISR_ENTRY 196
ISR_ENTRY 197
ISR_ENTRY 198
ISR_ENTRY 199
ISR_ENTRY 200
ISR_ENTRY 201
ISR_ENTRY 202
ISR_ENTRY 203
ISR_ENTRY 204
ISR_ENTRY 205
ISR_ENTRY 206
ISR_ENTRY 207
ISR_ENTRY 208
ISR_ENTRY 209
ISR_ENTRY 210
ISR_ENTRY 211
ISR_ENTRY 212
ISR_ENTRY 213
ISR_ENTRY 214
ISR_ENTRY 215
ISR_ENTRY 216
ISR_ENTRY 217
ISR_ENTRY 218
ISR_ENTRY 219
ISR_ENTRY 220
ISR_ENTRY 221
ISR_ENTRY 222
ISR_ENTRY 223
ISR_ENTRY 224
ISR_ENTRY 225
ISR_ENTRY 226
ISR_ENTRY 227
ISR_ENTRY 228
ISR_ENTRY 229
ISR_ENTRY 230
ISR_ENTRY 231
ISR_ENTRY 232
ISR_ENTRY 233
ISR_ENTRY 234
ISR_ENTRY 235
ISR_ENTRY 236
ISR_ENTRY 237
ISR_ENTRY 238
ISR_ENTRY 239
ISR_ENTRY 240
ISR_ENTRY 241
ISR_ENTRY 242
ISR_ENTRY 243
ISR_ENTRY 244
ISR_ENTRY 245
ISR_ENTRY 246
ISR_ENTRY 247
ISR_ENTRY 248
ISR_ENTRY 249
ISR_ENTRY 250
ISR_ENTRY 251
ISR_ENTRY 252
ISR_ENTRY 253
ISR_ENTRY 254
ISR_ENTRY 255



;;
;; -- This is the entry point for the generic IDT handler
;;    ---------------------------------------------------
IntCommonTarget:
        push    rax
        push    rbx

        mov     rax,[rsp+16]
        mov     rbx,rax
        shl     rax,5                   ;; 32 bytes in the structure; offset the service
        shl     rbx,4                   ;; add another 16 bytes to the structure;
        add     rax,rbx                 ;; total of 48 bytes.

        mov     rbx,vectorTable
        lea     rbx,[rbx+rax]           ;; load the table address

        call    CommonTarget            ;; jump to the common handler (below)
        jmp     ExitPoint



;;
;; -- This is the intenral function handler entry point
;;    -------------------------------------------------
InternalTarget:
        push    qword 0                 ;; fake error code
        push    rdi                     ;; service number
        push    rax
        push    rbx

        mov     rbx,rdi                 ;; capture the service number

        cmp     rbx,0
        jl      Einval
        cmp     rbx,MAX_HANDLERS
        jge     Einval

        push    rax
        mov     rax,rbx
        shl     rax,5                   ;; 32 bytes in the structure; offset the service
        shl     rbx,4                   ;; add another 16 bytes to the structure;
        add     rbx,rax                 ;; total of 48 bytes.

        mov     rax,internalTable
        lea     rbx,[rbx+rax]           ;; load the table address
        pop     rax

        call    CommonTarget            ;; jump to the common handler (below)
        mov     [rsp+8],rax             ;; set the return value
        jmp     ExitPoint



;;
;; -- This is the OS Service function handler entry point
;;    ---------------------------------------------------
ServiceTarget:
        push    qword 0                 ;; fake error code
        push    rdi                     ;; internal function number number
        push    rax
        push    rbx

        mov     rbx,rdi                 ;; capture the service number

        cmp     rbx,0
        jl      Einval
        cmp     rbx,MAX_HANDLERS
        jge     Einval

        push    rax                     ;; save rax as it may have relevant values
        mov     rax,rbx
        shl     rax,5                   ;; 32 bytes in the structure; offset the service
        shl     rbx,4                   ;; add another 16 bytes to the structure;
        add     rbx,rax                 ;; total of 48 bytes.

        mov     rax,serviceTable
        lea     rbx,[rbx+rax]           ;; load the table address
        pop     rax                     ;; restore rax as it may have relevant values

        call    CommonTarget            ;; jump to the common handler (below)
        mov     [rsp+8],rax             ;; set the return value
        jmp     ExitPoint


;;
;; -- Return the proper error code
;;    ----------------------------
Einval:
        mov     qword [rsp+8],-22
        jmp     ExitPoint


;;
;; -- All targets execute from here
;;    -----------------------------
CommonTarget:
        ;; -- check the function; exit if none to call
        cmp     qword [rbx+0],0
        je      Exit

        ;; -- Push general purpose registers
        push    rcx
        push    rdx
        push    rbp
        push    rsi
        push    rdi
        push    r8
        push    r9
        push    r10
        push    r11
        push    r12
        push    r13
        push    r14
        push    r15

        ;; -- Push control registers
        mov     rbp,cr0
        push    rbp
        mov     rbp,cr2
        push    rbp
        mov     rbp,cr3
        mov     r12,rbp         ;; save for later
        push    rbp
        mov     rbp,cr4
        push    rbp

        ;; -- Push segment registers
        xor     rbp,rbp
        mov     bp,ds
        push    rbp
        mov     bp,es
        push    rbp
        mov     bp,fs
        push    rbp
        mov     bp,gs
        push    rbp

        ;; -- Now we need to lock the stack until the address space and the stack are replaced
        cmp     qword [rbx+16],0
        je      NoLock

        PUSHA
        lea     rsi,[rbx+32]
        call    krn_SpinLock
        POPA

        ;; -- save the old stack location for register values
NoLock:
        mov     [rbx+24],rsp            ;; save the stack pointer containing the regs

        ;; -- no more stack activity!
        mov     rbp,[rbx+8]
        cmp     rbp,0
        je      NoCr3

        mov     cr3,rbp                 ;; maps the new address space

NoCr3:
        mov     r11,rsp                 ;; save the stack location for later
        mov     rbp,[rbx+16]
        cmp     rbp,0
        je      NoStack

        ;; -- get the new stack location
        mov     rsp,rbp

NoStack:
        ;; -- restore stack operations
        mov     rdi,rbx                 ;; function call parameter

        push    r11                     ;; save the old stack value
        push    r12                     ;; save the old cr3 value
        push    rbx                     ;; save the structure location

        call    [rbx+0]                 ;; make the actual handler function call

        pop     rbx                     ;; restore the structure location
        pop     r12                     ;; get the old cr3
        pop     r11                     ;; get the old stack value

        ;; -- lock stack operations again; address space not static
        mov     rsp,r11                 ;; unconditionally restore the stack

        ;; -- do we need to restore a virtual address context?
        mov     r11,cr3
        cmp     r11,r12
        je      NoCr3Restore

        mov     cr3,r12                 ;; restore the old cr3

NoCr3Restore:
        ;; -- Restore stack operations -- back in an original context
        cmp     qword [rbx+16],0
        je      NoUnlock

NoUnlock:
        ;; -- Unlock the stack
        PUSHA
        lea     rsi,[rbx+32]
        call    krn_SpinUnlock
        POPA

        ;; -- pop the segment registers
        pop     rbp                     ;; discard gs
        pop     rbp                     ;; discard fs

        pop     rbp
        mov     es,bp
        pop     rbp
        mov     ds,bp

        ;; -- pop the control registers
        pop     rbp                     ;; discard cr4
        pop     rbp                     ;; discard cr3 -- already handled
        pop     rbp                     ;; discard cr2
        pop     rbp                     ;; discard cr0

        ;; -- pop the general purpose registers
        pop     r15
        pop     r14
        pop     r13
        pop     r12
        pop     r11
        pop     r10
        pop     r9
        pop     r8
        pop     rdi
        pop     rsi
        pop     rbp
        pop     rdx
        pop     rcx

Exit:
        ret

ExitPoint:
        pop     rbx
        pop     rax
        add     rsp,16                  ;; discard error and function/service/interrupt #

        iretq


