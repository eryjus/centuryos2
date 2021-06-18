# The Century OS

This project is a 64-bit focused version of Century-OS.  This version takes from the 32-bit version of Century-OS, but is better streamlined for the 64-bit CPUs (where any 32-bit archs will be added in later if desired).


---

## Version 0.0.9 -- Complete the scheduler module

In this version, we will complete these tasks:
* Create and load the scheduler module
* Complete the early init of the scheduler module
  * includes creating the initialization Process_t structure
* Enable interrupts in the kernel
* Complete the later initialization of modules (pmm)
  * includes creating a new process for the pmm cleaner


---

### 2021-May-09

Just getting started on this version; not sure how far I will get today.


---

### 2021-May-10

I also need to implement a current timer count for sleeping.

I also realize I need to port the heap code into `libk`.

With this realization, I think I am better off breaking this version up into several commits.


## Version 0.0.9a -- Port the Heap code into `libk`

This first task will be to port the `heap` code into the `libk` module.  This should be relatively straight-forward since this code is well debugged.


---

### 2021-May-11

Picking up where I left off yesterday, I am going to branch into `v0.0.9a`.


---

### 2021-May-12

The `heap` code has been copied over.  Only some minor changes, but a compile should take care of the cleanup.  That said, there may be some lingering issues from not testing.


---

## Version 0.0.9b -- Port the rest of the scheduler code

This task will complete moving the scheduler from CenturyOS (Process).

---

I am going to have to figure out how to pull the cpu abstractions structures to the kernel and expose functions to access that.


---

### 2021-May-15

So, I am left wondering if I really need to populate all the stack info for the `kInit` process....  I technically should not need it, but may be appropriate for debugging and reporting.


---

### 2021-May-16

`clang` does not like the `gcc` variable suggestions....  So, I had to create some inline functions to accommodate the compiler.  It was not too difficult to do.  But it allows me to start to pull mode functions into the source.


---

### 2021-May-17

I have a need to be able to create stacks for processes.  A module may have several processes and each may have its own stack.  I think I need to set this into `libk` so that any module can allocate stacks.  I need to keep in mind that this is a virtual memory space allocator.  I will have to do a partial re-write of for the stacks module.


---

### 2021-May-24

OK, so....  stacks.  I need to set up 1 PML4 entry for driver stacks.  Moreover, I need to figure out how to specify the stack start/end location from the linker script.


---

### 2021-May-26

I have the process code all ported over.  There are problems.  There are lots of problems.  I have several things to clean up.  There is no way this will schedule anything.  Each function needs to be revisited and cleaned up.  I also need to create the internal functions.

* I need to develop and expose a timer counter function.
* The startup stack is not right.
* Several inline functions need to be converted.
* Several functions are not complete (outstanding TODO cleanup).
* `ProcessSwitch()` will not work; it is not properly x86_64.
* I still need to port the Timer interrupt handler.
* Functions need to be exposed in the driver startup block.

There also needs to be a separation between the timer and the scheduler.  I think I am going to need to map an interrupt for scheduling.


---

### 2021-May-27

I need to hook IRQ0 with a timer interrupt.  This interrupt needs to be part of the xapic/x2apic driver to maintain a timer counter.  Then I need to hook a software interrupt from the scheduler to perform a process reschedule.  This interrupt would be called from several locations to perform a reschedule -- and will also be an OS function like POSIX `sched_yield()` -- to voluntarily yield the processor.


---

### 2021-May-28

OK, I have documented the interrupt usage.  So now I need to decide which interrupt to use for the scheduler to trigger a reschedule.  I am trying to figure out interrupt priorities.


---

### 2021-Jun-01

I have been able to get the process functions sorted out.  The only things left are to review and redo `ProcessSwitch()` to conform to ABI standards and to clean up the startup stack.


---

### 2021-Jun-02

OK, I think I have the scheduler source worked out.  I am able to compile.  Now I need to include it in the boot `grub.cnf`.

