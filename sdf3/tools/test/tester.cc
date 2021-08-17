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
 * $Id: tester.cc,v 1.1.4.13 2010-08-19 07:18:00 mgeilen Exp $
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


#include "tester.h"
#include "base/xml/xml.h"

#include <stdlib.h>
#include <cmath>

// regular expression support
#ifdef _MSC_VER
#include <regex>
using namespace std::tr1;
#else
#include <boost/regex.hpp>
using namespace  boost;
#endif

/**
 * constructor
 * creates a shell object
 */
Tester::Tester()
{
    this->shell = createShell();
}

/**
 * runTest ()
 * Run the actual tests.
 */
void Tester::runTest(const CString &startdir, ostream &out)
{
    // Get all the tests in the directory subtry of startdir
    ListTests lt(startdir, TEST_SCRIPT_NAME);

    // will refer to the stdout and stderr output of the script
    CString *redstdout;
    CString *redstderr;

    // make the output xml doc
    CDoc *xmlResults = CNewDoc();
    CAddProcessingInstruction(xmlResults, "xml", "version=\"1.0\"");
    // FIXME: in the Linux version the xml document does not get the second PI:
    CAddProcessingInstruction(xmlResults, "xml-stylesheet", "href=\"report.xslt\" type=\"text/xsl\"");

    // create the xml root node
    CNode *rootNode = CNewNode(CString("test_results"));

    // iterate over all tests
    list<CString>::const_iterator i;
    for (i = lt.results.begin(); i != lt.results.end(); i++)
    {
        // s is the next test directory
        const CString &s = *i;
        // show what we are doing
        cout << "Processing test: " << s;

        // initialize results in case of exceptions
        CString errorMsg = "";
        bool pass = false;
        bool completed = false;

        // try to execute the script, problems will raise an exception
        try
        {
            // execute the script
            completed = this->shell->execute(s, TEST_SCRIPT_NAME, &redstdout, &redstderr, TEST_TIMEOUT_MILS);
            if (completed)
            {
                // compare results.
                pass = this->compareToCorrectOutput(s, *redstdout);
                if (pass)
                {
                    cout << " pass." << endl;
                }
                else
                {
                    cout << " fail." << endl;
                }
            }
            else
            {
                // script did not complete, timeout
                cout << " timeout." << endl;
            }
        }
        catch (CShellException *e)
        {
            // an exception occurred. report error message
            errorMsg = e->getMessage();
            pass = false;
            cout << " error during test." << endl;
        }

        // build the corresponding xml
        CNode *testNode = CNewNode(CString("test"));
        CNode *resultNode = CNewNode(CString("result"));
        if (!completed)
        {
            CSetContent(resultNode, CString("timeout"));
        }
        else
        {
            if (pass)
            {
                CSetContent(resultNode, CString("pass"));
            }
            else
            {
                CSetContent(resultNode, CString("fail"));
            }
        }
        CAddNode(testNode, resultNode);

        CNode *dirNode = CNewNode(CString("test_script"));
        CSetContent(dirNode, s);
        CAddNode(testNode, dirNode);

        CNode *outputNode = CNewNode(CString("stdout_output"));
        CSetContent(outputNode, *redstdout);
        CAddNode(testNode, outputNode);

        CNode *errorNode = CNewNode(CString("stderr_output"));
        CSetContent(errorNode, *redstderr);
        CAddNode(testNode, errorNode);

        if (!errorMsg.empty())
        {
            CNode *errorMsgNode = CNewNode(CString("error_message"));
            CSetContent(errorMsgNode, errorMsg);
            CAddNode(testNode, errorMsgNode);
        }

        CAddNode(rootNode, testNode);

        // cleanup strings
        delete redstdout;
        delete redstderr;
    }

    // finish up the xml doc
    CSetRootNode(xmlResults, rootNode);
    CSaveFile("." PATHSEPARATOR "testresult.xml", xmlResults);

    //TODO: copy the xslt file to the same location.

}

/**
 * Tester::extractOutputNumbers
 * remove all numbers and wildcard symbols from output string
 * and collect them in a separate list.
 */
