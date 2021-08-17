/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   type.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 22, 1009
 *
 *  Function        :   FSM-based SADF type definitions
 *
 *  History         :
 *      22-04-09    :   Initial version.
 *
 * $Id: type.h,v 1.1.2.2 2010-04-22 07:07:11 mgeilen Exp $
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

#ifndef FSMSADF_BASE_TYPE_H_INCLUDED
#define FSMSADF_BASE_TYPE_H_INCLUDED

#include "component.h"
#include "memorytype.h"

namespace FSMSADF
{

    // Bandwidth
    typedef double Bandwidth;

    // Throughput
    typedef CDouble Throughput;

    // Time
    typedef int Time;

#define TIME_MAX        INT_MAX

    // Size
    typedef uint Size;

#define SIZE_MAX        UINT_MAX

    /**
     * Buffer Location
     */
    class BufferLocation
    {
        public:

            enum Value
            {
                Src = 0,
                Dst,
                Mem,
            };
            BufferLocation(const CString &l)
            {
                _v = asLoc(l);
            }
            BufferLocation(const Value &l)
            {
                _v = l;
            }
            static CString toString(BufferLocation::Value v)
            {
                BufferLocation a = v;
                return a;
            }
            CString asString() const
            {
                if (_v == Src) return "src";
                else if (_v == Dst) return "dst";
                else return "mem";

            }
            Value asLoc(const CString &l) const
            {
                if (l == "src") return Src;
                else if (l == "dst") return Dst;
                else return Mem;
            }

            operator CString() const
            {
                return asString();
            }

            operator int() const
            {
                return _v;
            }
            void operator =(Value const &a)
            {
                _v = a;
            }
            void operator =(CString const &a)
            {
                _v = asLoc(a);
            }
            bool operator ==(Value const &b)
            {
                return this->_v == b;
            }
            bool operator ==(BufferLocation const &b)
            {
                return _v == b._v;
            }
            bool operator ==(CString const &b)
            {
                return _v == asLoc(b);
            }

            bool operator !=(Value const &b)
            {
                return this->_v != b;
            }
            bool operator !=(BufferLocation const &b)
            {
                return _v != b._v;
            }
            bool operator !=(CString const &b)
            {
                return _v != asLoc(b);
            }

        protected:
            Value _v;
    };
    // Buffer size

    class BufferSize : public vector<Size>
    {
        public:
            Size &operator[](size_type n)
            {
                if (size() < n)
                    resize(n + 1, 0);

                return at(n);
            }
            const Size operator[](size_type n) const
            {
                if (size() < n)
                    return 0;

                return at(n);
            }
    };

    // Storage distribution

    class StorageDistribution : public map<CString, Size>
    {
        public:
            // Construct
            void constructFromXML(const CNodePtr storageDistributionNode);

            // Convert
            void convertToXML(const CNodePtr storageDistributionNode);

            // Size of the distribution
            Size getSize() const
            {
                Size sz = 0;
                for (StorageDistribution::const_iterator i = begin(); i != end(); i++)
                    sz = sz + i->second;
                return sz;
            };

            // Throughput
            Throughput getThroughput() const
            {
                return throughput;
            };
            void setThroughput(const Throughput thr)
            {
                throughput = thr;
            };

            // Access storage size of a specific schannel
            Size &operator[](GraphComponent *c)
            {
                if (find(c->getName()) == end())
                    insert(std::pair<CString, Size > (c->getName(), 0));

                return find(c->getName())->second;
            };

            // Access storage size of a specific schannel
            const Size operator[](GraphComponent *c) const
            {
                if (find(c->getName()) == end())
                    return 0;

                return find(c->getName())->second;
            };

            // Compare storage distributions
            bool operator<(const StorageDistribution &s) const
            {
                if (getThroughput() < s.getThroughput())
                    return true;
                return (map<CString, Size > (*this) < map<CString, Size > (s));
            };

        private:
            Throughput throughput;
    };

    // Storage distributions

    class StorageDistributions : public set<StorageDistribution>
    {
        public:
            // Construct
            void constructFromXML(const CNodePtr storageDistributionsNode);

            // Convert
            void convertToXML(const CNodePtr storageDistributionsNode);
    };

} // End namespace FSMSADF

#endif

