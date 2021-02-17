# The Century OS

This project is a 64-bit focused version of Century-OS.  This version takes from the 32-bit version of Century-OS, but is better streamlined for the 64-bit CPUs (where any 32-bit archs will be added in later if desired).


---

## Version 0.0.3 -- Add spinlocks and hook an internal function for managing them

This version will have a few goals:
1. Add Spinlocks (non-POSIX implementation)
1. Create an internal function array
1. Add spinlock management into the function array
1. Install the handler for the internal functions and install to the IDT
1. Establish error codes (errno.h)


---

### 2021-Jan-20

I was able to pull the spinlock implementation from the old code.  I did make some changes to the implementation.

So, I need to designate some ISRs for some specific things.  First is for OS services from user-space.  This in my mind has always been `INT 64h` (or 100 for Century).  I also need to have several for IPI interrupts and these need to have priority over the others.  For that reason, these will be from `0xe0` and up.

So that leaves the internal interrupts for internal functions.  At this point, I think I will use `INT c8h` or 200.  It is this table that I will start to populate.

I also copied `errno.h` from the old code.  I plan on integrating these error codes early into the process.

---

As I have been coding today, I am not really happy with the internal function design I have been working on.  I like the spinlock implementation, but I need to create a better interface for the internal functions.  This does not work well as a bottom-up approach.  I'm going to have to step back and work top down -- or at least think it through like that.

One of the problems I am having is that the internal functions interface is going to end up having all the individual functions in them and I do not want that.  For now, I'm going to think on this.


---

### 2021-Jan-21

OK, so let's see here.  I need the following:

* A library function to handle the internal task (InternalLockSpinlock)
* A low level function to place parameters in the correct registers and generate the software interrupt (InternalLockSpinlockDispatch)
* A low level function to receive he interrupt and save registers; determine the function address and call it (InternalTarget)
* A high-level function to handle the request (SpinlockLock)

I'm also going to take the time to duplicate the register usage here.  This is as much as it is for me than anything.

| Register | Usage | Parameter Register | Return Register | Called Function Saves Reg |
|:--------:|:------|:------------------:|:---------------:|:-------------------------:|
| %rax | Temporary Register; with Variable Arguments, the number of SSE registers used | | 1st | No|
| %rbx | optionally used as base pointer | | | Yes |
| %rcx | integer argument to functions | 4th | | No |
| %rdx | | 3rd | 2nd | No |
| %rbp | optionally used as frame pointer | | | Yes |
| %rsi | | 2nd | | No |
| %rdi | | 1st | | No |
| %r8 | | 5th | | No |
| %r9 | | 6th | | No |
| %r10 | used for passing the function's static chain pointer | | | No |
| %r11 | temporary register | | | No |
| %r12 | | | | Yes |
| %r13 | | | | Yes |
| %r14 | | | | Yes |
| %r15 | | | | Yes |

So, by convention, I have the function number located in `%rdi` and up to 5 parameters located in registers.  Anything beyond that gets pushed onto the stack.


---

### 2021-Jan-22

So, I was able to get the coding for the functions to install an internal handler written; I have not yet started working on the function call side as that will require a new library to be created.  Before I go there, I want to first start looking at how I am going to get things initialized.  For now, I see the following rough tasks to be completed:
1. Perform kernel initialization -- this is stuff that is local to the kernel binary itself
2. Perform loader cleanup -- reclaim this space
3. Scan through the modules and create virtual memory for each
4. For each module, check the entry table for which internal functions, OS services, and interrupt vectors will be hooked -- ensure no conflicts
3. For each module, call the early initialization function -- this function takes care of as much initialization as it can while there is only 1 CPU active and no processes running
3. Turn on all AP cores
4. For each module, call the late initialization function -- this function will complete any initialization that could not be completed until all cores were active.

For this to work, I will need to get a structure together.  This will look something like this:

```c++
struct module_t {
    char signature[16] = {0xeb, 0xfe, 'C', 'e', 'n', 't', 'u', 'r', 'y', ' ', 'O', 'S', ' ', '6', '4', 0x00};
    char modName[16] = {...};
    int sequenceNbr;
    int interruptCnt;
    int internalCnt;
    int osServiceCnt;
    struct {
        int interruptNbr;
        Addr_t interruptsAddr;
    } [x];
    struct {
        int internalNbr;
        Addr_t internalAddr;
    } [y];
    struct {
        int osServiceNumber;
        Addr_t osServiceAddr;
    } [z];
};
```

The first 2 bytes of the `signature` are an infinite loop.  This is done in case the entry point is jumped to -- it will loop rather than execute garbage.

After having moved several include files around, the `#pragma once` directive is no longer working.  This is because several files are located in multiple places now.  I will have to encapsulate with `#ifdef` directives.


---

### 2021-Jan-24

OK, I have the foundation of this written and compiling.  Instead of implementing spinlocks, I have implemented the functions to read/set one of these functions.  This, then, gives me enough to complete this version and commit it.

