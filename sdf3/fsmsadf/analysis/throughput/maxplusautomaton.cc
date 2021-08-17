/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   maxplusautomaton.cc
 *
 *  Author          :   Marc Geilen (m.c.w.geilen@tue.nl)
 *
 *  Date            :   April 26, 2010
 *
 *  Function        :   FSM-based SADF throughput analysis algorithm using
 *                      Max-Plus automaton
 *
 *  History         :
 *      26-04-10    :   Initial version.
 *
 * $Id: maxplusautomaton.cc,v 1.1.2.15 2010-08-04 14:47:36 mgeilen Exp $
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

#include "maxplusautomaton.h"
#include "sdf/analysis/mcm/mcmgraph.h"
#include "sdf/analysis/mcm/mcmyto.h"


namespace FSMSADF
{

    /**
     * throughputAnalysisGraphUsingMaxPlusAutomaton()
     * Compute the throughput of an FSM-based SDAF graph through a
     * max-plus automaton.
     */
    Throughput throughputAnalysisGraphUsingMaxPlusAutomaton(Graph *g)
    {
        MPExploreMaxPlusAutomaton::ThroughputCalculator tc;
        Graph *gt;
        Throughput thr;

        // Create a copy of the graph
        gt = g->clone(GraphComponent(g->getParent(), g->getId(), g->getName()));

        // Compute throughput
        thr = tc.calculateThroughput(gt);

        // Cleanup
        delete gt;

        return thr;
    }

    /**
     * throughputAnalysisGraphUsingMaxPlusAutomatonWeaklyConsistent()
     * Compute the throughput of an FSM-based SDAF graph through a
     * max-plus automaton.
     */
    Throughput throughputAnalysisGraphUsingMaxPlusAutomatonWeaklyConsistent(Graph *g)
    {
        MPExploreMaxPlusAutomaton::ThroughputCalculator tc;
        Graph *gt;
        Throughput thr;

        // Create a copy of the graph
        gt = g->clone(GraphComponent(g->getParent(), g->getId(), g->getName()));

        // check consistency
        if (!tc.testScenarioConsistency(gt))
            throw CException("Weakly consistent FSMSADF graph is not consistent");

        // Compute throughput
        thr = tc.calculateThroughputWeaklyConsistent(gt);

        // Cleanup
        delete gt;

        return thr;
    }


    /**
     * criticalScenarioSequenceWeaklyConsistent()
     * Compute a critical scenario sequence of a weakly consistent FSM-based SDAF graph through a max-plus automaton.
     */
    ScenarioList *criticalScenarioSequenceWeaklyConsistent(Graph *g)
    {
        MPExploreMaxPlusAutomaton::ThroughputCalculator tc;
        Graph *gt;

        // Create a copy of the graph
        gt = g->clone(GraphComponent(g->getParent(), g->getId(), g->getName()));

        // check consistency
        if (!tc.testScenarioConsistency(gt))
            throw CException("Weakly consistent FSMSADF graph is not consistent");

        // Compute throughput
        ScenarioList *sl = tc.criticalScenarioSequenceWeaklyConsistent(gt);

        // Cleanup
        delete gt;

        return sl;
    }



    namespace MPExploreMaxPlusAutomaton
    {

