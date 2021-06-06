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
                extern      tmr_Interrupt
                extern      tmr_GetCurrentTimer

                section     .text


;;
;; -- Set up the header structure for parsing from the kernel
;;    -------------------------------------------------------
header:
                db          'C','e','n','t','u','r','y',' ','O','S',' ','6','4',0,0,0   ; Sig
                db          'x','2','A','P','I','C',0,0,0,0,0,0,0,0,0,0                 ; Name
                dq          X2ApicInitEarly                                             ; Early Init
                dq          Init                                                        ; Late Init
                dq          1                                                           ; interrupts
                dq          1                                                           ; internal Services
                dq          0                                                           ; OS services
                dq          32                                                          ; Interrupt 32 (IRQ0)
                dq          tmr_Interrupt                                               ; .. target address
                dq          13                                                          ; Internal fctn 13 (Tmr Cnt)
                dq          tmr_GetCurrentTimer                                         ; .. target address


