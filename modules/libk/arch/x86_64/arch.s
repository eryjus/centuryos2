;;===================================================================================================================
;;
;;  arch.s -- Some architecture-specific functions
;;
;;        Copyright (c)  2017-2021 -- Adam Clark
;;        Licensed under "THE BEER-WARE LICENSE"
;;        See License.md for details.
;;
;; -----------------------------------------------------------------------------------------------------------------
;;
;;     Date      Tracker  Version  Pgmr  Description
;;  -----------  -------  -------  ----  --------------------------------------------------------------------------
;;  2021-Feb-14  Initial  v0.0.4   ADCL  Initial version
;;
;;===================================================================================================================


                global      GetAddressSpace


GetAddressSpace:
                xor         rax,rax
                mov         rax,cr3
                ret

