#####################################################################################################################
##
##  Tupfile -- An alternative to 'make` build system -- build the object files for the arm architecture
##
##        Copyright (c)  2017-2021 -- Adam Clark
##        Licensed under "THE BEER-WARE LICENSE"
##        See License.md for details.
##
## -----------------------------------------------------------------------------------------------------------------
##
##     Date      Tracker  Version  Pgmr  Description
##  -----------  -------  -------  ----  ---------------------------------------------------------------------------
##  2021-Jan-02  Initial  v0.0.1   ADCL  Initial version
##
#####################################################################################################################


##
## -- These are the commands that will be used to build for that arm architecture
##    ---------------------------------------------------------------------------
AS = nasm
CC = clang
LD = ld.lld
#CC = x86_64-elf-gcc -fno-strict-aliasing
#LD = x86_64-elf-ld


##
## -- Add some options to the build
##    -----------------------------
CFLAGS += -mno-red-zone
CFLAGS += -fno-pic
CFLAGS += -ffreestanding
CFLAGS += -mcmodel=large
CFLAGS += -mno-mmx
CFLAGS += -mno-sse
CFLAGS += -mno-sse2


# LDFLAGS += --verbose=1


##
## -- The macros that are used for the build
##    --------------------------------------
!as = | $(WS)/targets/$(ARCH)-$(PLAT)/usr/include/constants.inc     |> nasm -felf64 $(NFLAGS) $(CCDEFINE)  -o %o %f     |> %B.o
!cc = | $(DEPS)                                                         |> $(CC) $(CFLAGS) $(CCDEFINE) -o %o %f             |> %B.o


