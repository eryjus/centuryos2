# The Century OS -- v0.0.12

This project is a 64-bit focused version of Century-OS.  This version takes from the 32-bit version of Century-OS, but is better streamlined for the 64-bit CPUs (where any 32-bit archs will be added in later if desired).


---

## Version 0.0.12 -- Start all the CPUs

This will be a big version for me.  We are going to start all the AP CPUs.  Up to this point, everything has been working on a single CPU.

Specifically, this version will include:
* Create the trampoline code
* Copy the trampoline code to the proper location (PMM & MMU considerations)
* Create IPI handlers and install them properly
* Send the Startup Sequence to the other CPUs and confirm they spin-up
* Create an idle process for each CPU
* Shore up the debugging module, stopping all CPUs (IPI) when a critical function is engaged


---

### 2021-Oct-22

This version is going to require a few things to get working properly.  First I will need to set aside the trampoline code into a separate section.  The linker script will need to be updated to accommodate that new section.  Once I do that I will need to get the kernel to build properly.  While I am working on that, I will create a stub function to start the CPUs.  This will be the first step.

By the way, I am hoping I can go directly to long mode with this wiki: https://wiki.osdev.org/Entering_Long_Mode_Directly.  I do not have much checking to do to get there since the boot processor (BSP) is already in long mode and I am working on homogenous processors.

The IDT is going to have to be loaded in the kernel.  This is not located in low address space.

---

So, I have the start of the trampoline code written.  The linker is updated with `.smptext` in its own section.  I really need to publish some addresses so I can find it.

I also created a `CpuApStart()` function in `cpus.cc` which is stubbed out properly.  There is a question in my mind about whether the APs should start before the BSP has interrupts enabled or not.  I really am not sure it matters much except that the startup from the BSP probably is timing critical and cannot tolerate interrupts.  This means that the APs will likely start servicing interrupts before the BSP will.

While I have not made any material changes in the logic, it still boots -- had to check.

Now, I need to map and copy the trampoline code to the proper location.  At the same time, I will need to fix-up the data elements at that address.


---

### 2021-Oct-23

Today I think I am going to start with trying to identify the `.smptext` section.  This will be done with my friend `kprintf()` and the new `#if DEBUG_ENABLED()` controls.

OK, actually the `__smp_start` variable is presented in the loader, not the kernel.  But, the kernel needs it.  So, I need to pass the location through the boot loader interface structure.

I finally broke down and enabled all the debugging output.  I will have to filter that down again as I get into each source file.  I think I will either take that on with v0.0.13 or after v0.0.15 as a special version.

From the old 32-bit version of the OS, this is the code which actually spun up the additional CPUs:

```c++
    cpus.perCpuData[0].location = ArchCpuLocation();
    cpus.perCpuData[0].tss.esp0 = currentThread->tosKernel;

    for (int i = 1; i < cpus.cpusDiscovered; i ++) {
        cpus.cpuStarting = i;
        AtomicSet(&cpus.perCpuData[cpus.cpuStarting].state, CPU_STARTING);

        kprintf("Starting core %d \n", i);
        picControl->PicBroadcastInit(picControl, i);
        picControl->PicBroadcastSipi(picControl, i, (archsize_t)trampoline);

        while (AtomicRead(&cpus.perCpuData[cpus.cpuStarting].state) == CPU_STARTING) {}
    }
```

The key for this was the PIC being able to broadcast an IPI.  At the moment, I have nothing of the sort.  I also am not sure I have the `cpus` structure dialled in the way I need it to be.

The `ArchCpuLocation()` was the APIC ID (x86) or the MultiProcessor Affinity Register (armv7).

So, I need to add 4 functions into the LAPIC code:
1. Get LAPIC ID for the current CPU
2. Send Init IPI
3. Send SIPI
4. Send general IPI

Each of these should be hooked into a special IPI group of functions.  The debugger took the `0x7x` group of functions.  Next is the `0x8x` grouping.

