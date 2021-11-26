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

        extern  MsgqEarlyInit
        extern  msg_MsgqGet
        extern  msg_MsgqCtl
        extern  msg_MsgqSnd
        extern  msg_MsgqRcv


%include        'constants.inc'

        section .text


;;
;; -- Set up the header structure for parsing from the kernel
;;    -------------------------------------------------------
header:
        db      'C','e','n','t','u','r','y',' ','O','S',' ','6','4',0,0,0   ;; Sig
        db      'M','S','G','Q',0,0,0,0,0,0,0,0,0,0,0,0                     ;; Name
        dq      MsgqEarlyInit                                               ;; Early Init
        dq      0                                                           ;; Late Init
        dq      0                                                           ;; interrupts
        dq      0                                                           ;; internal Services
        dq      4                                                           ;; OS services
        dq      SVC_MSGGET                                                  ;; OS Service 1
        dq      msg_MsgqGet                                                 ;; .. target address
        dq      SVC_MSGCTL                                                  ;; OS Service 2
        dq      msg_MsgqCtl                                                 ;; .. target address
        dq      SVC_MSGSND                                                  ;; OS Service 3
        dq      msg_MsgqSnd                                                 ;; .. target address
        dq      SVC_MSGRCV                                                  ;; OS Service 4
        dq      msg_MsgqRcv                                                 ;; .. target address



