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

---

Since the debugger will be implemented as a process, I really need to get processes working before I write a debugger.


---

## Version 0.0.9d -- Debug the scheduler

This micro-version will debug the process scheduler -- a prerequisite to getting the debugger running since it will be a process itself.

So, I started with being able to dump a process structure.  From the `ProcessInit` function, I have the following output after the process structure is cleared:

```
=========================================================
Dumping Process_t structure at address 0xffffb80000000018
---------------------------------------------------------
  TOS (last preemption)..: 0xd0d0d0d0d0d0d0d0
  TOS (kernel functions).: 0xd0d0d0d0d0d0d0d0
  TOS (last interrupted).: 0xd0d0d0d0d0d0d0d0
  Virtual Address Space..: 0xd0d0d0d0d0d0d0d0
  Process Status.........: -791621424 (Unknown!)
  Process Priority.......: -791621424 (Unknown!)
  Quantum left this slice: -791621424
  Process ID.............: -791621424
  Command Line...........: ������������������������������������������������������������������������������������������������������������������������������������������������������������������������ܺ���Һ
  Ticks used.............: -791621424
  Wake tick number.......: -791621424
  Pending Error Number...: -791621424
=========================================================
```

This isn't right, and frankly if `kMemSetB` is not right it will lead to many other issues as well.

Which ended up being a simple fix in `kMemSetB`.  Not sure how I missed that!

---

Now, the PMM is not actually calling/creating a new process:

```
ProcessInit() complete
.. enabling interrupts
PMM Late Initialization Complete!
Boot Complete!
```

I am expecting another `Process_t` structure dump.

That was a matter of not hooking enough functions.  Which now works.

---

The next problem here is that `ProcessCreate()` from another module is being passed a pointer to a constant string in some other module.  It is not in the same address space as the `scheduler` module.  As a result, I get things like this:

```
=========================================================
Dumping Process_t structure at address 0xffffb80000000378
---------------------------------------------------------
  TOS (last preemption)..: 0x0000402000007fd0
  TOS (kernel functions).: 0x0000402000008000
  TOS (last interrupted).: 0x0000000000000000
  Virtual Address Space..: 0x0000000000000000
  Process Status.........: 0 (INIT)
  Process Priority.......: 30 (OS)
  Quantum left this slice: 0
  Process ID.............: 2
  Command Line...........: ����H�eH�%
  Ticks used.............: 0
  Wake tick number.......: 0
  Pending Error Number...: 0
=========================================================
```

The command line is clearly garbage.  So, how do I pass a memory value from one process to another through an internal function call?  One option may be shared memory.  Another may be messaging.  A third may be copying the value from one address space to another or even into kernel memory space.

I believe the kernel memory copy is the best at this point, which means I need to protect the hell out of this function and ensure that memory is cleaned up.

Let's start by adding a kernel internal function to allocate some memory and copy the contents from one location to another.

---

I have a problem with converting an `int` to `void *` because `int` is 32-bit in the compiler.  So I need to define an OS return val which will be a 64-bit integer.  And then make lots of changes.

---

Those functions are done (copy to kernel and release).  But the kernel heap is not in the correct spots.  This is because the heap is compiled in the kernel and its limits are not set in the linker script.  That change has to be next.


---

### 2021-Jun-20

Today, I realize that the PML4 is being copied, but I do not have tables for each PNL4 entry.  So, when I add them later, they are not crossing over like they should.  Cleaning that up stops my faulting.

So, now I have 1 process structure created from `ProcessInit()` and 2 process structures created from `ProcessCreate()`.  All of these structures have problems.  So..., where to start cleaning them up.

I will start with `ProcessInit()`.  Currently, this structure looks like this:

```
=========================================================
Dumping Process_t structure at address 0xffffb80000000018
---------------------------------------------------------
  TOS (last preemption)..: 0x0000000000000000
  TOS (kernel functions).: 0x0000400000004000
  TOS (last interrupted).: 0x0000000000000000
  Virtual Address Space..: 0x0100500001004000
  Process Status.........: 1 (RUNNING)
  Process Priority.......: 30 (OS)
  Quantum left this slice: 30
  Process ID.............: 0
  Command Line...........: kInit
  Ticks used.............: 0
  Wake tick number.......: 0
  Pending Error Number...: 0
=========================================================
```

So, I believe the first thing to do is to remove the `TOS (kernel functions)` and `TOS (last interrupted)` elements.  There is more, but let's start there.  This results in the following:

```
=========================================================
Dumping Process_t structure at address 0xffffb80000000018
---------------------------------------------------------
  TOS (last preemption)..: 0x0000000000000000
  Virtual Address Space..: 0x0100500001004000
  Process Status.........: 1 (RUNNING)
  Process Priority.......: 30 (OS)
  Quantum left this slice: 30
  Process ID.............: 0
  Command Line...........: kInit
  Ticks used.............: 0
  Wake tick number.......: 0
  Pending Error Number...: 0
=========================================================
```

Now, the `Virtual Address Space` is incorrect.  This was caused by the asm pml4 data being set to the wrong size.

This now leaves the following:

```
=========================================================
Dumping Process_t structure at address 0xffffb80000000018
---------------------------------------------------------
  TOS (last preemption)..: 0x0000000000000000
  Virtual Address Space..: 0x0000000001004000
  Process Status.........: 1 (RUNNING)
  Process Priority.......: 30 (OS)
  Quantum left this slice: 30
  Process ID.............: 0
  Command Line...........: kInit
  Ticks used.............: 0
  Wake tick number.......: 0
  Pending Error Number...: 0
=========================================================
```

This now looks correct.

```
=========================================================
Dumping Process_t structure at address 0xffffb80000000110
---------------------------------------------------------
  TOS (last preemption)..: 0xffffaf0000003fd0
  Virtual Address Space..: 0x0000000000000000
  Process Status.........: 0 (INIT)
  Process Priority.......: 30 (OS)
  Quantum left this slice: 0
  Process ID.............: 1
  Command Line...........: Idle Process
  Ticks used.............: 0
  Wake tick number.......: 0
  Pending Error Number...: 0
=========================================================
Starting PMM Cleaner process
Creating a new process named at 0xffff900000000018 (PMM Cleaner), starting at 0xffffa00000001bd0
.. naming the process: PMM Cleaner
=========================================================
Dumping Process_t structure at address 0xffffb80000000358
---------------------------------------------------------
  TOS (last preemption)..: 0xffffaf0000007fd0
  Virtual Address Space..: 0x0000000000000000
  Process Status.........: 0 (INIT)
  Process Priority.......: 30 (OS)
  Quantum left this slice: 0
  Process ID.............: 2
  Command Line...........: PMM Cleaner
  Ticks used.............: 0
  Wake tick number.......: 0
  Pending Error Number...: 0
=========================================================
```

I need to set the `Virtual Address Space` field.  I also need to set the quantum left.

These appear to be set correctly now.

Now, I need to confirm that the scheduler's `tick` function is being called.

So, I have been able to determine that I am only finding a next process only once.  So I need to debug the Ready Queue.  So, I need a function to dump the state of the Ready Queue.


---

### 2021-Jun-21

The function to dump the Ready Queue is not operating properly at the moment.  It is not picking up the correct `Process_t` address.  There are other issues and I will have to "rubber duck" the entire asm function to perform an actual process swap.


---

### 2021-Jun-22

I am getting farther.  But I do have a problem with the virtual address space.  I am getting into some space that I am not totally sure is correct.  Or, I am getting some odd address space for one of the processes.  Or both.  I'm working on a dig.  This will take a few days.


