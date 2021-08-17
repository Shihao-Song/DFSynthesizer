/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdftowcfsmsadf.h
 *
 *  Author          :   Marc Geilen (m.c.w.geilen@tue.nl)
 *
 *  Date            :   August 6, 2012
 *
 *  Function        :   Convert a CSDF graph to weakly consistent FSMSADF graph
 *
 *  History         :
 *      06-08-12    :   Initial version.
 *
 * $Id: $
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

#ifndef CSDF_TRNSFORM_TO_WCFSMSADF_CSDFTOWCFSMSADF_H_INCLUDED
#define CSDF_TRNSFORM_TO_WCFSMSADF_CSDFTOWCFSMSADF_H_INCLUDED

#include "sdf/sdf.h"
#include "../../csdf.h"
#include "../../../fsmsadf/fsmsadf.h"
namespace CSDF
{
    /**
     * convertCSDFGtoWCFSMSADFG ()
     * The function converts a CSDFG to an equivalent WC-FSM-SADFG.
     */
    FSMSADF::Graph *convertCSDFGtoWCFSMSADFG(TimedCSDFgraph *sdfGraph);
} // namespace CSDF
#endif
