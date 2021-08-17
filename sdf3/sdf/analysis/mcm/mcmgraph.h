/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   mcmgraph.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   November 7, 2005
 *
 *  Function        :   Convert HSDF graph to weighted directed graph for MCM
 *                      calculation.
 *
 *  History         :
 *      07-11-05    :   Initial version.
 *
 * $Id: mcmgraph.h,v 1.1.1.1.2.2 2010-08-18 07:51:47 mgeilen Exp $
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

#ifndef SDF_ANALYSIS_MCM_MCMGRAPH_H_INCLUDED
#define SDF_ANALYSIS_MCM_MCMGRAPH_H_INCLUDED

#include "../../base/timed/graph.h"
namespace SDF
{
    class MCMnode;

    class MCMedge
    {
        public:
            // Constructor
            MCMedge(CId eId, bool eVisible);
            CId id;
            bool visible;
            MCMnode *src;
            MCMnode *dst;
            CDouble w;
            CDouble d;
    };


    typedef list<MCMedge *>             MCMedges;
    typedef MCMedges::iterator          MCMedgesIter;
    typedef MCMedges::const_iterator    MCMedgesCIter;

    class MCMnode
    {
        public:
            // Constructor
            MCMnode(CId nId, bool nVisible);
            CId id;
            bool visible;
            MCMedges in;
            MCMedges out;
    };


    typedef list<MCMnode *>             MCMnodes;
    typedef MCMnodes::iterator          MCMnodesIter;
    typedef MCMnodes::const_iterator    MCMnodesCIter;

    class MCMgraph
    {
        public:
            // Constructor
            MCMgraph();

            // Destructor
            ~MCMgraph();

            const MCMnodes &getNodes()
            {
                return nodes;
            };

            uint nrVisibleNodes()
            {
                uint nrNodes = 0;
                for (MCMnodesIter iter = nodes.begin(); iter != nodes.end(); iter++)
                    if ((*iter)->visible) nrNodes++;
                return nrNodes;
            };
            MCMnode *getNode(CId id)
            {
                for (MCMnodes::iterator i = nodes.begin(); i != nodes.end(); i++)
                    if ((*i)->id == id)
                        return (*i);
                return NULL;
            };

            const MCMedges &getEdges()
            {
                return edges;
            };

            MCMedge *getEdge(CId id)
            {
                for (MCMedges::iterator i = edges.begin(); i != edges.end(); i++)
                    if ((*i)->id == id)
                        return (*i);
                return NULL;
            };

            uint nrVisibleEdges()
            {
                uint nrEdges = 0;
                for (MCMedgesIter iter = edges.begin(); iter != edges.end(); iter++)
                    if ((*iter)->visible) nrEdges++;
                return nrEdges;
            };

            // Construction

            // Add a node to the MCM graph
            void addNode(MCMnode *n)
            {
                // Add the node to the MCM graph
                this->nodes.push_back(n);
            }

            // Remove a node from the MCMgraph.
            // Note: containers of nodes are lists, so remove is expensive!
            void removeNode(MCMnode *n)
            {
                // remove any remaining edges
                while (! n->in.empty())
                {
                    this->removeEdge(*(n->in.begin()));
                }
                while (! n->out.empty())
                {
                    this->removeEdge(*(n->out.begin()));
                }

                this->nodes.remove(n);
            }

            // Add an edge to the MCMgraph.
            MCMedge *addEdge(CId id, MCMnode *src, MCMnode *dst, CDouble w, CDouble d)
            {
                MCMedge *e = new MCMedge(id, true);
                e->src = src;
                e->dst = dst;
                e->w = w;
                e->d = d;
                this->addEdge(e);
                return e;
            }


            // Add an edge to the MCMgraph.
            void addEdge(MCMedge *e)
            {
                this->edges.push_back(e);
                e->src->out.push_back(e);
                e->dst->in.push_back(e);
            }

            // Remove an edge from the MCMgraph.
            // Note: containers of edges are lists, so remove is expensive!
            void removeEdge(MCMedge *e)
            {
                this->edges.remove(e);
                e->src->out.remove(e);
                e->dst->in.remove(e);
            }


            // reduce the MCM graph by removing obviously redundant edges
            // in particular if there are multiple edges between the same pair
            // of nodes and for some edge (w1, d1) there exists a different edge
            // (w2, d2) such that d2<=d1 and w2>=w1, then (w2, d2) is removed
            // Note this algorithm does currently not distinguish visible and invisible edges!
            MCMgraph *pruneEdges(void);

            CDouble calculateMaximumCycleMeanKarp();

        private:
            // Nodes
            MCMnodes nodes;

            // Edges
            MCMedges edges;

    };

    typedef list<MCMgraph *>      MCMgraphs;
    typedef MCMgraphs::iterator  MCMgraphsIter;

    /**
     * transformHSDFtoMCMgraph ()
     * The function converts an HSDF graph to a weighted directed graph
     * used in the MCM algorithm of Karp (and its variants). By default, the
     * a longest path calculation is performed to make the graph suitable
     * as input for an MCM algorithm. Setting the flag 'mcmFormulation' to false
     * result in an MCM graph which is suitable for an MCR (cycle ratio)
     * formulation. This avoids running the longest path algo.
     */
    MCMgraph *transformHSDFtoMCMgraph(TimedSDFgraph *g, bool mcmFormulation = true);

    /**
     * Extract the strongly connected components from the graph. These components
     * are returned as a set of MCM graphs. All nodes which belong to at least
     * one of the strongly connected components are set to visible in the graph g,
     * all other nodes are made invisible. Also edges between two nodes in (possibly
     * different) strongly connected components are made visible and all others
     * invisible. The graph g consists in the end of only nodes which are part of
     * a strongly connnected component and all the edges between these nodes. Some
     * MCM algorithms work also on this graph (which reduces the execution time
     * needed in some of the conversion algorithms).
     */
    void stronglyConnectedMCMgraph(MCMgraph *g, MCMgraphs &components);

    /**
     * relabelMCMgraph ()
     * The function removes all hidden nodes and edges from the graph. All visible
     * edges are assigned a new id starting in the range [0,nrNodes()).
     */
    void relabelMCMgraph(MCMgraph *g);

}//namespace SDF
#endif
