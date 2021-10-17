# The Century OS -- v0.0.10

This project is a 64-bit focused version of Century-OS.  This version takes from the 32-bit version of Century-OS, but is better streamlined for the 64-bit CPUs (where any 32-bit archs will be added in later if desired).


---

## Version 0.0.10 -- Create a Debugger process

This version will establish a kernel level debugger, with functions that can be published from each module.  Here are the goals:

* Establish a debugging function table
* Update/change the entry code to read the debugging table
* Add functions to map a function to its address
  * This will be handled by function name, not function number
* Create a debugging task and launch it
  * This will interact with the serial port, reading and writing
* Populate debugging functions


---

### 2021-Oct-10

So, the ability to debug the kernel is critical.  The 32-bit version of the kernel appeared to work very well -- until I was able to get some visibility into the internal data structures.  Since that kernel was mostly monolithic, it was easy to dump internal structures.  With this micro-kernel, the challenge will be to share information back and forth.  I want to keep the complete I/O interface in the debugger, but the individual modules will have to be responsible for creating that output -- I do not want the debugger to have to be responsible for every structure definition.

Another feature that I will need to maintain is that I do not want function numbers and have to update the debugger itself with each function that is added.  It would be better to maintain a dynamic function list in a string table and then have each module register its own debugging functions.  By having this take place in the `ModuleLateInit()` functions, I can query if a `RegisterDebuggerFunction()` internal function has been registered (or call a function to determine if we have a debugger installed) and then self-register the debugging functions.

The `DbgOutput()` function will need to have an `sprintf()` implementation into a buffer that can be copied into some kernel space in the `libk` library.  The reason for this is several strings would need to be copied from the module into the kernel address space on a call and those would have to be identified dynamically.

For the list of functions that are callable for each module, this can easily be implemented as a quick adaptation of a string table I have implemented in compilers in the past.  This uses the `List<t>` standard template class.  I am not fully implementing all the C++ features in my code, so that will not really work.  I will need to write my own table.


---

### 2021-Oct-11

The old debugger was implemented as a state machine.  This worked well, but it had all the debugging function built in.  What I really want to do is set up a few basic functions that the debugger itself handles and use those to implement the UI.  The Debugging Function Table itself would be a good thing to be able to debug, I think.

The module for these functions will be `debugger`.  Simple enough.

Some things I want to be able to do:
* List all functions
* List all functions for a module
* List the detailed info for a function

I eventually want to also dump the symbol table, but that will be linked into every module.  I think this will be enough to start for now.

I think the command to list the all the installed debugger functions will be `debugger list all`.  The command to list the functions for a specific module will be `debugger list <module>`; obviously I cannot have a module named `all` and if I do it will be handled with the former command.  Finally the command the dump the detailed info on a registered function will be `debugger info <module> <function>`.  Like the previous debugger, I would like to be able to get into a module and stay there until exited.  And therefore, a state.

For this, I should be able to keep a list of modules in an array and associate the index number as the state.  In fact, there may even be a need for a complete menu structure, including several levels of sub-states.  The key will be to keep things dynamic so I do not have to rewrite the debugger for each function added.

Hmmm....  This is going to take some thinking.  How do I get the debugging data from the module into the debugger and maintain a proper state and transitions?  Can this be added as an array?

* Index number current state (implied; 0 = main menu/entry point)
* Current state name
* Function to execute
* Number of transitions
* Transition table:
  * command
  * short command
  * next state index

So, much of this will need to be handled by the kernel.  I need the code to perform a context change to be in common code space.  This means adding another table into the kernel.  However, the UI can be built outside of the kernel, and will.  I also believe that the function to execute and its address space can be passed in via parameters, like the OS services.  This is feeling much better than the rocky (and incoherent) start I had prior.

Now, I had a centralized configuration files that were translated into simple includes in the old version.  I need to drag those out.  Both config and debugging were supported.  Since I need a global method to enable/disable the debugger, I will start with the config tables.

---

I have my first config option prepared: `KERNEL_DEBUGGER   ENABLED`.  I am able to use preprocessor code such as:

```c++
#if IS_ENABLED(KERNEL_DEBUGGER)
// do something really cool!
#endif

#if !IS_ENABLED(KERNEL_DEBUGGER)
// do something that is very boring
#endif
```


---

### 2021-Oct-12

