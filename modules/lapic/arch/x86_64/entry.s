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
;;  2021-May-05  Initial  v0.0.8   ADCL  Initial version
;;
;;===================================================================================================================


                global      header

                extern      X2ApicInitEarly
                extern      Init

                section     .text


;;
;; -- Set up the header structure for parsing from the kernel
;;    -------------------------------------------------------
header:
                db          'C','e','n','t','u','r','y',' ','O','S',' ','6','4',0,0,0   ; Sig
                db          'x','2','A','P','I','C',0,0,0,0,0,0,0,0,0,0                 ; Name
                dq          X2ApicInitEarly                                             ; Early Init
                dq          Init                                                        ; Late Init
                dq          0                                                           ; interrupts
                dq          0                                                           ; internal Services
                dq          0                                                           ; OS services


