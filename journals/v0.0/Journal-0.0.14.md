# The Century OS -- v0.0.14

This project is a 64-bit focused version of Century-OS.  This version takes from the 32-bit version of Century-OS, but is better streamlined for the 64-bit CPUs (where any 32-bit archs will be added in later if desired).


---

## Version 0.0.14 -- Implement messaging

This version is focused on messaging and completing its implementation in the kernel as well as starting a `libc` library specific to CenturyOS.

This is an important point that bears repeating: I have no interest in creating a generic C library that will work for all cases.  Therefore, I am not planning on porting anything existing (except maybe lifting some functions as required).  This is built for CenturyOS and will be used only on CenturyOS.


---

### 2021-Nov-14

This version is exciting for me.  It is with this version that I start to get into the actual OS services and the POSIX compliance.

The first thing to start with is the build directories, starting with the `libc` module.

Then we need to get into the rewrite of the `errno.h` file -- which is going to be tedious and will result in several days of work.

---

`errno.h` is redone.  The old one had lots of Linux extensions.  I made the change.  At the same time, I also created an `errno.inc` generated from the contents of `errno.h`.  This then allows me to use the same constants in `nasm`.  Again, CenturyOS-specific.


---

### 2021-Nov-16

I have the headers all done.  This includes the Doxygen documentation, which was lifted from the POSIX specification.

I think the next thing to do here is going to be to add the kernel message queue implementation.  I know I had a lot of this done in the old kernel -- it was working at least partially when I stopped working on that.  I may pick up a few things, but I do not think I am going to copy it all over blindly.  There were some parts I did not completely like.

I am going to need an internal kernel structure for a Message Queue itself.  I will need another structure for a Message as well.  There there will need to be a list on each Message Queue for processes waiting for a message -- and another list for processes waiting for room to become available to send a message.

The problem here is going to be calling module-to-module and sending a message: I will need to copy the message into kernel space and then back into the message module to send a message; and then copy it twice again on receive.  I think that is the price I need to pay for having a micro-kernel.

The other thing I need to consider is that I need some way to keep the different sets of queues separated.  Theoretically, one set of queues owned by one process cannot cross over into queues owned by another process.  They key to success here is going to be the key used -- which I cannot implement at this point since I do not have a file system I can `stat` a file or directory on.  What I have going for me is that I am implementing `key_t` as 64-bits, giving me the ability to use more unique data elements to construct this key.

Now, a `Message_t` structure.  This structure needs the following elements:
* List (`ListHead_t::List_t` type)
* Message Size (`size_t` type)
* Message Type (`long` type)
* Message Text (`void *` type or `char [1]` type)

Let me compare that to the old kernel....  The old kernel had the payload as an embedded structure.  I am not going to do that here.

