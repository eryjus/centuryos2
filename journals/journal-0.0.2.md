# The Century OS

This project is a 64-bit focused version of Century-OS.  This version takes from the 32-bit version of Century-OS, but is better streamlined for the 64-bit CPUs (where any 32-bit archs will be added in later if desired).


---

## Version 0.0.2 -- Create Faults and Interrupt Handlers

This version will have just a few goals:
1. Clean up debugging code from v0.0.1
1. Create the IDT and install it
1. Create a function to install a handler
1. Create a generic handler


### 2021-Jan-13

So, my big decision at this point is how to organize the kernel.  Do I keep everything together (initialization and code) or split them up to be able to reclaim physical memory?  For now, I am going to keep everything together; I can split them up later.

So, with that decision, the IDT table will be located in the kernel's data section.


---

### 2021-Jan-14

I have a problem with linking.  Apparently, there is some section that is not getting compiled or linked properly (64 vs 32 bits) that I have to sort out.  It may require a rewrite of the linker script.


---

### 2021-Jan-15

OK, so I realized I do not have a `kInit()` function to call and then take care of all the other things.  I need to add that to the list of goals.

---

All of the above is technically complete.  However, I also want to be able to produce some output.  So, I will copy in the Serial components and produce some output.  I think I might migrate `kprintf()` as well.


---

### 2021-Jan-16

I have been debating what I want to do with the debugging output.  Part of me wants to also put it to memory for later dumping to a file.  However, I think for now I will work on just the serial port.

```
* 0) targets/x86_64-pc/bin/boot: ld.lld -T /home/adam/workspace/centuryos2/modules/kernel/arch/x86_64/x86_64-pc.ld -z max-page-size=0x1000 -L /home/adam/workspace/centuryos2/lib/x86_64 -nostdlib -g -O2 -L /usr/bin/../lib/gcc/x86_64-redhat-linux/10/ -o kernel.elf ../../obj/kernel/x86_64/entry.o ../../obj/kernel/x86_64/idt-asm.o ../../obj/kernel/x86_64/idt.o ../../obj/kernel/x86_64/kInit.o ../../obj/kernel/x86_64/kprintf.o ../../obj/kernel/x86_64/serial.o -l gcc;
ld.lld: error: /home/adam/workspace/centuryos2/modules/kernel/src/kprintf.cc:55:(.text+0x26): relocation R_X86_64_32 out of range: 18446744071562067968 is not in [0, 4294967295]
ld.lld: error: /home/adam/workspace/centuryos2/modules/kernel/src/kprintf.cc:0:(.text+0xF1): relocation R_X86_64_32 out of range: 18446744071562068022 is not in [0, 4294967295]
ld.lld: error: /home/adam/workspace/centuryos2/modules/kernel/src/kprintf.cc:0:(.text+0x26A): relocation R_X86_64_32 out of range: 18446744071562068022 is not in [0, 4294967295]
ld.lld: error: /home/adam/workspace/centuryos2/modules/kernel/src/kprintf.cc:109:(.text+0x364): relocation R_X86_64_32 out of range: 18446744071562068059 is not in [0, 4294967295]
 *** tup messages ***
 *** Command ID=229 failed with return value 1
 [  ] 100%
 *** tup: 1 job failed.
make: *** [Makefile:109: all] Error 1
```

At this point I think my linker script is flawed.  I will need to spend some time on that so I have something that is more reliable.  Tomorrow.


---

### 2021-Jan-17

OK, it turns out that the linking problems appear to be related to generating non-position-independent-code (non-pic).  After forcing pic code generation, I am linking properly.

However, this also created a problem with the loader that needs to be sorted out again.

---

I am able to get this working the the gcc cross compiler tools, but not the clang compiler.  For now I will stay with the gcc cross compiler.

---

OK, `kprintf` will need to be redone as the first 4 parameters are located in registers (not really negotiable that I can find).  More on that tomorrow.


---

### 2021-Jan-18

So, `kprintf` really will need to be an arch-specific implementation.  So that needs to move.  That done I now need to capture the parameter values before anything else is done.  I think I need to look at some open source.

I think I am going to also have to revisit the position independent code decision.


---

### 2021-Jan-19

My problem was getting the code to compile properly while handling 64-bit addressing.  I have the kernel compiling now and the loader can jump to it.  I now need to work on getting the addresses of the handler functions and printing them properly -- making sure the code is picking up the correct addresses.  I have also confirmed that this is also working with the `clang` toolchain.

OK, I think I have the `kprintf()` parameter handling all cleaned up.  So, now to make sure I have the correct handler address.

I think I have the IDT set up correctly.

Before I close this micro-version out, I want to add registers to the output.

With that done, I think this micro-version is complete.  Just some commentary cleanup.




