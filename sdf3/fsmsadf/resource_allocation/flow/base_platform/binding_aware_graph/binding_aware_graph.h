/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   binding_aware_sdfg.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 14, 2009
 *
 *  Function        :   Binding-aware SDFG
 *
 *  History         :
 *      14-05-09    :   Initial version.
 *      19-04-11    :   Add accessors (M. Koedam)
 *                      Virtual functions, rename to SDF3FlowBase
 *      04-08-11    :   Complete rewrite to separate analysis from mapping
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

#ifndef FSMSADF_RESOURCE_ALLOCATION_BINDING_AWARE_GRAPH_BINDING_AWARE_GRAPH_H_INCLUDED
#define FSMSADF_RESOURCE_ALLOCATION_BINDING_AWARE_GRAPH_BINDING_AWARE_GRAPH_H_INCLUDED

#include "../../../platform_binding/graph.h"

namespace FSMSADF
{

    /**
     * Binding-aware Graph.
     */
    class BindingAwareGraphBase : public Graph
    {
        private:

            class BindingAwareProperties
            {
                private:
                    // Actor to processor mapping
                    map<Actor *, Processor *> actorToProcessor;

                    // Channel to connection mapping
                    map<Channel *, Connection *> channelToConnection;

                    // TimeSlice to processor mapping
                    map<Processor *, Time> processorToTimeslice;

                public:

                    map<Processor *, Time> &getProcessorToTimeSlice()
                    {
                        return processorToTimeslice;
                    };

                    map<Actor *, Processor *> &getActorToProcessor()
                    {
                        return actorToProcessor;
                    };

                    Time getTimesliceMappedToProcessors(Processor *proc)
                    {
                        return processorToTimeslice[proc];
                    };

                    void mapTimesliceToProcessor(Processor *proc, Time time)
                    {
                        processorToTimeslice[proc] = time;
                    };

                    Connection *getConnectionMappedToChannel(Channel *chan)
                    {
                        return channelToConnection[chan];
                    };

                    void mapChannelToConnection(Channel *chan, Connection *con)
                    {
                        channelToConnection[chan] = con;
                    };

                    Processor *getProcessorMappedToActor(Actor *actor)
                    {
                        return actorToProcessor[actor];
                    };

                    void mapActorToProcessor(Actor *actor, Processor *proc)
                    {
                        actorToProcessor[actor] = proc;
                    };

                    bool actorMappedToProcessor(Actor *a) const;

                    bool channelMappedToConnection(Channel *c) const;
            };

        protected:
            // Constructor
            BindingAwareGraphBase(PlatformBinding *b);

        public:
            // Destructor
            virtual ~BindingAwareGraphBase();

            // Function returns pointer to corresponding actor in the application graph
            Actor *getCorrespondingActorInApplicationGraph(Actor *a);

            // Model binding of mapping of the application to platform in an graph
            void constructBindingAwareGraph();

        private:
            // Extract actor and channel binding and scheduling properties
            void extractActorMapping();
            void extractChannelMapping();

        protected:
            // Transform application graph to include binding decisions
            virtual void modelBindingAndScheduling();
            virtual void modelActorToTileBinding(Actor *a, Processor *p, Scenario *s);
            virtual void modelChannelToConnectionBinding(Channel *c,
                    ChannelBindingConstraints *bc, Connection *co, Scenario *s);
            virtual void modelChannelToTileBinding(Channel *c,
                                                   ChannelBindingConstraints *bc, Scenario *s);

            // Transform binding-aware graph to include scheduling decisions
            virtual void modelStaticOrderScheduleOnProcessor();

            // Find scenario related to a scenario graph
            Scenario *getScenarioOfScenarioGraph(const ScenarioGraph *sg) const;

        private:
            // Platform binding

            PlatformBinding *getPlatformBinding()
            {
                return platformBinding;
            };

            void setPlatformBinding(PlatformBinding *b)
            {
                platformBinding = b;
            };

        protected:
            // Platform binding
            PlatformBinding *platformBinding;

            // Relation between actors in the application graph and the
            // binding-aware graph
            map<Actor *, Actor *> mapBindingAwareActorToApplicationGraphActor;

            // Binding-aware properties of each scenario graph
            map<ScenarioGraph *, BindingAwareProperties> bindingAwareProperties;
    };

} // End namspace FSMSADF

#endif
