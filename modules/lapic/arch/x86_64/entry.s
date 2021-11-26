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
                extern      ipi_LapicGetId
                extern      ipi_SendInit
                extern      ipi_SendSipi
                extern      ipi_SendIpi

%include        'constants.inc'

                section     .text


;;
;; -- Set up the header structure for parsing from the kernel
;;    -------------------------------------------------------
header:
                db          'C','e','n','t','u','r','y',' ','O','S',' ','6','4',0,0,0   ;; Sig
                db          'x','2','A','P','I','C',0,0,0,0,0,0,0,0,0,0                 ;; Name
                dq          X2ApicInitEarly                                             ;; Early Init
                dq          Init                                                        ;; Late Init
                dq          0                                                           ;; interrupts
                dq          8                                                           ;; internal Services
                dq          0                                                           ;; OS services
                dq          INT_TMR_CURRENT_COUNT                                       ;; Internal fctn 0x040 (Tmr Cnt)
                dq          tmr_GetCurrentTimer                                         ;; .. target address
                dq          INT_TMR_TICK                                                ;; Internal fctn 0x041 (Tick)
                dq          tmr_Tick                                                    ;; .. target address
                dq          INT_TMR_EOI                                                 ;; Internal fctn 0x042 (EOI)
                dq          tmr_Eoi                                                     ;; .. target address
                dq          INT_TMR_REINIT                                              ;; Internal fctn 0x043 (reInit)
                dq          X2ApicInitEarly                                             ;; .. target address
                dq          INT_IPI_CURRENT_CPU                                         ;; Internal fctn 0x080 (LAPICID)
                dq          ipi_LapicGetId                                              ;; .. target address
                dq          INT_IPI_SEND_INIT                                           ;; Internal fctn 0x081 (INIT)
                dq          ipi_SendInit                                                ;; .. target address
                dq          INT_IPI_SEND_SIPI                                           ;; Internal fctn 0x082 (SIPI)
                dq          ipi_SendSipi                                                ;; .. target address
                dq          INT_IPI_SEND_IPI                                            ;; Internal fctn 0x082 (SIPI)
                dq          ipi_SendIpi                                                 ;; .. target address

