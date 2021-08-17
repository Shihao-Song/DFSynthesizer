/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   shell.h
 *
 *  Author          :   Marc Geilen (m.c.w.geilen@tue.nl)
 *
 *  Date            :   August 12, 2010
 *
 *  Function        :   Windows support functions
 *                      running shell scripts
 *
 *  History         :
 *      12-08-10    :   Initial version.
 *
 * $Id: shellwin.h,v 1.1.4.7 2010-08-18 07:51:38 mgeilen Exp $
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

#ifndef BASE_SHELL_SHELLWIN_H_INCLUDED
#define BASE_SHELL_SHELLWIN_H_INCLUDED

#include "shell.h"
#include "base/string/cstring.h"

#ifdef _MSC_VER
#include <windows.h>

/**
 * ShellWin
 * utility class to support the use of shell scripts in Windows
 * TODO: check whether reading the pipe only after the execution may lead to problems if the pipe
 * gets full. Easier solution may be to redirect stdout and stderr to file and read from file
 */
class ShellWin: public Shell
{
    public:

        // constructor
        ShellWin();

        // initialization
        virtual void initialize(void);

        /**
        * execute bash script named script in directory dir
        * return stdout and stderr output of run in redstdout and redstderr
        * return false in case of timeout, true otherwise
        */
        virtual bool execute(const CString &dir, const CString &script, CString **redstdout, CString **redstderr, uint timeout);

        /**
        * read a file into a string
        */
        virtual CString *readFile(const CString &fullPath);

    private:

        // create a child process to execute the script, return false in case of timeout
        bool createChildProcess(const CString &dir, const CString &script, const CString &tmpstdout,
                                const CString &tmpstderr, uint timeout);

};

#endif

#endif
