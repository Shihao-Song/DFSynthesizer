/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   precedence_graph.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   June 2, 2009
 *
 *  Function        :   Precedence graph
 *
 *  History         :
 *      02-06-09    :   Initial version.
 *
 * $Id: precedence_graph.h,v 1.1 2009-12-23 13:37:23 sander Exp $
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

#ifndef FSMSADF_RESOURCE_ALLOCATION_SCHEDULING_EARLIEST_DEADLINE_FIRST_PRECEDENCE_GRAPH_H_INCLUDED
#define FSMSADF_RESOURCE_ALLOCATION_SCHEDULING_EARLIEST_DEADLINE_FIRST_PRECEDENCE_GRAPH_H_INCLUDED

#include "../../../base/graph.h"

namespace FSMSADF
{

    /**
     * PrecedenceGraph
     * A precedence graph in an acyclic graph which is created by expanding the
     * scenario-aware graph of a given scenario to an HSDFG in which all channels
     * with at least one initial token are removed. Each node is assigned a
     * deadline which represents the longest path of dependencies which needs
     * to be executed from this node till the end of the precedence graph.
     */
    class PrecedenceGraph
    {
        public:
            // Forward class definition
            class Node;

            // Set of nodes
            typedef set<Node *> Nodes;

            /**
             * Node
             * Precedence node.
             */
            class Node
            {
                public:
                    // Constructor
                    Node(Actor *a, CId n, Time t);

                    // Destructor
                    ~Node() {};

                    // Update the deadline of this node
                    void updateDeadline(const Time t);

                    // Node can be scheduled?
                    bool isSchedulable() const;

                public:
                    // Actor associated with this node
                    Actor *actor;

                    // Firing index of the actor
                    CId idx;

                    // Execution time of the actor
                    Time actorExecutionTime;

                    // Remaining time till completion
                    Time deadline;

                    // for cycle detection (colored DFS)
                    int seen;

                    // Precedence constraints
                    Nodes previous;
                    Nodes next;
            };

        public:
            // Constructor
            PrecedenceGraph(Scenario *s);

            // Destructor
            ~PrecedenceGraph();

            // Node with earliest deadline (i.e. largest value)
            Node *getEarliestDeadline() const;

            // Remove node n from the acyclic precedence graph
            void removeNode(Node *n);

            //
            bool cycle_detect();

        private:
            // Get node
            Node *getNode(const Actor *a, const CId idx) const;

        private:
            // Precedence nodes
            Nodes nodes;
    };

} // End namspace FSMSADF

#endif

