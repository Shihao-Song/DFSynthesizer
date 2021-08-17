/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   runtest.h
 *
 *  Author          :   Marc Geilen (m.c.w.geilen@tue.nl)
 *
 *  Date            :   August 13, 2010
 *
 *  Function        :   Utilities for testing
 *
 *  History         :
 *      13-08-10    :   Initial version.
 *
 * $Id: tester.h,v 1.1.4.6 2010-08-18 07:51:52 mgeilen Exp $
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

#ifndef TOOLS_TEST_TESTER_H_INCLUDED
#define TOOLS_TEST_TESTER_H_INCLUDED

#define TEST_SCRIPT_NAME        "runtest"
#define CORRECT_OUTPUT_NAME     "correct_output"
#define TEST_TIMEOUT_MILS       10000

#define NUMBER_REGEX "([-+]?[0-9]*(\\.)?[0-9]+([eE][-+]?[0-9]+)?)"
#define NUMBER_OR_WILDCARD_REGEX  NUMBER_REGEX "|(\\[\\*\\])"
#define WILDCARD "[*]"

// comparing floats
#define EPSILON_ZERO            1E-15
#define EPSILON_REL             1E-6

#include "base/string/cstring.h"
#include "base/shell/dirs.h"
#include "base/shell/shell.h"

typedef enum
{
    ON_DOUBLE,
    ON_INT,
    ON_WILDCARD
} OutputNumberType;



class OutputNumber
{
    public:
        OutputNumber(OutputNumberType t);
        OutputNumber(double dval);
        OutputNumber(long ival);

        /**
         * OutputNumber::match
         * test whether to numbers in the output file match
         * They match if
         * - at least one of them is a wildcard
         * - they are both int and identical
         * - they are both float and nearly equal
         */
        bool match(const OutputNumber &n) const;

    private:
        OutputNumberType type;

        union NumericValue
        {
            double dval;
            long ival;
        } val;
};

class ListTests: public DirUtil
{
    public:
        ListTests(const CString &path, const CString &pattern);
        virtual void fileFound(const CString &path, const CString &file);
        virtual void newDir(const CString &path, const CString &dir);
        list<CString> results;
};


class Tester
{
    public:
        Tester();
        // run the tests in startdir and subdirs, report on out
        void runTest(const CString &startdir, ostream &out);

        // compare the output to the correct output in dir
        bool compareToCorrectOutput(const CString &dir, const CString &output);

    private:
        Shell *shell;
        CString *extractOutputNumbers(const CString &s, list<OutputNumber>** floats);
};


#endif