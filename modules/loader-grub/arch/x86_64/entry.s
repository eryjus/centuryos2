;;===================================================================================================================
;;
;;  entry.s -- Entry point for x86_64 architecture
;;
;;        Copyright (c)  2017-2021 -- Adam Clark
;;        Licensed under "THE BEER-WARE LICENSE"
;;        See License.md for details.
;;
;;  Based on the multiboot specification, the multiboot loader will hand off control to the following file at the
;;  loader location.
;;
;; -----------------------------------------------------------------------------------------------------------------
;;
;;     Date      Tracker  Version  Pgmr  Description
;;  -----------  -------  -------  ----  --------------------------------------------------------------------------
;;  2021-Jan-02  Initial  v0.0.1   ADCL  Initial version
;;
;;===================================================================================================================


%include "constants.inc"


;;
;; -- expose some global symbols
;;    --------------------------
                global      entry
                global      earlyFrame
                global      mbSig
                global      mbData
                global      JumpKernel
                global      kernelInterface
                global      pml4
                global      gdtr64


;;
;; -- declare some external symbols
;;    -----------------------------
                extern      lInit


;;
;; -- Some equates to make things easier
;;    ----------------------------------
LEN2            equ         mb2HdrEnd - mb2HdrBeg
CHECK2          equ         (-(MAGIC2 + LEN2) & 0xffffffff)


;;
;; -- Start with the .mboot section, which will be located at the start of the final executable
;;    -----------------------------------------------------------------------------------------
                section     .mboot


;;
;; -- This is the multiboot 1 header
;;    ------------------------------
                align       4
multiboot_header:
;; -- magic fields
                dd          MAGIC1
                dd          MBFLAGS
                dd          -MAGIC1-MBFLAGS
;; -- address fields (unused placeholders)
                dd          0
                dd          0
                dd          0
                dd          0
                dd          0
;; -- video fields
                dd          MODE_TYPE
                dd          WIDTH
                dd          HEIGHT
                dd          DEPTH


;;
;; -- This is the multiboot 2 header
;;    ------------------------------
                align       8
mb2HdrBeg:
                dd          MAGIC2
                dd          0                               ;; architecture: 0=32-bit protected mode
                dd          LEN2                            ;; total length of the mb2 header
                dd          CHECK2                          ;; mb2 checksum

                align       8
Type1Start:
                dw          1                               ;; type=1
                dw          0                               ;; not optional
                dd          Type1End-Type1Start             ;; size = 40
                dd          1                               ;; provide boot command line
                dd          2                               ;; provide boot loader name
                dd          3                               ;; provide module info
                dd          4                               ;; provide basic memory info
                dd          5                               ;; provide boot device
                dd          6                               ;; provide memory map
                dd          8                               ;; provide frame buffer info
                dd          9                               ;; provide elf symbol table
Type1End:

                align       8
Type4Start:
                dw          4                               ;; type=4
                dw          0                               ;; not optional
                dd          Type4End-Type4Start             ;; size = 12
                dd          1                               ;; graphics
Type4End:

                align       8
Type5Start:
                dw          5                               ;; graphic mode
                dw          1                               ;; not optional
                dd          Type5End-Type5Start             ;; size = 20
                dd          WIDTH                           ;; 1024
                dd          HEIGHT                          ;; 768
                dd          DEPTH                           ;; 16
Type5End:

                align       8
Type6Start:
                dw          6                               ;; Type=6
                dw          1                               ;; Not optional
                dd          Type6End-Type6Start             ;; size = 8 bytes even tho the doc says 12
Type6End:

                align       8
Type8Start:
                dw          0                               ;; Type=0
                dw          0                               ;; flags=0
                dd          8                               ;; size=8
mb2HdrEnd:


;;
;; -- the next early frame to allocate
;;    --------------------------------
earlyFrame:     dd          (4 * 1024)                      ;; we start allocating frames at 16M
                dd          0                               ;; for when we use uint64_t...


kernelInterface:
                dq          0                               ;; the address of the interface structure


