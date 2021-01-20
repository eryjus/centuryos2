;;===================================================================================================================
;;
;;  entry.s -- Entry point for x86_64 kernel
;;
;;        Copyright (c)  2017-2021 -- Adam Clark
;;        Licensed under "THE BEER-WARE LICENSE"
;;        See License.md for details.
;;
;;  Based on the multiboot specification, the multiboot loader will hand off control to the following file at the
;;  loader location.
;;
;; -----------------------------------------------------------------------------------------------------------------
;;
;;     Date      Tracker  Version  Pgmr  Description
;;  -----------  -------  -------  ----  --------------------------------------------------------------------------
;;  2021-Jan-19  Initial  v0.0.2   ADCL  Initial version
;;
;;===================================================================================================================


                global      entry

                extern      kInit


;;
;; -- this is the entry point for the kernel
;;    --------------------------------------
entry:
                call        kInit


