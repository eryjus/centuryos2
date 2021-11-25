***The Century OS***

Century OS is a hobby Operating System Project.  Its objective is to run a *relatively* "common" kernel across multiple architectures.  Currently, I am targeting x86_64.  Additional architectures will be added once I get a relatively stable kernel.

        Copyright (c)  2017-2021 -- Adam Clark (see LICENSE for the specifics)


I chose "Century" as a name because it described me and my project in 2 ways:
1) I am a road bicyclist.  In the US, the bicycling version of a marathon is called a century -- 100 miles at a time.  No, I have no aspirations to ride in the TDF.
2) I expect this OS to take at least 100 years to complete.

Since this OS will be available for others to review and possibly glean nuggets of information, I want to make something very clear: I do not represent my code as the ideal solution for anything.  If you have feedback for me, please feel free to e-mail me at hobbyos@eryjus.com.

As a result putting my own development stages on display in a public repository, I plan to provide smaller commits to my public repository.  The hope behind the large number of smaller commits is that other readers can observe the evolution of my development.  OS development is a large project and many do not know in what order I will take tasks.  The public repository documents my own development path and changes in direction.

If you want to know more about my thinking, browse my Journals in the repository.


***Why?***

I admit, I have had several partial starts, some on my own and some with existing OSes.  This one is no different and you may be familiar with some of them.  So, why this version of the OS and why now?  That's a fair question.

First, I am by no means an expert.  Each system I have started has had specific objectives in mind.  My first incarnation was a basic learning project.  It lacked focus and direction.  However, I was able to get multiple processes running properly.  I also managed to break it and fix it several times.  It was 32-bit.

Then, I embarked on a 64-bit assembly system.  I took it on in assembly because I was not able to figure out how to commingle 32-bit and 64-bit code in C.  Well, if you can't fix it, feature it!  So, I took it on in 100% assembly.  Again, I was able to get multiple processes running.

Then I looked at xv6, a teaching kernel which I refer back to even today.  I took that as a starting point since it had a working filesystem and user-mode applications.  I learn better when I have a working system to look review and dig into.  I was going to slowly replace portions with things of my own invention.  This did not really go anywhere.

Well, after several years away, I decided to pick up the OS again.  I had acquired a Raspberry Pi 2 that I wanted to start playing with and never did.  I got tired of it sitting on my desk staring me down.  So, I started over with another incarnation of Century.
This one was supposed to be a micro-kernel and run on several architectures.  I was able to get the rpi2b and x86 running largely the same code -- but it was monolithic.  It was running well with 4 cpus supported.

Finally, I decided no more living with old technology.  I updated to x64.  I am pulling code from the prior version into this project.  However, I am far closer to a micro-kernel with this iteration.  My plans are to get a reasonable kernel established in x86_64 and then branch out.

The goals here are pretty simple:
1. Document the progress of this OS for those that learn through other's mistakes.  Use GitHub to keep track of my history.
2. Establish a common kernel base source that can be run on multiple architectures, with architecture-specific code handled properly.
3. Write this kernel as a micro-kernel.
4. Write the GUI desktop manager myself, but port as many other components as possible.  Therefore, take time to make the porting process as easy as possible.

But, again, I am no expert.  You might just want to follow this project just to watch the train wreck unfolding!  ;)


***The Build System***

`make` is out!  Well, mostly.

I have converted to `tup` for the bulk of the build system.

At the time, this was a rather large shift in direction with my build system.  I started to use `tup` to execute the core of my build, and then use `make` to do more scripting things.  The reason for the shift is simple: `tup` is FAR easier to maintain than make files, especially for a complicated build like an OS on multiple architectures and platforms.  One of the key reasons for this is that the `Tupfile` is located in the target directory (where `Makefile`s are typically located in the source directories).  Therefore, if there is something that is needed to satisfy a dependency, `tup` only needs to look in the directory where that dependency should be and if it is not there, read the `Tupfile` in that directory on how to create that object.

However, there is also give and take with `tup`.  Since I do not want to clutter up my `sysroot` folders with a bunch of `Tupfile`s that would end up on the .iso or .img, I have implemented the parts that would copy these files into the `sysroot` folders in the only `Makefile`.  The result is MUCH simpler.  I have a default 'all' target that simply calls `tup` to refresh the build.  Any of the other specialized targets (.iso, or running QEMU) depend on the 'all' target and then run the steps needed to complete the script.

The only key function I am giving up is the ability to build an architecture or a module independently.  For the moment, I think I can live with that by creating stub functions when needed.

You can find `tup` at: http://gittup.org/gittup/.

I have updated my core `Makefile` to use all user-level commands to build the boot image.  You no longer need to maintain a list of sudo permissions.  The exception to this would be `dd`ing an image to a thumb drive for booting on real hardware.


***Issue Tracking***

Many of the things I find and log myself are not placed into the public repo bug tracking system.  There are several things that I identify as things to do that I want to write down before I forget them and all the things I keep track of that can clutter up a more public system.  You can visit my issues list at http://eryjus.ddns.net:3000.  Currently, this is read-only.  If you happen to find something and want to e-mail me about it, my e-mail is hobbyos@eryjus.com.

Redmine is an open source issue tracking application can be found here: https://www.redmine.org/.


***Web Page***

I have created a web page for this project.  You can visit the web page at http://centuryos.eryjus.com.  Documentation os the OS (Doxygen generated) is available there.


***Doxygen-generated documentation***

Several documentation components are lifted from the POSIX.1 specification:

```
The Open Group Base Specifications Issue 7, 2018 edition
IEEE Std 1003.1-2017 (Revision of IEEE Std 1003.1-2008)
Copyright © 2001-2018 IEEE and The Open Group
```