The actual `Message_t` structure came together quickly.  The `MessageQueue_t` structure will take a little more thought.  The old implementation did not have all the required elements at the ready and had to be calculated.  I need to rethink what I need:
* The ListHead of the queue
* The `msgqid` for this queue
* The `key` for this queue
* The current effective `ipc_perm` structure
* The number of messages on the queue (`AtomicInt_t`)
* The max number of bytes allowed on the queue (total length of Message Text -- starting at `msgmnb`; see [Redmine #554](http://eryjus.ddns.net:3000/issues/554))
* The current number of bytes on the queue
* The last send pid
* The last receive pid
* The last send time
* The last receive time
* The last change time

Finally, I need a structure to keep track of some global information about all messages.
* The max number of queues allowed; #555
* The max number of bytes per queue (starting value); #554
* The max length of messages; #553
* The current number of queues


---

### 2021-Nov-17

So, what do I work on today?  I think I need to work on the actual Message Module code to support the messages.

Oh, and I had a thought yesterday as I wandering around away from my keyboard: what if I publish Heap functions from the kernel for global memory allocations?  It sounds risky, but maybe I can make something like that work -- like mapping the memory into the necessary address spaces?  Who knows..., maybe that is what shared memory is all about and I just have not researched it yet.

So, the first thing I need to be able to do is to allocate a Message Queue.  I will start there.

---

OK, I have moved on to `msgctl()` and `IPC_RMID` and I have come across a problem.  I need to keep track of the processes waiting for something to happen and be able to release them en-masse at the correct time.  This is going to be a user-space module, so managing processes directly is out of scope for this.  Seems like I need to publish a function to release all processes on a given wait queue (as in `ListHead_t`).  For that to work, I also need to publish a function to add the current process to a list (or add the list into the `ProcessBlock` API).

Well, as I wrap up the evening, I added about a half-dozen Redmines to complete to this version.  I think I created more work today than I actually got done.


---

### 2021-Nov-18

Today I have been researching the real time clock (RTC).  This should be simple to implement -- just need to be able to convert a date/time to an number of seconds since epoch (1-Jan-1970).  Note that I am using a 64-bit type to avoid the Y2K38 "Epochalypse".

So, the first thing is to decide what module will be responsible for communicating with the CMOS to read the date/time.  I think this is risky enough (may corrupt the CMOS and then stop the ability to boot on real hardware) that I want to keep it in the kernel.  The alternative would be to add it into the LAPIC or create a new module to deal with the CMOS RTC.

I think the best thing to do is to create an `rtc` module.  But do I do that now or in another version?

In the meantime, I am going to move on to the real and effective uid and gid into each process.  Currently, these will all be uid:gid = 0:0 (or effectively root:root).  Those fields were quickly added into the `Process_t` structure.  I added inline functions to pull the current uid and current gid from the current process structure.  However, this will need to change at some point in the future -- will need some better security for a process structure.


---

### 2021-Nov-19

Today, I am going to start the process of adding a desired list to the `ProcessBlock` function.  After searching, there are no instances where that function is called from assembly language, so this should be relatively minor.

First, `sch_ProcessBlock()`....  That was easy because all functions that call `sch_ProcessBlock()` need a null parameter for the list.  Now for the public function `SchProcessBlock()`....  A good compile and a quick test show success.

---

Now to code the `msgsnd()` internal function.


---

### 2021-Nov-20

Today I completed the `MsgqSnd()` and `MsgqRcv()` functions.  I also created the `msgget()` POSIX function for `libc`.

At the moment, I am working on getting the OS Services set up for that same function.  The key here is that I need to ensure that the `types.h` used are from `sys/types.h` and not the one for the kernel when creating the interfaces for `libc`.

I think I have everything I need for the `libc` message queue stuff.  The last thing is going to be to complete the module for `msgq` and get it loaded and initialized.

---

OK, it looks like I am terminating a process without actually getting it terminated -- `ProcessEnd()` is returning.  This means I already have a postpone pending.  I can correctly that easily from by never returning from `ProcessEnd()`.  Maybe not.  I need to confirm the number of locks getting into `ProcessEnd()`.


---

### 2021-Nov-21

Actually, it was the assert that was incorrect.  I removed it.

So, now I believe I am in a position to start testing and debugging the messaging code.

---

I actually have it somewhat working!  Mostly on the first pass, too.  I just do not have much message passing happening yet.

Looks like I have the message passing working now.

At this point, the only thing left to do is to be able to get the current time from the RTC.  Oh, and complete the documentation.

OK, so the POSIX specification has a `time()` function which is in turn required to return a `time_t` value.

According to https://filippo.io/linux-syscall-table/, `time()` is actually backed by a system call for Linux.  This does make sense because the system does need to interact with CMOS IO ports to get the current time.  So, long story short, I will need a rtc module, too.

The `<time.h>` header has quite a bit in it, which will not all be implemented in this version.  The header will be completed -- the functions that back the header will not.


---

### 2021-Nov-22

I see the problem -- I am shifting the parameters from one register to another, and that is clobbering some stuff I was not expecting.  So, I either need to undo the shifting on exit or come up with some other method.  For now, I just undid the shifting.  That also may have fixed what I thought was a race condition.

So, at this point all the goals of this version have been met with the exception of this line:

> Both the PMM and the Debugger modules will remove their internal functions and replace them with message passing.

I really should take the time to get that working properly.


---

### 2021-Nov-23

For the PMM, how do I want to do messaging instead of direct calls?  There are several things I should keep in mind:
* Security -- I need to set up the PMM interface message queue such that only kernel processes can interact with it.
* Separation of concerns -- I need to separate the cleaner process from the interface process and the interface process needs to run at a higher (OS) priority.
* Maintain the proper user interface -- I really do not want to have to redo the module-facing interface API; I want to be able to keep that as-is and only change the PMM back-end.
* Should I create a separate interface for allocating and freeing PMM frames?  I do not think it is necessary.  I can use the priority of the message such that I allocate before I free.

The first thing is going to be to start with the interface process.  Let me start on getting that up and running first.

---

Hmmmm... just getting the new process started locks the debugger....

Looks like interrupts are being disabled when we try to get a message and they are never being re-enabled again.  Actually, that is not the case.

I am getting too many messages (the heap is really bad).  I need to update the debugging output controls for the heap.


---

### 2021-Nov-24

Let me start with disabling the IPI code to see what is really going on.

I was able to narrow this down to the `kInit()` job, which is being re-tasked to be the butler.  There some sort of problem with getting a message as that process.

Ahhh!!!!!  I have 2 processes waiting for a message.  And the stack is still locked!  And I cannot use the process stack since it will be unmapped.  So this is a major design flaw.

Right now, I see 3 possible solutions to this problem:
1. Allocate all stacks from the kernel address space (so they are all always mapped) and make them large enough to handle all calls.
2. Dynamically get a stack for each module for each call.
3. Allocate a stack for every function for every process.

Right now, none of these are favorable: Option 1 is a security risk.  Option 2 is a performance problem (with interrupts disabled).  Option 3 is wasteful and messy.

Even changing the individual address spaces would not help this.

Another thought is to remap the existing stack physical memory into the new process address space temporarily.  This is not a real fun choice but may be the best option.  Except for the fact it would require a stack to complete remapping the stack.


---

### 2021-Nov-25

I think I am going to put the stacks in kernel address space.  I may come back to this decision (yet again) in the future, but for now, this seems the best option.  I may add in some stack checking code to ensure that the stack is not over- or under-flowed at some point in the future.

This change is going to be invasive.  I am going to make a commit where I stand now so I have a roll-back position.
