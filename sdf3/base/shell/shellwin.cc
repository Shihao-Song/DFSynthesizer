/*
*  TU Eindhoven
*  Eindhoven, The Netherlands
*
*  Name            :   shell.cc
*
*  Author          :   Marc Geilen (m.c.w.geilen@tue.nl)
*
*  Date            :   August 12, 2010
*
*  Function        :   Windows support functions
*                       running shell scripts
*
*  History         :
*      12-08-10    :   Initial version.
*
* $Id: shellwin.cc,v 1.1.4.11 2010-08-18 15:38:20 mgeilen Exp $
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

#include "dirs.h"
#include "shellwin.h"
#include "base/string/cstring.h"
#include "base/tempfile/tempfile.h"

#ifdef _MSC_VER

// platform includes
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>

#define ENVBUFSIZE  4096
// note: script output should not exceed this size:
#define FILEBUFSIZE 65536

// Avoid VS complaints about unsafe (but standard) string operations
#pragma warning( disable : 4995 )



/**
* constructor
*/
ShellWin::ShellWin(): Shell()
{
}

/**
 * ShellWin::execute
 * execute bash script named script in directory dir
 * return stdout and stderr output of run in redstdout and redstderr
 * return false in case of timeout, true otherwise
 */
bool ShellWin::execute(const CString &dir, const CString &script, CString **redstdout, CString **redstderr, uint timeout)
{

    // set attributes for the new child process
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.lpSecurityDescriptor = NULL;

    // temporary files to hold stdout and stderr
    char tempbuf[ENVBUFSIZE];
    char *filepart;
    CString tmpstdoutr = tempFileName(dir, "output");
    GetFullPathName(tmpstdoutr, ENVBUFSIZE, tempbuf, &filepart);
    CString tmpstdout = tempbuf;

    CString tmpstderrr = tempFileName(dir, "output");
    GetFullPathName(tmpstderrr, ENVBUFSIZE, tempbuf, &filepart);
    CString tmpstderr = tempbuf;

    // create and run the child process to run script in directory dir
    bool completed = this->createChildProcess(dir, script, tmpstdout, tmpstderr, timeout);

    // read the file contents to a string
    (*redstdout) = this->readFile(tmpstdout);
    (*redstderr) = this->readFile(tmpstderr);

    // delete the temporary files
    if ((!DeleteFile(tmpstdout.c_str())) || (!DeleteFile(tmpstderr.c_str())))
        throw CShellException("Could not delete temporary output file.");

    return completed;
}


/**
 * ShellWin::initialize
 * opportunity to do any necessary initialization
 */
void ShellWin::initialize(void)
{
}

/**
 * ShellWin::createChildProcess
 * create a child process to execute the script
 * return false in case of timeout
 */
bool ShellWin::createChildProcess(const CString &dir, const CString &script, const CString &tmpstdout,
                                  const CString &tmpstderr, uint timeout)
{
    bool completed = false;

    // define the command line string to execute calls script through bash through cmd.exe
    // redirects stdout and stderr to temporary files
    CString cmdLine = CString("cmd.exe /c bash \"") + script + CString("\" > \"" + tmpstdout +
                      "\" 2> \"" + tmpstderr + "\"");

    // copy to a buffer which may be modified in the call
    TCHAR *szCmdline = new TCHAR[cmdLine.length() + 1];
    strcpy(szCmdline, cmdLine.c_str());

    // process information and startup information for the new child process
    PROCESS_INFORMATION piProcInfo;
    STARTUPINFO siStartInfo;

    // Set up members of the PROCESS_INFORMATION structure.
    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

    // Set up members of the STARTUPINFO structure.
    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
    siStartInfo.cb = sizeof(STARTUPINFO);

    // Create the child process.
    BOOL bSuccess = CreateProcess(NULL,
                                  szCmdline,     // command line
                                  NULL,          // process security attributes
                                  NULL,          // primary thread security attributes
                                  FALSE,         // handles are inherited
                                  0,             // creation flags
                                  NULL,          // use parent's environment
                                  dir.c_str(),   // set current directory
                                  &siStartInfo,  // STARTUPINFO pointer
                                  &piProcInfo);  // receives PROCESS_INFORMATION

    // If an error occurred, raise an exception
    if (! bSuccess)
        throw new CShellException("CreateProcess");
    else
    {
        // otherwise, wait for child process to end
        DWORD result = WaitForSingleObject(piProcInfo.hProcess, timeout);
        completed = result != WAIT_TIMEOUT;

        // Close handles to the child process and its primary thread.
        CloseHandle(piProcInfo.hProcess);
        CloseHandle(piProcInfo.hThread);
    }

    // cleanup the command line buffer
    delete szCmdline;

    return completed;
}

/**
 * ShellWin::readFile
 * read a file into a string
 */
CString *ShellWin::readFile(const CString &fullPath)
{

    // open the file for reading
    HANDLE hFile = CreateFile(fullPath.c_str(), // file to open
                              GENERIC_READ,            // open for reading
                              FILE_SHARE_READ,         // share for reading
                              NULL,                    // default security
                              OPEN_EXISTING,           // existing file only
                              FILE_ATTRIBUTE_NORMAL,   // normal file
                              NULL);                   // no attr. template

    // was there a problem?
    if (hFile == INVALID_HANDLE_VALUE)
    {
        throw new CShellException("Could not open file: " + fullPath);
    }

    // Read one character less than the buffer size to save room for
    // the terminating NULL character.
    DWORD  dwBytesRead = 0;
    char   ReadBuffer[FILEBUFSIZE] = {0};
    if (ReadFile(hFile, ReadBuffer, FILEBUFSIZE - 1, &dwBytesRead, NULL) == FALSE)
    {
        CloseHandle(hFile);
        throw new CShellException("Error reading file: " + fullPath);
    }

    // close the file
    CloseHandle(hFile);

    // does the number of butes read make sense?
    if (dwBytesRead <= FILEBUFSIZE)
    {
        ReadBuffer[dwBytesRead] = '\0'; // NULL character
        return new CString(ReadBuffer);
    }
    else
    {
        throw new CShellException("Unexpected value for dwBytesRead");
    }

}

#endif