---

### 2021-Jun-24

Still debugging.


---

### 2021-Jun-25

I have a problem with allocating an early frame.  Not sure where this problem is.


---

### 2021-Jun-29

Still debugging.  Not sure where I am at anymore.  I know I am having problems swapping processes.  I need to reduce the debug output so I can refocus on the next problem to solve.


---

### 2021-Jul-01

OK, let see if I can work out some details here:

|  Module         |  CR3 @ module  |  Process Dump 1  |  Process CR3  |  Process CR3  |
|:---------------:|:--------------:|:----------------:|:-------------:|:-------------:|
|  Loader/Kernel  |  0x001004000   |  kInit           |  0x001004000  |  0x001004000  |
|  PMM            |  0x001005000   |  PMM Cleaner     |  0x001065000  |               |
|  x2APIC         |  0x1fffff000   |                  |               |               |
|  Scheduler      |  0x1ffff2000   |  Idle Process    |  0x1ffff2000  |               |

At the time of the crash, the CR3 is `0x00000001fffff000`, so the timer (x2APIC) has control of the virtual address space.

Also, the faulting address is `0xffffaf0000003f30`.  This is a stack address.  The faulting instructions is `ffff800000001eba: callq  *%rbx`, which also makes sense given the stack address.

So, that means I need to dig into the stack implementation.

This is confirmed.  The stack is always mapped into the scheduler address space.  So, this is going to be a problem.


---

### 2021-Jul-05

I have been considering my problem for a few days.  There is really no good way to get the scheduler to construct a Process and it's address space from another address space.  Well I could, but it would be nothing except a lot of brute force.

This, then, begs the question about how much needs to move and how much can stay in the scheduler implementation.
1. The process creation should be moved to the kernel, including creating the address space and building the proper stack.
1. The scheduler should be able to remain in its own module since it is dependent on the address space.
1. Process cleanup should probably be moved to the kernel so that it can be done with the process's address space.

So, with that, process creation and process tear-down both become kernel functions.  This still seems a reasonable separation of concerns.

This will, however, take some time to change.

There are several additional internal functions I need to create/expose from the scheduler now.  That said, nearly all the lines are in one block and all can be organized into one block.  So I can create a new function called `SchReadyNewProcess()` to perform this final work.

I can also update `SchProcessReady()` such that when the global list is empty, I can perform the other functions.


---

### 2021-Jul-06

Most everything is separated.  However, there are still some some things I need to get my arms around.  In particular, the `ProcessStart()` cannot be linked.

I think we need to discuss how a new process gets started.  What does the stack needs to look like once the process gets the CPU...?

The only way for the process to get control is from preemption.  This means from `ProcessSwitch()`, which will be the Scheduler context.  To get out of `ProcessSwitch()`, we need to pop the following registers from the stack (pop order):
* `r15`
* `r14`
* `r13`
* `r12`
* `rbp`
* `rbx`
* `rax`
* ... and then the return `rip` address

At this point, the return address should be `ProcessStart()`, which is also in the Scheduler context.  `ProcessStart()` should push its own stuff on the stack and then pop it back off.  And then it's return value should be something that prepared for an `iret` instruction.  This should come from the kernel and be part of returning from an internal call.  Or, perhaps a scheduler-centric `iret` instruction which will pop the following (again in pop order):

* `rip`
* `cs`
* `rflags`
* `rsp` -- after the `iret` instruction!
* `ss`

*This still does not do what I want it to do.*

I really need to end up at `InternalTarget.out`.  This stack looks like (in pop order):
* `InternalTarget.out`
* `cr3` -- new context!
* `r15`
* `r14`
* `r13`
* `r12`
* `r11`
* `r10`
* `r9`
* `r8`
* `rdi`
* `rs1`
* `rbp`
* `rdx`
* `rcx`
* `rbx`
* discarded
* `rip` -- should be the process starting address
* `cs`
* `rflags`
* `rsp` -- after the `iret` instruction!
* `ss`


---

### 2021-Jul-07

The problem with the above stack is that the address of `InternalTarget.out` is in the kernel module and not a fixed address.  To get around this, I may have to duplicate that logic in the scheduler for a local address.

Another thing is that the Virtual Address Space will change in `ProcessSwitch()`, which I missed.  So, the rest of the exit from `ProcessSwitch()` and beyond needs to be evaluated.

So, that ends up being my problem.  The context has changed away from the Scheduler when I start popping data off the stack.  The problem is the scheduler code is not mapped into every address space.

There are 2 possible solutions:
1. move the scheduler to the kernel so it is mapped properly.
2. move the address space of the scheduler and map it into all the address spaces.

I need to ponder these options....


---

### 2021-Jul-12

I believe I still may be able to handle the scheduler in its own module by passing in the entry point address to the scheduler initialization through the boot structure.  This should mitigate my issues.  This puts me back to publishing and pushing `InternalTarget.out`, as originally thought.

I want to think this through a bit more.

Ultimately, every call to `ProcessSwitch()` ends up with a change from the Scheduler virtual address space to the Scheduler virtual address space.  This means that the value in `Process_t` is irrelevant.  So, I can:
1. eliminate the virtual address space in the `Process_t`
2. change `ProcessSwitch()` to not change the virtual address space
3. populate the virtual address space with the Scheduler virtual address space

Option 2 is the best answer, so I should go with that.


---

### 2021-Jul-13

I have the changes backed out.  I think I have created a mess.


---

### 2021-Jul-14

I am back to problems with the MMU code.


---

### 2021-Jul-28

I was able to get a reasonable test yesterday....  It ran for several hours.  The problem is that the Ready Queue was depleted and ended up with nothing in it.  So, I think I need to focus on that problem for a while.

OK, now with a couple of things cleaned up, I am back to triple faulting on a process change.


---

### 2021-Jul-29

Looks like the stack is not mapped.

```
00182888318d[CPU0  ] page fault for address ffffaf0000003f20 @ ffff800000001ebc
00182888318i[      ] CPU 0: exception 0eh error_code=2
00182888318i[CPU0  ] CPU is in long mode (active)
00182888318i[CPU0  ] CS.mode = 64 bit
00182888318i[CPU0  ] SS.mode = 64 bit
00182888318i[CPU0  ] EFER   = 0x00000500
00182888318i[CPU0  ] | RAX=0000000000000200  RBX=ffffa00000001240
00182888318i[CPU0  ] | RCX=0000000001c9c380  RDX=ffffb80000000088
00182888318i[CPU0  ] | RSP=ffffaf0000003f28  RBP=0000000000000000
00182888318i[CPU0  ] | RSI=ffffa000000081e0  RDI=ffffaf0000003f20
00182888318i[CPU0  ] |  R8=00000000000000a8   R9=00000000000000a8
00182888318i[CPU0  ] | R10=00000001fffff000  R11=ffff80000000d044
00182888318i[CPU0  ] | R12=0000000000000000  R13=0000000000000000
00182888318i[CPU0  ] | R14=00000001ffff3000  R15=ffffa00000005921
00182888318i[CPU0  ] | IOPL=0 ID vip vif ac vm rf nt of df if tf SF zf af pf cf
00182888318i[CPU0  ] | SEG sltr(index|ti|rpl)     base    limit G D
00182888318i[CPU0  ] |  CS:0008( 0001| 0|  0) 00000000 00000fff 1 0
00182888318i[CPU0  ] |  DS:0028( 0005| 0|  0) 00000000 00000fff 1 0
00182888318i[CPU0  ] |  SS:0010( 0002| 0|  0) 00000000 00000fff 1 0
00182888318i[CPU0  ] |  ES:0028( 0005| 0|  0) 00000000 00000fff 1 0
00182888318i[CPU0  ] |  FS:0000( 0000| 0|  0) 00000000 00000000 0 0
00182888318i[CPU0  ] |  GS:0048( 0009| 0|  0) 0000e050 0000ffff 1 1
00182888318i[CPU0  ] |  MSR_FS_BASE:0000000000000000
00182888318i[CPU0  ] |  MSR_GS_BASE:ffff80000000e050
00182888318i[CPU0  ] | RIP=ffff800000001ebc (ffff800000001eba)
00182888318i[CPU0  ] | CR0=0xe0000011 CR2=0xffffaf0000003f20
00182888318i[CPU0  ] | CR3=0x00000001fffff000 CR4=0x000000a0
```