I am not planning to do too much to clean up the configurations, memory addresses, or function debugging.  That will come later.  However, for the debugger components that appear in the kernel itself I will want to be able to turn those off.

Side note: I may need to change how the assembly config file is set up.  We'll see.

I am going to start with the software interrupt, `0xe1`.  This interrupt should be able to take 2 parameters (located in `rdi` and `rsi`) and use those as `cr3` and `rip`, calling the proper function after saving the context.  Now, about the stack....  I really need to have a ToS pointer in the structure passed in to the debugger.  That would have to be parameter 3 (`rdx`).  This updates my structure:

* Index number current state (implied; 0 = main menu/entry point)
* Current state name
* Address space
* Function address
* Stack to use
* Number of transitions
* Transition table:
  * command
  * short command
  * next state index

Now, thinking about it, the Address space and the Stack to use could both be handled at the module level.  So there is no need to include those here since only one debugger process will ever be running and interacting with the other modules.  So I am back to the original structure:

* Index number current state (implied; 0 = main menu/entry point)
* Current state name (used for the prompt)
* Function address (NULL if not a leaf node)
* Number of transitions
* Transition table:
  * command (used to identify the options available)
  * short command
  * next state index

---

The interrupt target is created and installed (conditionally on compile, of course).

Now, I need some code to call the debugging functions.  This really does not need to be anywhere but the debugger itself, since only the debugger can call a debugging function.

---

That was a quick function.

Now, I want to create a list of modules that are available for debugging.  This will need to have a few elements tracked:

* Module name (should be from the module header)
* Address Space (as discussed above)
* Stack (as discussed above)
* The number of states
* The states array
* Linked List pointers
* Spinlock

Since this needs to be dynamic, the module list will also need to be a linked list.  I should also have a spinlock in case something else wants to do something while structures are being traversed.

---

Just building a state eco-system for 1 state is a pain in the ass in C++.  The nested flexible arrays make that a complete mess.  I may be better off writing this in assembly and have it able to statically compile, but that will be a mess too.

Can I un-nest the arrays and have them distinct?  Of course I can, but is it worth it?  Doing this would make the implementation simpler, but a little harder to register a module into the debugger.  The `DbgModule_t` structure can have pointers (states and transitions) to heap memory and still be relatively clean -- states would have an index range of transitions to allow.

Yes, that is a lot less fussy -- and it is statically built.


---

### 2021-Oct-13

Today I am working on an initialization sequence problem.  Apparently, the initialization array is ordered in the sequence in which they are encountered.  However, I need the heap to initialize before the debugger code is initialized.  Actually, the problem is the debugger initialization, calls for a stack initialization, which expects the heap to be initialized.  I should be OK if I remove the `StackFind()` call from the static init and in turn call it from `DebuggerEarlyInit()`.

---

With that sorted, I need to be able to accept input and change states.  I think I can leverage that from the old OS.

I am getting some poor performance with the debugger.  The whole interface is "laggy".  I believe the problem is the PMM cleaner running at the same time at a high priority (when it really should be low).  A quick comment should be able to confirm my thinking.

It helped but not enough.  Ending the `kInit()` process helped a bit as well.  What I need to do is create a function to set the priorities of processes -- and then make use of that function to drop the priorities of several processes.

Also, the input is getting some really odd characters.  I'll have some work to do.

Or, perhaps I can set the priority at create.


---

### 2021-Oct-14

So, I opted to set the priority on process creation.  It seemed the better scenario.  After a quick update and compile, the debugger seems to be getting better response.

Now, on to getting the modules to print their states and options.

---

Since I am only returning a state, and not the module the state is associated with, I need add a member in the state to indicate its module structure.

---

Working on a possible stack alignment problem:

