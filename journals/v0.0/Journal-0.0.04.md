# The Century OS -- v0.0.4

This project is a 64-bit focused version of Century-OS.  This version takes from the 32-bit version of Century-OS, but is better streamlined for the 64-bit CPUs (where any 32-bit archs will be added in later if desired).


---

## Version 0.0.4 -- Start building out a module

A couple thing to do with this version:
1. Create a module (Perhaps the PMM module); set its header structure
3. Find the module and call the early init function (in sequence)


---

### 2021-Jan-24

So, the first thing I am dealing with here is the ISR -> Driver -> VMM mess.  If the driver has a different VMM space, how do I go about inserting the interrupt address into the internal functions table or the OS services table?

For the X86-64 arch it is not possible to replace the `cr3` register on interrupt.


---

### 2021-Jan-25

So, how to keep track of the proper `cr3` value on a system call?  All the loaded modules cannot exist in the same address space without this feature.  Can I tuck this into an entry point location?  Or better yet change the structure to be contain things like `cr3` and `rsp`?  The better place to put this is in the interrupt function table, and the reason is the need to maintain mappings to the entry point for each function if I place that value with the header signature.

So, this, then, will change the structure from just addresses to addresses and other necessary values (`cr3` for now) which will later be expanded as needed.


---

### 2021-Jan-26

OK, so with some thinking, I have the following decisions I am going to try to implement:
1. Each internal function will have its own `cr3` value to load.  Several will have the same `cr3`.
1. I want to be able to have a stack per function call.  However, that will not work if I want the function calls to be reentrant over several CPUs.  At best I will need 1 stack per CPU/module to make this work.  More commentary on this below.
1. I could also have one interrupt stack per CPU (which is stored in the TSS) and that would work.  The key to this working properly is that the OS services and internal functions need to be in such a way that either the service calls are not re-entrant or that they share the same stack.

So, when the IST field is set to `'0'`, the value for the stack is taken from RSP0 field of the TSS.  Any other value (1-7), and the IST it takes from the proper corresponding field from the TSS.  Other than not changing the privilege level, there is no facility to keep the stack the same.

So using `int n` when running at PL0 to PL0 (so say an OS service call to an internal function call), the same stack will be used; when using `int n` when running at PL3 to PL0 (such as a user-space procedure calling an OS service call), a new stack wll be used.s

In either case, interrupts will need to be disabled on entry.

So, to be clear, the `int 0xc8` call and the `int 0x64` will both use `IST0` and will then reference the `TSS.RSP0` field for its stack pointer for the interrupt.

From here, I need to:
* confirm the IDT setup for `0xc8` and `0x64` interrupts.
* set up a TSS -- dummy for now (But I should go ahead and plan for multi CPUs)
* change/update the ELF parser functions to have them in both the loader and the kernel (moving to a lib)
* create the PMM module and set it header structure
* find and parse the header structure from the kernel
* call the early init function (if not NULL)
* figure out how to sequence these function calls


---


### 2021-Jan-27

I have confirmed that the IDT is set properly for an internal function.  That complete I will start with the TSS.  The first order of business is to get the structure handled.

I am realizing that to get there, I am going to need to get a better more permanent GDT set up.  I will do this in C code (static definition) but enable it in the asm entry point of the kernel.

OK, the static GDT code compiles.  I will later have to do something about the CPU structure address.  The last thing on this endeavor is to get the GDT actually being used.  This should be easy, but let's see.

And I do of course result in a triple fault.  I figured it would be as much.  Will investigate why -- likely a couple of days.

---

So, my first problem appears to be that the `.data` section is mapped to the incorrect physical address.  This problem would be in the loader.

The sauce:

```
<bochs:2> reg
CPU0:
rax: ffffffff_80004000
rbx: 00000000_01004000
rcx: 00000000_001002c0
rdx: 00000000_00107040
rsp: 00000000_01003ff0
rbp: 00000000_00000000
rsi: 00000000_00000005
rdi: ffffffff_80001180
r8 : 0000000f_ffffffff
r9 : ffffff80_00000000
r10: 0000ffff_fffff000
r11: ffffffff_c0000000
r12: 00000000_00000000
r13: 00000000_00000000
r14: 00000000_00000000
r15: 00000000_00000000
rip: ffffffff_80001194
eflags 0x00210016: ID vip vif ac vm RF nt IOPL=0 of df if tf sf zf AF PF cf
<bochs:4> x 0xffffffff80004000
[bochs]:
0xffffffff80004000 <bogus+       0>:    0x0905c689
<bochs:7> page 0xffffffff80001000
PML4: 0x0000000001004163    ps         A pcd pwt S W P
PDPE: 0x0000000001009063    ps         A pcd pwt S W P
 PDE: 0x000000000100a063    ps         A pcd pwt S W P
 PTE: 0x0000000000108021       g pat d A pcd pwt S R P
linear page 0xffffffff80001000 maps to physical page 0x000000108000
<bochs:8> page 0xffffffff80002000
PML4: 0x0000000001004163    ps         A pcd pwt S W P
PDPE: 0x0000000001009063    ps         A pcd pwt S W P
 PDE: 0x000000000100a063    ps         A pcd pwt S W P
 PTE: 0x0000000000109001       g pat d a pcd pwt S R P
linear page 0xffffffff80002000 maps to physical page 0x000000109000
<bochs:9> page 0xffffffff80003000
PML4: 0x0000000001004163    ps         A pcd pwt S W P
PDPE: 0x0000000001009063    ps         A pcd pwt S W P
 PDE: 0x000000000100a063    ps         A pcd pwt S W P
 PTE: 0x000000000010a003       g pat d a pcd pwt S W P
linear page 0xffffffff80003000 maps to physical page 0x00000010a000
<bochs:3> page 0xffffffff80004000
PML4: 0x0000000001004163    ps         A pcd pwt S W P
PDPE: 0x0000000001009063    ps         A pcd pwt S W P
 PDE: 0x000000000100a063    ps         A pcd pwt S W P
 PTE: 0x0000000000001023       g pat d A pcd pwt S W P
linear page 0xffffffff80004000 maps to physical page 0x000000001000
<bochs:11> page 0xffffffff80005000
PML4: 0x0000000001004163    ps         A pcd pwt S W P
PDPE: 0x0000000001009063    ps         A pcd pwt S W P
 PDE: 0x000000000100a063    ps         A pcd pwt S W P
 PTE: 0x000000000010c001       g pat d a pcd pwt S R P
linear page 0xffffffff80005000 maps to physical page 0x00000010c000
```

```
Section Headers:
  [Nr] Name              Type             Address           Offset
       Size              EntSize          Flags  Link  Info  Align
  [ 0]                   NULL             0000000000000000  00000000
       0000000000000000  0000000000000000           0     0     0
  [ 1] .text             PROGBITS         ffffffff80001000  00001000
       00000000000010e9  0000000000000000  AX       0     0     16
  [ 2] .rodata           PROGBITS         ffffffff800020f0  000020f0
       0000000000000448  0000000000000000 AMS       0     0     8
  [ 3] .init_array       INIT_ARRAY       ffffffff80003000  00003000
       0000000000000008  0000000000000000  WA       0     0     8
  [ 4] .data             PROGBITS         ffffffff80004000  00004000
       0000000000000018  0000000000000000  WA       0     0     8
  [ 5] .bss              NOBITS           ffffffff80005000  00004018
       0000000000005010  0000000000000000  WA       0     0     4096
  [ 6] .ldata            PROGBITS         ffffffff8000b010  0000b010
       0000000000000000  0000000000000000  WA       0     0     1
```

For some reason, this feels like a remap to me rather than bug in the loader.  Let me start with some debugging output.

Well, maybe it can be a bug since the `.bss` section is a `NOBITS` section.  Not sure.

Damn!  For some reason I have lost my output ability as well.

That fixed (the loader became more than 1 page), I have the following output:

