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
;;  2021-Oct-10  Initial  v0.0.10  ADCL  Initial version
;;
;;===================================================================================================================


        global  header

        extern  RtcInit
        extern  rtc_GetTime


%include        'constants.inc'

        section .text


;;
;; -- Set up the header structure for parsing from the kernel
;;    -------------------------------------------------------
header:
        db      'C','e','n','t','u','r','y',' ','O','S',' ','6','4',0,0,0   ;; Sig
        db      'R','T','C',0,0,0,0,0,0,0,0,0,0,0,0,0                       ;; Name
        dq      RtcInit                                                     ;; Early Init
        dq      0                                                           ;; Late Init
        dq      0xffffaf4000000000                                          ;; Stack Locations
        dq      0                                                           ;; interrupts
        dq      0                                                           ;; internal Services
        dq      1                                                           ;; OS services
        dq      SVC_RTCTIME                                                 ;; OS Service 1
        dq      rtc_GetTime                                                 ;; .. target address
        dq      0                                                           ;; .. stack



