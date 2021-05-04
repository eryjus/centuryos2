# The Century OS

This project is a 64-bit focused version of Century-OS.  This version takes from the 32-bit version of Century-OS, but is better streamlined for the 64-bit CPUs (where any 32-bit archs will be added in later if desired).


---

## Version 0.0.7 -- Complete the PMM

In this version, the PMM will be (mostly) completed:
* PMM Alloc
* PMM Release
* Code the PMM Late Init Function (will need to complete the CPU starts for this to work)


---


### 2021-Feb-17

Time to start working on the PMM.  I have to keep in mind that the everything I am working on in these functions must be multi-threaded.

To start with, I will work on `PmmAllocate()` first since I already have calls for that function.


---

### 2021-Feb-18

I think I have most of the PMM filled in.  Most of it anyway.

I am trying to think of how best to test the pmm....

I have a test and I am getting an `-ENOMEM` result.  However, trying to release that error code is also creating problems.  I need to put some protections in for that.

---

Page fault at ffff ff80 0802 0000 -- clearly in the recursive mapping range.


---

### 2021-Feb-19

I think I have a problem with the MMU -- specifically mapping a page.  I may need to split the functions into a loader and kernel version.

I think the problem will be with the early init frame allocator since it assumes that the allocated frame is cleared.


---

### 2021-Feb-20

Looks like i have a stack overflow.  Cleaning up this stack dump:

```
 | STACK 0xfffff80000001f10 [0xffff8000:0x00001490] (<IdtGenericEntryNoErr>)
 | STACK 0xfffff80000001f18 [0xffff8000:0x00001492] (<IdtGenericEntry>)
 | STACK 0xfffff80000001f40 [0xffff8000:0x00003160] (<PmmAlloc>)
 | STACK 0xfffff80000001f60 [0xffff8000:0x00003230] (<InternalDispatch>)
 | STACK 0xfffff80000001f68 [0xffff8000:0x00003232] (<InternalDispatch -- after int>)
 | STACK 0xfffff80000001f90 [0xffff8000:0x0000317e] (<PmmAlloc -- after call>)
 | STACK 0xfffff80000001fa0 [0xffff8000:0x0000256e] (<krn_MmuMapPage -- after call>)
 | STACK 0xfffff80000001fb0 [0xffff8000:0x00002450] (<krn_MmuMapPage>)
 | STACK 0xfffff80000001fd8 [0xffff8000:0x00003160] (<PmmAlloc>)
 | STACK 0xfffff80000001fe0 [0xffff8000:0x00001565] (<InternalTarget.nocr3 -- after call>)
 | STACK 0xfffff80000002010 [0xffff8000:0x00001490] (<IdtGenericEntryNoErr>)
 | STACK 0xfffff80000002018 [0xffff8000:0x00001492] (<IdtGenericEntry>)
 | STACK 0xfffff80000002028 [0xffff8000:0x00003230] (<InternalDispatch>)
 | STACK 0xfffff80000002040 [0xffff8000:0x00003160] (<PmmAlloc>)
 | STACK 0xfffff80000002068 [0xffff8000:0x00003232] (<InternalDispatch -- after int>)
 | STACK 0xfffff80000002090 [0xffff8000:0x000011c1] (<PmmEarlyFrame -- after call>)
 | STACK 0xfffff80000002098 [0xffff8000:0x00001180] (<PmmEarlyFrame>)
 | STACK 0xfffff800000020b0 [0xffff8000:0x00001565] (<InternalTarget.nocr3 -- after call>)
 | STACK 0xfffff800000020e0 [0xffff8000:0x00001490] (<IdtGenericEntryNoErr>)
 | STACK 0xfffff800000020e8 [0xffff8000:0x00001492] (<IdtGenericEntry>)
 | STACK 0xfffff80000002110 [0xffff8000:0x00003160] (<PmmAlloc>)
 | STACK 0xfffff80000002130 [0xffff8000:0x00003230] (<InternalDispatch>)
 | STACK 0xfffff80000002138 [0xffff8000:0x00003232] (<InternalDispatch -- after int>)
 | STACK 0xfffff80000002160 [0xffff8000:0x0000317e] (<PmmAlloc -- after call>)
 | STACK 0xfffff80000002170 [0xffff8000:0x0000256e] (<krn_MmuMapPage -- after call>)
 | STACK 0xfffff80000002180 [0xffff8000:0x00002450] (<krn_MmuMapPage>)
 | STACK 0xfffff800000021a8 [0xffff8000:0x00003160] (<PmmAlloc>)
 | STACK 0xfffff800000021b0 [0xffff8000:0x00001565] (<InternalTarget.nocr3 -- after call>)
 | STACK 0xfffff800000021e0 [0xffff8000:0x00001490] (<IdtGenericEntryNoErr>)
 | STACK 0xfffff800000021e8 [0xffff8000:0x00001492] (<IdtGenericEntry>)
 | STACK 0xfffff800000021f8 [0xffff8000:0x00003230] (<InternalDispatch>)
 | STACK 0xfffff80000002210 [0xffff8000:0x00003160] (<PmmAlloc>)
 | STACK 0xfffff80000002238 [0xffff8000:0x00003232] (<InternalDispatch -- after int>)
 | STACK 0xfffff80000002260 [0xffff8000:0x000011c1] (<PmmEarlyFrame -- after call>)
 | STACK 0xfffff80000002268 [0xffff8000:0x00001180] (<PmmEarlyFrame>)
 | STACK 0xfffff80000002280 [0xffff8000:0x00001565] (<InternalTarget.nocr3 -- after call>)
 | STACK 0xfffff800000022b0 [0xffff8000:0x00001490] (<IdtGenericEntryNoErr>)
 | STACK 0xfffff800000022b8 [0xffff8000:0x00001492] (<IdtGenericEntry>)
 | STACK 0xfffff800000022e0 [0xffff8000:0x00003160] (<PmmAlloc>)
 | STACK 0xfffff80000002300 [0xffff8000:0x00003230] (<InternalDispatch>)
 | STACK 0xfffff80000002308 [0xffff8000:0x00003232] (<InternalDispatch -- after int>)
 | STACK 0xfffff80000002330 [0xffff8000:0x0000317e] (<PmmAlloc -- after call>)
```