        /**
         * Compute the throughput of an FSM-based SDAF graph through a
         * max-plus automaton.
         */
        Throughput ThroughputCalculator::calculateThroughput(Graph *g)
        {

            ScenarioMatrices *scenarioMatrices;
            scenarioMatrices = GraphConversion::computeMaxPlusMatricesFromScenarios(g);

            // create a minimizal edge labelled scenario automaton
            EdgeLabeledScenarioFSM *elsFSM = FSMConverter::minimalEdgeLabeledFSM(g->getFSM());

            MaxPlusAutomaton *fsm = new MaxPlusAutomaton();

            // add state for every initial token and location of the fsm

            // get the number of initial tokens
            unsigned int nrTokens = (*scenarioMatrices->begin()).second->getSize();
            if (nrTokens == 0)
                throw CException("Graph has no initial tokens. Cannot compute throughput");

            // create the FSM states for every pair of a state of the FSMSADF's FSM
            // and an initial token
            ELSSetOfStates *Q = elsFSM->getStates();
            ELSSetOfStates::CIter q;
            for (q = Q->begin(); q != Q->end(); q++)
            {
                for (unsigned int k = 0; k < nrTokens; k++)
                {
                    // create a state for (q, k)
                    CId qId = ((ELSState *)(*q))->getLabel();
                    MPAState *s = new MPAState(make_mpastatelabel(qId, k));
                    fsm->addState(s);
                }
            }


            // add the edges. For every edge (q1, q2) in the original fsm,
            // let s be the scenario of q2 and let M_s be the matrix of s
            // For every non -inf element d=M_s(k,m) add an edge from state (q1,k)
            // to (q2,m) labelled with d.

            // for every state of the fsm...
            for (q = Q->begin(); q != Q->end(); q++)
            {
                ELSState *q1 = (ELSState *) *q;
                CId q1Id = q1->getLabel();

                // for every outgoing edge of the state
                ELSSetOfEdges  *t = (ELSSetOfEdges *) q1->getOutgoingEdges();
                ELSSetOfEdges::CIter e;
                for (e = t->begin(); e != t->end(); e++)
                {
                    ELSEdge *tr = (ELSEdge *) *e;
                    CId q2Id = ((ELSState *)tr->getDestination())->getLabel();
                    CString sc = tr->label;
                    Matrix *Ms = (*scenarioMatrices)[sc];

                    // for every entry in the matrix
                    for (unsigned int k = 0; k < nrTokens; k++)
                    {
                        for (unsigned int m = 0; m < nrTokens; m++)
                        {
                            MPDelay d = Ms->get(k, m);
                            if (!MP_ISMINUSINFINITY(d))
                            {
                                MPAState *src = fsm->getStateLabeled(make_mpastatelabel(q1Id, k));
                                MPAState *dst = fsm->getStateLabeled(make_mpastatelabel(q2Id, m));
                                fsm->addEdge(src, make_mpaedgelabel(d, &sc), dst);
                            }
                        }
                    }
                }
            }


            // calculate MCM of the FSM
            double mcm = fsm->calculateMCM();

            delete scenarioMatrices;
            delete elsFSM;
            delete fsm;

            return 1.0 / mcm;
        }



        /**
         * Test whether the scenario graph is consistent.
         */
        bool ThroughputCalculator::testScenarioConsistencyOfScenarioGraph(Scenario *s)
        {

            ScenarioGraph *sg = s->getScenarioGraph();

            // test relation between initial tokens, repetitions and final tokens.

            // for each channel test tokens
            Channels chs = sg->getChannels();
            Channels::const_iterator chi;
            for (chi = chs.begin(); chi != chs.end(); chi++)
            {
                const Channel *ch = *chi;
                Port *sp = ch->getSrcPort();
                Port *dp = ch->getDstPort();
                uint final = ch->getInitialTokens() +
                             sp->getRateOfScenario(s) * sp->getActor()->getRepetitionsOfScenario(s) -
                             dp->getRateOfScenario(s) * dp->getActor()->getRepetitionsOfScenario(s);
                if (final != ch->getFinalTokens())
                    return false;
            }

            return true;
        }


        /**
         * Test whether the scenario specifications are consistent.
         */
        bool ThroughputCalculator::testScenarioConsistency(Graph *g)
        {

            // test the individual scenarios for consistency
            Scenarios ss = g->getScenarios();
            Scenarios::const_iterator si;
            for (si = ss.begin(); si != ss.end(); si++)
            {
                if (! this->testScenarioConsistencyOfScenarioGraph(*si))
                    return false;
            }

            // test consistency of the fsm transistions
            FSM *fsm = g->getFSM();
            FSMstates &st = fsm->getStates();
            FSMstatesCIter sti;
            for (sti = st.begin(); sti != st.end(); sti++)
            {
                const FSMstate *s = *sti;
                const FSMtransitions &tr = s->getTransitions();
                FSMtransitionsCIter j;
                for (j = tr.begin(); j != tr.end(); j++)
                {
                    const FSMtransition *t = *j;
                    FSMstate *sts = t->getSrcState();
                    FSMstate *std = t->getDstState();
                    Scenario *scs = sts->getScenario();
                    Scenario *scd = std->getScenario();
                    CStrings ptd = g->getPersistentTokensOfScenario(*scd);
                    CStrings pts = g->getPersistentTokensFinalOfScenario(*scs);
                    if (ptd != pts) return false;
                }
            }

            return true;
        }