```
<bochs:4> page 0xffffaf0000003f20
PML4: 0x0000000000000000    ps         a pcd pwt S R p
physical address not available for linear 0xffffaf0000003000
```

So I need to think through stacks again.

I think the reality is that the module creating the process needs to also be responsible for allocating and handing off a stack address.  Currently, this is determined in the Scheduler module and that address is not available to all modules.  This should be easy to test -- just do not create the PMM Cleaner process.  Nope, there is still something else.


---

### 2021-Aug-04

Well, this is clearly wrong:

```
In address space 0x0000000001065000, request was made to map address 0xffffa00000002000 to frame 0x0000000000000176
In address space 0x0000000001065000, request was made to map address 0xffffa0000000239b to frame 0x0000000000000176
In address space 0x0000000001065000, Unmapping page at address 0xffffa0000000239b
```

This will be related to the `.rodata` being appended to the `.text` and being loaded as separate sections.  This was a simple fix.

The following is the MMU work for loading and early-initing the PMM module:

```
In address space 0x0000000001004000, request was made to map address 0x0000000001065000 to frame 0x0000000000001065
Mapping the module
In address space 0x0000000001004000, Unmapping page at address 0x0000000001065000
Loading Module located at 0x0000000000174000
.. Old CR3: 0x0000000001004000; New: 0x0000000001065000
.. Module Address is getting mapped to 0x0000000000174000
In address space 0x0000000001065000, request was made to map address 0x0000000000174000 to frame 0x0000000000000174
.. Image header mapped
In address space 0x0000000001065000, request was made to map address 0x0000000000174000 to frame 0x0000000000000174
In address space 0x0000000001065000, Unmapping page at address 0x0000000000174000
In address space 0x0000000001065000, request was made to map address 0xffffa00000001000 to frame 0x0000000000000175
In address space 0x0000000001065000, request was made to map address 0xffffa00000002000 to frame 0x0000000000000176
In address space 0x0000000001065000, request was made to map address 0xffffa00000003000 to frame 0x0000000000000177
In address space 0x0000000001065000, request was made to map address 0xffffa00000004000 to frame 0x000000000000106d
In address space 0x0000000001065000, request was made to map address 0xffffa00000004000 to frame 0x0000000000000178
In address space 0x0000000001065000, Unmapping page at address 0xffffa00000004000
In address space 0x0000000001065000, request was made to map address 0xffffa00000005000 to frame 0x0000000000000179
In address space 0x0000000001065000, Unmapping page at address 0x0000000000174000
.. Elf Loaded
Checking Module for validity
.. checking earlyInit function
.. checking published feature count
.. valid!
.. module name is PMM
.. calling early init function at 0xffffa00000001270
In address space 0x0000000001065000, request was made to map address 0xffffff0000000000 to frame 0x0000000000000001
In address space 0x0000000001065000, Unmapping page at address 0xffffff0000000000
In address space 0x0000000001065000, request was made to map address 0xffffff0000000000 to frame 0x0000000000001170
In address space 0x0000000001065000, request was made to map address 0xffffff0000003000 to frame 0x0000000000001170
In address space 0x0000000001065000, Unmapping page at address 0xffffff0000000000
In address space 0x0000000001065000, request was made to map address 0xffffff0000000000 to frame 0x0000000000100000
In address space 0x0000000001065000, Unmapping page at address 0xffffff0000003000
In address space 0x0000000001065000, request was made to map address 0xffffff0000003000 to frame 0x0000000000100000
In address space 0x0000000001065000, Unmapping page at address 0xffffff0000000000
.. early init completed
.. Hooking services: 0 interrupts; 2 internal functions; 0 OS services
.... Hooking Internal Function 10: 0xffffa000000013f0 from 0x0000000001065000
.... Hooking Internal Function 11: 0xffffa000000019e0 from 0x0000000001065000
In address space 0x0000000001065000, Unmapping page at address 0x0000000000174000
```

There are a few concerns here.  I will take each of them in turn and research.

1. Remapping the image header:

```
In address space 0x0000000001065000, request was made to map address 0x0000000000174000 to frame 0x0000000000000174
.. Image header mapped
In address space 0x0000000001065000, request was made to map address 0x0000000000174000 to frame 0x0000000000000174
In address space 0x0000000001065000, Unmapping page at address 0x0000000000174000
```

There is a mapping in `elf.c` and another one in `modules.cc`.  They appear to be completely redundant.  I am going to comment out the one in `modules.cc` since it will be mapped deeper in the call stack.

2. Page at `0x...4000` is mapped to 2 different frames:

```
In address space 0x0000000001065000, request was made to map address 0xffffa00000004000 to frame 0x000000000000106d
In address space 0x0000000001065000, request was made to map address 0xffffa00000004000 to frame 0x0000000000000178
In address space 0x0000000001065000, Unmapping page at address 0xffffa00000004000
```

Looking at the elf, the `.init_array` section and the `.bss` section overlap as well.  This leads me to believe I am not in control of my linker script the way I want to be.  I know I am not properly loading ELF images and that will need to be cleaned up.  But these problems should not be the top of my mind at the moment.


---

### 2021-Aug-06

