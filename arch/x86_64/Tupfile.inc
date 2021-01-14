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
#CC = clang
CC = x86_64-elf-gcc
LD = ld.lld


##
## -- Add some options to the build
##    -----------------------------
CFLAGS += -mno-red-zone
CFLAGS += -fno-pic


##
## -- The macros that are used for the build
##    --------------------------------------
!as =       |> nasm -felf64 $(NFLAGS) $(CCDEFINE)  -o %o %f     |> %B.o
!cc =       |> $(CC) $(CFLAGS) $(CCDEFINE) -o %o %f             |> %B.o


