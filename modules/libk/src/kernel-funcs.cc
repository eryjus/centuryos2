//====================================================================================================================
//
//  kernel-funcs.cc -- Functions to interface with function calls
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-Feb-16  Initial  v0.0.6   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "kernel-funcs.h"



//
// -- A hack for kStrCmp
//    ------------------
int kStrCmp(const char *str1, const char *str2)
{
    if (str1 == NULL && str2 == NULL) return 0;
    if (str1 == NULL) return 1;
    if (str2 == NULL) return -1;

    do {
        int rv = *str1 - *str2;

        if (rv != 0) return rv;
    } while (*str1 ++ && *str2 ++);

    return 0;
}




