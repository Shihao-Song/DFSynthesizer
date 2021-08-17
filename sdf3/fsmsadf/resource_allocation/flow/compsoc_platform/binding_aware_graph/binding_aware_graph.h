/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   binding_aware_sdfg.h
 *
 *  Author          :   Martijn Koedam (m.l.p.j.koedam@tue.nl)
 *
 *  Date            :   April 15, 2011
 *
 *  Function        :   Binding-aware SDFG
 *
 *  History         :
 *      15-04-11    :   Initial version.
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

#ifndef FSMSADF_RESOURCE_ALLOCATION_FLOW_COMPSOC_PLATFORM_BINDING_AWARE_GRAPH_BINDING_AWARE_GRAPH_H_INCLUDED
#define FSMSADF_RESOURCE_ALLOCATION_FLOW_COMPSOC_PLATFORM_BINDING_AWARE_GRAPH_BINDING_AWARE_GRAPH_H_INCLUDED


#include "../../base_platform/binding_aware_graph/binding_aware_graph.h"
#include "../../../platform_binding/graph.h"

namespace FSMSADF
{

    /**
     * BindingAwareGraph ()
     * Binding-aware Graph.
     */
    class BindingAwareGraphCompSoCPlatform : public BindingAwareGraphBase
    {

        public:
            // Constructor
            BindingAwareGraphCompSoCPlatform(PlatformBinding *pb);

            // Destructor
            ~BindingAwareGraphCompSoCPlatform();

        protected:
            // Transform application graph to include binding decisions
            void modelActorToTileBinding(Actor *a, Processor *p, Scenario *s);
            void modelChannelToConnectionBinding(Channel *c,
                                                 ChannelBindingConstraints *bc, Connection *co, Scenario *s);
    };

} // End namspace FSMSADF

#endif

