/****************************************************************************************************************//**
*   @file               time.h
*   @brief              Implementation of the POSIX.1 `<time.h>` specification
*   @author             Adam Clark (hobbyos@eryjus.com)
*   @date               2021-Nov-21
*   @since              v0.0.14
*
*   @copyright          Copyright (c)  2017-2021 -- Adam Clark\n
*                       Licensed under "THE BEER-WARE LICENSE"\n
*                       See \ref LICENSE.md for details.
*
*   This file is the CenturyOS implementation of the POSIX.1 `time.h` specification.  It is intended for use
*   with the CenturyOS libc and is not intended to work on any generic OS.
*
* ------------------------------------------------------------------------------------------------------------------
*
*   |     Date    | Tracker |  Version | Pgmr | Description
*   |:-----------:|:-------:|:--------:|:----:|:--------------------------------------------------------------------
*   | 2021-Nov-21 | Initial |  v0.0.14 | ADCL | Initial version
*
*///=================================================================================================================



#ifndef __TIME_H__
#define __TIME_H__



/****************************************************************************************************************//**
*   @typedef            time_t
*   @brief              Used for time in seconds.
*
*   @posix              The <time.h> header shall define the `clock_t`, `size_t`, `time_t` types as described in
*                       <sys/types.h>.
*
*   `time_t` will be implemented as a `long` (64-bit).
*///-----------------------------------------------------------------------------------------------------------------
#ifndef __time_t_defined
# ifndef __DOXYGEN__
#  define __time_t_defined
# endif
typedef long time_t;
#endif



/****************************************************************************************************************//**
*   @fn                 time_t time(time_t *)
*   @brief              Get the current time, in seconds from Epoch
*
*   The `time()` function obtains the current time in seconds from 1-Jan-1970.
*
*   @param              t               On optional pointer in which to also store the time when not NULL
*
*   @returns            The time in seconds from Epoch
*///-----------------------------------------------------------------------------------------------------------------
time_t time(time_t *);


#endif