```
An unknown error has occurred (Error Code 0x0000000000000000)
  RAX: 0xffffa00000005188       R8 : 0xffffa00000003337
  RBX: 0xffffa00000005230       R9 : 0x0000000000081230
  RCX: 0x00000000000000e0       R10: 0xffffa00000001ba0
  RDX: 0x0000000000020000       R11: 0xffffa00000001bd7
  RBP: 0x0000000000000000       R12: 0xffffa00000003310
  RSI: 0x0000000000000000       R13: 0xffffa000000021d0
  RDI: 0xffffa00000002e10       R14: 0xffffb80000000000
                                R15: 0xffffa00000003470
   SS: 0x10     RSP: 0xfffff80000003f18
   CS: 0x8      RIP: 0x0000000000000006
   DS: 0x28      ES: 0x28
   FS: 0x0       GS: 0x48
```

... and let the debugging begin!


---

### 2021-Jun-03

I found that `kMemSetB()` was not using the proper ABI.  That has been corrected.

I also found that `GS` was not being set up properly.  That has also been corrected.

---

OK, the `gs` cannot be set up like it was in i586....  There is no starting segment address (it all starts at 0) and there is not limit on data segments.  So, I need to revisit the whole CPU structure implementation.  It may need to be an internal function; the problem will be ensuring the CPU number does not change between the call and the evaluation.


---

### 2021-Jun-04

I have found a `swapgs` instruction, which will swap the contents of `gs` with `IA32_KERNEL_GS_BASE`.  This MSR appears to be at `0xc000 0102`.  So, that said, I should be able to continue to do this proper offset.  I just need a proper architecture-specific initialization to set things up.


---

### 2021-Jun-05

I was able to get `gs` working again.  I do have some more cleanup to do around the GDT.

I also found that the PMM was not able to allocate a block of frames.


---

### 2021-Jun-06

OK, I am able to complete the early initialization.  Now, I need to enable interrupts, and then complete the late initialization with interrupts enabled.  This is also going to break all kinds things again -- just enabling interrupts.  Before I get into that, I think it is time to commit a micro-version.


---

## Version 0.0.9c -- Enable interrupts

Enable interrupts and work out all the issues.

---


### 2021-Jun-07

OK, I figured out my current crash when interrupts are enabled.  The problem is that I am registering a function from the x2apic module into the IDT in the kernel.  This means that the interrupt target address is in some unknown virtual address space but the interrupt must land in the kernel address space.

For this to work, I will need to install generic handlers and then use a table for determining where to jump for the handler.


---

### 2021-Jun-10

qemu boots.  vbox boots.  Bochs does not boot.  Bochs also happens to be the emulator with an x2APIC.  I need to change that back to an xAPIC.

I have the same problem with both types of XAPIC.


---

### 2021-Jun-13

I have figured some things out.  First and foremost, I uncovered a bug in the xapic initialization code.  Now, I have a problem with the `cpus` structure initialization.  The problem is that the compiler has an initialization function that is to be called on load and is not being called.  This is in the kernel module, but I'm certain the same problems exist in the other modules.

I also now know that the `.bss` section is not being cleared.  I need to be able to clear this section.

---

Alright, for the record, I was already calling the calling the init functions and I was already "clearing" the bss.  The problem is I was setting the contents of the bss with the address of the start of the bss.  Now that I have both of those fixed up, I am having problems with the heap.  My problem is with the heap, not the other init functions.

```
00177794993i[      ] CPU 0 at 0xffff800000006950: mov rdi, 0xffff800000009291   (reg results):
00177794993i[      ] LEN 10	BYTES: 48bf919200000080ffff
00177794993i[      ]   RAX: 0x0000000000000017; RBX: 0xffff800000006970; RCX 0x00000001ffff2000; RDX: 0x00000000000003f8
00177794993i[      ]   RSP: 0xfffff80000003ff0; RBP: 0x0000000000100b50; RSI 0x0000000000166000; RDI: 0xffff800000009291
00177794993i[      ]   R8 : 0x00000000cccccccd; R9 : 0x00000000000000a8; R10 0xffffa000000061b0; R11: 0xffffa00000001c87
00177794993i[      ]   R12: 0x0000000000101030; R13: 0x00000000001000a0; R14 0x000000000010d000; R15: 0xfffff80000004000
00177794993i[      ]   CS: 0x0008; DS: 0x0028; ES: 0x0028; FS: 0x0000; GS: 0x0048; SS: 0x0010;
00177794993i[      ]   RFLAGS: 0x0000000000200246 (ID vip vif ac vm rf nt IOPL=0 of df IF tf sf ZF af PF cf)
```

