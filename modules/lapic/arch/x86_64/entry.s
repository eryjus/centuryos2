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
                extern      tmr_GetCurrentTimer
                extern      tmr_Tick
                extern      tmr_Eoi

                section     .text


;;
;; -- Set up the header structure for parsing from the kernel
;;    -------------------------------------------------------
header:
                db          'C','e','n','t','u','r','y',' ','O','S',' ','6','4',0,0,0   ;; Sig
                db          'x','2','A','P','I','C',0,0,0,0,0,0,0,0,0,0                 ;; Name
                dq          X2ApicInitEarly                                             ;; Early Init
                dq          Init                                                        ;; Late Init
                dq          0xffffaf4000000000                                          ;; Stack Locations
                dq          0                                                           ;; interrupts
                dq          3                                                           ;; internal Services
                dq          0                                                           ;; OS services
                dq          0x040                                                       ;; Internal fctn 0x040 (Tmr Cnt)
                dq          tmr_GetCurrentTimer                                         ;; .. target address
                dq          0                                                           ;; .. stack
                dq          0x041                                                       ;; Internal fctn 0x041 (Tick)
                dq          tmr_Tick                                                    ;; .. target address
                dq          0                                                           ;; .. stack
                dq          0x042                                                       ;; Internal fctn 0x042 (EOI)
                dq          tmr_Eoi                                                     ;; .. target address
                dq          0                                                           ;; .. stack

