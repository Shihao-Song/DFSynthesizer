/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   thr_util.cc
 *
 *  Author          :   Marc Geilen (m.c.w.geilen@tue.nl)
 *
 *  Date            :   May 6, 2010
 *
 *  Function        :   definitions shared for different throughput
 *                      algorithms.
 *
 *  History         :
 *      06-05-10    :   Initial version.
 *
 * $Id: thrutils.cc,v 1.1.2.5 2010-08-18 09:01:01 mgeilen Exp $
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

#include "thrutils.h"
#include "../../base/graph.h"


namespace FSMSADF
{


    // Get the maxplus matrices for each of the scenarios.
    // The initial tokens are ordered
    ScenarioMatrices *GraphConversion::computeMaxPlusMatricesFromScenarios(Graph *g)
    {
        ScenarioMatrices *sm = new ScenarioMatrices();
        Scenarios scenarios = g->getScenarios();
        CStrings persistentTokens = g->getPersistentTokens();

        for (Scenarios::const_iterator i = scenarios.begin();
             i != scenarios.end(); i++)
        {
            PersistentTokenIndexMap mp = (*i)->getScenarioGraph()->getPersistentTokenIndexMap();
            list<uint> tokenIndices;
            CStrings::const_iterator ti;
            for (ti = persistentTokens.begin(); ti != persistentTokens.end(); ti++)
            {
                tokenIndices.push_back(mp[*ti]);
            }


            TimedSDFgraph *sg = g->extractSDFgraph(*i);
            GraphDecoration::Graph *G = new GraphDecoration::Graph(sg, false, NULL);
            MaxPlusAnalysis::Exploration mpe;
            mpe.G = G;
            mpe.g = sg;
            MaxPlus::Matrix *matrix = mpe.convertToMaxPlusMatrix();
            // create the submatrix of only the persistent tokens.
            MaxPlus::Matrix *smatrix = matrix->getSubMatrix(tokenIndices);
            (*sm)[(*i)->getName()] = smatrix;
            delete matrix;
            delete sg;
            delete G;
        }
        return sm;
    }

    // Get the maxplus matrices for each of the scenarios.
    ScenarioMatrices *GraphConversion::computeMaxPlusMatricesFromScenariosForWeakFSMSADF(Graph *g)
    {
        ScenarioMatrices *sm = new ScenarioMatrices();
        Scenarios scenarios = g->getScenarios();

        for (Scenarios::const_iterator i = scenarios.begin();
             i != scenarios.end(); i++)
        {
            CStrings persistentTokens;
            CStrings persistentTokensFinal;
            Scenario *sc = *i;
            PersistentTokenIndexMap mp = sc->getScenarioGraph()->getPersistentTokenIndexMap();
            PersistentTokenIndexMap mpf = sc->getScenarioGraph()->getPersistentTokenIndexMapFinal();

            // collect names of the initial anf ifnal persistent tokens
            PersistentTokenIndexMap::const_iterator ptmi;
            for (ptmi = mp.begin(); ptmi != mp.end(); ptmi++)
            {
                persistentTokens.push_back((*ptmi).first);
            }
            for (ptmi = mpf.begin(); ptmi != mpf.end(); ptmi++)
            {
                persistentTokensFinal.push_back((*ptmi).first);
            }

            // sort them to get a canonical order
            persistentTokens.sort();
            persistentTokensFinal.sort();

            list<uint> tokenIndicesInitial;
            CStrings::const_iterator ti;
            for (ti = persistentTokens.begin(); ti != persistentTokens.end(); ti++)
            {
                tokenIndicesInitial.push_back(mp[*ti]);
            }
            list<uint> tokenIndicesFinal;
            for (ti = persistentTokensFinal.begin(); ti != persistentTokensFinal.end(); ti++)
            {
                tokenIndicesFinal.push_back(mpf[*ti]);
            }


            TimedSDFgraph *sg = g->extractSDFgraph(sc);
            RepetitionVector *partialRepVec = g->getPartialRepetitionVector(*i);
            GraphDecoration::Graph *G = new GraphDecoration::Graph(sg, true, partialRepVec);
            MaxPlusAnalysis::Exploration mpe;
            mpe.G = G;
            mpe.g = sg;
            MaxPlus::Matrix *matrix = mpe.convertToMaxPlusMatrixForWeakFSMSADF();
            // create the submatrix of only the persistent tokens.
            MaxPlus::Matrix *smatrix = matrix->getSubMatrix(tokenIndicesFinal, tokenIndicesInitial);
            (*sm)[(*i)->getName()] = smatrix;
            delete matrix;
            delete sg;
            delete G;
        }
        return sm;
    }

