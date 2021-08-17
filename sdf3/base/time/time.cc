/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   time.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 25, 2005
 *
 *  Function        :   Measure elapsed time for program fragments.
 *
 *  History         :
 *      25-07-05    :   Initial version.
 *
 * $Id: time.cc,v 1.1.2.2 2010-04-24 23:52:42 mgeilen Exp $
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * In other words, you are welcome to use, share and improve this program.
 * You are forbidden to forbid anyone else to use, share and improve
 * what you give them.   Happy coding!
 */

#include "time.h"

#ifdef _MSC_VER

#include <windows.h>
#include <iostream>
#include <sstream>
#include <stdlib.h>

FILETIME FILETIMESum(FILETIME fa, FILETIME fb)
{
    ULARGE_INTEGER a, b, c;
    a.HighPart = fa.dwHighDateTime;
    a.LowPart = fa.dwLowDateTime;
    b.HighPart = fb.dwHighDateTime;
    b.LowPart = fb.dwLowDateTime;
    c.QuadPart = a.QuadPart + b.QuadPart;
    FILETIME fc;
    fc.dwHighDateTime = c.HighPart;
    fc.dwLowDateTime = c.LowPart;
    return fc;
}

FILETIME FILETIMEMinus(FILETIME fa, FILETIME fb)
{
    ULARGE_INTEGER a, b, c;
    a.HighPart = fa.dwHighDateTime;
    a.LowPart = fa.dwLowDateTime;
    b.HighPart = fb.dwHighDateTime;
    b.LowPart = fb.dwLowDateTime;
    c.QuadPart = a.QuadPart - b.QuadPart;
    FILETIME fc;
    fc.dwHighDateTime = c.HighPart;
    fc.dwLowDateTime = c.LowPart;
    return fc;
}

/**
 * startTimer ()
 * The function start measuring the elapsed user and system time
 * from this point onwards.
 */
void startTimer(CTimer *t)
{
    HANDLE currentProcess = GetCurrentProcess();
    FILETIME lpCreationTime;
    FILETIME lpExitTime;
    FILETIME lpKernelTime;
    FILETIME lpUserTime;

    GetProcessTimes(currentProcess, &lpCreationTime, &lpExitTime, &lpKernelTime, &lpUserTime);
    t->rStart = FILETIMESum(lpKernelTime, lpUserTime);
}

/**
 * stopTimer ()
 * The function stops measuring the elapsed user and system time and
 * it computes the total elapsed time. This time is accesible via
 * t->time (struct timeval).
 */
void stopTimer(CTimer *t)
{
    HANDLE currentProcess = GetCurrentProcess();
    FILETIME lpCreationTime;
    FILETIME lpExitTime;
    FILETIME lpKernelTime;
    FILETIME lpUserTime;

    GetProcessTimes(currentProcess, &lpCreationTime, &lpExitTime, &lpKernelTime, &lpUserTime);
    t->rEnd = FILETIMESum(lpKernelTime, lpUserTime);

    // Calculate elapsed time
    t->time = FILETIMEMinus(t->rEnd, t->rStart);
}

/**
 * printTimer ()
 * The function prints the value of the timer in milliseconds to the supplied
 * output stream.
 */
void printTimer(ostream &out, CTimer *t)
{
    ULARGE_INTEGER i;
    i.HighPart = t->time.dwHighDateTime;
    i.LowPart = t->time.dwLowDateTime;
    ULONGLONG ms = i.QuadPart / (ULONGLONG) 10000;
    out << ms << "ms" ;
}

/**
 * printTimer ()
 * The function prints the value of the timer in milliseconds to the supplied
 * output stream.
 */
CString printTimer(CTimer *t)
{
    ULARGE_INTEGER i;
    i.HighPart = t->time.dwHighDateTime;
    i.LowPart = t->time.dwLowDateTime;
    ULONGLONG ms = i.QuadPart / (ULONGLONG) 10000;

    std::ostringstream s1;
    s1 << ms << "ms";

    CString s = CString(s1.str());
    return s;
}

/**
 * secondsElapsed ()
 * The function returns the number of seconds elapsed
 */
unsigned int secondsElapsed(CTimer *t)
{
    ULARGE_INTEGER i;
    i.HighPart = t->time.dwHighDateTime;
    i.LowPart = t->time.dwLowDateTime;
    ULONGLONG ms = i.QuadPart / (ULONGLONG) 10000;
    return (unsigned int)(ms / 1000);
}


#else

/**
 * subtractTimeval()
 * The function accepts 2 time values and computes the difference.
 * X = end and Y = Start.
 */
void subtractTimeval(struct timeval *result, struct timeval *x,
                     struct timeval *y)
{
    unsigned long long diff = x->tv_sec * 1000000 + x->tv_usec;
    diff = diff - (y->tv_sec * 1000000 + y->tv_usec);

    result->tv_sec = diff / 1000000;
    result->tv_usec = diff % 1000000;
}

/**
 * startTimer ()
 * The function start measuring the elapsed user and system time
 * from this point onwards.
 */
void startTimer(CTimer *t)
{
    getrusage(RUSAGE_SELF, &(t->rStart));
}

/**
 * stopTimer ()
 * The function stops measuring the elapsed user and system time and
 * it computes the total elapsed time. This time is accesible via
 * t->time (struct timeval).
 */
void stopTimer(CTimer *t)
{
    getrusage(RUSAGE_SELF, &(t->rEnd));

    // Calculate elapsed time (user + system)
    struct timeval udiff, sdiff;
    subtractTimeval(&(udiff), &(t->rEnd.ru_utime), &(t->rStart.ru_utime));
    subtractTimeval(&(sdiff), &(t->rEnd.ru_stime), &(t->rStart.ru_stime));

    t->time.tv_sec = (((udiff.tv_sec + sdiff.tv_sec) * 1e6)
                      + (udiff.tv_usec + sdiff.tv_usec)) / 1e6;
    t->time.tv_usec = (unsigned long long)(((udiff.tv_sec + sdiff.tv_sec) * 1e6)
                                           + (udiff.tv_usec + sdiff.tv_usec)) % (unsigned long long)1e6;
}

/**
 * printTimer ()
 * The function prints the value of the timer in milliseconds to the supplied
 * output stream.
 */
void printTimer(ostream &out, CTimer *t)
{
    double s = (t->time.tv_sec * (double)(1000))
               + (t->time.tv_usec / (double)(1000));

    out << s << "ms";
}

/**
 * printTimer ()
 * The function returns the value of the timer in milliseconds as a string.
 */
CString printTimer(CTimer *t)
{
    double s = (t->time.tv_sec * (double)(1000))
               + (t->time.tv_usec / (double)(1000));

    return (CString(s) + CString("ms"));
}

unsigned int secondsElapsed(CTimer *t)
{
    return t->time.tv_sec;
}

#endif

