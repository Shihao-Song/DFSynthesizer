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
 *  Function        :   Cross-platform support functions
 *                      running shell scripts
 *
 *  History         :
 *      12-08-10    :   Initial version.
 *
 * $Id: shell.cc,v 1.1.4.3 2010-08-16 09:42:45 mgeilen Exp $
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

#include "shell.h"

// include platform specific subclass
#ifdef _MSC_VER
#include "shellwin.h"
#else
#include "shellnix.h"
#endif

#include "base/string/cstring.h"


/**
 * createShell
 * factory to create the shell appropriate for the platform being built.
 */
Shell *createShell()
{
    Shell *s;

#ifdef _MSC_VER
    s = new ShellWin();
#else
    s = new ShellNix();
#endif

    s->initialize();
    return s;
}
