# The Century OS -- v0.0.13

This project is a 64-bit focused version of Century-OS.  This version takes from the 32-bit version of Century-OS, but is better streamlined for the 64-bit CPUs (where any 32-bit archs will be added in later if desired).


---

## Version 0.0.13 -- General Stabilization Version

This version is specifically to address a number of outstanding issues which have been punted to later.  Well, it's later.  It's time to clean them up.

There are no new features in this version.


### 2021-Oct-30

As I get into this version, I have a feeling I am going to find things are actually working better than I thought they were.  Some things do not yet have a driving need for them, so I may kick cans down the road for things that I am not sure will be used in the immediate future.  If things get kicked enough for non-use, I may just park them for another time.

As I start with the issues, there are 16 items on the list.  I have wondering the best way to approach this: pick them and work them easiest to hardest?  Or work on them in sequence?  Also, should I create micro-versions for each item (committing them as I go)?

Well, several of these "fixes" are going to be large in nature.  I am going to create micro-versions for each.  I will also take them in order -- I know as I get to the end I will want to punt the harder ones down the road.  I would prefer to wrap up as many of these a I can now.  I am actually really excited to get into messaging, so I need to temper that excitement a bit so I can actually accomplish the goals for this version.


## Version 0.0.13a -- [Redmine #486](http://eryjus.ddns.net:3000/issues/486)

This Redmine was entered about 8 months ago, and I do not have much documented about it.  It may have been cleaned up already.  Let's trace this through a bit.

The actual allocation function is `pmm_PmmAllocateAligned()`.  That function has the following condition:

```c++
    if (count == 1 && bitsAligned == 12) {
        Frame_t rv = PmmAllocate(low);
```

... which clearly passes the low flag on to allocate a single frame.  It also has the following block for aligned blocks of frames:

```c++
    if (low) {
        SpinLock(&pmm.lowLock);
        rv = PmmDoAllocAlignedFrames(&pmm.lowStack, count, bitsAligned);
        SpinUnlock(&pmm.lowLock);
    } else {
        SpinLock(&pmm.normLock);
        rv = PmmDoAllocAlignedFrames(&pmm.normStack, count, bitsAligned);
        SpinUnlock(&pmm.normLock);
    }
```

This is also properly allocating from the *low* and *normal* stacks.  The only concern I may have here is that this block does not consider the *scrub* stack before it returns that no memory is available.  However, it is also very difficult to assemble a block from the scrub stack, so that is not really feasible.

So, for `PmmAllocate()`....  It is also properly considering the *normal* and *scrub* stack as long as the memory requested is not low.  If the other 2 stacks fail to allocate memory, the *low* stack is considered for all allocations.  This is also correct.

In fact this was addressed in code committed on 3-May (v0.0.7).

This is why (at least in part) I want to take care of these stabilization versions once in a while.


---

## Version 0.0.13b -- [Redmine #480](http://eryjus.ddns.net:3000/issues/480)

This one should have been addressed prior to the other one if I was following my rules.  I just missed the order they were presented in was not numerical by issue number.

There are a number of things that are hard coded in the loader.  I really want to identify them and pull them into the `constants` files where appropriate.

