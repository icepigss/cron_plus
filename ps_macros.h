#ifndef _H_PS_MACROS_
#define _H_PS_MACROS_

#define __sum(a, b) ((a)+(b))

#define __max(a,b)  (((a) > (b)) ? (a) : (b))

#define __min(a,b)  (((a) < (b)) ? (a) : (b))

#define Skip_Terms(c, f, term) \
    while (strchr(terms, c)) \
        c = get_str(f);

#define Skip_Line(c, f) \
    while (c != '\n') \
        c = get_str(f);

#define Set_LineNum(ln) \
    LineNumber = ln;

#define Get_LineNum(ln) \
    return LineNumber;

#define Tran_Zero(n) \
	((n == 0) ? TIME_MAP_LEN-3 : n)

#define get_gmtoff(c, t)    ((t)->tm_gmtoff)

#define SECONDS_PER_MINUTE  60

#endif
