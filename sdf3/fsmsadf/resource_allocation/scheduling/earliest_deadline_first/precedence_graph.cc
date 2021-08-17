/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   precedence_graph.cc
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
 * $Id: precedence_graph.cc,v 1.1 2009-12-23 13:37:23 sander Exp $
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

#include "precedence_graph.h"
#include "../../../analysis/base/repetition_vector.h"

namespace FSMSADF
{

    /**
     * Node()
     * Constructor
     */
    PrecedenceGraph::Node::Node(Actor *a, CId n, Time t)
        :
        actor(a),
        idx(n),
        actorExecutionTime(t),
        deadline(-1),
        seen(0)
    {
    }

    /**
     * updateDeadline()
     * Update the deadline time of this node to t in case its current deadline
     * is less then t.
     */
    void PrecedenceGraph::Node::updateDeadline(const Time t)
    {
        // Time t is smaller then current remaining time
        if (t + actorExecutionTime <= deadline)
        {
            return;
        }

        // Update the deadline
        deadline = t + actorExecutionTime;

        // Update the deadline of all nodes which are a predecessor of this node
        for (Nodes::iterator i = previous.begin(); i != previous.end(); i++)
        {
            Node *n = *i;
            n->updateDeadline(deadline);
        }
    }

    /**
     * isSchedulable()
     * A node can be scheduled when it has no previous constraints left.
     */
    bool PrecedenceGraph::Node::isSchedulable() const
    {
        if (previous.empty())
            return true;
        return false;
    }

