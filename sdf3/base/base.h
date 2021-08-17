/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   base.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   June 18, 2002
 *
 *  Function        :   type definitions
 *
 *  History         :
 *      18-06-04    :   Initial version.
 *      19-02-08    :   Additions by Bart Theelen.
 *
 * $Id: base.h,v 1.2.2.4 2010-04-22 06:48:18 mgeilen Exp $
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

#ifndef BASE_BASE_H_INCLUDED
#define BASE_BASE_H_INCLUDED

/* Exception and assertion */
#include "exception/exception.h"

/* Fractions */
#include "fraction/fraction.h"

/* XML */
#include "xml/xml.h"

/* Temporary file */
#include "tempfile/tempfile.h"

/* Math */
#include "math/cmath.h"

/* Random */
#include "random/random.h"

/* Sort */
#include "sort/sort.h"

/* Timer */
#include "time/time.h"

/* Logging of messages */
#include "log/log.h"

/* Sequences */
#include "sequence/sequence.h"

/* Sparse Matrices */
#include "matrix/matrix.h"

/* MaxPlus algebra */
#include "maxplus/maxplus.h"

/* Lookups (i.e. hash tables)  */
#include "lookup/clookup.h"

#endif
