//===================================================================================================================
//  kprintf.cc -- Write a formatted string to the serial port COM1 (like printf)
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  Write a formatted string to the serial port.  This function works similar to `printf()` from the C runtime
//  library.  I used to have a version publicly available, but I think it better to have a purpose-built version.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Jul-08  Initial   0.1.0   ADCL  Initial version
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================



#include "types.h"
#include "serial.h"
#include "printf.h"


//
// -- Several flags
//    -------------
enum {
    ZEROPAD = 1<<0,            /* pad with zero */
    SIGN    = 1<<1,            /* unsigned/signed long */
    PLUS    = 1<<2,            /* show plus */
    SPACE   = 1<<3,            /* space if plus */
    LEFT    = 1<<4,            /* left justified */
    SPECIAL = 1<<5,            /* 0x */
    LARGE   = 1<<6,            /* use 'ABCDEF' instead of 'abcdef' */
};


//
// -- Variable argument types
//    -----------------------
typedef char *va_list;
#define va_start()      (curParm = 1)
#define va_arg(t)       (*(t *)&p[curParm ++])
#define va_end()


//
// -- Used for Hex numbers
//    --------------------
static const char *digits = "0123456789abcdefghijklmnopqrstuvwxyz";
static const char *upper_digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";


//
// -- This is a printf()-like function to print to the serial port
//    ------------------------------------------------------------
int kprintf(const char *fmt, ...)
{
    uint8_t pCnt;
    void *p[6];

    __asm volatile(" \
        mov %%rdi,%0\n \
        mov %%rsi,%1\n \
        mov %%rdx,%2\n \
        mov %%rcx,%3\n \
        mov %%r8,%4 \n \
        mov %%r9,%5 \n \
        mov %%al,%6 \n \
    " : "=m"(p[0]), "=m"(p[1]), "=m"(p[2]), "=m"(p[3]), "=m"(p[4]), "=m"(p[5]), "=m"(pCnt));


    if (pCnt > 6) {
        kprintf("ERROR: Too many `kprintf()` parameters; use 6 or less!\n");
        return 0;
    }

   int printed = 0;
   int curParm = 1;
    const char *dig = digits;

    va_start();

    for ( ; *fmt; fmt ++) {
        // -- for any character not a '%', just print the character
        if (*fmt != '%') {
            SerialPutChar(*fmt);
            printed ++;
            continue;
        }

        // -- we know the character is a '%' char at this point
        fmt ++;
        if (!*fmt) goto exit;

        // --Begin a new scope
        {
           int fmtDefn = 1;
           int flags = 0;
            bool isLong = false;

            if (isLong) {}          // TODO: remove this

            // -- we need to check for the format modifiers, starting with zero-fill
            if (*fmt == '0') {
                flags |= ZEROPAD;
                fmtDefn ++;
                fmt ++;
            }

            if (*fmt == 'l') {
                isLong = true;
                fmtDefn ++;
                fmt ++;
            }

            // -- now, get to the bottom of a formatted value
            switch (*fmt) {
            default:
                fmt -= fmtDefn;
                // fall through

            case '%':
                SerialPutChar('%');
                printed ++;
                continue;

            case 'c': {
               int c = va_arg(int);
                SerialPutChar(c & 0xff);
                continue;
            }

            case 's': {
                char *s = va_arg(char *);
                if (!s) s = (char *)"<NULL>";
                while (*s) {
                    SerialPutChar(*s ++);
                    printed ++;
                }
                continue;
            }

            case 'P':
                flags |= LARGE;
                dig = upper_digits;
                // fall through

            case 'p':
                {
                    Addr_t val = va_arg(Addr_t);
                    SerialPutChar('0');
                    SerialPutChar('x');
                    printed += 2;

                    for (int j = sizeof(Addr_t) * 8 - 4; j >= 0; j -= 4) {
                        SerialPutChar(dig[(val >> j) & 0x0f]);
                        printed ++;
                    }

                    break;
                }

            case 'X':
                flags |= LARGE;
                dig = upper_digits;
                // fall through

            case 'x':
                {
                    Addr_t val = va_arg(Addr_t);
                    SerialPutChar('0');
                    SerialPutChar('x');
                    printed += 2;

                    bool allZero = true;

                    for (int j = sizeof(Addr_t) * 8 - 4; j >= 0; j -= 4) {
                       int ch = (val >> j) & 0x0f;
                        if (ch != 0) allZero = false;
                        if (!allZero || flags & ZEROPAD) {
                            SerialPutChar(dig[ch]);
                            printed ++;
                        }
                    }

                    if (allZero && !(flags & ZEROPAD)) {
                        SerialPutChar('0');
                        printed ++;
                    }

                    break;
                }

            case 'd':
                {
                   int val = va_arg(int64_t);
                    char buf[30];
                   int i = 0;

                    if (val < 0) {
                        SerialPutChar('-');
                        printed ++;
                        val = -val;
                    }

                    if (val == 0) {
                        SerialPutChar('0');
                        printed ++;
                    } else {
                        while (val) {
                            buf[i ++] = (val % 10) + '0';
                            val /= 10;
                        }

                        while (--i >= 0) {
                            SerialPutChar(buf[i]);
                            printed ++;
                        }
                    }

                    break;
                }

            case 'u':
                {
                    uint64_t val;

                    val = va_arg(uint64_t);
                    char buf[30];
                   int i = 0;

                    if (val == 0) {
                        SerialPutChar('0');
                        printed ++;
                    } else {
                        while (val) {
                            buf[i ++] = (val % 10) + '0';
                            val /= 10;
                        }

                        while (--i >= 0) {
                            SerialPutChar(buf[i]);
                            printed ++;
                        }
                    }

                    break;
                }
            }
        }
    }

exit:
    va_end();

    return printed;
}
