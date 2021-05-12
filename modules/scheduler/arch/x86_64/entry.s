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


                section     .text


;;
;; -- Set up the header structure for parsing from the kernel
;;    -------------------------------------------------------
header:
                db          'C','e','n','t','u','r','y',' ','O','S',' ','6','4',0,0,0   ; Sig
                db          'S','C','H','E','D','U','L','E','R',0,0,0,0,0,0,0           ; Name
                dq          0                                                           ; Early Init
                dq          0                                                           ; Late Init
                dq          0                                                           ; interrupts
                dq          0                                                           ; internal Services
                dq          0                                                           ; OS services