```
Kernel image at 0x0010_b000
Parsing ELF
Mapping 0x0000_0000_0010_b000 to frame 0x0000_0000_0000_010b
.. mapping elf; mSize = 0x0000_0000_0000_1538
Mapping 0xffff_ffff_8000_1000 to frame 0x0000_0000_0000_010c
.. mapping elf; mSize = 0x0000_0000_0000_0538
Mapping 0xffff_ffff_8000_2000 to frame 0x0000_0000_0000_010d
.. mapping elf; mSize = 0x0000_0000_0000_0008
Mapping 0xffff_ffff_8000_3000 to frame 0x0000_0000_0000_010e
.. mapping elf; mSize = 0x0000_0000_0000_7010
Mapping 0xffff_ffff_8000_4000 to frame 0x0000_0000_0000_010f
.. mapping elf; mSize = 0x0000_0000_0000_6010
Mapping 0xffff_ffff_8000_5000 to frame 0x0000_0000_0000_0110
.. mapping elf; mSize = 0x0000_0000_0000_5010
Mapping 0xffff_ffff_8000_6000 to frame 0x0000_0000_0000_0111
.. mapping elf; mSize = 0x0000_0000_0000_4010
Mapping 0xffff_ffff_8000_7000 to frame 0x0000_0000_0000_0112
.. mapping elf; mSize = 0x0000_0000_0000_3010
Mapping 0xffff_ffff_8000_8000 to frame 0x0000_0000_0000_0113
.. mapping elf; mSize = 0x0000_0000_0000_2010
Mapping 0xffff_ffff_8000_9000 to frame 0x0000_0000_0000_0114
.. mapping elf; mSize = 0x0000_0000_0000_1010
Mapping 0xffff_ffff_8000_a000 to frame 0x0000_0000_0000_0115
.. mapping elf; mSize = 0x0000_0000_0000_0010
Mapping 0xffff_ffff_8000_b000 to frame 0x0000_0000_0000_0116
.. mapping elf; mSize = 0x0000_0000_0000_1000
Mapping 0xffff_ffff_8000_3000 to frame 0x0000_0000_0000_010e
Unmapping 0xffff_ffff_8000_3000
.. mapping elf; mSize = 0x0000_0000_0000_0000
Mapping 0xffff_ffff_8000_4000 to frame 0x0000_0000_0000_0001
Unmapping 0xffff_ffff_8000_4000
.. mapping elf; mSize = 0x0000_0000_0000_0000
Mapping 0x0000_0000_0000_0000 to frame 0x0000_0000_0000_0001
```

So:

```
Mapping 0xffff_ffff_8000_4000 to frame 0x0000_0000_0000_010f
...
Mapping 0xffff_ffff_8000_4000 to frame 0x0000_0000_0000_0001
```

So, this is a remap problem.  Now why?  Could this be related to the type?

It was.  By only loading the sections indicated to load, I have fixed the remapping issue.  However, for some reason I cannot explain the `.bss` is also in the binary image.  How certain about that, but it seems like it is the case.

Except the page I am looking for is remapped still.

So, my conclusion at this point is that the loader is mapping the kernel properly.


---

### 2021-Jan-28

OK, I'm starting to think that the GDT is not being statically initialized.

```
Next at t=158248979
(0) [0x0000001081a9] 0008:ffffffff800011a9 (unk. ctxt): iret                      ; 48cf
(1) [0x00000009f02c] 9f00:002c (unk. ctxt): jmp .-3 (0x0009f02b)      ; ebfd
(2) [0x00000009f02c] 9f00:002c (unk. ctxt): jmp .-3 (0x0009f02b)      ; ebfd
(3) [0x00000009f02c] 9f00:002c (unk. ctxt): jmp .-3 (0x0009f02b)      ; ebfd
<bochs:2> info gdt
Global Descriptor Table (base=0xffffffff80005000, limit=167):
GDT[0x0000]=??? descriptor hi=0x00000000, lo=0x00000000
GDT[0x0008]=??? descriptor hi=0x00000000, lo=0x00000000
GDT[0x0010]=??? descriptor hi=0x00000000, lo=0x00000000
GDT[0x0018]=??? descriptor hi=0x00000000, lo=0x00000000

[snip]

You can list individual entries with 'info gdt [NUM]' or groups with 'info gdt [NUM] [NUM]'
<bochs:3> page 0xffffffff80005000
PML4: 0x0000000001004163    ps         A pcd pwt S W P
PDPE: 0x0000000001009063    ps         A pcd pwt S W P
 PDE: 0x000000000100a063    ps         A pcd pwt S W P
 PTE: 0x000000000010c021       g pat d A pcd pwt S R P
linear page 0xffffffff80005000 maps to physical page 0x00000010c000
<bochs:4> x /4 0xffffffff80005000
[bochs]:
0xffffffff80005000 <bogus+       0>:    0x00000000      0x00000000      0x00000000      0x00000000
```