I currently have the CPU count hard-coded at 1.  I am updating that to 2 for testing.  However, I have entered [Redmine #515](http://eryjus.ddns.net:3000/issues/515) to address this within this version.

---

Looks like the second core did not start:

```
(0) [0x00000010d563] 0008:ffff800000001563 (unk. ctxt): cmp rax, 0x0000000000000001 ; 4883f801
(1) [0x00000009f02c] 9f00:002c (unk. ctxt): jmp .-3 (0x0009f02b)      ; ebfd
(2) [0x00000009f02c] 9f00:002c (unk. ctxt): jmp .-3 (0x0009f02b)      ; ebfd
(3) [0x00000009f02c] 9f00:002c (unk. ctxt): jmp .-3 (0x0009f02b)      ; ebfd
```

This is Bochs, so: `apic=xapic`.

The first order of business here is to confirm that the INIT is being executed properly.  I forgot to increase my internal function count!

But still:

```
00197099078d[APIC0 ] LAPIC write 0x01000000 to register 0x0310
...
00197099083d[APIC0 ] LAPIC write 0x0000cd00 to register 0x0300
00197099083d[APIC0 ] comparing MDA 01 to my LDR 00 -> Not a match
00197099083d[APIC0 ] An IPI wasn't accepted, raise APIC_ERR_TX_ACCEPT_ERR
```

Time for a bit compare.  OK, the INIT IPI is working (I think).  I am going to comment out the SIPI to see if I can get a clean boot.

This is positive:

```
00197099808i[APIC1 ] Deliver Start Up IPI
00197099808i[CPU1  ] CPU 1 started up at 0800:00000000 by APIC
```

I do not like the address....  Actually, in real mode this does not matter much.

I was able to confirm I am actually getting the CPU started.  That's a step.  But the real mode code is bad.

---

I had some addressing issues to sort out.  I missed a couple of adjustments.  With those taken care of, I can confirm I am in long mode:

```
00197118599i[CPU1  ] CPU is in long mode (active)
00197118599i[CPU1  ] CS.mode = 64 bit
00197118599i[CPU1  ] SS.mode = 64 bit
00197118599i[CPU1  ] EFER   = 0x00000500
00197118599i[CPU1  ] | RAX=0000000000000010  RBX=00000000e0000011
00197118599i[CPU1  ] | RCX=00000000c0000080  RDX=0000000000000000
00197118599i[CPU1  ] | RSP=0000000000009000  RBP=0000000000000000
00197118599i[CPU1  ] | RSI=0000000000000000  RDI=0000000000000000
00197118599i[CPU1  ] |  R8=0000000000000000   R9=0000000000000000
00197118599i[CPU1  ] | R10=0000000000000000  R11=0000000000000000
00197118599i[CPU1  ] | R12=0000000000000000  R13=0000000000000000
00197118599i[CPU1  ] | R14=0000000000000000  R15=0000000000000000
00197118599i[CPU1  ] | IOPL=0 id vip vif ac vm rf nt of df if tf SF zf af PF cf
00197118599i[CPU1  ] | SEG sltr(index|ti|rpl)     base    limit G D
00197118599i[CPU1  ] |  CS:0008( 0001| 0|  0) 00000000 00000fff 1 0
00197118599i[CPU1  ] |  DS:0010( 0002| 0|  0) 00000000 00000fff 1 0
00197118599i[CPU1  ] |  SS:0010( 0002| 0|  0) 00000000 00000fff 1 0
00197118599i[CPU1  ] |  ES:0010( 0002| 0|  0) 00000000 00000fff 1 0
00197118599i[CPU1  ] |  FS:0010( 0002| 0|  0) 00000000 00000fff 1 0
00197118599i[CPU1  ] |  GS:0010( 0002| 0|  0) 00000000 00000fff 1 0
00197118599i[CPU1  ] |  MSR_FS_BASE:0000000000000000
00197118599i[CPU1  ] |  MSR_GS_BASE:0000000000000000
00197118599i[CPU1  ] | RIP=000000000000809f (000000000000809f)
00197118599i[CPU1  ] | CR0=0xe0000011 CR2=0x0000000000000000
00197118599i[CPU1  ] | CR3=0x0000000001004000 CR4=0x000000a0
```

Next, I take care of jumping to a new `lInitAp()` function (to write) and get a proper kernel stack set up.


---

### 2021-Oct-24

It dawned on my last night that I might be able to allocate a stack and provide a target kernel address prior to starting each CPU and the jump directly from the trampoline code to the kernel.  Then, I also realized that there is nothing really needed in the loader that needs to be completed.  It's a completely extra step.  This also means I can move the trampoline code from the loader to the kernel (since the loader is intended to be reclaimed by the butler at the end of booting).  I also think I need to move the trampoline code a little closer to physical memory 0 (not all the way there, but closer).  I want to be able to reclaim as much of the low memory as possible.  I think `0x3000` is better than `0x8000`.  This move would lock out frames 0-3 from being allocated.  That's not a huge loss.

So, I have the data elements for the stack and entry point created (not populated -- so I expect faults).  I also was able to move the trampoline code to `0x3000`.  So, this is good so far.

Now, I need to back out the changes to the `BootInterface_t` structure and see if I can get the trampoline code relocated to the kernel rather than the loader.

That change is complete and working properly.  Now, I need to get a stack prepared for each boot.  At this point, I am triple faulting because I am jumping to address `0` with a stack `0`.  I assume that getting a proper stack mapped will handle the triple fault to a simple `#PF`.

---

I am now getting into higher level kernel code.  I am about to output data.  But when I try to get the CPU number, I triple fault.  I do not think I have the IDT installed.  Come to think of it, I do not have the proper GDT installed either.  I should be able to do both of those before I jump to the kernel.

---

I am in the kernel code now.  Before I go too much farther with this, I want to make sure I am detecting all the proper CPUs.  Then, I will circle back and complete the initialization for all the CPUs together.

Now, I have the ACPI parsing routines build from the prior kernel.  I should be able to copy those functions over into the current loader -- yeah, I need to scrape this data in the loader and not the kernel.  Several memory locations will be mapped and then unmapped when completed and I expect to be able to reclaim this memory later.

Problem #1: this code has a hard-coded location for the EBDA.  I need to figure out it that is correct.  Actually, it does appear to be correct.  But I need to set that in the `constants` file.

---

That mess has been ported over.  It is not fully documented, though.  That needs to happen before I can move forward.  I think I better do that sooner than later.


---

### 2021-Oct-25

Documentation today....

OK, I was able to get the `acpi.cc` file documented properly.  So, now I have all 3 CPUs starting up and confirming they are all started.

I think the next best thing is going to be to set up the `Process_t` for the kInitAp(cpu#) and get that added into the Global Process List.  I did manage to get 4 idle processes started without intervention:

```
List All Known Processes:
+---------------------------+----------+----------+----------+------------------+------------------+------------------+
| Command                   | PID      | Priority | Status   | Proc Address     | Time Used        | Top of Stack     |
+---------------------------+----------+----------+----------+------------------+------------------+------------------+
| kInit                     |        0 | IDLE     | READY    | ffff900000000018 | 0000800001c8c222 | fffff80000003e00 |
| Idle Process              |        1 | IDLE     | READY    | ffff900000000110 | 0000000000000000 | ffffaf0000003f70 |
| Idle Process              |        2 | IDLE     | READY    | ffff900000000358 | 0000000000000000 | ffffaf0000007f70 |
| Idle Process              |        3 | IDLE     | READY    | ffff900000000450 | 0000000000000000 | ffffaf000000bf70 |
| Idle Process              |        4 | IDLE     | READY    | ffff900000000548 | 0000000000000000 | ffffaf000000ff70 |
| PMM Cleaner               |        5 | LOW      | READY    | ffff900000000880 | 0000000000000000 | ffffaf0000023f70 |
| Debugger                  |        6 | OS       | RUNNING  | ffff900000000978 | 0000000000000000 | ffffaf0000027f70 |
+---------------------------+----------+----------+----------+------------------+------------------+------------------+
```

I should probably check to make sure I do not exceed the CPU counts expected.  Done.

Oh, and I need to set `GS` when I establish these processes.

---

I now have a `#PF` to sort out.  The problem is the CPU dump is not locking the serial port.  So, I am not getting anything I can read.

I was not properly loading `gs` for the 64-bit arch.  But there is still cleanup to happen there.

Now, I need to get the timer firing for these CPUs.  I think I need to init the LAPIC again for these CPUs.  I am not sure what I will get if I just enable interrupts.  I tried it just for fun and it did not work.

After getting the timer init function called for each core, I think the other CPUs are running and scheduling.  However, I am getting a *ton* of mapping/unmapping messages.  I need to turn those off in `debug`.

And I *do* have all the CPUs processing and swapping data!  Woohoo!

```
List All Known Processes:
+---------------------------+----------+----------+----------+------------------+------------------+------------------+
| Command                   | PID      | Priority | Status   | Proc Address     | Time Used        | Top of Stack     |
+---------------------------+----------+----------+----------+------------------+------------------+------------------+
| kInit                     |        0 | LOW      | RUNNING  | ffff900000000018 | 0000000000000000 | 0000000000000000 |
| Idle Process              |        1 | IDLE     | RUNNING  | ffff900000000110 | 000000004f927bc0 | ffffaf0000003df0 |
| Idle Process              |        2 | IDLE     | READY    | ffff900000000358 | 0000000049ca6180 | ffffaf0000007df0 |
| Idle Process              |        3 | IDLE     | READY    | ffff900000000450 | 000000004ecc1e80 | ffffaf000000bdf0 |
| Idle Process              |        4 | IDLE     | READY    | ffff900000000548 | 000000004ffd4b80 | ffffaf000000fdf0 |
| kInitAp(2)                |        5 | LOW      | TERM     | ffff900000000738 | 0000000000000000 | ffffaf0000017ed8 |
| kInitAp(1)                |        6 | LOW      | TERM     | ffff900000000640 | 0000000000000000 | ffffaf0000013ed8 |
| kInitAp(3)                |        7 | LOW      | TERM     | ffff900000000830 | 0000000000000000 | ffffaf000001bed8 |
| PMM Cleaner               |        8 | LOW      | RUNNING  | ffff900000000b68 | 0000000000000000 | ffffaf0000023f70 |
| Debugger                  |        9 | OS       | RUNNING  | ffff900000000c60 | 0000000000000000 | ffffaf0000027f70 |
+---------------------------+----------+----------+----------+------------------+------------------+------------------+
sched:list :>
 (allowed: exit, all)
```

Notice that there are 3 processes terminated.  This really does meet the goal for this version.  But, there are several other things I wanted to get done in parallel.

Well it ran for about 30 minutes and then something `#PF`d.  I think it was the PMM since it was the only thing that was running.  If so, it would have been in the cleaner process (confirmed the `rip` matches anyway).  The cleaner may have run out of things to do and did not recognize it properly.  I'm wondering if I shouldn't terminate the running process from the kernel when a fault like that happens....


---

### 2021-Oct-27

Here are the details of the PMM crash:

```
List All Known Processes:
+---------------------------+----------+----------+----------+------------------+------------------+------------------+
| Command                   | PID      | Priority | Status   | Proc Address     | Time Used        | Top of Stack     |
+---------------------------+----------+----------+----------+------------------+------------------+------------------+
| kInit                     |        0 | LOW      | RUNNING  | ffff900000000018 | 0000000000000000 | 0000000000000000 |
| Idle Process              |        1 | IDLE     | READY    | ffff900000000110 | 0000000725be4e80 | ffffaf0000003df0 |
| Idle Process              |        2 | IDLE     | READY    | ffff900000000358 | 0000000728a9fcc0 | ffffaf0000007df0 |
| Idle Process              |        3 | IDLE     | READY    | ffff900000000450 | 00000007328ef240 | ffffaf000000bdf0 |
| Idle Process              |        4 | IDLE     | RUNNING  | ffff900000000548 | 00000007299e20c0 | ffffaf000000fdf0 |
| kInitAp(1)                |        5 | LOW      | TERM     | ffff900000000640 | 0000000000000000 | ffffaf0000013ed8 |
| kInitAp(2)                |        6 | LOW      | TERM     | ffff900000000738 | 0000000000000000 | ffffaf0000017ed8 |
| kInitAp(3)                |        7 | LOW      | TERM     | ffff900000000830 | 0000000000000000 | ffffaf000001bed8 |
| PMM Cleaner               |        8 | LOW      | RUNNING  | ffff900000000b68 | 0000000000000000 | ffffaf0000023f70 |
| Debugger                  |        9 | OS       | RUNNING  | ffff900000000c60 | 0000000000000000 | ffffaf0000027f70 |
+---------------------------+----------+----------+----------+------------------+------------------+------------------+
sched:list :> An exception has occurred
An error (interrupt 0xe) has occurred (Error Code 0x0000000000000000)
  RAX: 0xffffa00000004000       R8 : 0x0000000000000000
  RBX: 0xffffa00000004030       R9 : 0x0000000000000000
  RCX: 0x0000000000000001       R10: 0x0000000000000000
  RDX: 0x0000000000001199       R11: 0x0000000000000000
  RBP: 0xffffa00000004038       R12: 0xffffa00000001fbe
  RSI: 0xffffff0000003000       R13: 0xffffff0000005000
  RDI: 0x0000000000000010       R14: 0xffffff0000005038
                                R15: 0xffffa00000004078
   SS: 0x10     RSP: 0xffffaf0000023fa0
   CS: 0x8      RIP: 0xffffa00000001d8a
   DS: 0x28      ES: 0x28
   FS: 0x0       GS: 0x78       RFLAGS: 0x0000000000000286

CR0: 0x00000000e0000011
CR2: 0xffffff0000003000
CR3: 0x0000000001092000
CR4: 0x00000000000000a0
```

Also, I have added a number of Redmine issues to be resolved with this version before it gets wrapped up.

The code fix for the PMM did not work, so I am going to have to get some debugging code added.  In the meantime, I have several other bits of code written I need to test.

---

This is an odd one since the page it not mapped (per `MmuDump()`) but `MmuIsMapped()` reports it is.  I'm trying Bochs and its instrumentation to see if it will shed some light.

---

Well, it turns out the `MmuIsMapped()` function was setting `%al` and the `PmmCleaner()` was testing `rax`.  This would not do, so I changed the return type from `bool` to `Return_t`.  A Redmine was also entered to check and address anything remaining.  That was fun to figure out.  I wonder if it is a bug in the compiler....  But I am not going to research it.

I have 2 debugger modules to write and a module to document before I can wrap this version up.


---

### 2021-Oct-28

There are only 2 tasks left to complete for the goals for this version:
1. Create IPI handlers and use them to lock the cpus during debugging tasks
2. Establish (really copy) the CPU status from the prior century version

I want to take on the IPI first because that is going to require some conversation with myself.

The previous version wrote to some common memory for the debugger, setting the number of cores engaged to 1.  Then sent an IPI to the other CPUs and waited for all the cores to report in (by incrementing the number of cores engaged) until the count matched the number of cores running.  It held there until the count in that memory location was reset to 0.  All of that seems reasonable to implement as a synchronization boundary.  I can control the memory in the debugger module and have it accessible from the IPI; I can set up the IPI to trigger into the debugger module; what I cannot do at this point is query how many CPUs are running.

I can either expose that as a kernel call or I can assume for now that they are all running.  Since I do not have anything prepared for this, I am going to use MAX_CPU and assume that they are all running all the time.

Now, if I work on a system that has only 2 CPUs, this will not work properly.  So, I need to do this right, meaning I need to keep track of the number of active CPUs.  This will need to be in the `cpus` structure.

I was able to knock out the code quickly, but the system deadlocks.  I'm too tired to search for it tonight.


---

### 2021-Oct-29

Ah-ha!!  The IPI interrupt is actually locking on a stack spinlock.  Only 1 CPU gets the stack and blocks properly.  The other 2 are waiting for the stack to be released, which will never happen.

I need to figure out how to not have that lock for some things and actually use the process stack.  I also cannot use the current stack since the address space changes.  I need to think this through a little bit.

The big problem here is that all the cores will be trying to execute the same code at the same time in a different address space than is currently running and potentially using the same stack.  Yup, that's a problem.  The easiest solution is to move the IPI handlers to the kernel.

I will need an IPI to flush the TLB for an address.  I expect that most other reasons will be from the kernel as well (panic!).  It might be worthwhile to move the IPI to pause the CPU to the kernel as well.  In fact, a panic can use the same feature without actually releasing the CPUs.

I think that will be the plan tomorrow.


---

### 2021-Oct-30

I realized as I started looking at the code, I had to make the same move (to the kernel) for the timer interrupt.  There is an inherent flaw in my design but I cannot put my finger on it.  Well, I can the flaw, not the solution.  That said, my kernel is only 589K at the moment.  When I compare that to the previous version (which is about equivalent in functionality at the moment, but a monolithic binary) which was 1.2M, I am unconcerned so far.

The last thing to wrap up in this version is the CPU debugger module, which will be leveraged from the 32-bit version.

---

This can together quick and wraps up this version.


