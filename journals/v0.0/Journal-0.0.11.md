# The Century OS -- v0.0.11

This project is a 64-bit focused version of Century-OS.  This version takes from the 32-bit version of Century-OS, but is better streamlined for the 64-bit CPUs (where any 32-bit archs will be added in later if desired).


---

## Version 0.0.11 -- Get the kernel to run on real hardware

The goal for this version is simple: Get the kernel to boot on real hardware.

How big of an effort this will be is very unclear.  I expect to find several bugs to solve, add a bunch of debugging code (for which I will add flags to enable and disable), and maybe even add some additional debugging modules.


### 2021-Oct-17

For my first test, I triple fault with no output.  Let's start by confirming I have 64-bit hardware.  A Core 2 Duo is a 64-but CPU.  That said, I am faulting before the very first output.

First thing is to confirm I am making it to my own code without a reboot.  A busy loop in the code will check for that.  No reboot.

Next is a check for entry into long mode.  Same busy loop.  No reboot.

Last step in `entry.s` is right before the call to `lInit()`.  Still no reboot.

Now, I am in the `lInit()` code.  From there, I will uncomment the `#define USE_SERIAL` line so I get the loader output.  From here, I should see `Hello` in the serial console.

```
### Listening to /dev/ttyUSB0...
Hello
Getting the kernel
  cr3 = 0x0000_0000_0100_4000
```

So, I am getting rather far into the loader but not quite jumping to the kernel.

```c++
    SerialPutHex64(GetCr3());
    SerialPutChar('\n');

    Addr_t kernel = MBootGetKernel();
    Addr_t stack = (earlyFrame);
    earlyFrame += 4;
    MmuMapPage(0xfffff80000000000, stack + 0, PG_WRT);
    MmuMapPage(0xfffff80000001000, stack + 1, PG_WRT);
    MmuMapPage(0xfffff80000002000, stack + 2, PG_WRT);
    MmuMapPage(0xfffff80000003000, stack + 3, PG_WRT);
    SerialPutString("Stack mapped\n");
```

Now, this confirms I am in long mode.  Plus, I have a problem in either `MBootGetKernel()` or in `MmuMapPage()`.  Get a couple of things done here:
1. Add the debug output controls
2. Add a debugging line to split the difference

Well, I have a problem in `MBootGetKernel()`.  Enabling the output in `mboot.cc` to see what I get.

That gets me into the MMU code.  First checking `MmuIsMapped()`.  Sine there is undocumented code in `ldr_MmuMapPage()`, I expect the problem will be in `MmuIsMapped()`.

I did find some code in the loader MMU source that was not updated to match the kernel MMU source.  Copied that over and now for another test.  That had no impact.

Well, it also looks like I am not actually getting all the data off the serial line before the reboot kills the send queue.  I need some busy waits to halt the code as well.

I think I am getting inconsistent results.  I need to narrow this down a bit more for consistent results.

I think my loader has expanded past 4K with all the debugging code I have added, so I am getting odd page faults!

```S
                mov         ebx,[pt]                        ;; get the address of the pt table
                mov         eax,0x100103                    ;; set this loader address and flags
                mov         [ebx + (256*8)],eax             ;; set the page

                add         eax,0x1000                      ;; next frame
                mov         [ebx + (257*8)],eax             ;; set the page
```

No, it appears to have the right mappings.

I believe I have a problem in `MmuEmptyPdpt()`.  This function does not appear to be clearing the PDPT tables properly after they are mapped.  In particular, this is not right:

```c++
        uint64_t *tbl = (uint64_t *)(0xffffffffffe00000 | (index << 21));
```

Ha!!  And with that, I have a bootable OS on real hardware!  I am absolutely floored it was only 1 bug!

I am going to spend some time documenting the loader sources properly since I made a number of changes there for debugging purposes.


---

### 2021-Oct-18

Today was a documentation day.  I have a *LOT* to learn about Doxygen.


---

### 2021-Oct-19

Mode documentation today.  I intend to complete documenting `mboot.cc` and `elf-loader.cc`.  Both source files have a lot of data structures to document, so I may be a little too ambitious.  There are also 3 `.h` files to document as well as `serial.cc`, so I will be here for a while.


---

### 2021-Oct-20

Well, the `elf-loader.cc` file is giving me trouble getting documented and Doxygen to recognize it.  It's annoying me.

There may be an opportunity to pull some of this elf stuff into a common header.  It may simplify the problems here.  It seems I may have some duplicate names here.  It will also mean I need to run several versions of documentation -- especially when I get into multiple archs again.

So, I believe my problem here is that the `typedef`s are not documented the first time they are encountered.  Therefore, the non-documentation problem follows me through the entire build.

---

I want to sleep on this, but at the moment I believe the best thing to do here is to create a `common` "pseudo-module and place the `elf.cc` source there, removing it from both the `libk` and the `loader-grub` modules.  I would then update the `obj/Tupfile`s for both the `libk` and `loader-grub` modules compile from that "pseudo-module".  It would certainly eliminate code redundancy, which is a good thing.


---

### 2021-Oct-21

I agree with myself.  I am going to move the `elf-loader.cc` to a `common` pseudo-module.  I just need to do a side-by-side comparison of the code first.

Well, this is going to be more difficult than I thought, but I think still worth it.  I think some things are overly messy.

The first task is going to be to consolidate the `mmu.h` file into the arch inc folder.  I was able to get the `mmu.h` file moved.  That works well.  I may or may not need to move `mmu.cc` and will have to check that when I get to documenting it.

Now, did that clean up Doxygen?  It did.  Now to finish documenting it.

The goal was to document the loader module.  I have the code done, but not the headers.

So, what am I really left with here?  Looks like 3 very short `.h` files in `inc`.   And `mmu-loader.cc`, which may be able to be consolidated.

This is really more than this version was intended....  I will add the consolidation of the MMU sources into v0.0.13, which is a stabilization version.  It is better suited in that version than this.  In the meantime, I am going to skip documenting this source for the moment.

I think I will take care of the constants next.  That was easy as well.

Now on to the `.h`. files.

I was also able to eliminate the `types.h` file from the loader and use the common one in `arch`.  `mmu-loader.h` will be handled with the `mmu-loader.cc` file for consolidation.  So, I think I am done.

I guess I should confirm that with all the changes I made this version will still run on real hardware....