AH-HA!!!  That's because it's not statically initialized!

```S
Disassembly of section .init_array:

ffffffff80003000 <.init_array>:
ffffffff80003000:       00 10                   add    %dl,(%rax)
ffffffff80003002:       00 80 ff ff ff ff       add    %al,-0x1(%rax)
```

So, I really need to get that (and other) calls incorporated into the `entry.s` startup.  While I am at it, I may as well force-clear the `.bss` section.

It's written but it also needs to be rewritten -- the code is crap.  Gotta take a look at that tomorrow.


---

### 2021-Jan-29

A quick rewrite puts me testing again.  Now, I wonder if the `.bss` clear is clearing too much data.

After realizing that I was loading the TR twice and cleaning that up, I am able to boot properly.

---

At this point, it is time to get the module started.  I have convinced myself that I will start the PMM.


---

### 2021-Jan-30

So, here is a trivial module to be loaded:

```S
                section     .text


;;
;; -- Set up the header structure for parsing from the kernel
;;    -------------------------------------------------------
header:
                db          'C','e','n','t','u','r','y',' ','O','S',' ','6','4',0   ; Sig
                db          'P','M','M',0,0,0,0,0,0,0,0,0,0,0,0                     ; Name
                dq          10                                                      ; Seq
                dq          PmmInitEarly                                            ; Early Init
                dq          0                                                       ; Late Init
                dq          0                                                       ; interrupts
                dq          2                                                       ; internal Services
                dq          0                                                       ; OS services
                dq          10                                                      ; internal function 1
                dq          PmmAllocate                                             ; .. target address
                dq          11                                                      ; internal function 2
                dq          PmmRelease                                              ; .. target address


PmmInitEarly:
PmmAllocate:
PmmRelease:
                ret
```

It does absolutely nothing.

So, the last thing I need to do before I can call the early init function is create a new paging structure.  This will be part of the kernel and part of the initialization code (but will be reused).

For the new paging structures, I really need an early kernel version of `PmmAllocate()`.  For this to work, I need to get the next frame from the `loader`.

---

I believe I have this coded.  I have not yet tested this change -- which also included adding it into the internal function table until I have a proper replacement.

Thinking more about this, I am certain that all the initialization needs to be completed on the first pass of initialization.  The sequence is critical since we will need to allocate several page tables for the other modules.


---

### 2021-Jan-31

I'm debating the next task to take on....  New VMM structures?  Centralize ELF parsers as a lib?  Update the internal function table?

SO, I go back to a good top-down design/coding methodology.  With the objective of this version, I need to start with some internal structures for initializing the different modules.


---

### 2021-Feb-01

Trying to code again today.  Let's see how it goes today!

I moved several things from specific modules into a `libk.a` module.  I finally got a compile... and a good test.  With that, I need to split the `elf.cc` into a lib version and a loader function.

A quick function rename solves the problem for splitting the `elf.cc` file -- and transferred it to the `libk` module.

I need a location to put several bits of information from the loader accessible to the kernel.  Not sure that this should be in the loader or in the kernel.  The big thing about putting it in the loader is that it can be reclaimed after init; however, depending on how I implement it (static or dynamic) the size of the loader may increase beyond what I am planning.  So, I can allocate a frame or two and identity map it and pass it along to the kernel in a second parameter.

I managed to break the MB1 loader.


---

### 2021-Feb-02

So, the MB1 structure had a `char *` field, but that is 64-bit on a 64-bit kernel.  It was supposed to be a 32-bit field.  I was able to correct that with a fresh head today.  A quick test confirmed the solution.

Now, I have a list of modules from the loader passed to the kernel.  The next step is to loop through each of them and create new paging structures for each.


---

### 2021-Feb-03

Having completed the virtual memory map, I realized that my kernel was really located in the recursive mapping area.  Not good.  So, I needed to move it to the proper location.  This broke the link:

