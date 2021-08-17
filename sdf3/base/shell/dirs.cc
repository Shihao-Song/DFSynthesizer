/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   dirs.cc
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
 * $Id: dirs.cc,v 1.1.4.3 2010-08-16 07:24:56 mgeilen Exp $
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
#include "../string/cstring.h"

#include <list>

// platform specific includes
#ifdef _MSC_VER
#include <io.h>
#else
#include "dirent.h"
#include <fnmatch.h>
#endif

using namespace std;

/**
 * get_dirs
 * return a list of subdirectories of 'path'
 */
list<CString> *DirUtil::get_dirs(const CString &path)
{
    // create list for results
    list<CString> *l = new list<CString>();

    // platform specific implementations
#ifdef _MSC_VER

    // The Windows way

    // struct for file information
    struct _finddata_t c_file;
    // handle for search
    long hFile;

    // find the first entry in directory
    hFile = _findfirst(path + PATHSEPARATOR + "*", &c_file);
    if (hFile != -1)
    {

        // loop through all entries
        int cont = 0;
        while (cont == 0)
        {
            // if the entry is a subdirectory
            if ((c_file.attrib & _A_SUBDIR) != 0)
            {
                // skip '.' and '..'
                if (strcmp(c_file.name, ".") != 0)
                {
                    if (strcmp(c_file.name, "..") != 0)
                    {
                        // add to the list
                        l->push_back(*new CString(c_file.name));
                    }
                }
            }
            // continue if another one is found
            cont = _findnext(hFile, &c_file);
        }
    }
    // close the search
    _findclose(hFile);

#else

    // The Linux way

    // struct for file attributes
    struct dirent *entry;

    // open the directory
    DIR *dir = opendir(path);
    if (dir != NULL)
    {
        // loop through all entries
        bool cont = true;
        while (cont)
        {
            // read the next entry
            entry = readdir(dir);
            if (entry != NULL)
            {
                // if it is a directory
                if (entry->d_type == DT_DIR)
                {
                    // skip '.' and '..'  (not sure if readdir reports these)
                    if (strcmp(entry->d_name, ".") != 0)
                    {
                        if (strcmp(entry->d_name, "..") != 0)
                        {
                            // add to the list
                            l->push_back(*new CString(entry->d_name));
                        }
                    }
                }
            }
            else
            {
                // no more entries, quit loop
                cont = false;
            };
        }
    }
    // close the search
    closedir(dir);

#endif

    // return the result list
    return l;
}

/**
 * DirUtil::get_files
 * Return a list of all files matching 'pattern' in directory 'path' of type ft
 * TODO: change name, get_files doe not only return files
 */
list<CString> *DirUtil::get_files(const CString &path, const CString &pattern, FileType ft)
{
    // create list for the results
    list<CString> *l = new list<CString>();

    // determine whether to include dirs / files in search
    bool acceptDirs = !(ft == FT_FILE);
    bool acceptFiles = !(ft == FT_DIR);

    // platform specific implementation
#ifdef _MSC_VER

    // The Windows way

    // struct for file attributes
    struct _finddata_t c_file;
    // handle for search
    long hFile;

    // find the first one
    hFile = _findfirst(path + PATHSEPARATOR + pattern, &c_file);
    if (hFile != -1) // something was found
    {
        // loop through all entries
        int cont = 0;
        while (cont == 0)
        {
            // found a file and we accept files
            if (acceptFiles && (c_file.attrib & _A_SUBDIR) == 0)
            {
                // add to the results
                l->push_back(*new CString(c_file.name));
            }
            // if found a dir and we accept dirs
            if (acceptDirs && (c_file.attrib & _A_SUBDIR) != 0)
            {
                // add to the results
                l->push_back(*new CString(c_file.name));
            }
            // search next entry
            cont = _findnext(hFile, &c_file);
        }
    }
    // close the search
    _findclose(hFile);

#else

    // The Linux way

    // struct for entry attributes
    struct dirent *entry;

    // open the dir
    DIR *dir = opendir(path);
    if (dir != NULL) // success?
    {
        // iterate through dir entries
        bool cont = true;
        while (cont)
        {
            // read next entry
            entry = readdir(dir);
            if (entry != NULL) // something found
            {
                // check pattern
                if (fnmatch(pattern.c_str(), entry->d_name, 0) == 0)
                {
                    // if file found and we accept files
                    if (acceptFiles && (entry->d_type != DT_DIR))
                    {
                        // add it to the results
                        l->push_back(*new CString(entry->d_name));
                    }
                    // if we found a dir and we accept dirs
                    if (acceptDirs && (entry->d_type == DT_DIR))
                    {
                        // add it to the results
                        l->push_back(*new CString(entry->d_name));
                    }
                }
            }
            else
            {
                // nothing more, stop loop
                cont = false;
            }
        }
    }
    else
    {
        // problem occurred
        throw new CShellException("Could not open directory " + path);
    }
    // close the search
    closedir(dir);

#endif

    // return the results
    return l;
}

/**
 * DirUtile::find
 * find all entries of type ft that match pattern in path
 * call fileFound for every result found
 */
void DirUtil::find(const CString &path, const CString &pattern,
                   FileType ft)
{
    // get all the files
    list<CString> *fls = this->get_files(path, pattern, ft);
    // call fileFound for all results
    list<CString>::const_iterator i;
    for (i = fls->begin(); i != fls->end(); i++)
    {
        const CString f = *i;
        this->fileFound(path, f);
    }

}

/**
 * find all files of type ft that match pattern in directory path
 * or any of its subdirectories
 * for every file found, the method fileFound() is invoked.
 * for every directory searched, the method newDir() is invoked.
 */
void DirUtil::findRecursive(const CString &path, const CString &pattern,
                            FileType ft)
{
    // get all matchiong entries
    list<CString> *fls = this->get_files(path, pattern, ft);
    // get all subdirectories
    list<CString> *drs = this->get_dirs(path);

    // report all files found
    list<CString>::const_iterator i;
    for (i = fls->begin(); i != fls->end(); i++)
    {
        const CString f = *i;
        this->fileFound(path, f);
    }

    // recurse all subdirectories
    for (i = drs->begin(); i != drs->end(); i++)
    {
        const CString d = *i;
        this->newDir(path, d);
        this->findRecursive(path + PATHSEPARATOR + d, pattern, ft);
    }
}