This means that `PmmEarlyFrame` calling `MmuMapPage` is really a problem.  Removing that and testing again, I actually make it into the PMM to allocate a frame.  This returns `-12`, or `-ENOMEM`.

To get anywhere with this, I am going to need to get some output.  This will mean duplicating the serial port and `kprintf()` code.

---

Well, Bochs and QEMU have different problems.  Bochs is having problems with performing mappings.  QEMU is having problems with completing the PMM early init.

No matter what, I need to make sure the MMU is rock solid.  This limits my debugging to Bochs.

I think I am unmapping the GDT since a jump reloads the segment selector.

I have this information:

```
.. The page is guaranteed unmapped
.. PML4 address is still 0xfffffffffffff000 (contents 0x0000000000000000)?
.. (new early frame: 0x0000000000001019)
.... PML4 address is now 0xfffffffffffff000 (contents 0x0000000001019003)?
.. PDPT address is still 0xffffffffffe00000 (contents 0x0000000001006163)?
```

So, I am creating a new table (frame `0x1019`), but there is data in that frame, which is creating a problem.  I need to figure out how to clear that data.  I should be able to mask out the address of the start of this table of entries and clear it.

---

I think I have `MmuMapPage()` working now.  Bochs is failing in the PMM Early Init routine.


---

### 2021-Feb-21

OK, I am getting past the PMM Early Init function, but I do not think it is doing it right.  I need a function to dump the contents, but that also means I need to figure out how to send output to the serial port.  The question is about how to implement it.

So, I created a special-purpose function call that takes a small number of variable arguments to pass to the kernel.  This works with a basic test.

Now, with the structure dumped:

```
==========================================

Dumping PMM Structure
  Number of frames available: 3140766

  Low Lock State: unlocked
  Low Stack Address: 0x0000000000000000

  Normal Lock State: unlocked
  Normal Stack Address: 0x0000000000000000

  Scrub Lock State: unlocked
  Scrub Stack Address: 0xffffff0000003000
    Scrub Stack TOS frame: 0x0000000000100000
    Scrub Stack TOS count: 1572864

==========================================
```

I see a couple of things:
1. The number of free frames in wrong
1. The starting point for the scrub frame should be far bigger than 1M

Both of those ended up being accurate.

This is working properly now.  I am able to allocate a frame.

---

Now for the PMM....  I need to confirm that the push and pop functions are working properly.  Right now it is not.  The problem is that an empty stack is represented as an `NULL` address in the `pmm` structure.  This means that determining an empty stack is a little difficult to do.  I either need to pass in the address of the stack (pointer-to-a-pointer) or I need to come up with a variable which indicates the stack we are operating on.  Alternatively, I can publish `MmuIsMapped()`.


---

### 2021-Feb-22

There is a new issue where I am faulting on output.  Grrrrr!!!!

I have little patience for this tonight.


---

### 2021-Feb-28

At this point I have been reading about caches for several days.  One of my thoughts is whether the contents of `IA32_MTRR_DEF_TYPE` is having some kind of effect on the caching of the emulators.  It does not seem likely..., but it does get me thinking about what is processor family that is being emulated.  The 64-bit processor choices are numerous versus 32-bit processors.  So, I need to make an effort to read `cpuid` to determine which processor family is being emulated -- if fo no other reason than to know.

So, they are all the same family, but all different model numbers.  I do not think that should be creating the problem -- not at this level.


---

### 2021-Mar-08

I am still debugging the problems with the PMM.  When I have time.


---

### 2021-Mar-17

Still trying to debug.


---

### 2021-Mar-18

OK, I might be onto something....  I have been working with `gdb` and trying to understand how to debug the kernel.  I am unable in `gdb` to set the memory at `0xffffff0000004000` to any value once it has been mapped.  This means I am pointing to some read-only memory or off the end of the emulation.  So, I need to go back to the `mboot` code and make sure I am interpreting that properly.

What I was able to figure out is that MB1 does not work:

```
Pushing frame 0x000000000027ffff with 1 frames onto stack 0xffffff0000003000
.. The prepared top node (0xffffff0000004000) states: 0x0000000000000000, 0
```

... while MB2 does:

```
Pushing frame 0x000000000023ffff with 1 frames onto stack 0xffffff0000003000
.. The prepared top node (0xffffff0000004000) states: 0x000000000023ffff, 1
```

Finally, something I can research!

This ended up being a stupid little error!!!  And cleaning up the line fixed that problem!

Now, I gotta figure out where I put all my debugging code several weeks ago.

---

With that out of the way, I just need to get the spinlocks working properly.  Currently, they are faulting for some reason.


---

### 2021-Mar-19

I found that the flags were not being properly collected and filtered.  After correcting that, the locks are working now.  However, `vbox` is faulting still.


---

### 2021-Mar-26

I was able to get one bug taken care of.  Bochs and QEMU boot.  However, VBox does not.  VBox is still triple faulting for some reason in the MMU mapping (and clearing) a page table.  This is happening in the loader, not the kernel.

Hmmmm.....

```
Mapping 0xffff_ff80_0000_0000 to frame 0x0000_0000_0000_0001
Checking if the page is mapped
Checking if address 0xffff_ff80_0000_0000 is mapped
Mapped? PML4 at 0xffff_ffff_ffff_fff8
mapped? PDPT at 0xffff_ffff_ffff_f000
mapped? PD at 0xffff_ffff_ffe0_0000
mapped? PT at 0xffff_ffff_c000_0000
mapped.
Unmapping 0xffff_ff80_0000_0000
Checking if address 0xffff_ff80_0000_0000 is mapped
Mapped? PML4 at 0xffff_ffff_ffff_fff8
mapped? PDPT at 0xffff_ffff_ffff_f000
mapped? PD at 0xffff_ffff_ffe0_0000
mapped? PT at 0xffff_ffff_c000_0000
mapped.
.. Done -- guaranteed unmapped
.. Mapping PML4 @ 0xffff_ffff_ffff_fff8
.... [hex 0x0000_0000_0100_4163]
.. Mapping PDPT @ 0xffff_ffff_ffff_f000
.... [hex 0x0000_0000_0100_5123]
.. Mapping PD @ 0xffff_ffff_ffe0_0
```