;;
;; -- we will use these variables to keep track of the paging tables
;;    --------------------------------------------------------------
pml4:           dq          0
pdpt:           dq          0
pd:             dq          0
pt:             dq          0


;;
;; -- TOS for mapping the pages
;;    -------------------------
tos:            dd          0


;;
;; -- Multiboot information
;;    ---------------------
mbSig:          dd          0
mbData:         dd          0

;;
;; -- The following is our GDT we need to enter 64-bit code
;;    -----------------------------------------------------
                align       8

gdt64:
                dq          0                   ; GDT entry 0x00 (NULL)
                dq          0x00a09a0000000000  ; GDT entry 0x08 (KERNEL CODE)
                dq          0x00a0920000000000  ; GDT entry 0x10 (KERNEL DATA)
gdt64End:

gdtr64:                                     ; this is the GDT to jump into long mode
                dw          gdt64End-gdt64-1
                dd          gdt64


;;
;; == This is the main entry point for the loader
;;    ===========================================
                align       4
                bits        32


entry:
                mov         [mbSig],eax
                mov         [mbData],ebx

;;
;; -- allocate a stack
;;    ----------------
                add         dword [earlyFrame],4
                mov         eax,[earlyFrame]
                shl         eax,12
                mov         [tos],eax

;;
;; -- now, start setting up the paging tables
;;    ---------------------------------------
                mov         eax,[earlyFrame]                ;; get the next frame
                inc         dword [earlyFrame]              ;; and move to the next frame
                shl         eax,12                          ;; adjust to an address
                mov         [pml4],eax                      ;; save its location

                mov         edi,eax                         ;; get ready to clear the block
                mov         ecx,1024                        ;; number of words
                xor         eax,eax                         ;; clear eax
                rep         stosd                           ;; clear the block

;; -- pdpt table
                mov         eax,[earlyFrame]                ;; get the next frame
                inc         dword [earlyFrame]              ;; and move to the next frame
                shl         eax,12                          ;; adjust to an address
                mov         [pdpt],eax                      ;; save its location

                mov         edi,eax                         ;; get ready to clear the block
                mov         ecx,1024                        ;; number of words
                xor         eax,eax                         ;; clear eax
                rep         stosd                           ;; clear the block

;; -- pd table
                mov         eax,[earlyFrame]                ;; get the next frame
                inc         dword [earlyFrame]              ;; and move to the next frame
                shl         eax,12                          ;; adjust to an address
                mov         [pd],eax                        ;; save its location

                mov         edi,eax                         ;; get ready to clear the block
                mov         ecx,1024                        ;; number of words
                xor         eax,eax                         ;; clear eax
                rep         stosd                           ;; clear the block

;; -- pt table
                mov         eax,[earlyFrame]                ;; get the next frame
                inc         dword [earlyFrame]              ;; and move to the next frame
                shl         eax,12                          ;; adjust to an address
                mov         [pt],eax                        ;; save its location

                mov         edi,eax                         ;; get ready to clear the block
                mov         ecx,1024                        ;; number of words
                xor         eax,eax                         ;; clear eax
                rep         stosd                           ;; clear the block


;;
;; -- create the mappings require for this address.   this code is loaded at 1M (0x100000).
;;    for this, we take care of the following entries:
;;    pml4[0] = pdpt
;;    pdpt[0] = pd
;;    pd[0] = pt
;;    pt[256] = 0x100000
;;    -------------------------------------------------------------------------------------
                mov         ebx,[pml4]                      ;; get the address of the pml4 table
                mov         eax,[pdpt]                      ;; get the address of the pdpt table
                or          eax,0x103                       ;; set the flags
                mov         [ebx],eax                       ;; set the pdpt table

                mov         eax,ebx                         ;; get the pml4 for recursive mapping
                or          eax,0x103                       ;; set the flags
                mov         [ebx + (511*8)],eax             ;; recursively map the tables

                mov         ebx,[pdpt]                      ;; get the address of the pdpt table
                mov         eax,[pd]                        ;; get the address of the pd table
                or          eax,0x103                       ;; set the flags
                mov         [ebx],eax                       ;; set the pd table

                mov         ebx,[pd]                        ;; get the address of the pd table
                mov         eax,[pt]                        ;; get the address of the pt table
                or          eax,0x103                       ;; set the flags
                mov         [ebx],eax                       ;; set the pt table

                mov         ebx,[pt]                        ;; get the address of the pt table
                mov         eax,0x100103                    ;; set this loader address and flags
                mov         [ebx + (256*8)],eax             ;; set the page

                add         eax,0x1000                      ;; next frame
                mov         [ebx + (257*8)],eax             ;; set the page

                add         eax,0x1000                      ;; next frame
                mov         [ebx + (258*8)],eax             ;; set the page

                add         eax,0x1000                      ;; next frame
                mov         [ebx + (259*8)],eax             ;; set the page