```
ld.lld: error: /home/adam/workspace/centuryos2/modules/kernel/arch/x86_64/entry.s:(.text+0x4): relocation R_X86_64_32S out of range: -140737488338926 is not in [-2147483648, 2147483647]
ld.lld: error: /home/adam/workspace/centuryos2/modules/kernel/arch/x86_64/idt-asm.s:(.text+0x6D): relocation R_X86_64_32S out of range: -140737488338908 is not in [-2147483648, 2147483647]
ld.lld: error: /home/adam/workspace/centuryos2/modules/kernel/arch/x86_64/entry.s:(.text+0x18): relocation R_X86_64_32S out of range: -140737488338904 is not in [-2147483648, 2147483647]
ld.lld: error: /home/adam/workspace/centuryos2/modules/kernel/arch/x86_64/entry.s:(.text+0x58): relocation R_X86_64_32S out of range: -140737488338918 is not in [-2147483648, 2147483647]
```

OK, so this turned out to be how I was loading addresses.  Changing how memory addresses were loaded solved this problem.  But now I am back to a triple fault.

Hmm......

```
Mapping 0xffff_8000_0000_4000 to frame 0x0000_0000_0000_0001
.. mapping elf; mSize = 0x0000_0000_0000_4010
Mapping 0xffff_8000_0000_5000 to frame 0x0000_0000_0000_0001
.. mapping elf; mSize = 0x0000_0000_0000_3010
Mapping 0xffff_8000_0000_6000 to frame 0x0000_0000_0000_0001
.. mapping elf; mSize = 0x0000_0000_0000_2010
Mapping 0xffff_8000_0000_7000 to frame 0x0000_0000_0000_0001
.. mapping elf; mSize = 0x0000_0000_0000_1010
Mapping 0xffff_8000_0000_8000 to frame 0x0000_0000_0000_0001
.. mapping elf; mSize = 0x0000_0000_0000_0010
Mapping 0xffff_8000_0000_9000 to frame 0x0000_0000_0000_0001
Loaded...
```

Notice that there are several pages mapped to the same frame.

Ok, I was adjusting an address into a frame when it was already a frame.  Took a bit to find, but I got it.

So, back to the original task:
> The next step is to loop through each of them and create new paging structures for each.

---

```
An unknown error has occurred
  RAX: 0x0000000000000025       R8 : 0x00000000cccccccd
  RBX: 0xffff800000009010       R9 : 0xffff800000001250
  RCX: 0x0000000000000001       R10: 0x0000000000000000
  RDX: 0x00000000000003f8       R11: 0x0000000000000000
  RBP: 0xffff8000000024c0       R12: 0xffff8000000017b0
  RSI: 0x0000000000119000       R13: 0xffff800000002760
  RDI: 0x000000000000000a       R14: 0xffff800000002400
                                R15: 0x0000000000000000
   SS: 0xffff8000000024c0       RSP: 0x0000000000000010
   CS: 0x200006 RIP: 0x0000000000000008
   DS: 0x28      ES: 0x28
   FS: 0x0       GS: 0x48
```

So, it looks like I have an alignment issue with some of my exceptions.  Several have error codes which need an extra value pushed to maintain stack alignment.


---

### 2021-Feb-04

Well, my code to load a new page table is causing triple faults.  Debugging again.

Oh it's a stack problem!!  The stack is located in low memory still.  Gotta fix this.  The fix will be in the kernel `entry.s`.


---

### 2021-Feb-07

I was able to get the stack resolved -- I was adjusting the frame to an address, but still using it as a frame.

---

OK, this is working now.  I am getting the mapping set and reset back to the kernel mappings.  However, I am not yet calling the early init function.  So, the next step is to actually *call* the function.


---

### 2021-Feb-08

So, the next thing to do is complete the mappings for the module.  If I have done my stuff properly, this should be a relatively trivial function call.


---

Spend most of the day working on a `#PF`.  So, the problem ended up being with the recursive mapping addresses.  I made a change to the `MmuIsMapped()` function to `INVLPG` for each address before checking whether it is mapped.  I'm tired.


---

### 2021-Feb-09

Now, the key is to call the function to perform the early init.  The return value from this function must indicate if the module is to stay loaded with a `0` return value meaning to continue and a negative return value to indicate why the module it is not going to load.  There is also a question of sequence which needs to be addressed.

Before that, let's take care of [Redmine #481](http://eryjus.ddns.net:3000/issues/481) and call the proper PMM allocation function.


---

Well, I have the most trivial instance working!

Time for some cleanup and a commit.