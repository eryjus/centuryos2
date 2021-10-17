# The Century OS -- v0.0.6

This project is a 64-bit focused version of Century-OS.  This version takes from the 32-bit version of Century-OS, but is better streamlined for the 64-bit CPUs (where any 32-bit archs will be added in later if desired).


---

## Version 0.0.6 -- Spinlocks

In this version, I will implement spinlocks into the kernel and register them as internal functions:
* Lock
* Unlock
* Try (framework)


---

### 2021-Feb-15

This was nearly completely implemented.  I just had to rework some functions and hook the functions in.


---

### 2021-Feb-16

So, since this was easier to implement, I will go back through the code and work on naming conventions.  I want anything that is going to be "installed" into a system table to have a prefix in the name, and related to its module.  This will be something like `krn_` or `pmm_`... and then the name.  So, something like `pmm_PmmAllocate()`.  This way, I can have library functions that are called regularly named more naturally: `PmmAllocate()`.

This rework will be added into this version as well.

Let's start with Spinlocks.

Mmu is much harder to get right -- since I have things in `libk` that reference both versions, I have to figure out how to work this out.

I'm glad I took this on now (before I had too much written) since I have a lot of messy problems.

I think the best action here is going to be to eliminate the `internal-funcs` library and put that into `libk`.  So this will break a bunch of stuff.

`MmuMapFrame()` is cleaned up....  Next, the rest of Mmu.

Everything looks to be cleaned up.

This was a quick version, but time to commit.