    // ensure the FSM SADF is strongly bounded by introducing a dummy
    // actor with a self loop, so that sub collections of initial tokens
    // cannot develop independently on particular scenario sequences
    Graph *GraphConversion::ensureStronglyBounded(Graph *g)
    {
        Graph *gt;
        // Create a copy of the graph
        gt = g->clone(GraphComponent(g->getParent(), g->getId(), g->getName()));

        // Add dummy actor with self-edge to each scenario graph
        for (ScenarioGraphs::iterator i = gt->getScenarioGraphs().begin();
             i != gt->getScenarioGraphs().end(); i++)
        {
            ScenarioGraph *sg = *i;

            // Use first channel in the graph
            Channel *cf = sg->getChannels().front();

            // Create dummy actor
            Actor *a = sg->createActor();
            a->setName("sync-actor");
            a->setExecutionTimeOfScenario(gt->getDefaultScenario(), "sync", 0);
            a->setDefaultProcessorType("sync");

            // Add self-edge to the dummy actor
            Channel *c = sg->createChannel(a, a);
            c->setName("sync-selfedge");
            c->getSrcPort()->setRateOfScenario(gt->getDefaultScenario(), 1);
            c->getDstPort()->setRateOfScenario(gt->getDefaultScenario(), 1);
            c->setInitialTokens(1);
            c->setPersistentTokenNames("sync-selfedge-persistent-token");

            // Attach dummy actor to the scenario graph
            Channel *ca = sg->createChannel(cf->getSrcActor(), a);
            ca->setName("sync-src");
            ca->getSrcPort()->setRate(cf->getSrcPort()->getRate());
            ca->getDstPort()->setRateOfScenario(gt->getDefaultScenario(), 1);
            ca->setInitialTokens(0);

            Channel *cb = sg->createChannel(a, cf->getDstActor());
            cb->setName("sync-dst");
            cb->getSrcPort()->setRateOfScenario(gt->getDefaultScenario(), 1);
            cb->getDstPort()->setRate(cf->getDstPort()->getRate());
            cb->setInitialTokens(cf->getInitialTokens());
            for (uint j = 0; j < cb->getInitialTokens(); j++)
                cb->addPersistentTokenName("sync-dst-persistent-" + CString(j));
        }
        return gt;
    }


    /**
     * Create a new state machine where edges are labelled instead of states
     */
    EdgeLabeledScenarioFSM *FSMConverter::convertFSMToEdgeLabeled(FSMSADF::FSM *fsm)
    {
        EdgeLabeledScenarioFSM *result = new EdgeLabeledScenarioFSM();
        FSMstates &states = fsm->getStates();

        // remember which state maps to which new state
        map<FSMstate *, ELSState *> stateMap;

        // create the states
        CId maxId = 0;
        FSMstates::const_iterator i;
        for (i = states.begin(); i != states.end(); i++)
        {
            FSMstate *q = *i;
            ELSState *nq = new ELSState(q->getId());
            result->addState(nq);
            stateMap[q] = nq;
            maxId = MAX(maxId, q->getId());
        }

        // create the edges
        for (i = states.begin(); i != states.end(); i++)
        {
            FSMstate *q = *i;
            const FSMtransitions &tr = q->getTransitions();
            FSMtransitionsCIter j;
            for (j = tr.begin(); j != tr.end(); j++)
            {
                FSMtransition *t = *j;
                result->addEdge(stateMap[q], t->getDstState()->getScenario()->getName(), stateMap[t->getDstState()]);
            }
        }

        // set the initial state.
        // note that our scenario fsm have the peculiar semantics that the first
        // scenario is encountered after the first transition an not in the initial state
        result->setInitialState(stateMap[fsm->getInitialState()]);
        return result;
    }

