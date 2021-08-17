/*
*  TU Eindhoven
*  Eindhoven, The Netherlands
*
*  Name            :   shellnix.cc
*
*  Author          :   Marc Geilen (m.c.w.geilen@tue.nl)
*
*  Date            :   August 12, 2010
*
*  Function        :   Linux support functions for
*                       running shell scripts
*
*  History         :
*      12-08-10    :   Initial version.
*
* $Id: shellnix.cc,v 1.1.4.12 2010-08-18 20:20:08 mgeilen Exp $
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

#include "shellnix.h"
#include "dirs.h"
#include "base/string/cstring.h"
#include "base/tempfile/tempfile.h"


#include "stdlib.h"
#include <unistd.h>
#include <sstream>

// should hold the working directory
#define ENVBUFSIZE  4096


/**
* constructor
*/
ShellNix::ShellNix(): Shell()
{
}

/**
 * ShellNix::execute
 * execute bash script named script in directory dir
 * return stdout and stderr output of run in redstdout and redstderr
 * return false in case of timeout, true otherwise
 */
bool ShellNix::execute(const CString &dir, const CString &script, CString **redstdout, CString **redstderr, uint timeout)
{

    // buffer for current working directory
    char cwdbuf[ENVBUFSIZE];
    if (getcwd(cwdbuf, ENVBUFSIZE) != cwdbuf)
        throw new CShellException("Could not read current working dir.");

    // change current directory to dir
    if (chdir(dir.c_str()) != 0)
        throw new CShellException("Could not change the working directory.");

    // give me two temporary file names for the stdout and stderr output
    CString tmpstdout = tempFileName(".", "output");
    CString tmpstderr = tempFileName(".", "output");

    // call the bash script redirecting the output
    int res = system(script + " > " + tmpstdout +  " 2> " + tmpstderr);

    //  pid_t pID=fork();
    //  if(pID==0){
    //      // code executed by child process
    //      exec(script + " > " + tmpstdout+  " 2> " + tmpstderr);
    //  } else {
    //      // code executed by parent process
    //      wait for termination of pID or timeout.
    //  }


    // FIXME: add timeout to call and set flag.
    bool completed = true;
    if (res != 0) throw CShellException("execution of script failed");

    // read the contents of the temporary files
    (*redstdout) = this->readFile(tmpstdout);
    (*redstderr) = this->readFile(tmpstderr);

    // remove temporary files
    if ((remove(tmpstdout) != 0) || (remove(tmpstderr) != 0))
        throw new CShellException("Could not remove temporary file.");

    // restore the working directory
    if (chdir(cwdbuf) != 0)
        throw new CShellException("Could not restore the working directory.");

    return completed;
}

/**
 * ShellNix::initialize
 * opportunity to do any necessary initialization
 */
void ShellNix::initialize(void)
{
}

/**
 * ShellNix::readFile
 * read a file into a string
 */
CString *ShellNix::readFile(const CString &fullPath)
{
    // buffer to hold single line
    CString line;
    // stream for the file
    ifstream thefile;

    // string stream to generate resulting string
    stringstream out(stringstream::out);

    // open the file
    thefile.open(fullPath, ifstream::in);

    // if successful
    if (thefile.is_open())
    {
        // read all lines
        while (! thefile.eof())
        {
            getline(thefile, line);
            out << line << endl;
        }
        thefile.close();
    }
    else
    {
        // there was a problem
        throw CShellException("cannot open file: " + fullPath);
    }

    // return result
    return new CString(out.str());

}


