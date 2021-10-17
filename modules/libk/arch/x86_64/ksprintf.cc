//====================================================================================================================
//
//  ksprintf.cc -- An `sprintf()` implementation
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  WARNING: This function makes no attempt to prevent overflowing the buffer.  It will be the caller's
//  to ensure that this does not happen.
//
//  -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-Oct-10  Initial  v0.0.10  ADCL  Initial version
//
//===================================================================================================================



#include "types.h"
#include "kernel-funcs.h"



//
// -- Variable argument types
//    -----------------------
typedef char *va_list;
#define va_start()      (curParm = 1)
#define va_arg(t)       (*(t *)&p[curParm ++])
#define va_end()


//
// -- This is an sprintf implementation
//    ----------------------------------
char *ksprintf(char *buf, const char *fmt, ...)
{
    uint8_t pCnt;
    void *p[5];

    __asm volatile(" \
        mov %%rsi,%0\n \
        mov %%rdx,%1\n \
        mov %%rcx,%2\n \
        mov %%r8,%3\n \
        mov %%r9,%4 \n \
        mov %%al,%5 \n \
    " : "=m"(p[0]), "=m"(p[1]), "=m"(p[2]), "=m"(p[3]), "=m"(p[4]), "=m"(pCnt));


    if (!assert(pCnt <= 5)) return buf;
    if (!buf) return buf;
    if (!fmt) {
        buf[0] = 0;
        return buf;
    }

    int cnt = 0;
    const char *digits = "0123456789abcdefghijklmnopqrstuvwxyz";
    const char *DIGITS = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int curParm = 1;

    va_start();

    for (; *fmt; fmt ++) {
        bool leftJustify = false;
//        bool leadingZeros = false;
        bool showBase = false;
        bool big = false;
        int width = 0;          // minimum
        int precision = 999999; // maximum
        const char *dig = digits;


        // -- check for special character sequences
        if (*fmt == '\\') {
            char ch = 0;

            fmt ++;

            switch (*fmt) {
            case '\\':
                buf[cnt ++] = '\\';
                continue;

            case '"':
                buf[cnt ++] = '"';
                continue;

            case 't':
                buf[cnt ++] = '\t';
                continue;

            case 'n':
                buf[cnt ++] = '\n';
                continue;

            case 'r':
                buf[cnt ++] = '\r';
                continue;

            case 'x':
            case 'X':
                fmt ++;

                if (*fmt >= '0' && *fmt <= '9') ch = ch * 16 + (*fmt - '0');
                else if (*fmt >= 'a' && *fmt <= 'f') ch = ch * 16 + (*fmt - 'a' + 10);
                else if (*fmt >= 'A' && *fmt <= 'F') ch = ch * 16 + (*fmt - 'A' + 10);

                fmt ++;

                if (*fmt >= '0' && *fmt <= '9') ch = ch * 16 + (*fmt - '0');
                else if (*fmt >= 'a' && *fmt <= 'f') ch = ch * 16 + (*fmt - 'a' + 10);
                else if (*fmt >= 'A' && *fmt <= 'F') ch = ch * 16 + (*fmt - 'A' + 10);

                buf[cnt ++] = ch;
                continue;

            default:
                continue;
            }
        }

        // -- first, take care of anything other than the '%'
        if (*fmt != '%') {
            buf[cnt ++] = *fmt;
            continue;
        }

        // -- if we get here, we know we have a '%' format specifier; just increment past it
        fmt ++;

        // -- now, check for another '%'
        if (*fmt == '%') {
            buf[cnt ++] = '%';
            continue;
        }

        // -- now, check if we will left justify
        if (*fmt == '-') {
            leftJustify = true;
            fmt ++;
        }

        // -- now we check for width digits; assume width for now
        while (*fmt >= '0' && *fmt <= '9') {
            width = width * 10 + (*fmt - '0');
            fmt ++;
        }

        // -- check for a switch to precision
        if (*fmt == '.') {
            fmt ++;
            precision = 0;
        }

        // -- now we check for precision digits; assume width for now
        while (*fmt >= '0' && *fmt <= '9') {
            precision = precision * 10 + (*fmt - '0');
            fmt ++;
        }

        if (*fmt == '#') {
            fmt ++;
            showBase = true;
        }

        if (*fmt == 'l') {
            big = true;
            fmt ++;
        }

        // -- Finally we get the the format specifier
        switch (*fmt) {
        case 's':
            {
                const char *s = va_arg(const char *);
                if (!s) s = "(NULL)";

                int ex = width - kStrLen(s);

                if (!leftJustify) {
                    while (ex > 0) {
                        buf[cnt ++] = ' ';
                        ex --;
                    }
                }

                while (*s && precision > 0) {
                    buf[cnt] = *s;
                    cnt ++;
                    s ++;
                    precision --;
                }

                if (leftJustify) {
                    while (ex > 0) {
                        buf[cnt ++] = ' ';
                        ex --;
                    }
                }

                continue;
            }

        case 'P':
            dig = DIGITS;
            // -- fall through

        case 'p':
            {
                Addr_t ptr = va_arg(Addr_t);

                if (showBase) {
                    buf[cnt++] = '0';
                    buf[cnt++] = dig[33];
                }

                for (int i = 15; i >= 0; i --) {
                    buf[cnt] = dig[(ptr >> (i << 2)) & 0xf];
                    cnt ++;
                }

                continue;
            }

        case 'd':
            if (big) {
                long val = va_arg(long);
                int p = 0;
                char n[40];

                if (val == 0) {
                    n[p++] = '0';
                } else {
                    while(val != 0) {
                        int dig = val % 10;
                        n[p++] = '0' + dig;
                        val /= 10;
                    }
                }

                int ex = width - p;

                if (!leftJustify) {
                    while (ex > 0) {
                        buf[cnt ++] = ' ';
                        ex --;
                    }
                }

                p --;

                while (p >= 0) {
                    buf[cnt] = n[p];
                    p --;
                    cnt ++;
                }

                if (leftJustify) {
                    while (ex > 0) {
                        buf[cnt ++] = ' ';
                        ex --;
                    }
                }

                continue;
            } else {
                int val = va_arg(int);
                int p = 0;
                char n[20];

                if (val == 0) {
                    n[p++] = '0';
                } else {
                    while(val != 0) {
                        int dig = val % 10;
                        n[p++] = '0' + dig;
                        val /= 10;
                    }
                }

                int ex = width - p;

                if (!leftJustify) {
                    while (ex > 0) {
                        buf[cnt ++] = ' ';
                        ex --;
                    }
                }

                p --;

                while (p >= 0) {
                    buf[cnt] = n[p];
                    p --;
                    cnt ++;
                }

                if (leftJustify) {
                    while (ex > 0) {
                        buf[cnt ++] = ' ';
                        ex --;
                    }
                }

                continue;
            }

        case 'u':
            if (big) {
                unsigned long val = va_arg(unsigned long);
                int p = 0;
                char n[40];

                if (val == 0) {
                    n[p++] = '0';
                } else {
                    while(val != 0) {
                        int dig = val % 10;
                        n[p++] = '0' + dig;
                        val /= 10;
                    }
                }

                int ex = width - p;

                if (!leftJustify) {
                    while (ex > 0) {
                        buf[cnt ++] = ' ';
                        ex --;
                    }
                }

                p --;

                while (p >= 0) {
                    buf[cnt] = n[p];
                    p --;
                    cnt ++;
                }

                if (leftJustify) {
                    while (ex > 0) {
                        buf[cnt ++] = ' ';
                        ex --;
                    }
                }

                continue;
            } else {
                unsigned int val = va_arg(unsigned int);
                int p = 0;
                char n[20];

                if (val == 0) {
                    n[p++] = '0';
                } else {
                    while(val != 0) {
                        int dig = val % 10;
                        n[p++] = '0' + dig;
                        val /= 10;
                    }
                }

                int ex = width - p;

                if (!leftJustify) {
                    while (ex > 0) {
                        buf[cnt ++] = ' ';
                        ex --;
                    }
                }

                p --;

                while (p >= 0) {
                    buf[cnt] = n[p];
                    p --;
                    cnt ++;
                }

                if (leftJustify) {
                    while (ex > 0) {
                        buf[cnt ++] = ' ';
                        ex --;
                    }
                }

                continue;
            }
        }
    }


    // -- finally NULL terminate the string
    va_end();
    buf[cnt] = 0;
    return buf;
}