This is the wrong frame for the mapping.  Need to figure that out.


---

### 2021-Mar-28

This is what I find in the `vbox` log:

```
00:00:07.242632 !!
00:00:07.242633 !! {cpumguest, verbose}
00:00:07.242634 !!
00:00:07.242650 Guest CPUM (VCPU 0) state:
00:00:07.242653 rax=000000000000000a rbx=00000000001011e0 rcx=ffffff8000000000 rdx=00000000000003f8
00:00:07.242656 rsi=00000000001017ed rdi=000000000010195d r8 =0000000000000000 r9 =0000000000000000
00:00:07.242659 r10=0000000000000000 r11=0000000000000000 r12=0000000000101430 r13=ffffff8000000000
00:00:07.242661 r14=0000fffffffff000 r15=00000000001013e0
00:00:07.242662 rip=0000000000100c7d rsp=0000000001003f78 rbp=00000000001013b0 iopl=0      rf nv up di pl nz na po nc
00:00:07.242665 cs={0008 base=0000000000000000 limit=00000fff flags=0000a09b}
00:00:07.242666 ds={0010 base=0000000000000000 limit=00000fff flags=0000a093}
00:00:07.242667 es={0010 base=0000000000000000 limit=00000fff flags=0000a093}
00:00:07.242668 fs={0010 base=0000000000000000 limit=00000fff flags=0000a093}
00:00:07.242670 gs={0010 base=0000000000000000 limit=00000fff flags=0000a093}
00:00:07.242671 ss={0010 base=0000000000000000 limit=00000fff flags=0000a093}
00:00:07.242672 cr0=0000000080000011 cr2=0000000000100c7d cr3=0000000001004000 cr4=00000000000000a0
00:00:07.242674 dr0=0000000000000000 dr1=0000000000000000 dr2=0000000000000000 dr3=0000000000000000
00:00:07.242675 dr4=0000000000000000 dr5=0000000000000000 dr6=00000000ffff0ff0 dr7=0000000000000400
00:00:07.242677 gdtr=00000000001000d0:0017  idtr=0000000000000000:0000  eflags=00010046
00:00:07.242679 ldtr={0000 base=00000000 limit=0000ffff flags=00000082}
00:00:07.242681 tr  ={0000 base=00000000 limit=0000ffff flags=0000008b}
```

In particular, that I am faulting on `rip`.  Since this is still loader memory, this should be identity mapped.  It is also after a return from a call to `SerialPutString()`.  The faulting instruction is in `MmuMapPage()`.

This line:

```C
MmuMapPage(0xffffff8000000000, 1, true);
```

... is wrong.  In particular, in relation to its address: `0xffff ff80 0000 0000` is part if the recursive mapping structures and should never be remapped as if for temporary storage.  As a matter of fact, the following is supposed to be used for temporary structures:

```
| 1fe |	ff00 0000 0000 |	ff7f ffff ffff |	Temporary Addressing |
```

So, that address really needs to be in that range (`0xffffff0000000000` to `0xffffff7fffffffff`).

Everything boots properly except `vbox`.  I am, however, getting farther than I was -- I am at least getting into the kernel and loading the modules.  This happens to be back where the original problem was in mapping the module's ELF header.


---

### 2021-May-03

Been a while....

But I found it rather quickly in `MmuIsMapped()`.

```
    INVLPG((Addr_t)GetPML4Entry(a));
    INVLPG((Addr_t)GetPDPTEntry(a));
    INVLPG((Addr_t)GetPDEntry(a));
    INVLPG((Addr_t)GetPTEntry(a));
```

All emulators boot equally well.

With this, it is time to commit.