```
00177795398i[      ] CPU 0 at 0xffff800000001f06: iret    (reg results):
00177795398i[      ] LEN 2	BYTES: 48cf
00177795398i[      ]   RAX: 0x0000000000000017; RBX: 0xffff800000006970; RCX 0x00000001ffff2000; RDX: 0x00000000000003f8
00177795398i[      ]   RSP: 0xfffff80000003ff0; RBP: 0x0000000000100b50; RSI 0x0000000000166000; RDI: 0xffff800000009291
00177795398i[      ]   R8 : 0x00000000cccccccd; R9 : 0x00000000000000a8; R10 0xffffa000000061b0; R11: 0xffffa00000001c87
00177795398i[      ]   R12: 0x0000000000101030; R13: 0x00000000001000a0; R14 0x000000000010d000; R15: 0xfffff80000004000
00177795398i[      ]   CS: 0x0008; DS: 0x0028; ES: 0x0028; FS: 0x0000; GS: 0x0048; SS: 0x0010;
00177795398i[      ]   RFLAGS: 0x0000000000200286 (ID vip vif ac vm rf nt IOPL=0 of df IF tf SF zf af PF cf)
```

```
00177795738i[      ] CPU 0 at 0xffff800000001f06: iret    (reg results):
00177795738i[      ] LEN 2	BYTES: 48cf
00177795738i[      ]   RAX: 0x0000000000000017; RBX: 0xffff800000006970; RCX 0x00000001ffff2000; RDX: 0x00000000000003f8
00177795738i[      ]   RSP: 0xfffff80000003ff0; RBP: 0x0000000000100b50; RSI 0x0000000000166000; RDI: 0xffff800000009291
00177795738i[      ]   R8 : 0x00000000cccccccd; R9 : 0x00000000000000a8; R10 0xffffa000000061b0; R11: 0xffffa00000001c87
00177795738i[      ]   R12: 0x0000000000101030; R13: 0x00000000001000a0; R14 0x000000000010d000; R15: 0xfffff80000004000
00177795738i[      ]   CS: 0x0008; DS: 0x0028; ES: 0x0028; FS: 0x0000; GS: 0x0048; SS: 0x0010;
00177795738i[      ]   RFLAGS: 0x0000000000200286 (ID vip vif ac vm rf nt IOPL=0 of df IF tf SF zf af PF cf)
```


---

### 2021-Jun-14

So, `qemu` appears to work properly 100% with no exceptions.  `vbox` appears to work for a short period and then faults.  `Bochs` faults right away (within 1 or 2 ticks of the timer).  I have not yet tried it on real hardware, but it is on the horizon.

I may have to add some things into the instrumentation to see if I can get more details from it.

---

Wait a minute!!!  What if the first timer interrupt triggers interrupt `0x20`.  I am issuing an EOI by writing a 0 to the EOI register.  But the second timer tick is actually being interpreted as a `#DF`?  This would mean that I am not properly acknowledging the EOI.  Or maybe it's the flags for the page table?

I think I am going to go into the Mmu flags first.


---

### 2021-Jun-16

I am now writing debugging code into Bochs.  I need to figure out where this fault is coming from.  This is a long process, but at the same time I am going to learn a lot about the internals of Bochs.


---

### 2021-Jun-18

It dawned on me a couple of days ago as I headed to bed that the problem was that I was not remapping the 8259 PIC.  The default for the 8259 is for PIC1 to interrupt IRQ0 for Vector 0x08 -- the timer interrupt!  So, Bochs was letting a spurious interrupt through.

My research today confirmed this fact:
* https://wiki.osdev.org/8259_PIC#The_IBM_PC_8259_PIC_Architecture
* https://wiki.osdev.org/APIC#Local_APIC_configuration

After moving the 8259 interrupts to start at 0x40, this is working now in Bochs.  Now to back out all my debugging changes.

---

OK, the next step here will be to "publish" all the Process functions and then complete the PMM Late Initialization function so that it can be called -- which will require `ProcessCreate()`.

---

I am getting the late initialization completed.  That said, the process is not getting started.

I am debating the next steps and whether to complete this version and move on to creating a debugger.  The debugger will likely be the next thing I do, but do I want to fight with debugging the scheduler before I create the debugger to help debug the scheduler?  That sounds very leading.  While I consider that, I think I will wrap up this micro-version.  If I do complete this debugging in v0.0.9, I will do it as its own micro-version.






