/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   generate.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 14, 2009
 *
 *  Function        :   Generate FSM-based SADF graphs
 *
 *  History         :
 *      14-07-09    :   Initial version.
 *
 * $Id: generate.h,v 1.1.2.2 2010-04-22 07:07:11 mgeilen Exp $
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

#ifndef FSMSADF_GENERATE_GENERATE_H_INCLUDED
#define FSMSADF_GENERATE_GENERATE_H_INCLUDED

#include "../base/graph.h"

namespace FSMSADF
{

    /**
     * RandomGraph
     * Generate a random FSM-based SADF graph.
     */
    class RandomGraph
    {
        public:
            // Constructor
            RandomGraph();

            // Destructor
            ~RandomGraph();

            // Generate random graph
            Graph *generateGraph();

            // Initialize generator from XML
            void initializeFromXML(CNode *settingsNode);

        public:
            // Structure of the graph
            bool stronglyConnected;
            uint nrScenarioGraphs;
            uint nrActors;

            // Degree of the actors
            double avgDegree;
            double varDegree;
            double minDegree;
            double maxDegree;

            // Actor iteration counts
            double avgIterCnt;
            double varIterCnt;
            double minIterCnt;
            double maxIterCnt;

            // Initial tokens
            double initialTokenChannelProp;
            double initialTokenProp;

            // Scenarios
            uint nrScenarios;

            // Processor types
            uint nrProcTypes;
            double mapChance;
            bool unifiedActorProperties;

            // Execution time
            bool execTime;
            double avgExecTime;
            double varExecTime;
            double minExecTime;
            double maxExecTime;

            // State size
            /// @todo rename this to actor memroy (or so)
            bool stateSize;
            double avgStateSize;
            double varStateSize;
            double minStateSize;
            double maxStateSize;

            // Token size
            bool tokenSize;
            double avgTokenSize;
            double varTokenSize;
            double minTokenSize;
            double maxTokenSize;

            // Throughput constraint
            bool throughputConstraint;
            uint autoConcurrencyDegree;
            double throughputScaleFactor;

        private:
            // Graph creation
            Graph *createGraph();
            ScenarioGraph *createScenarioGraph(Graph *g);
            Channel *createChannel(ScenarioGraph *sg, Ports &inPorts, Ports &outPorts);
            Actor *createActor(ScenarioGraph *sg, Ports &inPorts, Ports &outPorts);
            Port *createPort(Actor *a, const Port::PortType type);
            void assignRatesAndInitialTokens(Graph *g);
            Channels selectInitialTokenChannels(ScenarioGraph *sg);
            void getChannelsBetweenActors(Actor *a, Actor *b, Channels &channels);
            void assignInitialTokensToChannels(Channels &initialTokenchannels,
                                               RepetitionVector &repVec);
            void assignRatesToPorts(ScenarioGraph *sg, Scenario *s,
                                    RepetitionVector &repVec);
            void findCycles(Actor *p, Actor *a, map<Actor *, uint> &color,
                            map<Actor *, Actor *> &parent, Channels &channels);
            void dfsStronglyConnected(Actor *a, vector<bool> &visit,
                                      bool reverseChannels);
            bool isStronglyConnected(ScenarioGraph *sg);

            // Scenario creation
            Scenario *createScenario(Graph *g);

            // FSM creation
            FSM *createFSM(Graph *g);

            // Graph properties
            void assignProperties(Graph *g);
            void assignActorProperties(Actor *a, Scenarios &scenarios,
                                       CStrings &processorTypes);
            void assignChannelProperties(Channel *c, Scenarios &scenarios);
            void assignThroughputConstraint(Graph *g);

            // Random graph components
            Ports::iterator randomPort(Ports &p);
            Channel *randomChannel(Channels &c);
            ScenarioGraph *randomScenarioGraph(ScenarioGraphs &s);
            FSMstate *randomState(FSMstates &s);
            CStrings randomProcessorTypes();

        private:
            // Random number generator
            MTRand mtRand;
    };

} // End namespace FSMSADF

#endif

