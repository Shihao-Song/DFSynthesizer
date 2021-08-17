/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   binding.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 14, 2009
 *
 *  Function        :   Tile binding and scheduling
 *
 *  History         :
 *      29-05-09    :   Initial version.
 *      19-04-11    :   Add accessors (M. Koedam)
 *                      Virtual functions, rename to SDF3FlowBase
 * $Id: binding.h,v 1.1 2009-12-23 13:37:22 sander Exp $
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

#ifndef FSMSADF_RESOURCE_ALLOCATION_FLOW_TILE_BINDING_H_INCLUDED
#define FSMSADF_RESOURCE_ALLOCATION_FLOW_TILE_BINDING_H_INCLUDED

#include "graph_binding.h"

namespace FSMSADF
{

    /**
     * Tile binding and scheduling
     */
    class TileBindingAlgoBase
    {
        public:
            // Constructor
            TileBindingAlgoBase();

            // Destructor
            virtual ~TileBindingAlgoBase();

            // Override this function to get an alternative GraphBinding algorithm.
            virtual GraphBindingBase *createGraphBinding(PlatformBinding *pb);

            // Initialize algorithm
            void init();

            // Estimate bandwidth requirement
            bool estimateBandwidthConstraint();

            // Bind graph to tiles
            bool bindGraphtoTiles(const uint maxNrAppBindings);

            // Estimate connection delay
            bool estimateConnectionDelay();

            // Construct schedules
            virtual bool constructTileSchedules(const uint maxNrAppBindings);

            ////////////////
            // Accessors //
            //////////////

            // Application graph
            Graph *getApplicationGraph() const
            {
                return applicationGraph;
            };
            void setApplicationGraph(Graph *g)
            {
                applicationGraph = g;
            };

            // Platform graph
            PlatformGraph *getPlatformGraph() const
            {
                return platformGraph;
            };
            void setPlatformGraph(PlatformGraph *g)
            {
                platformGraph = g;
            };

            // Platform bindings
            PlatformBindings *getPlatformBindings() const
            {
                return platformBindings;
            };
            void setPlatformBindings(PlatformBindings *b)
            {
                platformBindings = b;
            };
            PlatformBinding *getPlatformBinding(const CString &name) const;
            PlatformBinding *getInitialPlatformBinding() const;

        protected:
            virtual bool isThroughputConstraintSatisfied(PlatformBinding *pb);
            // Throughput
            virtual Throughput analyzeThroughput(PlatformBinding *pb);

            // Sort scenarios on occurance count
            list<Scenario *> sortScenariosOnOccuranceCount(Graph *g) const;

            // Allocate actor binding
            bool allocateActorBinding(GraphBindingBase *applicationBinding,
                                      CString &name, ActorBinding &actorBinding);

            // Release actor binding
            void releaseActorBinding(GraphBindingBase *applicationBinding, CString &name);

            // Allocate channel binding
            bool allocateChannelBinding(GraphBindingBase *applicationBinding,
                                        CString &name, ChannelBinding &channelBinding);

            // Release channel binding
            void releaseChannelBinding(GraphBindingBase *applicationBinding, CString &name);

            // Find all possible actor bindings given existing (partial) binding
            ActorBindings findActorBindingCandidates(GraphBindingBase *applicationBinding,
                    Actor *a);

            // Find all possible channel bindings given existing (partial) binding
            ChannelBindings findChannelBindingCandidates(
                GraphBindingBase *applicationBinding, Channel *c);

            // Check wether actor with same name has been bound already
            bool hasActorBindingBeenFixed(GraphBindingBase *applicationBinding,
                                          const CString &name) const;

            // Get an actor binding for an actor with the supplied name
            ActorBinding getActorBinding(GraphBindingBase *applicationBinding,
                                         const CString &name) const;

            // Check wether channel with same name has been bound already
            bool hasChannelBindingBeenFixed(GraphBindingBase *applicationBinding,
                                            const CString &name) const;

            // Get a channel binding for a channel with the supplied name
            ChannelBinding getChannelBinding(GraphBindingBase *applicationBinding,
                                             const CString &name) const;

            // Create all valid bindings of the actor name to the platform
            void createAllBindingsOfActor(CString &name, GraphBindings &bindings);

            // Create a binding of the channel name to the platform
            bool createBindingOfChannel(CString &name, GraphBindingBase *b);

            // Pareto minimization of application bindings
            void paretoMinimizationOfApplicationBindings(
                GraphBindings &applicationBindings);

            // Reduce the number of application bindings
            void reduceApplicationBindings(GraphBindings &applicationBindings,
                                           const uint maxNrAppBindings);

            // Replace the platfrom bindings
            void replacePlatformBindings(GraphBindings newBindings);

            // Sort actor names on criticality
            list<CString> sortActorNamesOnCriticality();

            // Scenario occurance count
            map<Scenario *, uint> computeScenariosOccuranceCount(Graph *g) const;

            // Initialize the relation between the application and platform graph
            void initGraphRelations();

            // Application graph
            Graph *applicationGraph;

            // Platform graph
            PlatformGraph *platformGraph;

            // Platform bindings
            PlatformBindings *platformBindings;

            // Repetition vectors
            map<Scenario *, RepetitionVector > repetitionVectors;

            // Actor names to actors
            map<CString, Actors > actorNamesToActors;

            // Actors to scenarios
            map<Actor *, Scenarios > actorToScenarios;

            // Actor names to processor candidates
            map<CString, Processors > actorNamesToProcessorCandidates;

            // Channel names to channels
            map<CString, Channels > channelNamesToChannels;

            // Channels to scenarios
            map<Channel *, Scenarios > channelToScenarios;
    };

} // End namespace FSMSADF

#endif