    /**
     * PrecedenceGraph()
     * Constructor.
     */
    PrecedenceGraph::PrecedenceGraph(Scenario *s)
    {
        ScenarioGraph *sg;
        RepetitionVector v;

        // Get scenario graph sg which is associated with scenario s
        sg = s->getScenarioGraph();

        // Compute repetition vector of the scenario graph
        v = computeRepetitionVector(sg, s);

        // Create nodes for each actor in the scenario graph
        for (Actors::iterator i = sg->getActors().begin();
             i != sg->getActors().end(); i++)
        {
            Actor *a = *i;

            // Create as many nodes as repetition vector entries
            for (int j = 0; j < v[a->getId()]; j++)
            {
                Node *n = new Node(a, j, a->getExecutionTimeOfScenario(s,
                                   a->getDefaultProcessorType()));

                // Add node n to the set of nodes
                nodes.insert(n);
            }
        }

        // Create precedence constraints between the nodes
        for (Channels::iterator it = sg->getChannels().begin();
             it != sg->getChannels().end(); it++)
        {
            Channel *c = *it;

            uint nA = c->getSrcPort()->getRateOfScenario(s);
            uint nB = c->getDstPort()->getRateOfScenario(s);
            uint qA = v[c->getSrcActor()->getId()];
            uint qB = v[c->getDstActor()->getId()];
            uint d = c->getInitialTokens();

            // Iterate over all source nodes related to the source actor of
            // the channel
            for (uint i = 1; i <= qA; i++)
            {
                // Source node
                Node *srcN = getNode(c->getSrcActor(), i - 1);

                // Create an edge for each
                for (uint k = 1; k <= nA; k++)
                {
                    uint j = 1 + (uint)floor((double)((d + (i - 1) * nA + k - 1)
                                                      % (nB * qB)) / (double)(nB));
                    uint t = (uint)floor((double)(d + (i - 1) * nA + k - 1)
                                         / (double)(nB * qB));

                    // Destination node
                    Node *dstN = getNode(c->getDstActor(), j - 1);


                    // No initial tokens on this edge?
                    if (t == 0)
                    {
                        dstN->previous.insert(srcN);
                        srcN->next.insert(dstN);
                    }
                }
            }
        }

        // Validate that the graph is acyclic.
        // this avoid updateDeadline going into a deadlock.
        if (cycle_detect())
        {
            throw CException("Graph is not acyclic");

        }

        // Compute deadlines of the nodes (starting from nodes on which no
        // precedence constraints exist)
        for (Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
        {
            Node *n = *i;

            if (n->next.empty())
            {
                n->updateDeadline(0);
            }
        }
    }

    /**
     * updateThe node (recursive) using DFS.
     *
     * @ returns true is graph is cyclic
     */

    static bool visit(PrecedenceGraph::Node *n)
    {
        n->seen = 1;
        for (PrecedenceGraph::Nodes::iterator i = n->previous.begin();
             i != n->previous.end(); i++)
        {
            PrecedenceGraph::Node *c = *i;
            if (c->seen == 1)
            {
                /* found a cycle */
                return true;
            }
            else if (c->seen == 0)
            {
                if (visit(c))
                {
                    return true;
                }
            }
        }
        n->seen = 2;
        return false;
    }
    /**
     * This function does a modified depth first search (colored DFS).
     * All nodes are initially marked white (0).
     * When a node is encountered, it is marked grey (1),
     * and when its descendants are completely visited, it is marked black (2).
     * If a grey node is ever encountered, then there is a cycle.
     *
     * @return true is a cycle is detected.
     */
    bool PrecedenceGraph::cycle_detect()
    {
        // set all nodes white
        for (Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
        {
            (*i)->seen = 0;
        }
        //
        for (Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
        {
            Node *n = *i;
            if (n->seen == 0)
            {
                if (visit(n))
                {
                    return true;
                }
            }
        }
        return false;
    }

    /**
     * ~PrecedenceGraph()
     * Destructor.
     */
    PrecedenceGraph::~PrecedenceGraph()
    {
        for (Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
            delete(*i);
    }

    /**
     * getEarliestDeadline()
     * The function returns from the node with the earliest deadline (i.e. largest
     * value). When the set of nodes is empty, the function returns NULL.
     */
    PrecedenceGraph::Node *PrecedenceGraph::getEarliestDeadline() const
    {
        Node *n = NULL;

        // Iterate over all nodes
        for (Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
        {
            Node *m = *i;

            // Is this node schedulable?
            if (m->isSchedulable())
            {
                // The deadline of node m is larger then the deadline of node n?
                if (n == NULL || m->deadline > n->deadline)
                    n = m;
            }
        }

        return n;
    }

    /**
     * removeNode()
     * The function removes the node n from the precedence graph. A node can only
     * be removed if all its predecessors or succesors have been removed.
     */
    void PrecedenceGraph::removeNode(Node *n)
    {
        // Node has no predecessors
        if (n->previous.empty())
        {
            // Remove the node from all successor nodes (if any)
            for (Nodes::iterator i = n->next.begin(); i != n->next.end(); i++)
            {
                Node *m = *i;

                m->previous.erase(n);
            }

            // Remove node n from the set of nodes
            nodes.erase(n);

            // Cleanup
            delete n;
        }
        else if (n->next.empty())
        {
            // Remove the node from all predecessor nodes (if any)
            for (Nodes::iterator i = n->previous.begin();
                 i != n->previous.end(); i++)
            {
                Node *m = *i;

                m->next.erase(n);
            }

            // Remove node n from the set of nodes
            nodes.erase(n);

            // Cleanup
            delete n;
        }
        else
        {
            throw CException("Cannot remove node which has predecessors "
                             "and successors.");
        }
    }

    /**
     * getNode()
     * The function returns a pointer to the node with the supplied actor and id.
     * When no such pointer exists, an exception is thrown.
     */
    PrecedenceGraph::Node *PrecedenceGraph::getNode(const Actor *a,
            const CId idx) const
    {
        for (Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
        {
            Node *n = *i;

            if (n->actor == a && n->idx == idx)
                return n;
        }

        throw CException("Precedence graph contains no node with idx " + idx);
    }

} // End namespace FSMSADF

