/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   mptype.h
 *
 *  Author          :   Marc Geilen (m.c.w.geilen@tue.nl)
 *
 *  Date            :   March 23, 2009
 *
 *  Function        :   maxplus type definitions and operations
 *
 *  History         :
 *      23-03-09    :   Initial version.
 *      23-12-09    :   Peter Poplavko. Rewritten. Use C++ tools such as
 *                      inline functions, classes and operator overloading instead
 *                      of C tools.
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

#ifndef BASE_MAXPLUS_MPTYPE_H_INCLUDED
#define BASE_MAXPLUS_MPTYPE_H_INCLUDED

#include <assert.h>
#include "../basic_types.h"
#include "../string/cstring.h"

namespace MaxPlus
{

    typedef CDouble MPThroughput;

    class MPTime
    {
        public:
            MPTime(CDouble val = 1.0e+30)
            {
                myVal = val;
            }

            operator CDouble() const
            {
                return myVal;
            }
            MPTime &operator-();
            MPTime &operator+=(MPTime val);
            MPTime &operator-=(MPTime val);

        private:
            CDouble myVal;
    };

    typedef MPTime MPDelay;

    //==============================
    // MP_MAX()
    //==============================

    inline MPTime MP_MAX(MPTime a, MPTime b)
    {
        return ((a) > (b)) ? (a) : (b);
    }

    inline MPTime MP_MAX(CDouble a, MPTime b)
    {
        return MP_MAX(MPTime(a), b);
    }

    inline MPTime MP_MAX(MPTime a, CDouble b)
    {
        return MP_MAX(a, MPTime(b));
    }

    inline CDouble MP_MAX(CDouble a, CDouble b)
    {
        return CDouble(MP_MAX(MPTime(a), MPTime(b)));
    }

    //==============================
    // MP_MIN()
    //==============================

    inline MPTime MP_MIN(MPTime a, MPTime b)
    {
        return ((a) < (b)) ? (a) : (b);
    }

    inline MPTime MP_MIN(CDouble a, MPTime b)
    {
        return MP_MIN(MPTime(a), b);
    }

    inline MPTime MP_MIN(MPTime a, CDouble b)
    {
        return MP_MIN(a, MPTime(b));
    }

    inline CDouble MP_MIN(CDouble a, CDouble b)
    {
        return CDouble(MP_MIN(MPTime(a), MPTime(b)));
    }

    //==============================
    // MP_INFINITY
    //==============================

    // the quick and dirty way of representing -infinity
    const MPTime MP_MINUSINFINITY = -1.0e+30;

    inline bool MP_ISMINUSINFINITY(CDouble a)
    {
        return a < -1.0e+20;
    }

    inline const MPTime MP_PLUS(CDouble a, CDouble b)
    {
        return (MP_ISMINUSINFINITY(a) || MP_ISMINUSINFINITY(b)) ? MP_MINUSINFINITY : (MPTime)((CDouble)a + (CDouble)b);
    }


    // MaxPlus epsilon (used to compare floating point numbers for equality)
    const MPTime MP_EPSILON = 1e-10;


    //==============================
    // MPTime operators
    //==============================
    inline const MPTime operator+(MPTime a, MPTime b)
    {
        return MP_PLUS(a, b);
    }

    inline const MPTime operator+(MPTime a, CDouble b)
    {
        return MPTime(a) + MPTime(b);
    }

    inline const MPTime operator+(CDouble a, MPTime b)
    {
        return MPTime(a) + MPTime(b);
    }

    inline const MPTime operator-(MPTime a, MPTime b)
    {
        assert(!MP_ISMINUSINFINITY(b));
        return a + MPTime(-b);
    }

    inline const MPTime operator-(MPTime a, CDouble b)
    {
        return MPTime(a) - MPTime(b);
    }

    inline const MPTime operator-(CDouble a, MPTime b)
    {
        return MPTime(a) - MPTime(b);
    }

    inline MPTime &MPTime::operator-()
    {
        assert(!MP_ISMINUSINFINITY(myVal));
        myVal = -myVal;
        return *this;
    }

    inline const MPTime operator*(MPTime a, MPTime b)
    {
        if (MP_ISMINUSINFINITY(a))
        {
            assert(b > 0);
            return MP_MINUSINFINITY;
        }
        if (MP_ISMINUSINFINITY(b))
        {
            assert(a > 0);
            return MP_MINUSINFINITY;
        }
        return CDouble(a) * CDouble(b);
    }

    inline const MPTime operator*(CDouble a, MPTime b)
    {
        return MPTime(a) * MPTime(b);
    }

    inline const MPTime operator*(MPTime a, CDouble b)
    {
        return MPTime(a) * MPTime(b);
    }

    inline MPTime &MPTime::operator+=(MPTime a)
    {
        *this = *this + a;
        return *this;
    }

    inline MPTime &MPTime::operator-=(MPTime a)
    {
        assert(!MP_ISMINUSINFINITY(a));
        *this = *this + MPTime(CDouble(-a));
        return *this;
    }


    //==============================
    // toString
    //==============================

    inline const CString timeToString(MPTime val)
    {
        // We intentionally dont use MP_ISMINUSINFINITY(val) here,
        // so that we can expose the unwanted "impure" infinities here.
        //
        if (val == MP_MINUSINFINITY)
        {
            return CString("-mp_inf");
        }
        else return CString(val);
    }


}
#endif