        /**
         * Compute the gthroughput of the weakly consistent fsmsadf graph as well as a corresponding critical scenario sequence.
         */
        ScenarioList *ThroughputCalculator::throughputAndCriticalScenarioSequenceWeaklyConsistent(Graph *g, Throughput &th)
        {

            ScenarioMatrices *scenarioMatrices;
            scenarioMatrices = GraphConversion::computeMaxPlusMatricesFromScenariosForWeakFSMSADF(g);

            // create a minimizal edge labelled scenario automaton
            EdgeLabeledScenarioFSM *elsFSM = FSMConverter::minimalEdgeLabeledFSM(g->getFSM());

            MaxPlusAutomatonWithRewards *fsm = new MaxPlusAutomatonWithRewards();

            // add state for every initial token and location of the fsm

            // create the FSM states for every pair of a state of the FSMSADF's FSM
            // and an initial token
            ELSSetOfStates *Q = elsFSM->getStates();
            ELSSetOfStates::CIter q;
            for (q = Q->begin(); q != Q->end(); q++)
            {
                // get the number of initial tokens
                ELSState *s = (ELSState *) *q;
                ELSEdge *e = (ELSEdge *) * (s->getOutgoingEdges()->begin());
                unsigned int nrTokens = (*scenarioMatrices)[e->label]->getCols();
                for (unsigned int k = 0; k < nrTokens; k++)
                {
                    // create a state for (q, k)
                    CId qId = ((ELSState *)(*q))->getLabel();
                    MPARState *s = new MPARState(make_mpastatelabel(qId, k));
                    fsm->addState(s);
                }
            }

            // add the edges. For every edge (q1, q2) in the original fsm,
            // let s be the scenario of q2 and let M_s be the matrix of s
            // For every non -inf element d=M_s(k,m) add an edge from state (q1,k)
            // to (q2,m) labelled with d.

            // for every state of the fsm...
            for (q = Q->begin(); q != Q->end(); q++)
            {
                ELSState *q1 = (ELSState *) *q;
                CId q1Id = q1->getLabel();

                // for every outgoing edge of the state
                ELSSetOfEdges  *t = (ELSSetOfEdges *) q1->getOutgoingEdges();
                ELSSetOfEdges::CIter e;
                for (e = t->begin(); e != t->end(); e++)
                {
                    ELSEdge *tr = (ELSEdge *) *e;
                    CId q2Id = ((ELSState *)tr->getDestination())->getLabel();
                    CString *sc = &(tr->label);
                    Matrix *Ms = (*scenarioMatrices)[*sc];
                    CDouble rew = g->getScenario(*sc)->getReward();

                    // for every entry in the matrix
                    for (unsigned int r = 0; r < Ms->getRows(); r++)
                    {
                        for (unsigned int c = 0; c < Ms->getCols(); c++)
                        {
                            MPDelay d = Ms->get(r, c);
                            if (!MP_ISMINUSINFINITY(d))
                            {
                                MPARState *src = fsm->getStateLabeled(make_mpastatelabel(q1Id, c));
                                MPARState *dst = fsm->getStateLabeled(make_mpastatelabel(q2Id, r));
                                fsm->addEdge(src, make_rewedgelabel(d, sc, rew), dst);
                            }
                        }
                    }
                }
            }

            // calculate MCR of the FSM
            MPARCycle *critCycle;
            double mcr = fsm->calculateMCRAndCycle(&critCycle);
            th = 1.0 / mcr;

            ScenarioList *scenarios = new ScenarioList();
            MPARCycle::const_iterator i = critCycle->begin();
            for (; i != critCycle->end(); i++)
            {
                MPAREdge *e = (MPAREdge *) *i;
                scenarios->push_back(g->getScenario(*(e->label.scenario)));
            }

            // cleanup
            delete critCycle;
            delete scenarioMatrices;
            delete elsFSM;
            delete fsm;

            return scenarios;
        }


        /**
         * Compute the throughput of a weakly consistent FSM-based SDAF graph through a
         * max-plus automaton.
         */
        Throughput ThroughputCalculator::calculateThroughputWeaklyConsistent(Graph *g)
        {

            Throughput th;
            ScenarioList *sl = this->throughputAndCriticalScenarioSequenceWeaklyConsistent(g, th);
            delete sl;

            return th;
        }


        /**
         * Compute a critical scenario sequence of a weakly consistent FSM-based SDAF graph through a
         * max-plus automaton.
         */
        ScenarioList *ThroughputCalculator::criticalScenarioSequenceWeaklyConsistent(Graph *g)
        {
            Throughput th;
            return this->throughputAndCriticalScenarioSequenceWeaklyConsistent(g, th);
        }


