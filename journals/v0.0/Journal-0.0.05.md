# The Century OS -- v0.0.5

This project is a 64-bit focused version of Century-OS.  This version takes from the 32-bit version of Century-OS, but is better streamlined for the 64-bit CPUs (where any 32-bit archs will be added in later if desired).


---

## Version 0.0.5 -- Further develop out the module initialization

In this version, the following are the goals:
1. Make the module header checking/validation more robust
1. Properly sort the modules for initialization in the proper sequence
1. Prepare for and enter Redmine for unloaded module cleanup
1. Pass in the `loaderInterface` structure for early init calls
1. Create the OS services and interrupts tables interfaces (includes [Redmine #479](http://eryjus.ddns.net:3000/issues/479))
1. Complete the setup of the interrupts, internal function calls, and os services
1. Call the late init functions where they exist
1. Flesh out more of the PMM early initialization function

This is an ambitious version.


---

### 2021-Feb-09

Adding additional checks into the header parsing is complete.

Working on sorting the modules will require some additional information, some of which I think may need to be stored in the module header itself.  The problem is that the actual kernel at this point does not have a heap.  So, I may need to determine the loading sequence by the order in which the modules are specified for GRUB.  Later, when I work on an initrd, the order they are placed in that file may also be used.  So, that said, the sequence should not be needed and can be removed.  That makes the sequence problem a quick solve as well.

The task to clean up an unused modules is [Redmine #483](http://eryjus.ddns.net:3000/issues/483).  So that was simple.

Also, passing in the `loaderInterface` is also trivial since the `PmmEarlyInit()` function is still trivial.  So that is also done.

The Service Table is set up.  Now to clean up the interrupt handler table.  But that will be tomorrow.


---

### 2021-Feb-10

So, several things will need to happen now:
* Refit the interrupt table to handle an interrupt number (leverage from 32-bit code)
* Change the internal functions to save/replace the `cr3` and restore it on the way out (only if a change)
* Change the OS services to save/replace the `cr3` and restore it on the way out (only if a change)
* Change the IRQ Interrupts to save/replace the `cr3` and restore it on the way out (only if a change)

Need to find some motivation....


---

### 2021-Feb-11

The internal functions were changed over yesterday to contain a `cr3` register, with the old value being stored on the stack.  I end up with a page fault and I cannot help to think I am having a problem with the TLB... and reloading `cr3` is not flushing it properly.

---

What the hell is this?

```
.. Done -- guaranteed unmapped
.. PML4 @ 0xffff_ffff_ffff_f000
.. PDPT @ 0xffff_ffff_ffe0_0200
New table at frame 0x0000_0000_0100_4000
.. PD @ 0xffff_ffff_c004_0100
New table at frame 0x0000_0000_0100_4000
.. PT @ 0xffff_ff80_0802_0000
```

The same table is returned twice!


---

### 2021-Feb-12

I was not able to get very far getting to the root of the problem.  I did start to rename the variables in order to provide a separation between the loader code and kernel code.  It broke the compile.

OK, compile fixed, I figure out that the PMM is not implemented!  DUH!

I'm dumb.  The PMM is not even being used!  So, the problem is in the kernel version of early pmm alloc.  Sigh.


---

### 2021-Feb-13

So, I think I have this figured out....  I am trying to map `loaderInterface` and requiring a value from withing `loaderInterface`.  Gotta figure that out.


---

### 2021-Feb-14

OK, I think the best thing to do here is going to be to map the `loaderInterface` structure into the kernel address space.  This way, the address is made available automatically to the other modules.  I am going to start with the address `0xffff 9fff ffff f000`.

That worked once I got that debugged.  Yay!

So, the early initialization code is complete.  I do still have some issues to resolve related to capturing the `cr3` register.  This is done.

I now have everything working except for the interrupts table.


---

So adding some meat to the PMM, I have to consider the sequence of initialization.  The PMM init will need to allocate frames from the PMM in order to map some memory.  The problem is that the kernel is still in control of the PMM after the loader interface has been passed on to the PMM initialization.  I think this will work since both the kernel and pmm are referencing the same structure in the same place in memory.  I just need to make sure I have everything allocated before I start putting the PMM in charge.  Ultimately, once the PMM is ready, it needs to force-replace the `PmmAlloc()` function in favor of its own handler so that later module initialization can be completed.

In order to initialize the PMM, I need to put the MB memory information into the `loaderInterface` structure.  For this to be effective, I will need to limit the number of blocks I put in there -- say to 10 block of available memory.


---

### 2021-Feb-15

I was able to get the blocks of free memory added to the `BootInterface_t` structure.  I also added a redefinition of the PMM Allocator into the end of the `PmmEarlyInit()` function.

I was able to recreate the PMM initialization.  So far, everything is on the scrub stack for clearing.

I need to re-think the interface.  There several types of allocation (aligned, normal, low-memory), so I need to simplify the interface.  I think I need to add 3 parameters:
* Low Memory Request
* Alignment Bits
* Number of Frames

Then, I can create all the other functions that are needed to get into this interface.  I also have a range I can release.

To go much farther, I need to have a Spinlock implementation -- which is outside the scope of this version.

The final thing to consider is an interrupt table.  It was supposed to be included in this version.  I have nothing to test it with at this point, so I am going to postpone it until another version.  See [Redmine #484](http://eryjus.ddns.net:3000/issues/484).

The other thing I have not done yet is call the late init functions.  This cannot be done until the other CPUs are started and interrupts are enabled.  See [Redmine #485](http://eryjus.ddns.net:3000/issues/485).

At this point, I am ready for a commit.

