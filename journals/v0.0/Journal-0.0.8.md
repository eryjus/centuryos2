# The Century OS

This project is a 64-bit focused version of Century-OS.  This version takes from the 32-bit version of Century-OS, but is better streamlined for the 64-bit CPUs (where any 32-bit archs will be added in later if desired).


---

## Version 0.0.8 -- Complete the Interrupt Controller Device

In this version, we will complete these tasks:
* Confirm the Interrupt table and the ability to hook an interrupt
* Confirm an Interrupt can be hooked when loading a module
* Create the APIC driver module and load it
* Set up the APIC Timer and hook the IRQ
* Enable interrupts


---

### 2021-May-04

OK, the APIC is a split architecture.  This means I am combining the equivalent of the PIC and PIT into a single module.

Now I have to decide how I am going to deal with the the different architectures.  The APIC is architecture-specific.  So, do I deal with the timer in general?  Or do I deal with the APIC architecture in `arch/x86_64`?

I think I am going to have to create a common interface, but have everything implemented in `arch/x86_64`.  It's the only solution that makes any sense.


---

### 2021-May-05

So, I need to define what the PIC should be able to do.  First, the APIC is a split architecture.  There is the Local APIC (LAPIC) which is embedded in each CPU.  This LAPIC controls timers and Inter-Process-Interrupts (IPI).

There is also the IOAPIC and there may be several on the system, depending on how the buses are put together physically.

Finally, there are a couple different versions to consider:
* x2APIC
* xAPIC
* APIC (external Intel 82489DX chip)
* PIC (Intel 8259 PIC)

Finally, I think I want to incorporate the timer and PIC into the same driver, since the APIC contains both.

I think the best thing to do is to start with an x2APIC implementation and then add in the others.


---

### 2021-May-06

It dawns on me today that I may have multiple APICs, and more to the point a different count of LAPIC and IOAPIC devices.  So in reality I probably need to split these devices up into separate structures.  In fact, the Local APIC is only addressable from its local CPU.  The IOAPIC on the other hand needs to be found via the MADT ACPI tables -- which would need to be parsed at boot and stored.

Since the PIC will not work with multiple processors, I need to not support it.


---

### 2021-May-08

Well, I have been writing the interface for the xAPIC (Memory Mapped I/O interfaces) but putting it in the x2APIC source files....  The X2APIC uses MSRs as it's interface.

I have also been looking at where/how to relocate the xAPIC in 64-bit systems.  The Intel documentation is unclear on this -- with parts saying there is a 36-bit limit and others saying there is a limit up to `MAXPHYADDR`.

In short, I have to clean this up a bit.

---

The x2APIC code is now cleaned up....  Now I need to build a binary and install it.

I get this far....  and none of my emulators support the x2APIC....


---

### 2021-May-09

I found that Bochs does support the x2apic and after enabling that I found a single bug and the kernel boots.

Now, I have to do that same for the xapic.

Also, just for fun and to show just how old I really am, I am going to change the Mmio functions to be PEEK and POKE.  Just for fun....

---

I now have the xAPIC and the x2APIC drivers completing their early initialization.

Going back and reviewing the goals of this module, I have met them all except 1: Hook the Timer IRQ.  Is that for the scheduler to do?  Or, should the APIC timer call the scheduler function?  I think I am going to leave that for the scheduler to do as I do not want to have to call an internal function with every timer interrupt.

So, that completes this micro-version.

But, I need to rename `x2apic` to `lapic` since this is only handling the Local APIC functions -- not the IOAPIC functions.


