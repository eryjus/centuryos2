;;===================================================================================================================
;;
;;  entry.s -- Entry point for x86_64 architecture
;;
;;        Copyright (c)  2017-2021 -- Adam Clark
;;        Licensed under "THE BEER-WARE LICENSE"
;;        See License.md for details.
;;
;; -----------------------------------------------------------------------------------------------------------------
;;
;;     Date      Tracker  Version  Pgmr  Description
;;  -----------  -------  -------  ----  --------------------------------------------------------------------------
;;  2021-Jan-29  Initial  v0.0.4   ADCL  Initial version
;;
;;===================================================================================================================


                global      header

                extern      PmmInitEarly
                extern      PmmAllocateAligned
                extern      PmmReleaseFrame


                section     .text


;;
;; -- Set up the header structure for parsing from the kernel
;;    -------------------------------------------------------
header:
                db          'C','e','n','t','u','r','y',' ','O','S',' ','6','4',0,0,0   ; Sig
                db          'P','M','M',0,0,0,0,0,0,0,0,0,0,0,0,0                       ; Name
                dq          PmmInitEarly                                                ; Early Init
                dq          0                                                           ; Late Init
                dq          0                                                           ; interrupts
                dq          2                                                           ; internal Services
                dq          0                                                           ; OS services
                dq          10                                                          ; internal function 1
                dq          PmmAllocateAligned                                          ; .. target address
                dq          11                                                          ; internal function 2
                dq          PmmReleaseFrame                                             ; .. target address


PmmRelease:
                ret