| Addr | push | value              | pop |
|:----:|:----:|:------------------:|:---:|
| bf70 | rax  | 0xffffa00000001b18 | rax |
| bf68 | rbx  | 0xffffa00000004522 | rbx |
| bf60 | rcx  | 0x0000000000000004 | rcx |
| bf58 | rdx  | 0x0000000000000000 | rdx |
| bf50 | rbp  | 0xffffa00000006030 | rbp |
| bf48 | rsi  | 0xffffa00000001390 | rsi |
| bf40 | rdi  | 0x00000001fffe8000 | rdi |
| bf38 | r8   | 0x0000000000000000 | r8  |
| bf30 | r9   | 0x0000000000000000 | r9  |
| bf28 | r10  | 0x0000000000000000 | r10 |
| bf20 | r11  | 0x0000000000000000 | r11 |
| bf18 | r12  | 0xffffa00000007028 | r12 |
| bf10 | r13  | 0xffffa00000004522 | r13 |
| bf08 | r14  | 0xffffa00000001130 | r14 |
| bf00 | r15  | 0xffffa00000007000 | r15 |
| fff8 | rsp  | 0xffffaf000000bf00 | rsp |
| fff0 | cr3  | 0x00000001fffe8000 | cr3 |

Actually, that all looks good.

The stack right after the interrupt:

```
<bochs:3> print-stack 12
Stack address size 8
 | STACK 0xffffaf000000bf78 [0xffffa000:0x00001b1c] (<unknown>)     <-- rip
 | STACK 0xffffaf000000bf80 [0x00000000:0x00000008] (<unknown>)     <-- cs
 | STACK 0xffffaf000000bf88 [0x00000000:0x00200286] (<unknown>)     <-- rflags
 | STACK 0xffffaf000000bf90 [0xffffaf00:0x0000bfa8] (<unknown>)     <-- rsp
 | STACK 0xffffaf000000bf98 [0x00000000:0x00000010] (<unknown>)     <-- ss
 | STACK 0xffffaf000000bfa0 [0xffffa000:0x00006030] (<unknown>)     <-- skipped value for alignment?
 | STACK 0xffffaf000000bfa8 [0xffffa000:0x000014c6] (<unknown>)     <-- new rsp after iret
 | STACK 0xffffaf000000bfb0 [0x00000000:0x00000000] (<unknown>)
 | STACK 0xffffaf000000bfb8 [0xffffa000:0x00007028] (<unknown>)
 | STACK 0xffffaf000000bfc0 [0xffffa000:0x00006030] (<unknown>)
 | STACK 0xffffaf000000bfc8 [0x00000000:0x00000000] (<unknown>)
 | STACK 0xffffaf000000bfd0 [0x00000000:0x00000000] (<unknown>)
```

The stack right before the `iret`:
```
<bochs:5> print-stack 12
Stack address size 8
 | STACK 0xffffaf000000bf78 [0xffffa000:0x00001b1c] (<unknown>)
 | STACK 0xffffaf000000bf80 [0x00000000:0x00000008] (<unknown>)
 | STACK 0xffffaf000000bf88 [0x00000000:0x00200286] (<unknown>)
 | STACK 0xffffaf000000bf90 [0xffffaf00:0x0000bfa8] (<unknown>)
 | STACK 0xffffaf000000bf98 [0x00000000:0x00000010] (<unknown>)
 | STACK 0xffffaf000000bfa0 [0xffffa000:0x00006030] (<unknown>)
 | STACK 0xffffaf000000bfa8 [0xffffa000:0x000014c6] (<unknown>)
 | STACK 0xffffaf000000bfb0 [0x00000000:0x00000000] (<unknown>)
 | STACK 0xffffaf000000bfb8 [0xffffa000:0x00007028] (<unknown>)
 | STACK 0xffffaf000000bfc0 [0xffffa000:0x00006030] (<unknown>)
 | STACK 0xffffaf000000bfc8 [0x00000000:0x00000000] (<unknown>)
 | STACK 0xffffaf000000bfd0 [0x00000000:0x00000000] (<unknown>)
```

It looks the same to me!

The GDT still looks intact:

