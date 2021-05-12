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



