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


                extern      ProcessInit
                extern      sch_Tick
                extern      sch_ProcessBlock
                extern      sch_ProcessReady
                extern      sch_ProcessUnblock
                extern      sch_ProcessMicroSleepUntil


                section     .text


;;
;; -- Set up the header structure for parsing from the kernel
;;    -------------------------------------------------------
header:
                db          'C','e','n','t','u','r','y',' ','O','S',' ','6','4',0,0,0   ; Sig
                db          'S','C','H','E','D','U','L','E','R',0,0,0,0,0,0,0           ; Name
                dq          ProcessInit                                                 ; Early Init
                dq          0                                                           ; Late Init
                dq          0                                                           ; interrupts
                dq          5                                                           ; internal Services
                dq          0                                                           ; OS services
                dq          25                                                          ; Interrupt 32 (IRQ0)
                dq          sch_Tick                                                    ; .. target address
                dq          26                                                          ; Self-block
                dq          sch_ProcessBlock                                            ; .. target address
                dq          27                                                          ; Ready a process
                dq          sch_ProcessReady                                            ; .. target address
                dq          28                                                          ; Unblock a blocked process
                dq          sch_ProcessUnblock                                          ; .. target address
                dq          29                                                          ; Sleep until specified tick count
                dq          sch_ProcessMicroSleepUntil                                  ; .. target address



