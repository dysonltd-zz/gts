#ifndef WINTIME_H_
#define WINTIME_H_

#include "windows.h"

/** On Windows, the timespec struct is not defined. However, note that the
    timespec struct is also defined in the pthread.h header on Windows, so
    the guard symbols must match here to avoid a duplicate declaration.
 **/

#ifndef HAVE_STRUCT_TIMESPEC
#define HAVE_STRUCT_TIMESPEC 1

    struct timespec
	{
        long tv_sec;
        long tv_nsec;
    };

#endif /* HAVE_STRUCT_TIMESPEC */

extern int clock_gettime(int X, timeval *tv);

#endif // WINTIME_H
