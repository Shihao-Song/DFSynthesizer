/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   runtest.cc
 *
 *  Author          :   Marc Geilen (m.c.w.geilen@tue.nl)
 *
 *  Date            :   August 10, 2010
 *
 *  Function        :   Program to run all tests
 *
 *  History         :
 *      10-08-10    :   Initial version.
 *
 * $Id: runtest.cc,v 1.1.4.9 2010-08-18 07:51:51 mgeilen Exp $
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

#include "runtest.h"
#include "tester.h"

#include "base/exception/exception.h"
#include "base/shell/dirs.h"

#include <iostream>
#include <fstream>

using namespace std;


/**
 * helpMessage ()
 * Function prints help message for the tool.
 */
void helpMessage(ostream &out)
{
    out << "SDF3 " << TOOL << " (version " << DOTTED_VERSION ")" << endl;
    out << endl;
    out << "Usage: " << TOOL;
    out << " [--output <file>]";
    out << endl;
}





/**
 * initSettings ()
 * The function initializes the program settings.
 */
void initSettings(int argc, char **argv)
{
}




/**
 * main ()
 */
int main(int argc, char **argv)
{
    int exit_status = 0;
    ofstream out;

    try
    {
        // Initialize the program
        initSettings(argc, argv);
        // TODO: get from settings:
        CString startdir = ".";

        // Set output stream
        //        if (!settings.outputFile.empty())
        //            out.open(settings.outputFile.c_str());
        //        else
        ((ostream &)(out)).rdbuf(cout.rdbuf());

        // Perform requested actions
        Tester tester;
        tester.runTest(startdir, out);
    }
    catch (CException &e)
    {
        cerr << e;
        exit_status = 1;
    }

    return exit_status;
}


