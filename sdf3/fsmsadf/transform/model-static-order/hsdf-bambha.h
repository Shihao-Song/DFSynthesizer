/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   hsdf-bambha.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 28, 2011
 *
 *  Function        :   Model static-order schedule in graph using technique
 *                      from Bambha.
 *
 *  History         :
 *      28-07-11    :   Initial version.
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

#ifndef FSMSADF_TRANSFORM_MODEL_STATIC_ORDER_HSDF_BAMBHA_Y_H_INCLUDED
#define FSMSADF_TRANSFORM_MODEL_STATIC_ORDER_HSDF_BAMBHA_Y_H_INCLUDED

#include "../../base/graph.h"
#include "../../resource_allocation/platform_binding/graph.h"

namespace FSMSADF
{

    /**
     * Models the static-order schedules of all processors in the graph g using
     * the technique from Bambha.
     * This function will create a seperate scenario graph for each scenario.
     * These scenario graphs are converted to their corresponding HSDFGs and
     * additional edges are introduced to model the static-order schedules.
     * @param Platform binding pb
     * @param Graph g
     * @param mapActorToActorInputGraph
     */
    void modelStaticOrderScheduleInGraphUsingBambha(PlatformBinding *pb,
            Graph *g, map<Actor *, Actor *> &mapActorToActorInputGraph);

    /**
     * Create a graph in which the static-order schedules of all processors are
     * modelled explicitly using the technique from Bambha.
     * This function will create a seperate scenario graph for each scenario.
     * These scenario graphs are converted to their corresponding HSDFGs and
     * additional edges are introduced to model the static-order schedules.
     * @param Platform binding pb
     * @return Graph with static-order scheduled modeled into it
     */
    Graph *modelStaticOrderScheduleInGraphUsingBambha(PlatformBinding *pb);

} // End namespace FSMSADF

#endif