        /**
         * Compute the MCM of the maxplus automaton.
         */
        MPDelay MaxPlusAutomaton::calculateMCM(void)
        {

            // Create a new MCM graph
            MCMgraph *mcmGraph = new MCMgraph();

            // Generate ids by countings
            CId id = 0;

            // associate states and ids in the mcm graph
            map<MPAState *, MCMnode *> stateMap;

            // Create the nodes of the MCM graph
            MPASetOfStates::const_iterator i;
            MPASetOfStates *states = this->getStates();
            for (i = states->begin(); i != states->end(); i++)
            {
                MPAState *const s = (MPAState *)(*i);

                // Create an MCM node for this state
                MCMnode *n = new MCMnode(id, true);

                // Store id of this state
                stateMap[s] = n;
                id++;

                // Add the node to the MCM graph
                mcmGraph->addNode(n);
            }

            // Add edges to the MCM graph
            // for all states of the fsm...
            CId edgeId = 0;
            for (i = states->begin(); i != states->end(); i++)
            {
                MPAState *const s = (MPAState *)(*i);
                MCMnode *srcNode = stateMap[s];
                MPASetOfEdges *edges = (MPASetOfEdges *) s->getOutgoingEdges();

                // ...iterate over the outgoing edges
                MPASetOfEdges::const_iterator j;
                for (j = edges->begin(); j != edges->end(); j++)
                {
                    MPAEdge *edg = (MPAEdge *)(*j);
                    MPAState *ds = (MPAState *) edg->getDestination();
                    // Create an edge in the MCM graph for this state transition
                    MCMedge *e = new MCMedge(edgeId++, true);
                    e->src = srcNode;
                    e->dst = stateMap[ds];
                    e->w = edg->label.delay;
                    e->d = 1;

                    // Add the edge to the MCM graph and the src and dst node
                    mcmGraph->addEdge(e);
                }
            }

            MCMgraph *pruned = mcmGraph->pruneEdges();

            delete mcmGraph;
            double mcm = pruned->calculateMaximumCycleMeanKarp();
            delete pruned;
            return  mcm;
        }

        /**
         * Compute the MCR of the maxplus automaton with rewards.
         */
        CDouble MaxPlusAutomatonWithRewards::calculateMCR()
        {
            return this->calculateMCRAndCycle(NULL);
        }


        /**
         * Compute the MCR of the maxplus automaton with rewards and return a critical cycle.
         */
        CDouble MaxPlusAutomatonWithRewards::calculateMCRAndCycle(MPARCycle **cycle)
        {

            MCMedge **mcmCritCycle;
            uint mcmCritCycleLen;

            // Create a new MCM graph
            MCMgraph *mcmGraph = new MCMgraph();

            // Generate ids by countings
            CId id = 0;

            // associate states and ids in the mcm graph
            map<MPARState *, MCMnode *> stateMap;
            map<MCMedge *, MPAREdge *> revEdgeMap;

            // Create the nodes of the MCM graph
            MPARSetOfStates::const_iterator i;
            MPARSetOfStates *states = this->getStates();
            for (i = states->begin(); i != states->end(); i++)
            {
                MPARState *const s = (MPARState *)(*i);

                // Create an MCM node for this state
                MCMnode *n = new MCMnode(id, true);

                // Store id of this state
                stateMap[s] = n;
                id++;

                // Add the node to the MCM graph
                mcmGraph->addNode(n);
            }

            // Add edges to the MCM graph
            // for all states of the fsm...
            CId edgeId = 0;
            for (i = states->begin(); i != states->end(); i++)
            {
                MPARState *const s = (MPARState *)(*i);
                MCMnode *srcNode = stateMap[s];
                MPARSetOfEdges *edges = (MPARSetOfEdges *) s->getOutgoingEdges();

                // ...iterate over the outgoing edges
                MPARSetOfEdges::const_iterator j;
                for (j = edges->begin(); j != edges->end(); j++)
                {
                    MPAREdge *edg = (MPAREdge *)(*j);
                    MPARState *ds = (MPARState *) edg->getDestination();
                    // Create an edge in the MCM graph for this state transition
                    MCMedge *e = new MCMedge(edgeId++, true);
                    e->src = srcNode;
                    e->dst = stateMap[ds];
                    e->w = edg->label.delay;
                    e->d = edg->label.reward;

                    revEdgeMap[e] = edg;

                    // Add the edge to the MCM graph and the src and dst node
                    mcmGraph->addEdge(e);
                }
            }

            // prune the graph
            MCMgraph *pruned = mcmGraph->pruneEdges();

            CDouble mcr;

            // do we need the critical cycle?
            if (cycle != NULL)
            {
                // compute MRC and cycle
                mcr = maxCycleRatioAndCriticalCycleYoungTarjanOrlin(pruned, &mcmCritCycle, &mcmCritCycleLen);
                // translate mcmgraph cycle back to MPA cycle
                *cycle = new MPARCycle();
                for (uint i = 0; i < mcmCritCycleLen; i++)
                {
                    (*cycle)->push_back(revEdgeMap[mcmGraph->getEdge(mcmCritCycle[i]->id)]);
                }
                // cleanup
                free(mcmCritCycle);
            }
            else
            {
                mcr = maxCycleRatioYoungTarjanOrlin(pruned);
            }

            // cleanup
            delete pruned;
            delete mcmGraph;

            // return the result
            return  mcr;
        }



    } // End namespace MPExploreStateSpace

} // End namespace FSMSADF