As I am going through this Redmine, I realize that the `wbnoinvd` instructions need to be moved to the `cpu.h` file as an inline (or #define as appropriate).  http://eryjus.ddns.net:3000/issues/542 was created for this and added to this version.

While I was at it, I also cleaned up `mmu-loader.cc`.

I should also take care of the kernel `mmu.cc` updates.

That done, I think this takes care of #480.


---

## Version 0.0.13c -- [Redmine #488](http://eryjus.ddns.net:3000/issues/488)

This Redmine asks to address the PMM aligned allocation.  `PmmDoAllocAlignedFrames()` is already coded and searches for a block big enough to hold the alignment and the number of frames requested.  There is also a function `PmmSplitBlock()` to split a block of frames into 3: the block before the aligned frames, the properly aligned frames, and the block after the aligned frames.

This is all already integrated into `pmm_PmmAllocateAligned()`.  So this is actually complete.


---

## Version 0.0.13d -- [Redmine #492](http://eryjus.ddns.net:3000/issues/492)

This one is still a problem and easy to demonstrate:

```
List All Known Processes:
+---------------------------+----------+----------+----------+------------------+------------------+------------------+
| Command                   | PID      | Priority | Status   | Proc Address     | Time Used        | Top of Stack     |
+---------------------------+----------+----------+----------+------------------+------------------+------------------+
| Butler                    |        0 | LOW      | MSGW     | ffff900000000018 | 00008000004b3872 | fffff80000003f30 |
| Idle Process              |        1 | IDLE     | READY    | ffff900000000110 | 000000004b295540 | ffffaf0000003df0 |
| Idle Process              |        2 | IDLE     | READY    | ffff900000000358 | 0000000047c39500 | ffffaf0000007df0 |
| Idle Process              |        3 | IDLE     | RUNNING  | ffff900000000450 | 000000004d3f6400 | ffffaf000000bdf0 |
| Idle Process              |        4 | IDLE     | RUNNING  | ffff900000000548 | 000000004a817c80 | ffffaf000000fdf0 |
| kInitAp(1)                |        5 | LOW      | TERM     | ffff900000000640 | 0000000000000000 | ffffaf0000013ed8 |
| kInitAp(2)                |        6 | LOW      | TERM     | ffff900000000738 | 0000000000000000 | ffffaf0000017ed8 |
| kInitAp(3)                |        7 | LOW      | TERM     | ffff900000000830 | 0000000000000000 | ffffaf000001bed8 |
| PMM Cleaner               |        8 | LOW      | RUNNING  | ffff900000000b68 | 0000000000000000 | ffffaf0000023f70 |
| Debugger                  |        9 | OS       | RUNNING  | ffff900000000e58 | 0000000000000000 | ffffaf0000027f70 |
+---------------------------+----------+----------+----------+------------------+------------------+------------------+
sched:list :>
 (allowed: exit, all)
```

Notice that the 3 `kInitAp(x)` processes are terminated with no time used.  Obviously this is still wrong.  I have some real debugging to do.

Every time a schedule change is considered (not just performed), the time used should be updated.  On top of that, the number of microseconds does not appear to be correct.

Turns out the time used was only being updated for 1 of the 3 possible conditions.  I have moved the update outside the condition checks.

Now, with the CPU running for only about a minute, the PMM Cleaner shows having been running for 2227000000 micro-seconds, or 2227 seconds, or 37+ minutes.  Not correct.

I made changes to make sure the last timer count was updated before turning on interrupts.  That still did not materially change the counts.

I believe it is time to create a timer module to show the timer counter.

I tried to run this on Bochs, and it deadlocks getting into the IPI.  I need to try this on real hardware.  Same result as Bochs.  This will need to be added to Redmine and corrected within this micro-version.  See http://eryjus.ddns.net:3000/issues/543.

This seems relevant:

```
00205823238i[CPU0  ] WARNING: HLT instruction with IF=0!
00205823438i[CPU1  ] WARNING: HLT instruction with IF=0!
00205825173i[CPU3  ] WARNING: HLT instruction with IF=0!
```

It was.  That corrected, I have 13 minutes showing up in Bochs after a few seconds of execution.


---

### 2021-Oct-31

It turns out with every timer tick (every 1 ms), I was incrementing the timer by 1 second.  Off by a factor of 1000.

QEMU is not keeping time well.  Bochs is closer but still not accurate.  I am still deadlocking on real hardware (not sure what happened here -- forgot to test maybe?).

Hmmm....  the cpu module is not available to the debugger on hardware.  This is one of those shitty cases where it works properly on the emulators but not on real hardware.  My real hardware only has 2 CPUs.  Maybe I can cut an emulator down to 2 for a test.

I can duplicate this on Bochs with only 2 CPUs.

---

Hmmm... CPU1 is deadlocked on a spinlock...  but why?  Not sure the IPI is even being delivered.

---

OK, Bochs is working now.  No changes made except for turning on instrumentation.  Race condition?

Well, this is not right:

```
List All Known Processes:
+---------------------------+----------+----------+----------+------------------+------------------+------------------+
| Command                   | PID      | Priority | Status   | Proc Address     | Time Used        | Top of Stack     |
+---------------------------+----------+----------+----------+------------------+------------------+------------------+
| Butler                    |        0 | LOW      | MSGW     | ffff900000000018 | ffff800000030bd6 | fffff80000003f30 |
| Idle Process              |        1 | IDLE     | READY    | ffff900000000110 | 0000000000003a98 | ffffaf0000003df0 |
| Idle Process              |        2 | IDLE     | READY    | ffff900000000358 | 00000000000036b0 | ffffaf0000007df0 |
| kInitAp(1)                |        3 | LOW      | TERM     | ffff900000000450 | 0000000000001388 | ffffaf000000bed8 |
| PMM Cleaner               |        4 | LOW      | RUNNING  | ffff900000000788 | 0000000006a35f10 | ffffaf0000013db0 |
| Debugger                  |        5 | OS       | RUNNING  | ffff900000000c70 | 0000000006a22690 | ffffaf0000017f70 |
+---------------------------+----------+----------+----------+------------------+------------------+------------------+
sched:list :> x
- :> scheduler
scheduler :> s
<pid> :> 0
Dumping the contents of the Process_t structure for pid 0 at ffff900000000018
+-------------------------+----------------------------+
| TOS (last preemption)   | fffff80000003f30           |
| Virtual Address Space   | 0000000001004000           |
| Process Status          |   7: MSGW                  |
| Process Priority        |   5: LOW                   |
| Quantum left this slice | 21                         |
| Process ID              | 0                          |
| Command Line            | Butler                     |
| Micros used             | 18446603336221395926           |
| Wake tick number        | 0                          |
| Pending Error Number    | 0                          |
+-------------------------+----------------------------+
scheduler :> [adam@adamlt2 ~]$
 (allowed: list, show, exit)
```

The Butler has an address in it for time used and the process dump is not clearing the screen.  Also, should blocking zero out the quantum?


---

### 2021-Nov-01

Let's start with the blocking question.  The real answer here is, "yes."  It should be a quick change.

Ahhh...  and the kInit timer is a timing issue (no pun intended).  I am calling that in `ProcessInit()` before the LAPIC timer has been initialized.

OK, let me try on real hardware again....  Nope, still deadlocking.  The problem is going to be how to determine why it is deadlocking.

Well, I tried Redmine #546 which did not work.

So, what do I need to determine what the problem is?  These will need to be written into the debugger to get information.
* Add a 10s pause into the start of the debugger -- to allow the rest of the boot to complete and make the greeting cleaner
* I believe the IPI is being delivered -- read and show the contents of the ESR; may need to sleep to allow it to be read
* There may also be a possibility that interrupts are disabled for some reason; figure out how to retrieve and show the flags register for each core (maybe all registers if I am feeling ambitious)

```
Welcome to the Century-OS kernel debugger
- :> scheduler
scheduler :> list
sched:list :> a
ESR Result: 0x0000000000000000
```

This real hardware result matches QEMU.  This also matches the expectation from the Intel manual.

I do have a register dump coded.  But, it will need to be checked/debugged.


---

### 2021-Nov-04

Well, I have some travel coming up.  I am not going to be able to take all my hardware with me for testing on this trip.  So, I will need to move on from this problem and come back to it.

Right now, I am not certain how to even debug this -- it feels like a spinlock on a stack but that should not happen (I guess I should double check from Bochs).  However, it will probably have to wait until I am on a plane and without my hardware.

In the meantime, I will also export a list of issues and try to work on those in turn on the plane.  I may punt a couple of issues from this version into the next and work on getting a `libc` started, trading some things from one version with some things from the next version.


---

### 2021-Nov-05

I am going to continue with the existing micro-version (v0.0.13d).  When I get the next thing resolved, I will commit this partially completed work.  If I am able to incidentally identify the problem, I will be able to confirm it when my travel is done.

---

Without reloading the thumb drive, I was able to get a boot.  Booting again results in a successful boot.  Third time is the same.  I'm going to commit this code and call it done.  It may  not be perfect, but it seems to be somewhat sorted and I want a fall-back position.


---

## Version 0.0.13e -- [Redmine #493](http://eryjus.ddns.net:3000/issues/493)

This Redmine issue is to address adding the memory space into the Process Dump.  This will be included in the command sequence `scheduler list all`.

This change will add an additional column to the process listing.

Hmmm....  QEMU just deadlocked (micro-version `d`).  I think I have a race condition here.


---

## Version 0.0.13f -- [Redmine #503](http://eryjus.ddns.net:3000/issues/503)

This Redmine is to replace all the `return 0` with `return SUCCESS`.  However, that is really not part of the POSIX specification and of the code I have reviewed the common return result is 0.  I am going to reject that change, but instead clean up all the `return SUCCESS` lines.

There was only 1 instance.

At the same time, I am going to look at [Redmine #505](http://eryjus.ddns.net:3000/issues/505).  This Redmine is to remove all instances of `GetCr3()` and should be replaced with `GetAddressSpace()` as a more arch-agnostic function name.

I did see an assert failure....   Race?


---

### 2021-Nov-06

Time to commit this version.
---

## Version 0.0.13g -- [Redmine #509](http://eryjus.ddns.net:3000/issues/509)

This version is a significant cleanup effort.  It will redo the internal function calls such that the extra parameter (function number) is not passed all the way to the actual function.

---

Well, I broke `KernelPrintf()`....  For some reason it is not printing anything because the service address is 0.

I broke `MmuMapPage()` as well.  So, I think I broke it all.

```
MmuDump: Walking the page tables for address 0xffff80000001c010
Level  Entry Address       Index               Next Frame          us  rw  p
-----  ------------------  ------------------  ------------------  --  --  -
PML4   0xfffffffffffff800  0x0000000000000100  0x0000000000001009  0   1   1
PDPT   0xfffffffffff00000  0x0000000000000000  0x0000000000001063  0   1   1
  PD   0xffffffffe0000000  0x0000000000000000  0x0000000000001064  0   1   1
  PT   0xffffffc0000000e0  0x000000000000001c  0x0000000000000001  0   0   1
```

This is not right.  Nothing should be actively mapped to frame 1.  Hmmmm...  but if I am overwriting the internal jump table with a mapping to frame 1, then the unmap will not properly find the correct function (likely 0).  This would leave the wrong address mapped to frame 1.

I found a place I was overwriting `rdi` with the contents of the function to call.  I know I had a reason for that at one point but it bit me in the rear-side.  Now I have a triple fault.  I'm sure I missed something that needs to be cleaned up.  I may have to do an exhaustive audit of every function.

I'm also going to have to look at the Interrupt handler as this appears to need this function number.  I may have to split it from CommonHandler.

First an audit.  All good.  Next is the timer firing and causing problems?  Need to stop before that point.

Nope, not getting there.  Still not getting through `ProcessInit()`.

Looks like I am having a problem getting through the `ProcessCreate()` for the Idle processes for each CPU.  A quick comment confirms.

This leads me into `ProcessNewStack()`.

This leads me to `MmuMapPageEx()`.

---

Looks like the problem is might be related to changing tasks.  It's going to be rough to debug this.  I may need to turn off the other CPUs to get to the bottom of it.

Yes, there is something going on with the scheduler:

```
List All Known Processes:
+---------------------------+----------+----------+----------+------------------+------------------+------------------+------------------+
| Command                   | PID      | Priority | Status   | Proc Address     | Time Used        | Top of Stack     | Address Space    |
+---------------------------+----------+----------+----------+------------------+------------------+------------------+------------------+
| Butler                    |        0 | LOW      | MSGW     | ffff900000000018 | 0000000000001388 | fffff80000003f30 | 0000000001004000 |
| Idle Process              |        1 | IDLE     | READY    | ffff900000000110 | 0000000000000000 | ffffaf0000003f70 | 0000000001004000 |
| Idle Process              |        2 | IDLE     | READY    | ffff900000000358 | 0000000000000000 | ffffaf0000007f70 | 0000000001004000 |
| Idle Process              |        3 | IDLE     | READY    | ffff900000000450 | 0000000000000000 | ffffaf000000bf70 | 0000000001004000 |
| Idle Process              |        4 | IDLE     | READY    | ffff900000000548 | 0000000000000000 | ffffaf000000ff70 | 0000000001004000 |
| PMM Cleaner               |        5 | LOW      | RUNNING  | ffff900000000a78 | 0000000000077a10 | ffffaf000001bdb0 | 0000000001092000 |
| Debugger                  |        6 | OS       | RUNNING  | ffff900000000f60 | 00000000002287d8 | ffffaf000001fd70 | 000000023ffe3000 |
+---------------------------+----------+----------+----------+------------------+------------------+------------------+------------------+
sched:list :>
 (allowed: exit, all)
```

There should not be 2 processes in the `RUNNING` status with this test -- only 1 CPU is started.

There was still a call to `sch_ProcesReady()` which being called from asm that had the wrong register being populated.  Now it appears to be working properly.

I'm also hoping I found the race and have that addressed -- I was starting the debug process before all the debug modules were registered.

I think I can commit this micro-version.  Once my current test is complete.


---

## Version 0.0.13h -- [Redmine #513](http://eryjus.ddns.net:3000/issues/513)

This Redmine is concerned with the quality of the code that has been moved to the `common` pseudo-module.  It all needs to be reviewed and cleaned up.

`elf.cc` looks rather clean.

`serial.cc`, on the other hand, has some opportunity.  There is some arch-specific functions and some common functions.  I need to clean that up a bit.

While I am at it, I need to properly document these files.  `elf.cc` has some documentation issues.

Now, do I want to move `serial.h` to the common folder?  Same for `elf-func.h`.  I should as it's appropriate.  I also will need to rename `elf-func.h` to `elf.h` since it is appropriate as well.

This is all done.  Ready to commit again.


---

### 2021-Nov-07

On to the next Redmine:

---

## Version 0.0.13i -- [Redmine #514](http://eryjus.ddns.net:3000/issues/514)

This Redmine is concerned with consolidating the mmu components into a single source.  The code should be relatively common except for the debugging code.

I have an assert failure:

```
Starting new process with address space 0x000000023fff4000

!!! ASSERT FAILURE !!!
/home/adam/workspace/centuryos2/modules/kernel/src/scheduler.cc(433) AtomicRead(&scheduler.schedulerLockCount) == 0 `ProcessStart()` still has a scheduler lock remaining
```

Could this be one of the races?  Since it went away on the next run, I'm thinking yes.

I think I will need to hold the scheduler locked until everything is ready.  For this, I think I can use a simple `AtomicInt_t`.

---

I believe that solved my race condition as well.

I am going to commit.  Nope!!

I am going to document all the new sources I created.

---

OK, the MMU sources have been documented.  Time for a commit.


---

## Version 0.0.13j -- [Redmine #517](http://eryjus.ddns.net:3000/issues/517)

This Redmine is concerned with addressing the code duplication in the LAPIC sources.  Several things are duplicated between the xAPIC and the x2APIC sources.

This should be fun because I have multiple implementations of the same function names.

---

Looks like I still have a problem with the scheduler.

Ok, I have the LAPIC code sorted out.  Most of the duplication has been removed.  However, the `...EarlyInit()` functions still have a lot of duplication.  For the moment, I am going to leave that alone.  There are enough differences sprinkled around the code I'm not sure I need to add all the conditionals.  It may be worth it, but I am going to sleep on that first.  In the meantime, I still have a lot of documentation to complete.

But that will be tomorrow as well.


---

### 2021-Nov-08

Debugging....

Got it.  The LAPIC module was no longer being loaded.  That is fixed.  Now for some documentation.

Looks like I broke it again.


---

### 2021-Nov-10

Documenting....


---

### 2021-Nov-13

Completing the documentation.


---

## Version 0.0.13k -- [Redmine #518](http://eryjus.ddns.net:3000/issues/518)

This Redmine is concerned with addressing the situation where the loader has not pages than is accounted for.  The loader will now dynamically map the proper number of pages accounting for the size of the loader.


---

### 2021-Nov-14

That issue is complete.  I am also going to take care of #520 at the same time.  This Redmine is concerned with making certain that the name of a process does not overwrite the bounds of the buffer -- just a little bit of defensive programming.

That change is also complete.


---

## Version 0.0.13l -- [Redmine #521](http://eryjus.ddns.net:3000/issues/521)

This Redmine is concerned with addressing the arch-specific code that is located in the `SchedulerCreateKInitAp()` function.

That was a quick fix.  Next will be #525, which is concerned with how the PMM scrubs its blocks -- making sure that it is completed in small blocks, not all at once.

This is done too....  The thing now is that the PMM leaves those blocks fragmented.  Therefore, when I get into allocating contiguous blocks, it will be harder to do that.  I am not going to work on that now and will wait until I have a real need to worry about it.  I can adjust the fragmentation with `SCRUB_LIMIT`.

The last 2 Redmines (#541 and #542) are some general cleanup tasks.  I will take those on here as well, hopefully wrapping up this version.

OK, those 2 items are also cleaned up, which completes the goals for this version.  I do still have a race condition to figure out.


