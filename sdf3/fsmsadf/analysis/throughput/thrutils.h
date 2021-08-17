/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   thr_util.h
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
 * $Id: thrutils.h,v 1.1.2.4 2010-08-06 07:17:49 mgeilen Exp $
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

#ifndef FSMSADF_ANALYSIS_THROUGHPUT_THRUTILS_H_INCLUDED
#define FSMSADF_ANALYSIS_THROUGHPUT_THRUTILS_H_INCLUDED

#include "base/fsm/fsm.h"
#include "base/maxplus/maxplus.h"
#include "../../base/graph.h"

using namespace ::FSM::Labeled;


namespace FSMSADF
{

    class ScenarioMatrices: public std::map<CString, MaxPlus::Matrix *>
    {
        public:
            ~ScenarioMatrices()
            {
                ScenarioMatrices::iterator i;
                for (i = this->begin(); i != this->end(); i++)
                {
                    Matrix *m = (*i).second;
                    delete m;
                }
            }
            // get the vector size for the matrices in the map
            // assumes at least one matrix has been stored
            size_t vectorSize(void)
            {
                return this->begin()->second->getSize();
            }
    };

    class GraphConversion
    {
        public:
            // Get the maxplus matrices for each of the scenarios.
            static ScenarioMatrices *computeMaxPlusMatricesFromScenarios(Graph *g);
            static ScenarioMatrices *computeMaxPlusMatricesFromScenariosForWeakFSMSADF(Graph *g);

            // ensure the FSM SADF is strongly bounded by introducing a dummy
            // actor with a self loop, so that sub collections of initial tokens
            // cannot develop independently on particular scenario sequences
            static Graph *ensureStronglyBounded(Graph *g);
    };

    // Edge Labelled Scenario Automaton
    typedef ::FSM::Labeled::State<CId, CString> ELSState;
    typedef ::FSM::Labeled::Edge<CId, CString> ELSEdge;
    typedef ::FSM::Labeled::SetOfStates<CId, CString> ELSSetOfStates;
    typedef ::FSM::Labeled::SetOfEdges<CId, CString> ELSSetOfEdges;
    class EdgeLabeledScenarioFSM: public
        ::FSM::Labeled::FiniteStateMachine<CId, CString>
    {
    public:
        virtual ~EdgeLabeledScenarioFSM() {};
    };

    // Matrix Labelled Scenario Automaton
    typedef ::FSM::Labeled::State<CId, Matrix *> MLSState;
    typedef ::FSM::Labeled::Edge<CId, Matrix *> MLSEdge;
    typedef ::FSM::Labeled::SetOfStates<CId, Matrix *> MLSSetOfStates;
    typedef ::FSM::Labeled::SetOfEdges<CId, Matrix *> MLSSetOfEdges;
    class MatrixLabeledScenarioFSM: public
        ::FSM::Labeled::FiniteStateMachine<CId, Matrix *>
    {
    };


    class FSMConverter
    {
        public:
            static EdgeLabeledScenarioFSM *convertFSMToEdgeLabeled(FSM *fsm);
            static EdgeLabeledScenarioFSM *minimalEdgeLabeledFSM(FSM *fsm);
            static MatrixLabeledScenarioFSM *convertToMatrixLabeledScenarioFSM(EdgeLabeledScenarioFSM *elsfsm, ScenarioMatrices *sm, bool combineEdges = true);
            static MatrixLabeledScenarioFSM *convertToFullMatrixLabeledScenarioFSM(EdgeLabeledScenarioFSM *elsfsm, ScenarioMatrices *sm);
    };
}

#endif
