/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   basic_types.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   June 18, 2002
 *
 *  Function        :   Basic type definitions
 *
 *  History         :
 *      18-06-04    :   Initial version.
 *      26-08-06    :   Additions by Bart Theelen.
 *
 * $Id: basic_types.h,v 1.7.2.2 2009-11-17 13:50:15 mgeilen Exp $
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

#ifndef BASE_BASIC_TYPES_H_INCLUDED
#define BASE_BASIC_TYPES_H_INCLUDED

#include <assert.h>
#include "float.h"

/* STL functionality */
#include <fstream>
#include <iostream>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <queue>
#include <algorithm>

using std::list;
using std::set;
using std::vector;
using std::map;
using std::istream;
using std::ostream;
using std::ifstream;
using std::ofstream;
using std::fstream;
using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::queue;

/* basic types */
typedef unsigned int            uint;
typedef std::vector<int>        v_int;
typedef std::vector<uint>       v_uint;

/* Id */
typedef uint CId;
typedef unsigned long long      CSize;

/* Doubles */
typedef double                  CDouble;

/* Queue */
typedef queue<CId>              CQueue;

#ifdef _MSC_VER
typedef unsigned long           ulong;
#endif

#define CID_MAX                 UINT_MAX

#ifdef _MSC_VER
#define CSIZE_MAX               ULONG_MAX
#define LONG_LONG_MAX           LONG_MAX
#define LONG_LONG_MIN           LONG_MIN

#else
#define CSIZE_MAX               ULONG_LONG_MAX
#endif

/**
 *
*  AUTO_DELETE:  Self-deleting pointer (when going out of scope or changing values)
*
*  int function() {
*     ...
*     AUTO_DELETE<Class1> c1;
*     if (cond1) {
*       c1_ptr = < function that returns new Class1 >
*     }
*     ...
*     c1_ptr = func2(); // previous c1_ptr deleted automatically
*     ...
*     if (cond2) {
*       return 0; // c1_ptr data deleted automatically
*     }
*     ...
*  }
*/

template< class T>
class AUTO_DELETE
{
    public:
        AUTO_DELETE(T *iniPtr = NULL)
        {
            myPtr = iniPtr;
        }
        T *operator=(T *newPtr)
        {
            destroy();
            myPtr = newPtr;
            return newPtr;
        }

        operator T *() const
        {
            return myPtr;
        }

        T *operator->() const
        {
            return myPtr;
        }

        ~AUTO_DELETE()
        {
            destroy();
        }

    private:
        void destroy()
        {
            if (myPtr) delete myPtr;
            myPtr = NULL;
        }

    private:
        // Copy: intentionally private and empty
        AUTO_DELETE(const AUTO_DELETE &);
        AUTO_DELETE &operator=(const AUTO_DELETE &);

    private:
        T *myPtr;
};

/**
*  TriBool
*    'tristate boolean' - has 'undefined' state.
*/

class TriBool
{
    public:
        TriBool()
        {
            defined = false;
            val = false;
        }
        TriBool(bool setVal)
        {
            defined = true;
            val = setVal;
        }
        operator bool() const
        {
            assert(defined);
            return val;
        }
        bool isDefined() const
        {
            return defined;
        }
    private:
        bool defined, val;
};

#endif