;; -- take care of some additional mappings now
                mov         eax,0xb0103                     ;; video output
                mov         [ebx + (0xb0*8)],eax            ;; map that


;;
;; -- now, prepare to enter long mode
;;    -------------------------------
                mov         eax,cr4                         ;; get cr4
                or          eax,0x000000a0                  ;; enable PAE and PGE
                mov         cr4,eax                         ;; write the control register back

                mov         eax,[pml4]                      ;; get the top level address
                mov         cr3,eax                         ;; write teh control register

                mov         ecx,0xC0000080                  ;; get the EFER
                rdmsr

                or          eax,0x00000100                  ;; set the LME bit
                wrmsr                                       ;; and write the register back

                mov         eax,cr0                         ;; get cr0
                or          eax,0x80000001                  ;; enable paging (and protection)
                mov         cr0,eax                         ;; write the control register back (paging is enabled!)

                lgdt        [gdtr64]                        ;; and load the GDT

                jmp         08:LongMode                     ;; jump into long mode


;;
;; == Anything after here must be 64-bit code!
;;    ========================================
                align       8
                bits        64


LongMode:
                mov         rax,0x10                        ;; set the segment register
                mov         ss,ax
                mov         ds,ax
                mov         es,ax
                mov         fs,ax
                mov         gs,ax


;;
;; -- So...  map the new stack.  This will be at 16M (plus a few pages).  So, we just need to map a new Page Table
;;    and then map the pages.  The Page Table covers 2M, so this should only be 1 table.
;;    ------------------------------------------------------------------------------------------------------------
                xor         rcx,rcx
                mov         ecx,[earlyFrame]                ;; get a table
                inc         dword [earlyFrame]              ;; and move to the next frame
                shl         rcx,12                          ;; convert this to an address
                or          rcx,0x103                       ;; set the flags

                mov         rax,(0xffffffffc0000000+(0x200000*0)+(0x1000*0))
                mov         [rax+(8*8)],rcx                 ;; perform the mapping

                mov         rax,(0xffffff8000000000+(0x40000000*0)+(0x200000*0)+(0x1000*8))
                xor         rdx,rdx
                mov         edx,[tos]
                sub         edx,0x1000                      ;; move to start of frame
                mov         rcx,rdx
                shr         edx,12
                and         edx,0x1ff


;; -- complete the mappings
                or          rcx,0x103                       ;; frame 4
                mov         [rax+(rdx*8)],rcx

                sub         rcx,0x1000                      ;; frame 3
                dec         rdx
                mov         [rax+(rdx*8)],rcx

                sub         rcx,0x1000                      ;; frame 2
                dec         rdx
                mov         [rax+(rdx*8)],rcx

                sub         rcx,0x1000                      ;; frame 1
                dec         rdx
                mov         [rax+(rdx*8)],rcx

;; -- set the stack
                xor         rbx,rbx
                mov         ebx,[tos]
                mov         ax,ss
                mov         ss,ax
                mov         rsp,rbx

                jmp         lInit


;;
;; -- function to jump to the kernel proper
;;    -------------------------------------
JumpKernel:
                mov         rax,rdi
                mov         rdi,[kernelInterface]
                mov         rsp,rsi

                jmp         rax


