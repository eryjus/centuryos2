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


                global      IdtGenericEntry
                global      IdtGenericEntryNoErr
                global      InternalTarget

                extern      IdtGenericHandler
                extern      maxHandlers
                extern      internalTable
                extern      serviceTable
                extern      vectorTable


                cpu         x64
                section     .text



;;
;; -- This is a macro to facilitate getting an ISR entry point built
;;    --------------------------------------------------------------
%macro ISR_ENTRY 1
global      isr%1                       ;; make the label available to other modules

isr%1:
    push    dword 0                     ;; since no error was pushed, align the stack
    push    dword %1                    ;; push the interrupt number
    jmp     IsrCommonStub               ;; jump to the common handler (below)
%endmacro


;;
;; -- Set up all the entry points for each interrupt vector
;;    -----------------------------------------------------
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
;; -- The common interrupt handling code
;;    ----------------------------------
IsrCommonStub:
                push        rax
                mov         rax,[rsp+8]                 ;; get the interrupt vector

                push        rbx
                push        rcx
                push        rdx
                push        rbp
                push        rsi
                push        rdi
                push        r8
                push        r9
                push        r10
                push        r11
                push        r12
                push        r13
                push        r14
                push        r15

                mov         rbx,cr3
                push        rbx

                cmp         rax,0
                jl          .invalid

                cmp         rax,256
                jge         .invalid

                mov         rbx,vectorTable
                shl         rax,4                   ;; -- adjust for 16 bytes per entry
                lea         rbx,[rbx + rax]

                cmp         qword [rbx],0
                je          .none

                mov         r10,[rbx + 8]
                mov         rbx,[rbx]

;; -- check for a change in cr3
                cmp         r10,0
                je          .nocr3

                mov         r14,cr3
                cmp         r10,r14
                je          .nocr3

                mov         cr3,r10

.nocr3:
                mov         rdi,rsp                     ;; the pionter to the stack containing the variables
                sub         rdi,8
                call        rbx
                jmp         .out

.invalid:
                mov         rax,-22
                jmp         .out

.none:
                mov         rax,-12

.out:
                pop         rbx                 ;; -- previous cr3
                mov         r14,cr3

                cmp         r14,rbx
                je          .skip

                mov         cr3,rbx

.skip:
                pop         r15
                pop         r14
                pop         r13
                pop         r12
                pop         r11
                pop         r10
                pop         r9
                pop         r8
                pop         rdi
                pop         rsi
                pop         rbp
                pop         rdx
                pop         rcx
                pop         rbx
                pop         rax

                add         rsp,16                      ;; skip the error code and vector
                iretq


;;
;; -- This is the entry point for the generic IDT handler
;;    ---------------------------------------------------
IdtGenericEntryNoErr:
                push        qword 0
IdtGenericEntry:
                push        qword 0         ;; -- This is the interrupt vector number
                push        rax
                push        rbx
                push        rcx
                push        rdx
                push        rbp
                push        rsi
                push        rdi
                push        r8
                push        r9
                push        r10
                push        r11
                push        r12
                push        r13
                push        r14
                push        r15

                mov         rax,cr3
                push        rax

                xor         rax,rax
                mov         ax,ds
                push        rax

                mov         ax,es
                push        rax

                mov         ax,fs
                push        rax

                mov         ax,gs
                push        rax

                mov         rdi,rsp                     ;; the pionter to the stack containing the variables
                sub         rdi,8
                call        IdtGenericHandler

                pop         rax
                mov         cr3,rax

                pop         rax                         ;; gs
                pop         rax                         ;; fs
                pop         rax                         ;; es
                pop         rax                         ;; ds

                pop         r15
                pop         r14
                pop         r13
                pop         r12
                pop         r11
                pop         r10
                pop         r9
                pop         r8
                pop         rdi
                pop         rsi
                pop         rbp
                pop         rdx
                pop         rcx
                pop         rbx
                pop         rax

                add         rsp,16                       ;; skip the error code and vector

                iretq


;;
;; -- This is the intenral function handler entry point
;;    -------------------------------------------------
InternalTarget:
                push        rax
                push        rbx
                push        rcx
                push        rdx
                push        rbp
                push        rsi
                push        rdi
                push        r8
                push        r9
                push        r10
                push        r11
                push        r12
                push        r13
                push        r14
                push        r15

                mov         rbx,cr3
                push        rbx

                mov         r11,maxHandlers
                mov         r15,[r11]
                cmp         rdi,0
                jl          .invalid

                cmp         rdi,r15
                jge         .invalid

                mov         rbx,internalTable
                shl         rdi,4                   ;; -- adjust for 16 bytes per entry
                lea         rbx,[rbx + rdi]

                cmp         qword [rbx],0
                je          .none

                mov         r10,[rbx + 8]
                mov         rbx,[rbx]

;; -- check for a change in cr3
                cmp         r10,0
                je          .nocr3

                mov         r14,cr3
                cmp         r10,r14
                je          .nocr3

                mov         cr3,r10


.nocr3:
                mov         rdi,rsi
                mov         rsi,rdx
                mov         rdx,rcx
                mov         rcx,r8
                mov         r8,r9
                call        rbx
                jmp         .out

.invalid:
                mov         rax,-22
                jmp         .out

.none:
                mov         rax,-12

.out:
                pop         rbx                 ;; -- previous cr3
                mov         r14,cr3

                cmp         r14,rbx
                je          .skip

                mov         cr3,rbx

.skip:
                pop         r15
                pop         r14
                pop         r13
                pop         r12
                pop         r11
                pop         r10
                pop         r9
                pop         r8
                pop         rdi
                pop         rsi
                pop         rbp
                pop         rdx
                pop         rcx
                pop         rbx
                add         rsp,8

                iretq


;;
;; -- This is the OS Service function handler entry point
;;    ---------------------------------------------------
ServiceTarget:
                push        rax
                push        rbx
                push        rcx
                push        rdx
                push        rbp
                push        rsi
                push        rdi
                push        r8
                push        r9
                push        r10
                push        r11
                push        r12
                push        r13
                push        r14
                push        r15

                mov         rax,cr3
                push        rax

                mov         rax,maxHandlers
                mov         r15,[rax]
                cmp         rdi,0
                jl          .invalid

                cmp         rdi,r15
                jge         .invalid

                mov         rbx,serviceTable
                shl         rdi,4
                lea         rbx,[rbx + rdi]

                cmp         qword [rbx],0
                je          .none

                mov         rax,[rbx + 8]
                mov         rbx,[rbx]

;; -- check for a change in cr3
                cmp         rax,0
                je          .nocr3

                mov         rcx,cr3
                cmp         rax,rcx
                je          .nocr3

                mov         cr3,rcx


.nocr3:
                mov         rdi,rsi
                mov         rsi,rdx
                mov         rdx,rcx
                mov         rcx,r8
                mov         r8,r9
                call        rbx
                jmp         .out

.invalid:
                mov         rax,-22
                jmp         .out

.none:
                mov         rax,-12

.out:
                pop         rbx
                mov         cr3,rbx

                pop         r15
                pop         r14
                pop         r13
                pop         r12
                pop         r11
                pop         r10
                pop         r9
                pop         r8
                pop         rdi
                pop         rsi
                pop         rbp
                pop         rdx
                pop         rcx
                pop         rbx
                add         rsp,8

                iretq


