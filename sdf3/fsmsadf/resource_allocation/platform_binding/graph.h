/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   graph.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 12, 2009
 *
 *  Function        :   Platform graph.
 *
 *  History         :
 *      12-05-09    :   Initial version.
 *
 * $Id: graph.h,v 1.1 2009-12-23 13:37:22 sander Exp $
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

#ifndef FSMSADF_RESOURCE_ALLOCATION_PLATFORM_BINDING_GRAPH_H_INCLUDED
#define FSMSADF_RESOURCE_ALLOCATION_PLATFORM_BINDING_GRAPH_H_INCLUDED

#include "tile.h"
#include "connection.h"
#include "constraint.h"

namespace FSMSADF
{

    // Forward class definition
    class PlatformBinding;

    // List of platform binding pointers
    typedef list<PlatformBinding *> PlatformBindings;

    /**
     * PlatformBinding
     * Container for platform binding.
     */
    class PlatformBinding : public GraphComponent
    {
        public:
            // Constructor
            PlatformBinding(GraphComponent c, PlatformGraph *pg, Graph *ag,
                            bool create = true);

            // Destructor
            ~PlatformBinding();

            // Construct
            PlatformBinding *clone(GraphComponent c);

            // Construct
            void constructFromXML(const CNodePtr mappingNode);

            // Convert
            void convertToXML(const CNodePtr mappingNode);

            // Initial binding
            bool isInitialBinding() const;

            // Application graph
            Graph *getApplicationGraph() const
            {
                return applicationGraph;
            };

            // Platform graph
            PlatformGraph *getPlatformGraph() const
            {
                return platformGraph;
            };

            // Tile bindings
            TileBindings &getTileBindings()
            {
                return tileBindings;
            };
            TileBinding *getTileBinding(const Tile *t);

            // Connection bindings
            ConnectionBindings &getConnectionBindings()
            {
                return connectionBindings;
            };
            ConnectionBinding *getConnectionBinding(const Connection *c);

            // Actor binding
            ProcessorBinding *getProcessorBindingOfActorInScenario(Scenario *s,
                    Actor *a);

            // Application graph binding constraints
            GraphBindingConstraints *getGraphBindingConstraints()
            {
                return graphBindingConstraints;
            };
            void setGraphBindingConstraints(GraphBindingConstraints *c)
            {
                graphBindingConstraints = c;
            };

        private:
            // Tile binding
            TileBinding *createTileBinding(Tile *t);

            // Connection binding
            ConnectionBinding *createConnectionBinding(Connection *c);

            // Construct
            void constructResourceUsageFromXML(const CNodePtr resourceUsageNode);

            // Convert
            void convertResourceUsageToXML(const CNodePtr resourceUsageNode);

        private:
            // Application graph
            Graph *applicationGraph;

            // Platform graph
            PlatformGraph *platformGraph;

            // Tile bindings
            TileBindings tileBindings;

            // Connection bindings
            ConnectionBindings connectionBindings;

            // Application graph binding constraints
            GraphBindingConstraints *graphBindingConstraints;
    };

} // End namespace FSMSADF

#endif

