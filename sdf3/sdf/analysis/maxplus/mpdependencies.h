/*
*  TU Eindhoven
*  Eindhoven, The Netherlands
*
*  Name            :   mpdependencies.h
*
*  Author          :   Marc Geilen (m.c.w.geilen@tue.nl)
*
*  Date            :   May 13, 2008
*
*  Function        :   Max-Plus based buffer sizing. Analyse and manage storage critical dependencies.
*
*  History         :
*      13-05-08    :   Initial version.
*
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

#include "mpexplore.h"
namespace SDF
{

    namespace MaxPlusAnalysis
    {
        using namespace GraphDecoration;

        namespace DependencyGraphs
        {


            // Forward class definition
            class Node;

            /**
             * class Dependency
             * Represents a critical dependency between two channels of the graph.
             */
            class Dependency
            {
                public:
                    Channel *fromChannel, *toChannel;
                    Node *toNode;
                    Dependency(Channel *from, Channel *to, Node *toNd);
            };

            /**
             * class Node
             * A node in the depencency graph, corresponds to a channel of the SDF graph.
             */
            class Node
            {
                public:
                    Channel *channel;
                    Dependency **dependencies;
                    unsigned int numberOfDependencies;
                    unsigned int dependenciesVisited;
                    bool partOfCycle;
                    void findCycle();
                    void markCycle();
                    void addDependency(Channel *from, Channel *to, Node *toNode);
                    Node(Channel *ch, const unsigned int numberOfChannels);

            };

            /**
             * class Graph
             * A depencency graph, captures critical dependencies between channels of the
             * SDF graph
             */
            class Graph
            {
                public:
                    const GraphDecoration::Graph *G;
                    Node **nodes;
                    Graph(const GraphDecoration::Graph *SG);
                    void addDependency(Channel *from, Channel *to);
                    ChannelList *findCriticalChannels();
                    void fire(State *x, Actor *a);
            };

            /**
             * class class StorageDistribution
             */
            class StorageDistribution
            {
                    unsigned int *table;
            } ;

            /**
             * class class SDFStorageDistribution
             */
            class SDFStorageDistribution : StorageDistribution
            {
            };

        }
    }
}