Before I get to rewriting the [linker scripts](http://eryjus.ddns.net:3000/issues/489).  At the same time the Redmine to solve the [ELF loader problem](http://eryjus.ddns.net:3000/issues/490) has been entered as well.  So those are tabled for later.

In the meantime, I will work on separating the `.init_array` from other sections or add it into the `.text` section.

So, now, the first part of the mappings look good:

```
Loading Module located at 0x0000000000174000
.. Old CR3: 0x0000000001004000; New: 0x0000000001065000
.. Module Address is getting mapped to 0x0000000000174000
.. Image header mapped
In address space 0x0000000001065000, request was made to map address 0x0000000000174000 to frame 0x0000000000000174
In address space 0x0000000001065000, request was made to map address 0xffffa00000001000 to frame 0x0000000000000175
In address space 0x0000000001065000, request was made to map address 0xffffa00000002000 to frame 0x0000000000000176
In address space 0x0000000001065000, request was made to map address 0xffffa00000003000 to frame 0x0000000000000177
In address space 0x0000000001065000, request was made to map address 0xffffa00000004000 to frame 0x0000000000000178
In address space 0x0000000001065000, request was made to map address 0xffffa00000005000 to frame 0x0000000000000179
In address space 0x0000000001065000, request was made to map address 0xffffa00000006000 to frame 0x000000000000017a
In address space 0x0000000001065000, Unmapping page at address 0x0000000000174000
.. Elf Loaded
```

This is now all I would expect it to be.

The next step is to determine what is going on here:

```
In address space 0x0000000001065000, request was made to map address 0xffffff0000000000 to frame 0x0000000000000001
In address space 0x0000000001065000, Unmapping page at address 0xffffff0000000000
```

Why is there an address being mapped to Frame 1?  That simply goes away.

Now:

```
In address space 0x0000000001065000, request was made to map address 0xffffff0000000000 to frame 0x0000000000001170
In address space 0x0000000001065000, request was made to map address 0xffffff0000003000 to frame 0x0000000000001170
In address space 0x0000000001065000, Unmapping page at address 0xffffff0000000000
```

... why do I have the same frame mapped to 2 addresses?  This may actually be proper based on the PMM design.  Let's see
* `0xffffff0000000000` is a temporary page for performing some setup.
* `0xffffff0000003000` is a scrub stack for frames that need to be purged.

And reviewing the code against the log, the rest is correct -- mapping a page to a temporary address space for the sake of filling in some values.

The other thing to keep in mind is that the address space `0x0000000001004000` is the loader/kernel module address space.

The rest of the mappings look reasonable.  But I am still getting a triple fault.

The Virtual Address Space for the Idle process is not being created:

```
=========================================================
Dumping Process_t structure at address 0xffffb80000000110
---------------------------------------------------------
  TOS (last preemption)..: 0xffffaf0000003fb8
  Virtual Address Space..: 0x0000000000000000
  Process Status.........: 0 (INIT)
  Process Priority.......: 30 (OS)
  Quantum left this slice: 30
  Process ID.............: 1
  Command Line...........: Idle Process
  Micros used............: 0
  Wake tick number.......: 0
  Pending Error Number...: 0
=========================================================
```

So, I need to look back at `ProcessCreate()` as I recall not populating that field.  That being corrected, I am still getting a triple fault with no indication why.

So, this is a stack problem:

```
00191759008i[      ] Instruction prepared
00191759008d[CPU0  ] page walk for address 0xffffaf0000003f20
00191759008d[CPU0  ] PAE PML4: entry not present
00191759008d[CPU0  ] page fault for address ffffaf0000003f20 @ ffff800000001ebc
00191759008i[      ] CPU 0: exception 0eh error_code=2
00191759008i[CPU0  ] CPU is in long mode (active)
00191759008i[CPU0  ] CS.mode = 64 bit
00191759008i[CPU0  ] SS.mode = 64 bit
00191759008i[CPU0  ] EFER   = 0x00000500
00191759008i[CPU0  ] | RAX=0000000000000200  RBX=ffffa00000001240
00191759008i[CPU0  ] | RCX=0000000001c9c380  RDX=ffffb80000000088
00191759008i[CPU0  ] | RSP=ffffaf0000003f28  RBP=0000000000000000
00191759008i[CPU0  ] | RSI=ffffa000000091e0  RDI=ffffaf0000003f20
00191759008i[CPU0  ] |  R8=00000000000000a8   R9=00000000000000a8
00191759008i[CPU0  ] | R10=00000001fffff000  R11=ffff80000000d044
00191759008i[CPU0  ] | R12=0000000000000000  R13=0000000000000000
00191759008i[CPU0  ] | R14=00000001ffff3000  R15=ffffa00000006551
00191759008i[CPU0  ] | IOPL=0 ID vip vif ac vm rf nt of df if tf SF zf af pf cf
00191759008i[CPU0  ] | SEG sltr(index|ti|rpl)     base    limit G D
00191759008i[CPU0  ] |  CS:0008( 0001| 0|  0) 00000000 00000fff 1 0
00191759008i[CPU0  ] |  DS:0028( 0005| 0|  0) 00000000 00000fff 1 0
00191759008i[CPU0  ] |  SS:0010( 0002| 0|  0) 00000000 00000fff 1 0
00191759008i[CPU0  ] |  ES:0028( 0005| 0|  0) 00000000 00000fff 1 0
00191759008i[CPU0  ] |  FS:0000( 0000| 0|  0) 00000000 00000000 0 0
00191759008i[CPU0  ] |  GS:0048( 0009| 0|  0) 0000e050 0000ffff 1 1
00191759008i[CPU0  ] |  MSR_FS_BASE:0000000000000000
00191759008i[CPU0  ] |  MSR_GS_BASE:ffff80000000e050
00191759008i[CPU0  ] | RIP=ffff800000001ebc (ffff800000001eba)
00191759008i[CPU0  ] | CR0=0xe0000011 CR2=0xffffaf0000003f20
00191759008i[CPU0  ] | CR3=0x00000001fffff000 CR4=0x000000a0
00191759008d[CPU0  ] exception(0x0e): error_code=0002
00191759008i[      ] CPU 0: interrupt 0eh
00191759008d[CPU0  ] interrupt(): vector = 0e, TYPE = 3, EXT = 1
```

We are faulting on the instruction:

```
ffff800000001eba:       ff d3                   callq  *%rbx
```

So, my virtual address space is `0x00000001fffff000`.  This does not appear to be a proper virtual address space.  As a matter of fact, the `cr3` from value of `0x00000001ffff3000` does not appear to be correct.

Could these supposed to be `0x000000023ffff000` and `0x000000023ffff000` respectively?  I doubt it, but I have to consider it.

It looks like the vector table `cr3` member is being set improperly or overridden.

```
<bochs:4> x /8 0xffff800000019210
[bochs]:
0xffff800000019210 <bogus+       0>:    0x00001240      0xffffa000      0xfffff000      0x00000001
0xffff800000019220 <bogus+      16>:    0x00000000      0x00000000      0x00000000      0x00000000
```

So, it is incorrect by the time I get to the first interrupt.

CRAP!!!!  This is a difference in the memory allocation!  So the address spaces are the same based on whether I am running QEMU or Bochs!  I'm betting a difference in the memory map from BIOS since they both are allocating 8G.

So, back to the stack allocation.

OK, so the stack is not mapped, but it is allocated in the correct address space.

```
In address space 0x000000023fff3000, allocating stack 0xffffaf0000000000
=========================================================
Dumping Process_t structure at address 0xffffb80000000110
---------------------------------------------------------
  TOS (last preemption)..: 0xffffaf0000003fb8
  Virtual Address Space..: 0x000000023fff3000
  Process Status.........: 0 (INIT)
  Process Priority.......: 30 (OS)
  Quantum left this slice: 30
  Process ID.............: 1
  Command Line...........: Idle Process
  Micros used............: 0
  Wake tick number.......: 0
  Pending Error Number...: 0
=========================================================
Readying process at 0xffffb80000000110
```

So, either I missed the mapping completely or I have changed address spaces before the stack is mapped.


---

### 2021-Aug-07

Everything looks to be correct for the idle process except that the stack is not mapped.  Now, I know I have problems with any other address space -- the stack mapping should probably be a kernel function.

Nope, looks like I am missing a frame:

```
In address space 0x00000001ffff3000, allocating stack 0xffffaf0000000000
Mapping the stack into address space 0x00000001ffff3000
In address space 0x00000001ffff3000, request was made to map address 0xffffaf0000000000 to frame 0x00000000001fffd9
In address space 0x0000000001065000, request was made to map address 0xffffff0000005000 to frame 0x00000000001fffd2
In address space 0x0000000001065000, Unmapping page at address 0xffffff0000005000
In address space 0x0000000001065000, request was made to map address 0xffffff0000005000 to frame 0x00000000001fffd1
In address space 0x0000000001065000, Unmapping page at address 0xffffff0000005000
In address space 0x0000000001065000, request was made to map address 0xffffff0000005000 to frame 0x00000000001fffd0
In address space 0x0000000001065000, Unmapping page at address 0xffffff0000005000
In address space 0x00000001ffff3000, request was made to map address 0xffffaf0000001000 to frame 0x00000000001fffd8
In address space 0x00000001ffff3000, request was made to map address 0xffffaf0000002000 to frame 0x00000000001fffd7
```

I am adding some code to check that.  I should be getting 4 mapped pages, but I am only seeing 3, almost like `frameCount` is being changed.

```C++
    KernelPrintf("Mapping the stack into address space %p\n", GetAddressSpace());
    for (int i = 0; i < frameCount; i ++) {
        MmuMapPage(stackLoc + (PAGE_SIZE * i), stackFrames[i], PG_WRT);
    }
```


---

### 2021-Aug-08

So, that ended up not being the problem.  I am still trying to find a good place to stop so I can single step the code.

OK, the problem appears to be in the `ProcessStart()` function in the call to the `ProcessUnlockScheduler()` function call:

```c++
void ProcessStart(void)
{
    assert_msg(AtomicRead(&scheduler.schedulerLockCount) > 0,
            "`ProcessStart()` is executing for a new process without holding the proper lock");

    assert_msg(AtomicRead(&scheduler.schedulerLockCount) == 1,
            "`ProcessStart()` is executing while too many locks are held");

    KernelPrintf("Starting new process with address space %p\n", GetAddressSpace());
    ProcessUnlockScheduler();    //    Problem here?
}
```

---

### 2021-Aug-11

Still debugging.

OK, I think I have it now.  The timer interrupt is using its own address space, but any old stack.  I need to see if I can get a proper stack mapped in the LAPIC Init function.  I will probably need to create a dedicated stack for each ISR and handle the stack change properly.


---

### 2021-Aug-21

Let's get some facts together for this change.  I will need to ensure I have all the proper data set up for this work.  Currently, the only interrupt function is `tmr_Interrupt`.  So, I do not have an extensive number of functions to retrofit.

However, the current structure for loading a module looks like this:

```c++
//
// -- This is the structure that will be available to determine how to load a module
//    ------------------------------------------------------------------------------
typedef struct Module_t {
    char sig[16];
    char name[16];
    uint64_t earlyInit;
    uint64_t lateInit;
    uint64_t intCnt;
    uint64_t internalCnt;
    uint64_t osCnt;
    struct {
        uint64_t loc;
        uint64_t target;
    } hooks [0];
} Module_t;
```

In particular, the `hooks` is used for all Internal Functions, IRQ Services, and OS Services.

To correct this problem, I am going to have to add a stack location to this structure for IRQ Services, as in this:

```c++
    struct {
        uint64_t loc;
        uint64_t target;
        uint64_t stack;
    } hooks [0];
```

But, do I do this for all hooks?  Or do I do this only for IRQ hooks?

---

### 2021-Aug-27

I will do this for all of them.  The concern is making certain I can read all the `hooks`.  I know I could get around it and save a few bytes, but I am going for simplicity at this point.

Some quick changes to several `entry.s` files and my structure and the data align.  I am able to get the modules loaded and initialized (at least to the same point).

The next step is to get the stack added into the 3 types of structures (IRQ, OS Service, and Internal Function).  For this, I may want to separate the concerns in code a bit more.

---

I have a problem with an internal function for setting a vector handler.  The function prototypes do not match.  I do not believe I am using that at this pont yet, but it will need to get fixed.


---

### 2021-Sep-02

I think it is time to drop back 10 and punt.  I do have issues with the internal functions.  The problem I have is that there are too many files that all need to be in sync in order for things to work properly.  I know that Linux uses a crap-ton of macros to accomplish the same thing.  I do not want to go down that path, but I may not have a choice.  In the meantime, [Redmine #491](http://eryjus.ddns.net:3000/issues/491) has been created to capture the vector problem.

Having cleaned up the surface mess, I am able to get a good boot.  But I do not recall what I commented out to get here.  For the moment, it looks like nothing.  However, I am not getting any interrupts where I think I should expect to.

OK, so Bochs reports interrupts are enabled.  So, either I broke the timer programming or the interrupt is bailing out before a handler is called:

```
<bochs:2> reg
CPU0:
rax: 00000000_00000014
rbx: ffff8000_00006a80
rcx: 00000000_00000000
rdx: 00000000_000003f8
rsp: fffff800_00003ff0
rbp: 00000000_00100
b60
rsi: 00000000_00000002
rdi: 00000000_0000000a
r8 : cccccccc_cccccccd
r9 : 00000000_000000a8
r10: ffffa000_00029208
r11: 00000000_00000000
r12: 00000000_00101170
r13: 00000000_001000a0
r14: 00000000_0010c000
r15: fffff800_00004000
rip: ffff8000_000069f0
eflags 0x00200286: ID vip vif ac vm rf nt IOPL=0 of df IF tf SF zf af PF cf
```

The Bochs log reports:

```
00245141503d[APIC0 ] local apic timer(periodic) triggered int, reset counter to 0x000000fa
00245145503d[APIC0 ] trigger interrupt vector=0x20
00245145503d[APIC0 ] triggered vector 0x20
00245145503d[APIC0 ] triggered vector 0x20 not accepted
```

I think I may need to revisit my internal function setup sooner rather than later.  Something is simply not right and I need to get that sorted.

---

### 2021-Sep-03

So, for the first bit of cleanup, I need to work with the Internal Functions table.  This will need to be moved to a new source file as I separate it from the IRQ table and the OS Services table.

I was able to get the 3 service tables re-worked and I still have the same problems.  At least I did not make the problems worse.

Do I have an outstanding bug in the LAPIC code?  I may since I am not getting a timer tick.

---

### 2021-Sep-06

Hmmm....

```
00187359833d[APIC0 ] trigger interrupt vector=0x20
00187359833d[APIC0 ] triggered vector 0x20
00187359833d[APIC0 ] lapic(0): not delivering int 0x20 because int 0x20 is in service
00187359833d[APIC0 ] local apic timer(periodic) triggered int, reset counter to 0x000000fa
```

Not delivering the interrupt, huh?  Where is that coming from?

```c++
void bx_local_apic_c::service_local_apic(void)
{
  if(bx_dbg.apic) {
    BX_INFO(("service_local_apic()"));
    print_status();
  }

  if(cpu->is_pending(BX_EVENT_PENDING_LAPIC_INTR)) return;  // INTR already up; do nothing

  // find first interrupt in irr.
  int first_irr = highest_priority_int(irr);
  if (first_irr < 0) return;   // no interrupts, leave INTR=0
  int first_isr = highest_priority_int(isr);
  if (first_isr >= 0 && first_irr <= first_isr) {
    BX_DEBUG(("lapic(%d): not delivering int 0x%02x because int 0x%02x is in service", apic_id, first_irr, first_isr));
    return;
  }
  if(((Bit32u)(first_irr) & 0xf0) <= (task_priority & 0xf0)) {
    BX_DEBUG(("lapic(%d): not delivering int 0x%02X because task_priority is 0x%02X", apic_id, first_irr, task_priority));
    return;
  }
  // interrupt has appeared in irr. Raise INTR. When the CPU
  // acknowledges, we will run highest_priority_int again and
  // return it.
  BX_DEBUG(("service_local_apic(): setting INTR=1 for vector 0x%02x", first_irr));
  cpu->signal_event(BX_EVENT_PENDING_LAPIC_INTR);
}
```

OK, so, I am getting the following early in the log:

```
00179835838d[APIC0 ] LAPIC write 0x00020020 to register 0x0320
00179839833d[APIC0 ] trigger interrupt vector=0x20
00179839833d[APIC0 ] triggered vector 0x20
00179839833d[APIC0 ] service_local_apic(): setting INTR=1 for vector 0x20
00179839833d[APIC0 ] local apic timer(periodic) triggered int, reset counter to 0x000000fa
```

And this is the only time I am getting the `setting INTR=1 for vector 0x20` message.  So, the interrupt is being generated and the system thinks it is still being serviced.  However, when I enable interrupts (`sti`), nothing is coming across.

The write the the LAPIC Timer register is as follows:

```
00179835838d[APIC0 ] LAPIC write 0x00020020 to register 0x0320
```

... and this does not quite make sense.  Let's pull this apart.

* Periodic (0x00020000)
* IRQ 0x20 (0x00000020)

So, this is correct.

---

### 2021-Sep-07

So, I need to change the several interrupt entry targets to have a common entry/exit so that I only have to think about and maintain one set of code.  This will be the focus of today's work.

There are 4 entry points to consider:
1. IsrCommonStub -- which is the entry point after the ISR pushes the interrupt number and 0 error code on the stack.
1. IdtGenericEntry and IdtGenericEntryNoErr -- which are the exception entry points for handling CPU exceptions.
1. InternalTarget -- which is the target for internal functions
1. ServiceTarget -- which is the target for OS service functions

Making these all use the same code is going to be a little difficult to do.  There are some requirements I need to consider:
* There is a jump table that needs to be passed into the `*Handler()` function.
* There are limits to check

Actually, I think those are all of them and the `*Handler()` function can probably handle the limit checking.  So, this means I need to move the '*Handler()` function into a register in each of the 4 entry points and call the function using that register.  It also means I need to push that register onto the stack in the entry.  If I set things up properly, I should be able to have a common function epilogue.

So, let's build a common stack in the order in which things are pushed:
* `ss` -- pushed by CPU exception/INT instruction
* `rsp` -- pushed by CPU exception/INT instruction
* `rflags` -- pushed by CPU exception/INT instruction
* `cs` -- pushed by CPU exception/INT instruction
* `rip` -- pushed by CPU exception/INT instruction
* error code -- pushed by CPU exception/entry point code
* interrupt -- pushed by entry point code
* `rax` -- pushed by entry point code and set to the table entry address
* `rbx` -- pushed by entry point code and set to the handler address
* `rcx`
* `rdx`
* `rbp`
* `rsi`
* `rdi`
* `r8`
* `r9`
* `r10`
* `r11`
* `r12`
* `r13`
* `r14`
* `r15`
* `cr1`
* `cr2`
* `cr3`
* `cr4`
* `ds`
* `es`
* `fs`
* `gs`
* old stack location

---

### 2021-Sep-09

OK, now I have a problem with the stack.  The called function will not have access to all the stack values.  For the software interrupts this should not be a problem since the values are passed in via registers.  For the IRQs, access to the registers should not be an issue since they are not in context.  For the faults and exceptions, this may be a problem if the system needs to dump the registers.  However, these should not need a replacement stack so it should be good.

So, the next step here is to create a common Service Routine Structure and replace the existing structures with the new one.

I will have a problem with the fault handler since it accepts a pointer to the stack of register values for dumping.  However, I think I can create a function that will handle that, or possible even store it in the Service Routine Structure itself.

---

OK, I am back to compiling again.  There is still a bunch of code commented out that I need to address.  I also need to rebuild the basic interface routines for each published internal function.

Oh, and a crap-ton of debugging!

---

OK, I am popping the old `rax` value from the stack, which is unconditionally overwriting the return value for internal function calls.

I think I have that cleaned up.  More unit testing tomorrow.

---

### 2021-Sep-10

So, the unit test that passed yesterday was a fluke.  I need to do some more debugging.  In addition, the `SetInternalHandler()` function is not working either.  One thing at a time.  I need to get `GetInternalHandler()` working properly first.

I do have the internal function handler get and set functions working properly now.  Next, I will move the IRQ/interrupt handler get/set functions up behind those.  Really, these are vector handlers.  So I need a rename here as well.

I have the vector handler internal functions working now as well.  Finally for the OS Service Handlers.  For this, I will also need a function to dump the table.

OK, that completes the tables interface for all 3 types of tables.  All 3 have tested out properly.  All 3 had problems.

The next thing to do is to bring `kprintf()` (`KernelPrintf()`) up and get it working.  For this first test, I get a triple fault.

I'm going to have to spend some time thinking about this due to the variable arguments.

---

### 2021-Sep-11

OK, the first thing I found was that the function number was not set correctly.  I was able to get that corrected quickly, but then `kprintf()` thinks I have more than 6 parameters, which I do not.

I had to create a stub function to massage out the parameter sequence properly and then jump to `kprintf()`.  This now works.

I have the following groups of kernel functions left to clean up:
* Spinlocks
* MMU
* Utility functions

I want to place utility functions last in the list of kernel functions as these utility functions are expected to grow.  I want to be able to leave a reasonable gap.  Spinlocks are not branded properly in the code, so I think I will take those on now.

And testing a Lock, I get a triple fault again.  This was a simple fix -- I missed adding in the new extra parameter.

MMU functions are mis-branded as well.  However, these are used all over the place.  I will need to clean that up.

Actually, that was not too bad.  Finally, the utility functions.  That was easy.

All of these so far use the existing stack and address space.  These values remain 0 in the respective table entries.  On the other hand, the modules require their own address space (which I know from `ModuleEarlyInit()`) and each function needs a stack to use.  But, now I need to be careful as several functions may be calling the same module at the same time.  Does the fact that each module and function have a global stack defined get me into trouble?

I believe it will, so each table entry will need its own spinlock:

```c++
typedef struct Spinlock_t {
    volatile int lock;
    Addr_t flags;
} Spinlock_t;
```

This should add another 12 or 16 bytes to each table entry.  First, let's figure out which:

```
Spinlocks:
  size: 16
  flags offset: 8
```

16 bytes.  So I will need to make the structure 64-bytes long, adding another 32-bytes to each entry to make the match easier.  Nah...  just a little judicious register usage.

With the calls to get and release a spinlock, I now need to test again.  Things still work.

The next thing to decide here is the number of stack frames I need for each function.  I do not want too many (a waste of memory) and I cannot overflow the stacks.  At 8 bytes per push:
* 4K stack is 512 pushes
* 8K stack is 1024 pushes
* 16K stack is 2048 pushes

Since most calls will use the registers to pass data, I should only really need a 4K stack.  My stack allocator is for 16K stacks.  But I do not really need that do I?  These are not processes, these are stacks for use with an interrupt when needed.  I should be able to start with a defined address and increment from there.  I might even be able to present this address in the entry structure, making the stack address configurable by module.

---

I am only going to load the `lapic.elf` for now.  I need to get everything working right with a single module and get the timer to fire before I start adding on other complications.

OK, I am having problems with the `gs` segment and its intended offsets:

```
00174630398i[      ] Instruction prepared
00174630398d[CPU0  ] page walk for address 0x0000000000000000
00174630398d[CPU0  ] PAE PTE: entry not present
00174630398d[CPU0  ] page fault for address 0000000000000000 @ ffffa000000012be
```

```
ffffa000000012b5:       65 48 8b 04 25 00 00    mov    %gs:0x0,%rax
ffffa000000012bc:       00 00
ffffa000000012be:       83 38 00                cmpl   $0x0,(%rax)
```

```
<bochs:4> sreg
es:0x0028, dh=0x00a09300, dl=0x00000000, valid=1
        Data segment, base=0x00000000, limit=0x00000fff, Read/Write, Accessed
cs:0x0008, dh=0x00a09b00, dl=0x00000000, valid=1
        Code segment, base=0x00000000, limit=0x00000fff, Execute/Read, Non-Conforming, Accessed, 64-bit
ss:0x0010, dh=0x00a09300, dl=0x00000000, valid=1
        Data segment, base=0x00000000, limit=0x00000fff, Read/Write, Accessed
ds:0x0028, dh=0x00a09300, dl=0x00000000, valid=1
        Data segment, base=0x00000000, limit=0x00000fff, Read/Write, Accessed
fs:0x0000, dh=0x00001000, dl=0x00000000, valid=0
gs:0x0048, dh=0x00e0f300, dl=0x0000000f, valid=1
        Data segment, base=0x00000000, limit=0x0000ffff, Read/Write, Accessed
ldtr:0x0000, dh=0x00008200, dl=0x0000ffff, valid=1
tr:0x0050, dh=0x00008b01, dl=0x2000006f, valid=1
gdtr:base=0xffff800000011380, limit=0xa7
idtr:base=0xffff800000013000, limit=0xfff
```

```
<bochs:3> info gdt
Global Descriptor Table (base=0xffff800000011380, limit=167):
GDT[0x0000]=??? descriptor hi=0x00000000, lo=0x00000000
GDT[0x0008]=Code segment, base=0x00000000, limit=0x00000fff, Execute/Read, Non-Conforming, Accessed, 64-bit
GDT[0x0010]=Data segment, base=0x00000000, limit=0x00000fff, Read/Write, Accessed
GDT[0x0018]=Code segment, base=0x00000000, limit=0x00000fff, Execute/Read, Non-Conforming, 64-bit
GDT[0x0020]=Data segment, base=0x00000000, limit=0x00000fff, Read/Write
GDT[0x0028]=Data segment, base=0x00000000, limit=0x00000fff, Read/Write, Accessed
GDT[0x0030]=Data segment, base=0x00000000, limit=0x00000fff, Read/Write
GDT[0x0038]=Code segment, base=0x00000000, limit=0x00000fff, Execute/Read, Non-Conforming, 64-bit
GDT[0x0040]=Data segment, base=0x00000000, limit=0x00000fff, Read/Write
GDT[0x0048]=Data segment, base=0x00000000, limit=0x0000ffff, Read/Write, Accessed
GDT[0x0050]=32-Bit TSS (Busy) at 0x00012000, length 0x0006f
GDT[0x0058]=??? descriptor hi=0x00000000, lo=0xffff8000
GDT[0x0060]=Data segment, base=0x00000000, limit=0x0000ffff, Read/Write
GDT[0x0068]=32-Bit TSS (Available) at 0x00012070, length 0x0006f
GDT[0x0070]=??? descriptor hi=0x00000000, lo=0xffff8000
GDT[0x0078]=Data segment, base=0x00000000, limit=0x0000ffff, Read/Write
GDT[0x0080]=32-Bit TSS (Available) at 0x000120e0, length 0x0006f
GDT[0x0088]=??? descriptor hi=0x00000000, lo=0xffff8000
GDT[0x0090]=Data segment, base=0x00000000, limit=0x0000ffff, Read/Write
GDT[0x0098]=32-Bit TSS (Available) at 0x00012150, length 0x0006f
GDT[0x00a0]=??? descriptor hi=0x00000000, lo=0xffff8000
```

So, it looks like I do not have a proper `gs` loaded.  I will set a breakpoint to see if the code is even executing.

I finally have the Timer interrupt firing again.

To wrap up the `lapic` module, I need to remap the internal functions.  This is done and I can call the function.  The timer does not mess up.

Next, let's get the PMM installed and running.  I am setting the PMM up before the LAPIC.  With that, I am getting stuck in the PMM stack initialization.

Ah-ha!!!  I have circular dependencies.  The `PmmAlloc()` is calling `MmuMapPage()` which will get a frame from `PmmAlloc()`.  This results in a deadlock.

---

### 2021-Sep-12

I have a problem with the `#PF` handler where it is generating a `#PF`....  This is the real result of the deadlock.  Objectively:

```
00175422893i[      ] CPU 0 at 0xffff80000000a7a4: lock cmpxchg dword ptr ds:[rsi], ecx   (reg results):
00175428713i[      ] CPU 0 at 0xffff80000000a7c8: xchg dword ptr ds:[rsi], eax   (reg results):
00175428778i[      ] CPU 0: exception 0eh error_code=0
00175428778d[CPU0  ] exception(0x0e): error_code=0000
00175428853i[      ] CPU 0 at 0xffff80000000a7a4: lock cmpxchg dword ptr ds:[rsi], ecx   (reg results):
00175428883i[      ] CPU 0: exception 0eh error_code=0
00175428883d[CPU0  ] exception(0x0e): error_code=0000
00175428953i[      ] CPU 0 at 0xffff80000000a7a4: lock cmpxchg dword ptr ds:[rsi], ecx   (reg results):
00175428968i[      ] CPU 0 at 0xffff80000000a7a4: lock cmpxchg dword ptr ds:[rsi], ecx   (reg results):
00175428978i[      ] CPU 0 at 0xffff80000000a7a4: lock cmpxchg dword ptr ds:[rsi], ecx   (reg results):
...
```

Where the instruction at `...a7a4` locks a spinlock and the instruction at `...a7c8` unlocks a spinlock.  I have no way in this log to know which spinlock is being locked, but it is clear there is a `#PF` which is locking a spinlock and a second `#PF` which infinitely attempts to lock a spinlock.

So, the recursion identified yesterday is not significant.  It almost makes me wonder if I should not get a lock when there is no stack to protect.  It would make sense.  This change puts me back to a triple fault.

I need to determine which fault to fix first.  The `#PF` in the `#PF` handler?  Or, the originating `#PF`?  I think I need my handlers to work properly, so let's get that working.  Yes, this is critical since I changed how the stack is loaded and where to find all the values required to print out the register contents.

A quick change to the code to not read from the stack has at least stopped the triple faults.  So, the next question is about what to do with the real problem.  Can I simply place the stack into the handler structure (where I have space allocated)?  Or do I need to pass the stack pointer in?

Here's what I have found out.  I have no handler installed for the `#PF` vector.  When this happens, the interrupt target function exits.  When it exits and hits the `iret` function, there appears to be a stack alignment problem.

I was able to get that figured out and also set up the `vectorTable` properly.  I am back in business now and will switch back to debugging the original cause for the page fault.

Nope.  The registers are not aligned in the output.  I need to get that sorted out first (while I have a fault to debug).

After a change to align the stack, the output now shows the following:

```
An exception has occurred
An error (interrupt 0xe) has occurred (Error Code 0x0000000000000000)
  RAX: 0x0000000000000000       R8 : 0x0000000000000000
  RBX: 0xffffa00000005000       R9 : 0x0000000100000000
  RCX: 0xffff8000000042a8       R10: 0x000089000000006f
  RDX: 0xffffa00000002260       R11: 0x0000000000000000
  RBP: 0x000000000000000c       R12: 0x0000000000000000
  RSI: 0xffffa000000020c0       R13: 0x000000000000000c
  RDI: 0xffffa00000005050       R14: 0xffffa00000005018
                                R15: 0xffffffffffffffff
   SS: 0x10     RSP: 0xfffff80000003dd0
   CS: 0x8      RIP: 0xffffa00000001905
   DS: 0xa0      ES: 0x28
   FS: 0x28      GS: 0x0        RFLAGS: 0x0000000000210046

CR0: 0x0000000000000000
CR2: 0x00000000e0000011
CR3: 0x0000000000000000
CR4: 0x0000000001065000
```

The Bochs log reveals:

```
00174963498i[      ] CPU 0: exception 0eh error_code=0
00174963498i[CPU0  ] CPU is in long mode (active)
00174963498i[CPU0  ] CS.mode = 64 bit
00174963498i[CPU0  ] SS.mode = 64 bit
00174963498i[CPU0  ] EFER   = 0x00000500
00174963498i[CPU0  ] | RAX=0000000000000000  RBX=ffffa00000005000
00174963498i[CPU0  ] | RCX=ffffa00000002260  RDX=000000000000000c
00174963498i[CPU0  ] | RSP=fffff80000003dd0  RBP=ffffa000000020c0
00174963498i[CPU0  ] | RSI=ffffa00000005050  RDI=0000000000000000
00174963498i[CPU0  ] |  R8=0000000100000000   R9=000089000000006f
00174963498i[CPU0  ] | R10=0000000000000000  R11=0000000000000000
00174963498i[CPU0  ] | R12=000000000000000c  R13=ffffa00000005018
00174963498i[CPU0  ] | R14=ffffffffffffffff  R15=0000000000000000
00174963498i[CPU0  ] | IOPL=0 ID vip vif ac vm rf nt of df if tf sf ZF af PF cf
00174963498i[CPU0  ] | SEG sltr(index|ti|rpl)     base    limit G D
00174963498i[CPU0  ] |  CS:0008( 0001| 0|  0) 00000000 00000fff 1 0
00174963498i[CPU0  ] |  DS:0028( 0005| 0|  0) 00000000 00000fff 1 0
00174963498i[CPU0  ] |  SS:0010( 0002| 0|  0) 00000000 00000fff 1 0
00174963498i[CPU0  ] |  ES:0028( 0005| 0|  0) 00000000 00000fff 1 0
00174963498i[CPU0  ] |  FS:0000( 0000| 0|  0) 00000000 00000000 0 0
00174963498i[CPU0  ] |  GS:0048( 0009| 0|  0) 00010050 0000ffff 1 1
00174963498i[CPU0  ] |  MSR_FS_BASE:0000000000000000
00174963498i[CPU0  ] |  MSR_GS_BASE:ffff800000010050
00174963498i[CPU0  ] | RIP=ffffa00000001908 (ffffa00000001905)
00174963498i[CPU0  ] | CR0=0xe0000011 CR2=0x0000000000000000
00174963498i[CPU0  ] | CR3=0x0000000001065000 CR4=0x000000a0
```

This reveals I missed a call (and return IP address) on the stack that needed to be skipped.

Working to clear that up, it is properly displaying all the data.  So, I can now turn my attention to the actual fault.

I forgot to set up the discarded handler number in the function prototypes.  I am now properly processing interrupts again.  I also have the PMM loading before the LAPIC, which I think is preferable.  Now to add the scheduler back in.

With the scheduler added back in, I am back to a triple fault.  I thought I had the interrupt handlers handled.  The triple fault is happening while in the PMM Late Init function.  I believe this is back to the original problem for which I needed to add individual stacks.

---

I need to make some notes about the stack to see how that conforms on exit:

| Stack Location     |  Reg        | Value              | Chk |
|:------------------:|:-----------:|:------------------:|:---:|
| 0xffffaf4000001f20 | Error Code  | 0x0000000000000000 |     |
| 0xffffaf4000001f18 | `rdi` (fct) | 0x0000000000000050 |     |
| 0xffffaf4000001f10 | `rax`       | 0xffffa00000004cbe |     |
| 0xffffaf4000001f08 | `rbx`       | 0xffffb80000000358 |     |
| 0xffffaf4000001f00 | `rip`       | 0xffff8000000042e7 |     |
| 0xffffaf4000001ef8 | `rcx`       | 0x0000000000000001 |     |
| 0xffffaf4000001ef0 | `rdx`       | 0x000000000000000c |     |
| 0xffffaf4000001ee8 | `rbp`       | 0xffffa00000004f60 |     |
| 0xffffaf4000001ee0 | `rsi`       | 0x0000000000000000 |     |
| 0xffffaf4000001ed8 | `rdi`       | 0x0000000000000050 |     |
| 0xffffaf4000001ed0 | `r8`        | 0xffffb80000010000 |     |
| 0xffffaf4000001ec8 | `r9`        | 0x00000000000000a8 |     |
| 0xffffaf4000001ec0 | `r10`       | 0xffffa00000029208 |     |
| 0xffffaf4000001eb8 | `r11`       | 0xfffff80000003e58 |     |
| 0xffffaf4000001eb0 | `r12`       | 0xffffb80000000450 |     |
| 0xffffaf4000001ea8 | `r13`       | 0xffffa00000004c90 |     |
| 0xffffaf4000001ea0 | `r14`       | 0xffffa00000006f9d |     |
| 0xffffaf4000001e98 | `r15`       | 0xffffa00000001c70 |     |
| 0xffffaf4000001e90 | `cr0`       | 0x00000000e0000011 |     |
| 0xffffaf4000001e88 | `cr2`       | 0x0000000000000000 |     |
| 0xffffaf4000001e80 | `cr3`       | 0x00000001fffe9000 |     |
| 0xffffaf4000001e78 | `cr4`       | 0x00000000000000a0 |     |
| 0xffffaf4000001e70 | `ds`        | 0x0000000000000028 |  N  |
| 0xffffaf4000001e68 | `es`        | 0x0000000000000028 |  N  |
| 0xffffaf4000001e60 | `fs`        | 0x0000000000000000 |  N  |
| 0xffffaf4000001e58 | `gs`        | 0x0000000000000048 |  N  |
| 0xffffaf4000000ff8 | `rsp`       | 0xffffaf4000001e58 |  Y  |
| 0xffffaf4000000ff0 | `rip`       | 0xffff8000000043e5 |  Y  |

I see that these are all returning 0 values.  Including a return to the `0x0000000000000000` address, which is a `#PF` for sure.  So, what is going on with that mapping or that memory?

---

### 2021-Sep-13

Well, it does not appear that the mapping is being overwritten and it does not appear that the frame is being mapped too many times.

Actually, I may be reading the wrong stack.  Perhaps.

I can say for certain that when I do not create the PMM Cleaner process (comment the code out) the system boots completely.  So, I am going to go with a flaw in the scheduler code (`ProcessCreate()`) and how the stack needs to be created.  It is likely not being created and mapped in the correct address space.

---

So, I think this is going to boil down to the design decision I made to split the scheduler into its own module.  The scheduler just cannot develop the proper context for building a process stack.  That whole process creation logic needs to be revisited and handled differently.  Most likely I will need to start with a change to the MMU code so that I can pass in an address space in which to make the change.

I have interrupts enabled and without the PMM Cleaner process the code boots properly.  I do not yet have the idle processes getting CPU -- so no task swapping.  But I have enough changes I really should commit something.  It's time for a micro-commit.









