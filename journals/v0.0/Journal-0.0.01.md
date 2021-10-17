# The Century OS -- v0.0.1

This project is a 64-bit focused version of Century-OS.  This version takes from the 32-bit version of Century-OS, but is better streamlined for the 64-bit CPUs (where any 32-bit archs will be added in later if desired).


---

## Version 0.0.1 -- Complete the x86_64 loader

This version will have just a couple goals:
1. Put the CPU into long mode
1. Find the kernel and jump to it


---

### 2021-Jan-02

So, with this project I will be changing a few things.  First, I want to use `clang` for the compiler.  This is already by definition a cross-compiler, so there is no need to maintain a special cross-compiler -- I hope anyway.  I am not certain here, but I'm sure I will find out.

I am also changing up my directory layout a bit.  I feel a need to simplify a bit.

I am also going to be separating the loader from the kernel physically in order to be able to substitute in a UEFI loader at some point, which would be a separate module.

---

I have a trivial `entry.s` file that is prepared and all the controls to compile the object.  This gives me intermediate object files to play with.  Now, to add some additional code into this file.

---

OK, I have the code written to get into Long Mode.  However, I know there are some problems to resolve:
1. The stack is not identity mapped
1. The video buffer is not identity mapped
1. There is no custom GDT to handle the 64-bit long mode

Any of these problems will cause a triple fault since there is no IDT table set up to handle any faults.

I also need to link (using `clang`) the final executable.

---

### 2021-Jan-03

This morning I was able to get the linker script working properly and I now have a properly linked executable suitable for loading with `grub`.  To get there, I was not able to use `clang` as my linker; I did have to use `ld`.  That said, I also found that there is an `lld` package available from `dnf` which was not installed.  I am going to try to change that up.

which worked; I am now using `ld.lld` as my linker.

So, at this point, I have the video buffer mapped.  The stack I do not need yet (and may remove that from the pre-64-bit code and place it in 64-bit code).  So, I need a proper 64-bit GDT.  This, I can pull from Century64.  This was a simple copy -- no need to reinvent the wheel!

---

With a little debugging, I was able to get into long mode.  I have a good stack and I have been able to test it.

---

OK, I have the CPU in long mode.  So, now I need to find and jump to the actual kernel executable.

---

I have a few basic functions written to find the kernel, as known by the name `'kernel'`.  I am now working on debugging these functions.

With very few issues, I was able to find the kernel module with MB1; checking with MB2....  MB2 still has issues....

It appears that I am not getting any module information from MB2....  That's going to have to be a tomorrow task.

---

### 2021-Jan-04

OK, so the MB2 structure....  This ended up being a quick fix with the `grub.cfg` file -- using `module2` instead of `module`.

Now, back to my goals, I also need to find the kernel starting point and jump to it.  For that I need to be able to do 2 things:
1. Parse an `.elf` file
1. Map the sections into memory properly

---

I have the elf parser written, but not working.  This will have to be looked at tomorrow.

---

### 2021-Jan-07

Well, I have spent a few days trying to debug this problem without much success.  What I have been able to identify is that it is related to the MMU functions and mapping a page.  This appears to be related to creating new tables.

What I am used to is being able to output data to a serial port to track what is going on.  I need to add this into the loader.  When I do I want to be able to disable it for most builds.  First, I forgot about the Bochs debugger and I need to try that first.

---

### 2021-Jan-11

It turned out to be a problem with `int` and `uint64_t`.  After changing to an unsigned type, things are working better.

However, I now have other problems.

---

### 2021-Jan-13

I'm spinning my wheels and not getting anywhere.  I am going to add in the code to output to the serial port.

---

That was a good choice; I was able to get everything sorted somewhat quickly.  I have been able to get the kernel loaded and I have confirmed that the kernel really does have control.

So, this, then, completes all the goals of this version.  Time to write the commit and push it up to github.

