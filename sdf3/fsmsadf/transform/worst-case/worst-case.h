/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   worst-case.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   June 29, 2009
 *
 *  Function        :   Transform FSM-based SADF to worst-case FSM-based SADF
 *
 *  History         :
 *      29-06-09    :   Initial version.
 *
 * $Id: worst-case.h,v 1.1.2.2 2010-04-22 07:07:19 mgeilen Exp $
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

#ifndef FSMSADF_TRANSFORM_WORST_CASE_WORST_CASE_H_INCLUDED
#define FSMSADF_TRANSFORM_WORST_CASE_WORST_CASE_H_INCLUDED

#include "../../base/graph.h"

namespace FSMSADF
{

    /**
     * transformToWorstCaseGraph ()
     * The function converts the graph to a graph with once scenario which
     * represents the worst-case behavior over all scenarios in the original
     * graph.
     */
    Graph *transformToWorstCaseGraph(Graph *g);

} // End namespace FSMSADF

#endif

