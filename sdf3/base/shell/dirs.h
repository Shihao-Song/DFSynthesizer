/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   dirs.h
 *
 *  Author          :   Marc Geilen (m.c.w.geilen@tue.nl)
 *
 *  Date            :   August 10, 2010
 *
 *  Function        :   Cross-platform support functions
 *                      for traversing directories
 *
 *  History         :
 *      10-08-10    :   Initial version.
 *
 * $Id: dirs.h,v 1.1.4.3 2010-08-16 07:24:57 mgeilen Exp $
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

#ifndef BASE_DIRS_DIRS_H_INCLUDED
#define BASE_DIRS_DIRS_H_INCLUDED

#include <list>
#include "base/string/cstring.h"
#include "base/exception/exception.h"

using namespace std;


typedef enum
{
    FT_DIR,     // a directory
    FT_FILE,    // a file
    FT_ANY      // any kind
} FileType;


// platform specific path separator symbol
#ifdef _MSC_VER
#define PATHSEPARATOR  "\\"
#else
#define PATHSEPARATOR  "/"
#endif

// For shell related exceptions
class CShellException: public CException
{
    public:
        CShellException(const CString &msg): CException(msg) {}
};

/**
 * DirUtil
 * utility class to traverse a directory tree
 */
class DirUtil
{
    public:
        /**
         * find all files of type ft that match pattern in directory path
         * for every file found, the function fileFound() is invoked.
         */
        void find(const CString &path, const CString &pattern,
                  FileType ft);

        /**
         * find all files of type ft that match pattern in directory path
         * or any of its subdirectories
         * for every file found, the method fileFound() is invoked.
         * for every directory searched, the method newDir() is invoked.
         */
        void findRecursive(const CString &path, const CString &pattern,
                           FileType ft);


        // calback functions

        // called for every file found
        virtual void fileFound(const CString &path, const CString &file) = 0;

        // called for every directory traversed
        virtual void newDir(const CString &path, const CString &dir) = 0;

    private:
        list<CString>* get_dirs(const CString &path);
        list<CString>* get_files(const CString &path, const CString &pattern, FileType ft);
};



#endif
