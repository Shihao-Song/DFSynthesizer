/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   csdf.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 13, 2005
 *
 *  Function        :   CSDF graph
 *
 *  History         :
 *      13-07-05    :   Initial version.
 *
 * $Id: csdf.h,v 1.2 2008-03-22 14:24:21 sander Exp $
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

#ifndef CSDF_H_INCLUDED
#define CSDF_H_INCLUDED

// Basic untimed CSDF graph
#include "csdf/base/untimed/graph.h"

// Timed CSDF graph
#include "csdf/base/timed/graph.h"

// Basic graph algorithms
#include "base/algo/components.h"
#include "base/algo/dfs.h"

// Analysis algorithms
#include "analysis/analysis.h"

// Graph generation
#include "generate/generate.h"

// Output functions
#include "output/buffer_throughput/buffy.h"
#include "output/xml/xml.h"

// SDF functions
#include "transform/to_sdf/sdftocsdf.h"

using namespace CSDF;
#endif