```
<bochs:8> info gdt
Global Descriptor Table (base=0xffff800000013380, limit=167):
GDT[0x0000]=??? descriptor hi=0x00000000, lo=0x00000000
GDT[0x0008]=Code segment, base=0x00000000, limit=0x00000fff, Execute/Read, Non-Conforming, Accessed, 64-bit
GDT[0x0010]=Data segment, base=0x00000000, limit=0x00000fff, Read/Write, Accessed
GDT[0x0018]=Code segment, base=0x00000000, limit=0x00000fff, Execute/Read, Non-Conforming, 64-bit
GDT[0x0020]=Data segment, base=0x00000000, limit=0x00000fff, Read/Write
GDT[0x0028]=Data segment, base=0x00000000, limit=0x00000fff, Read/Write, Accessed
GDT[0x0030]=Data segment, base=0x00000000, limit=0x00000fff, Read/Write
GDT[0x0038]=Code segment, base=0x00000000, limit=0x00000fff, Execute/Read, Non-Conforming, 64-bit
GDT[0x0040]=Data segment, base=0x00000000, limit=0x00000fff, Read/Write
GDT[0x0048]=Data segment, base=0x00000000, limit=0x0000ffff, Read/Write
GDT[0x0050]=32-Bit TSS (Available) at 0x00014000, length 0x0006f
GDT[0x0058]=??? descriptor hi=0x00000000, lo=0xffff8000
GDT[0x0060]=Data segment, base=0x00000000, limit=0x0000ffff, Read/Write
GDT[0x0068]=32-Bit TSS (Available) at 0x00014070, length 0x0006f
GDT[0x0070]=??? descriptor hi=0x00000000, lo=0xffff8000
GDT[0x0078]=Data segment, base=0x00000000, limit=0x0000ffff, Read/Write
GDT[0x0080]=32-Bit TSS (Available) at 0x000140e0, length 0x0006f
GDT[0x0088]=??? descriptor hi=0x00000000, lo=0xffff8000
GDT[0x0090]=Data segment, base=0x00000000, limit=0x0000ffff, Read/Write
GDT[0x0098]=32-Bit TSS (Available) at 0x00014150, length 0x0006f
GDT[0x00a0]=??? descriptor hi=0x00000000, lo=0xffff8000
```

So, is there a stack alignment problem prior to the interrupt?  And if so, why have I not seen this before now?  I think the CPU is handling that for me....

Stupid me!!  I used `iret` instead of `iretq`!!!


---

### 2021-Oct-15

I now have my states worked out and can navigate the menus as I wish.  The next thing to sort out is handling the `ksprintf()` function -- which is faulting at the moment.

While I work on that, I will write the debugger function to dump the modules known to the debugger.

---

I have this working rather well at this point.  The last thing I will need to do is actually create debugging functions and hook them into the debugger.  There is also some cleanup to accomplish with the debugger interface -- which will be easy to identify by adding in the debugging stuff.

So, now, the problem with the kernel stuff is several things (MMU) are actually address space focused.  I would like to start with the MMU and be able to dump a page.  To do that, I would need to change the address space.  To do that, I would need some way to identify a process.  I'm not there yet.

The scheduler might be a good place to start.  I should be able to dump all the known processes in the scheduler.  So, `scheduler list all` seems like a good module & command to start with.

---

