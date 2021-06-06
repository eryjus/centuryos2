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

