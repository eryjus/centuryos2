;;====================================================================================================================
;;
;;  internal-dispatch.h -- dispach an internal function call
;;
;;        Copyright (c)  2017-2021 -- Adam Clark
;;        Licensed under "THE BEER-WARE LICENSE"
;;        See License.md for details.
;;
;;  -----------------------------------------------------------------------------------------------------------------
;;
;;     Date      Tracker  Version  Pgmr  Description
;;  -----------  -------  -------  ----  ---------------------------------------------------------------------------
;;  2021-Jan-24  Initial  v0.0.3   ADCL  Initial version
;;
;;===================================================================================================================


                global  InternalDispatch
                global  InternalDispatch0
                global  InternalDispatch1
                global  InternalDispatch2
                global  InternalDispatch3
                global  InternalDispatch4
                global  InternalDispatch5


;;
;; -- dispatch an internal function call
;;    ----------------------------------
InternalDispatch:
InternalDispatch0:
InternalDispatch1:
InternalDispatch2:
InternalDispatch3:
InternalDispatch4:
InternalDispatch5:
                int     0xc8
                ret