Now that I have the debugger running even a little bit, I see bugs to address:
* [#492 -- Time used is not being updated properly yet](http://eryjus.ddns.net:3000/issues/492)
* [#493 -- Add address space into the Process listing](http://eryjus.ddns.net:3000/issues/493)
* [#494 -- CPU 0 should not have an idle process](http://eryjus.ddns.net:3000/issues/494)
* [#495 -- The Debugger process is hogging CPU](http://eryjus.ddns.net:3000/issues/495)
* [#496 -- Debugger is not handling all typed commands; dropping some at the end](http://eryjus.ddns.net:3000/issues/496)
* [#497 -- Debugger modules list need alignment](http://eryjus.ddns.net:3000/issues/497)

And that is just listing out one bit of information!

Why have I started into Redmine again now?  Because I have enough of a working kernel at this point to need to capture thoughts so I do not lose them.  I will not be able to do everything at once.  I anticipate finding problems faster than I can solve them.  It was like that with the 32-bit version.

OK, it's late.  More tomorrow.


---

### 2021-Oct-16

So, the question of the day is whether I am going to address the new bugs immediately or move on and collect more.  Of the 6 new issues, 5 are Bugs and 1 is a TODO.  I will not yet work on the TODO.  Of the 5 Bugs, 1 I am unable to complete yet (not enough foundation in place to complete) and 2 more are not related to the debugger.  That will leave [#496](http://eryjus.ddns.net:3000/issues/496) and [#497](http://eryjus.ddns.net:3000/issues/497) that are debugger-related.  Since the point of this version is the debugger, I will focus my attention on any debugger bugs first before I move on.  The other bugs may or may not be addressed in this release -- yet to be determined.

**Sidebar**: at some point, I want to start doing a `doxygen` build and document the code properly using that tool.  The code is getting big enough it will make sense to do that.  This will require me getting started and adding that into the build.  That said, I am not sure at what point I will do that.

Let's start with cleaning up #496.  That turns out to be a bad code consolidation.  I need to move that back into the `...Parse...` functions.

#498 is also working now.

Now, I want to list the processes in the Scheduler's Ready Queue.  This will be something like `scheduler list ready`.  The difference where will be to confirm that they all actually in the READY status.  This should be easy on a single CPU system, but will be a little harder on a multi-CPU system.

At this point, though, I finally have a roadmap to get to about the same place I was with the 32-bit version -- by `v0.0.12`.  From that point, I will likely diverge from the older kernel and take a different route, since this is far closer to a micro-kernel than the other kernel was (monolithic!).

Now, on to the ready list.

I had a ready-made debugging function that I just had to cut and paste into the correct location -- plus a little cleanup.

The next thing to leverage is a dump of a single process.  This command will look like `scheduler show <pid>`.  This one will be a little trickier since I need to pass in a value to the actual debugging output function.

So I did remove the prompt function to get some data back from the debugger module.  I will need to reinstate that in order to get a PID.  Or, I will need to be able to pass a value to the function -- which means I need to know what to pass.  Can I get away with only 1 value?  `scheduler priority <pid> <pty>`....  No, maybe 2?  Maybe change the call to support 4 parameters and then the callee can filter out what they want.  Well, I have room for 3 more parameters in registers.  I tried it; I didn't like it.

I will need to think a while on this.  There does not seem to be a good solution.  The front-runner is to create a prompt callback which will perform the prompt and then return the results of `dbgCmd`.  The problem is that will require memory to be allocated in the kernel and then rely on the calling function to release that memory -- or maybe I can do that in the `libk` published function for the user and consume process memory?  Or get a buffer for the result which is managed by the process?


---

### 2021-Oct-17

Yesterday, which I was "thinking" I worked on documenting the `dbg-ui.cc` file for Doxygen.  I think I have a well documented file now.  Lots of searching about how to document certain aspects of the file (functions, variables, typedefs, etc.) but I think I have a good workable template and don't really give up anything at the start of the file.  I also think it looks cleaner.

At the same time, while rubber-ducking with the comments, I found and resolved several bugs in the code.  So I see benefits here as well.

Back to this whole get a prompt thing.  What I need to do is be able to send the debugger a specific state to prompt.  This will issue the prompt and get the input (which should be able to use the regular functions, maybe protecting the current state).  However, since I am messaging from one process to another and I do not yet have inter-process messaging developed (which will solve this problem), I need a hack to use the kernel to support this.  The flow will need to be something like:
* debugged module creates local buffer for a response
* debugged module asks kernel to allocate space for the response
* debugged module calls internal function to prompt a debugger state and get a response
* debugger gets the input back from the user
* debugger will terminate the input command as usual and copy the singular input into the kernel buffer
* debugger will return to the debugged module
* debugged module will copy the input back into local buffer
* debugged module will release the kernel memory
* debugged module will validate the input and take any appropriate actions

What I do not like here is that there is no shared memory and the data is copied twice.  However, at this stage in the kernel it cannot be helped.

Now, do I really need a state?  Or can I just send a prompt?  I think I can just send a prompt, which of course needs to be copied into the kernel memory.  Fair enough.  That also simplifies the overhead.

---

```
- :> scheduler
scheduler :> show
<pid> :> 0
Dumping the contents of the Process_t structure for pid 0 at ffff900000000018
+-------------------------+----------------------------+
| TOS (last preemption)   | fffff80000003e00           |
| Virtual Address Space   | 0000000001004000           |
| Process Status          |   2: READY                 |
| Process Priority        |   1: IDLE                  |
| Quantum left this slice | 0                          |
| Process ID              | 0                          |
| Command Line            | kInit                      |
| Micros used             | 140737518291954            |
| Wake tick number        | 0                          |
| Pending Error Number    | 0                          |
+-------------------------+----------------------------+
scheduler :>
```

NICE!!

---

At this point, this leaves a conversation about how much farther I should go with the debugger before I move on?  In the kernel, I hae the GDT, IDT, TSS & MMU.  The PMM is also a good structure for including debugging information.  Also the LAPIC with timer counts.  Plus, really everything could benefit from an MMU section.  While the heap seems solid, the heap may not be bad to add as well.

