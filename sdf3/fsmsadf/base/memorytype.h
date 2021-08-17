/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   memorytype.h
 *
 *  Author          :   Martijn Koedam (m.l.p.j.koedam@tue.nl
 *
 *  Date            :   May 09, 2011
 *
 *  Function        :   Memory Type.
 *
 *  History         :
 *      09-05-11    :   Initial version. (move from memory.h in arch)
 *
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

#ifndef FSMSADF_BASE_MEMORY_TYPE_H_INCLUDED
#define FSMSADF_BASE_MEMORY_TYPE_H_INCLUDED
namespace FSMSADF
{
    /**
     * Memory Type.
     * Enumeration class denoting the type of memory.
     * Instruction, Data, Communication (in/out) and shared.
     */
    class MemoryType
    {
        public:

            enum Value
            {
                Instr = 0, // Instruction memory
                Data, // Data memory
                CommIn, // Communication  (in)
                CommOut, // Communication (out)
                Shared // Unspecified memory
            };

            MemoryType(const CString &l)
            {
                _v = asType(l);
            }

            MemoryType(const Value &l)
            {
                _v = l;
            }

            CString asString() const
            {
                if (_v == Instr) return "instr";
                else if (_v == Data) return "data";
                else if (_v == CommIn) return "commin";
                else if (_v == CommOut) return "commout";
                else return "shared";
            }

            Value asType(const CString &l) const
            {
                if (l == "instr") return Instr;
                else if (l == "data") return Data;
                else if (l == "commin") return CommIn;
                else if (l == "commout") return CommOut;
                else return Shared;
            }

            operator CString() const
            {
                return asString();
            }

            void operator =(Value const &a)
            {
                _v = a;
            }

            void operator =(CString const &a)
            {
                _v = asType(a);
            }

            bool operator ==(Value const &b)
            {
                return this->_v == b;
            }
            bool operator == (MemoryType const &b)
            {
                return _v == b._v;
            }
            bool operator == (CString const &b)
            {
                return _v == asType(b);
            }

            bool operator !=(Value const &b)
            {
                return this->_v != b;
            }
            bool operator != (MemoryType const &b)
            {
                return _v != b._v;
            }

            bool operator != (CString const &b)
            {
                return _v != asType(b);
            }

        protected:
            Value _v;
    };

}// close namespace FSMSADF
#endif // clse FSMSADF_BASE_MEMORY_TYPE_H_INCLUDED