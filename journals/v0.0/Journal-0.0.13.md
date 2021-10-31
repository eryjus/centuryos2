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


