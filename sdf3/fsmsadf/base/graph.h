/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   graph.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 22, 1009
 *
 *  Function        :   FSM-based SADF graph
 *
 *  History         :
 *      22-04-09    :   Initial version.
 *
 * $Id: graph.h,v 1.1.2.11 2010-04-22 07:07:10 mgeilen Exp $
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
#ifndef FSMSADF_BASE_GRAPH_H_INCLUDED
#define FSMSADF_BASE_GRAPH_H_INCLUDED

#include "../../sdf/sdf.h"
#include "channel.h"
#include "fsm.h"

namespace FSMSADF
{

    // List of scenario graph pointers
    typedef list<ScenarioGraph *> ScenarioGraphs;

    // maps a persisten token by its name to its channel and index on that channel
    typedef map< CString, pair<Channel *, uint> > PersistentTokenMap;

    // maps a persisten token by its name to its channel and index on that channel
    typedef map< CString, uint > PersistentTokenIndexMap;

    /**
     * ScenarioGraph
     * Container for FSM-based SADF scenario graph.
     */
    class ScenarioGraph : public GraphComponent
    {
        public:

            // Constructor
            ScenarioGraph(GraphComponent c);

            // Destructor
            ~ScenarioGraph();

            // Construct
            ScenarioGraph *create(GraphComponent c) const;
            ScenarioGraph *createCopy(GraphComponent c) const;
            ScenarioGraph *clone(GraphComponent c) const;

            // Construct
            void constructFromXML(const CNodePtr scenarioGraphNode);
            void constructPropertiesFromXML(Scenario *s, const CNodePtr scenarioNode);

            // Convert
            void convertToXML(const CNodePtr scenarioGraphNode);
            void convertPropertiesToXML(Scenario *s, const CNodePtr scenarioNode);

            // Extract SDF graph
            TimedSDFgraph *extractSDFgraph(Scenario *s) const;

            // get partial repetition vector from spec of weakly consistent fsmsadf
            RepetitionVector *getPartialRepetitionVector(Scenario *s) const;

            // set partial repetition vector for scenario
            void setPartialRepetitionVector(Scenario *s, const RepetitionVector &v);

            // Isolate scenario
            void isolateScenario(Scenario *s);

            // Type
            CString getType() const
            {
                return type;
            };
            void setType(const CString &t)
            {
                type = t;
            };

            // Actors
            Actors &getActors()
            {
                return actors;
            };
            Actor *getActor(const CId id);
            Actor *getActor(const CString &name);
            bool hasActor(const CString &name);
            void addActor(Actor *a);
            void removeActor(const CString &name);
            Actor *createActor();
            Actor *createActor(GraphComponent c);

            // Channels
            Channels &getChannels()
            {
                return channels;
            };
            Channel *getChannel(const CId id);
            Channel *getChannel(const CString &name);
            bool hasChannel(const CString &name);
            void addChannel(Channel *c);
            void removeChannel(const CString &name);
            Channel *createChannel(GraphComponent c);
            Channel *createChannel(Actor *src, Actor *dst);

            const PersistentTokenMap getPersistentTokenMap();
            const PersistentTokenMap getPersistentTokenMapFinal();
            const PersistentTokenIndexMap getPersistentTokenIndexMap();
            const PersistentTokenIndexMap getPersistentTokenIndexMapFinal();

            // Storage distribution sets
            const map<Scenario *, StorageDistributions> &getStorageDistributions() const
            {
                return storageDistributions;
            };
            void setStorageDistributions(const map<Scenario *, StorageDistributions> &d)
            {
                storageDistributions = d;
            }
            StorageDistributions &getStorageDistributionsOfScenario(Scenario *s);
            void setStorageDistributionsOfScenario(Scenario *s,
                                                   const StorageDistributions d);
            bool hasStorageDistributionsInScenario(Scenario *s) const;

            // Graph
            Graph *getGraph() const
            {
                return (Graph *)getParent();
            };

            // Print
            ostream &print(ostream &out);
            friend ostream &operator<<(ostream &out, ScenarioGraph &g)
            {
                return g.print(out);
            };

        private:
            // Default scenario
            Scenario *getDefaultScenario() const;

        private:
            // Graph type
            CString type;

            // Actors
            Actors actors;

            // Channels
            Channels channels;

            // persistent final tokens
            PersistentTokenMap persistentTokenMapFinal;

            // Storage distributions
            map<Scenario *, StorageDistributions> storageDistributions;
    };

    /**
     * Graph
     * Container for FSM-based SADF graph.
     */
    class Graph : public GraphComponent
    {
        public:
            // Constructor
            Graph(GraphComponent c);
            Graph(Graph *g);
            Graph();

            // Destructor
            ~Graph();

            // Construct
            Graph *create(GraphComponent c) const;
            Graph *createCopy(GraphComponent c) const;
            Graph *clone(GraphComponent c) const;

            // Construct
            void constructFromXML(const CNodePtr applicationGraphNode);

            // Convert
            void convertToXML(const CNodePtr applicationGraphNode);

            // Extract SDF graph
            TimedSDFgraph *extractSDFgraph(Scenario *s) const;

            // Get the partial repetition vector for a scenario of a weakly consistent FSMSADF
            RepetitionVector *getPartialRepetitionVector(Scenario *s) const;

            // Isolate scenarios
            void isolateScenarios();
            bool hasIsolatedScenarios() const;

            // Scenarios
            Scenario *getScenario(const CString &name) const;
            Scenarios &getScenarios()
            {
                return scenarios;
            };
            void addScenario(Scenario *s)
            {
                scenarios.insert(s);
            };

            // Default scenario
            Scenario *getDefaultScenario() const;

            // Scenario graphs
            ScenarioGraph *getScenarioGraph(const CString &name) const;
            ScenarioGraph *getScenarioGraph(const Scenario &name) const;
            ScenarioGraphs &getScenarioGraphs()
            {
                return scenarioGraphs;
            };
            void addScenarioGraph(ScenarioGraph *g)
            {
                scenarioGraphs.push_back(g);
            };
            void replaceScenarioGraph(ScenarioGraph *fromG, ScenarioGraph *toG);

            // FSM
            FSM *getFSM() const
            {
                return fsm;
            };
            void setFSM(FSM *f)
            {
                fsm = f;
            };

            // Get all persistent tokens
            const CStrings getPersistentTokens() const;

            // Get all persistent tokens of a scenario
            const CStrings getPersistentTokensOfScenario(const Scenario &s) const;

            // get persistent tokens
            const CStrings getPersistentTokensFinal() const;

            // Get all persistent tokens of a scenario
            const CStrings getPersistentTokensFinalOfScenario(const Scenario &s) const;

            // Throughput constraint
            Throughput getThroughputConstraint() const
            {
                return throughputConstraint;
            };
            void setThroughputConstraint(const Throughput &t)
            {
                throughputConstraint = t;
            };

            // Print
            ostream &print(ostream &out);
            friend ostream &operator<<(ostream &out, Graph &g)
            {
                return g.print(out);
            };

        private:
            // Create a default scenario
            void createDefaultScenario();

        private:
            // Scenarios
            Scenarios scenarios;
            Scenario *defaultScenario;

            // Scenario graphs
            ScenarioGraphs scenarioGraphs;

            // FSM
            FSM *fsm;

            // Throughput constraint
            Throughput throughputConstraint;
    };

} // End namespace FSMSADF

#endif

