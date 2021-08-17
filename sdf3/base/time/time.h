/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   time.h
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
 * $Id: time.h,v 1.1.1.1.2.2 2009-03-23 08:07:13 sander Exp $
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

#ifndef BASE_TIME_TIME_H_INCLUDED
#define BASE_TIME_TIME_H_INCLUDED

#include "../basic_types.h"
#include "../string/cstring.h"

#ifdef _MSC_VER

#include <windows.h>

typedef struct _CTimer
{
    FILETIME rStart;
    FILETIME rEnd;
    FILETIME time;
} CTimer;

#else

#include <time.h>
#include <sys/resource.h>

typedef struct _CTimer
{
    struct rusage rStart;
    struct rusage rEnd;
    struct timeval time;
} CTimer;

#endif

/**
 * startTimer ()
 * The function start measuring the elapsed user and system time
 * from this point onwards.
 */
void startTimer(CTimer *t);

/**
 * stopTimer ()
 * The function stops measuring the elapsed user and system time and
 * it computes the total elapsed time. This time is accesible via
 * t->time (struct timeval).
 */
void stopTimer(CTimer *t);

/**
 * printTimer ()
 * The function prints the value of the timer in milliseconds to the supplied
 * output stream.
 */
void printTimer(ostream &out, CTimer *t);

/**
 * printTimer ()
 * The function returns the value of the timer in milliseconds as a string.
 */
CString printTimer(CTimer *t);

/**
 * secondsElapsed ()
 * The function returns the number of seconds elapsed
 */
unsigned int secondsElapsed(CTimer *t);


#endif