    // create a minimizal edge labelled scenario automaton
    EdgeLabeledScenarioFSM *FSMConverter::minimalEdgeLabeledFSM(FSM *fsm)
    {

        // step 1: convert to edge labeled automaton
        EdgeLabeledScenarioFSM *elsFSMtemp = FSMConverter::convertFSMToEdgeLabeled(fsm);

        // optionally step2: determinize
        // Not clear whether that is a good idea in general.
        // Does not gain anything for the benchmark graphs, opportunistically remove :-)
        // EdgeLabeledScenarioFSM* detElsFSM = (EdgeLabeledScenarioFSM*) elsFSMtemp->determinizeEdgeLabels();

        // step3: minimize
        EdgeLabeledScenarioFSM *elsFSM = (EdgeLabeledScenarioFSM *) elsFSMtemp->minimizeEdgeLabels();
        //delete detElsFSM;
        delete elsFSMtemp;
        return elsFSM;
    }


    // create a matrix-labelled FSM where
    // the matrix corresponds to the scenario of
    // the edges on the original graph between the states.
    // ìf combineEdges is true (default), then create a
    // matrix-labelled FSM with at most one edge per pair of states where
    // the matrix represents the max of the matrices corresponding to scenario of
    // all edges on the original graph between the states.
    MatrixLabeledScenarioFSM *FSMConverter::convertToMatrixLabeledScenarioFSM(
        EdgeLabeledScenarioFSM *elsfsm, ScenarioMatrices *sm, bool combineEdges)
    {

        MatrixLabeledScenarioFSM *result = new MatrixLabeledScenarioFSM();

        // Add states
        map<ELSState *, MLSState *> stateMap;
        ELSSetOfStates::const_iterator i;
        for (i = elsfsm->getStates()->begin(); i != elsfsm->getStates()->end(); i++)
        {
            ELSState *s = (ELSState *) *i;
            // create new state
            MLSState *ns = new MLSState(s->stateLabel);
            result->addState(ns);
            stateMap[s] = ns;
        }

        // Add initialState
        result->setInitialState(stateMap[elsfsm->getInitialState()]);

        // Add edges to the new FSM
        map<pair<ELSState *, ELSState *>, MLSEdge *> stateEdges;
        for (ELSSetOfEdges::const_iterator i = elsfsm->getEdges()->begin();
             i != elsfsm->getEdges()->end(); i++)
        {
            ELSEdge *e = (ELSEdge *) *i;

            // if edge does not yet exist, cteate it
            if ((!combineEdges) || (stateEdges.find(make_pair((ELSState *)e->getSource(),
                                                    (ELSState *)e->getDestination())) == stateEdges.end()))
            {
                // Add the edge to the MCM graph and the src and dst node
                MLSEdge *me = result->addEdge(stateMap[(ELSState *)e->getSource()],
                                              (*sm)[e->label], stateMap[(ELSState *)e->getDestination()]);
                stateEdges[make_pair((ELSState *)e->getSource(), (ELSState *)e->getDestination())] = me;
            }
            else
            {
                // else update the edge
                MLSEdge *me = stateEdges[make_pair((ELSState *)e->getSource(),
                                                   (ELSState *)e->getDestination())];
                MaxPlus::Matrix *resmat = new MaxPlus::Matrix(sm->vectorSize());
                me->label->maximum((*sm)[e->label], resmat);
                me->label = resmat;
            }
        }
        return result;
    }

    // create a matrix-labelled FSM where
    // the matrix corresponds to the scenario of
    // the edges on the original graph between the states.
    MatrixLabeledScenarioFSM *FSMConverter::convertToFullMatrixLabeledScenarioFSM(
        EdgeLabeledScenarioFSM *elsfsm, ScenarioMatrices *sm)
    {
        return FSMConverter::convertToMatrixLabeledScenarioFSM(elsfsm, sm, false);
    }

}