CString *Tester::extractOutputNumbers(const CString &s, list<OutputNumber> **numbers)
{

    // create a new list of OutputNumbers
    *numbers = new list<OutputNumber>();

    // create a regular expression object for numbers or wildcard symbol
    regex rxnumber(NUMBER_OR_WILDCARD_REGEX);

    // to store the match result
    cmatch mr;

    // let i point to the start of the string
    const char *i = s.c_str();
    // end to the end of the string
    const char *end = i + s.length();
    // find all occurrences in s
    while (i < end)
    {
        // search for the regular expression
        // FIXME test whether regex_search works in Linux.
        if (regex_search(i, mr, rxnumber))
        {
            OutputNumber *n;
            if ((mr.str() == WILDCARD))
            {
                n = new OutputNumber(ON_WILDCARD);
            }
            else
            {
                // it is a float iff it contains a period
                if (mr.str().find('.') != std::string::npos)
                {
                    n = new OutputNumber(strtod(mr.str().c_str(), NULL));
                }
                else
                {
                    // otherwise it is an int
                    n = new OutputNumber(strtol(mr.str().c_str(), NULL, 10));
                }
            }
            // add the number to the list
            (*numbers)->push_back(*n);
            // move the position in the string forward
            i += mr.position() + mr.length();
            // cleanup
            delete n;
        }
        else
        {
            // nothing found anymore, set condition to exit the while loop
            i = end;
        }
    }
    // finally, replace all numbers in the string with the (arbitrary) text '[number]'
    // FIXME unify versions?
#ifdef _MSC_VER
    std::string result = regex_replace(s, rxnumber, std::string("[number]"));
#else
    std::string result = regex_replace(s, rxnumber, "[number]");
#endif
    // return the string with numbers removed
    return new CString(result);
}


/**
 * Check whether output is equal to the contents of the correct-output file
 * modulo differences in DOS/UNIX line ends (sigh...)
 * and modulo differences in float representation and rounding
 */
bool Tester::compareToCorrectOutput(const CString &dir, const CString &output)
{

    // read the correct output from file
    CString *co = this->shell->readFile(dir + PATHSEPARATOR + CORRECT_OUTPUT_NAME);

    // normalize all line endings
    co->replace("\r\n", "\n");
    CString *output_normal = new CString(output);
    output_normal->replace("\r\n", "\n");

    // extract the numbers occurring in the output
    list<OutputNumber> *conumbers, *outnumbers;
    CString *cof = this->extractOutputNumbers(*co, &conumbers);
    CString *outf = this->extractOutputNumbers(*output_normal, &outnumbers);

    // check that the remaing string are identical
    bool correct = (*cof) == (*outf);
    if (correct)
    {
        // check that they contained an equal count of numbers
        if (cof->length() != outf->length())
        {
            correct = false;
        }
        else
        {
            // match the numbers
            list<OutputNumber>::const_iterator i, j;
            i = conumbers->begin();
            j = outnumbers->begin();
            while (correct && i != conumbers->end())
            {
                correct = (*i).match(*j);
                i++;
                j++;
            }
        }
    }

    // cleanup
    delete co;
    delete cof;

    // return the result
    return correct;
}


/**
 * ListTests::ListTests
 * constructor
 * immediately searches recursively for pattern in path
 */
ListTests::ListTests(const CString &path, const CString &pattern)
{
    this->findRecursive(path, pattern, FT_FILE);
}

/**
 * ListTests::fileFound
 * callback function when a file is found, adds it to the result list
 */
void ListTests::fileFound(const CString &path, const CString &file)
{
    this->results.push_back(*new CString(path));
}

/**
 * ListTests::newDir
 * callback function when a new dir is being searched
 * we don't care, do nothing
 */
void ListTests::newDir(const CString &path, const CString &dir)
{
}

/**
 * OutputNumber::OutputNumber
 * constructor
 */
OutputNumber::OutputNumber(OutputNumberType t)
{
    if (t != ON_WILDCARD)
        throw CException("Numeric types need a value.");
    this->type = t;
}

/**
 * OutputNumber::OutputNumber
 * constructor
 */
OutputNumber::OutputNumber(double dv)
{
    this->type = ON_DOUBLE;
    this->val.dval = dv;
}

/**
 * OutputNumber::OutputNumber
 * constructor
 */
OutputNumber::OutputNumber(long iv)
{
    this->type = ON_INT;
    this->val.ival = iv;
}


/**
 * OutputNumber::match
 * test whether to numbers in the output file match
 * They match if
 * - at least one of them is a wildcard
 * - they are both int and identical
 * - they are both float and nearly equal
 */
bool OutputNumber::match(const OutputNumber &n) const
{

    // if either is a wildcard they match
    if (this->type == ON_WILDCARD || n.type == ON_WILDCARD)
        return true;
    // if they are of different type they don't match
    if (this->type != n.type) return false;

    if (this->type == ON_DOUBLE)
    {
        // they are doubles, compare fuzzy
        if (fabs(this->val.dval) < EPSILON_ZERO && fabs(n.val.dval) < EPSILON_ZERO)
        {
            return true;
        }
        else
        {
            if (fabs(this->val.dval) < EPSILON_ZERO || fabs(n.val.dval) < EPSILON_ZERO)
            {
                return false;
            }
            else
            {
                return fabs(this->val.dval - n.val.dval) / fabs(this->val.dval) < EPSILON_REL;
            }
        }
    }
    else
    {
        // they are ints
        return this->val.ival == n.val.ival;
    }
}
