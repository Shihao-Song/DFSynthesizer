/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   fsm.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 22, 1009
 *
 *  Function        :   FSM-based SADF finite state machine
 *
 *  History         :
 *      22-04-09    :   Initial version.
 *
 * $Id: fsm.h,v 1.1.2.2 2010-04-22 07:07:10 mgeilen Exp $
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

#ifndef FSMSADF_BASE_FSM_H_INCLUDED
#define FSMSADF_BASE_FSM_H_INCLUDED

#include "component.h"
#include "scenario.h"

namespace FSMSADF
{

    // Forward class definition
    class FSM;
    class FSMstate;
    class FSMtransition;

    // List of FSM state pointers
    typedef list<FSMstate *> FSMstates;
    typedef FSMstates::iterator FSMstatesIter;
    typedef FSMstates::const_iterator FSMstatesCIter;

    // List of FSM transition pointers
    typedef list<FSMtransition *> FSMtransitions;
    typedef FSMtransitions::iterator FSMtransitionsIter;
    typedef FSMtransitions::const_iterator FSMtransitionsCIter;

    /**
     * FSMstate
     * State in an FSM
     */
    class FSMstate : public GraphComponent
    {
        public:
            // Constructor
            FSMstate(GraphComponent c);

            // Desctructor
            ~FSMstate();

            // Construct
            FSMstate *create(GraphComponent c) const;
            FSMstate *createCopy(GraphComponent c) const;
            FSMstate *clone(GraphComponent c) const;

            // Scenario
            Scenario *getScenario() const
            {
                return scenario;
            };
            void setScenario(Scenario *s)
            {
                scenario = s;
            };

            // Transitions
            void connectTransition(FSMtransition *t)
            {
                transitions.push_back(t);
            };
            const FSMtransitions &getTransitions() const
            {
                return transitions;
            };

            // FSM
            FSM *getFSM() const
            {
                return (FSM *)getParent();
            };

            // Print
            ostream &print(ostream &out);
            friend ostream &operator<<(ostream &out, FSMstate &s)
            {
                return s.print(out);
            };

        private:
            // Scenario
            Scenario *scenario;

            // Transitions
            FSMtransitions transitions;
    };

    /**
     * FSMtransition
     * State transition in an FSM
     */
    class FSMtransition : public GraphComponent
    {
        public:
            // Constructor
            FSMtransition(GraphComponent c);

            // Desctructor
            ~FSMtransition();

            // Construct
            FSMtransition *create(GraphComponent c) const;
            FSMtransition *createCopy(GraphComponent c) const;
            FSMtransition *clone(GraphComponent c) const;

            // States
            FSMstate *getSrcState() const
            {
                return srcState;
            };
            FSMstate *getDstState() const
            {
                return dstState;
            };
            void connect(FSMstate *src, FSMstate *dst);

            // Print
            ostream &print(ostream &out);
            friend ostream &operator<<(ostream &out, FSMtransition &t)
            {
                return t.print(out);
            };

        private:
            // States
            FSMstate *srcState;
            FSMstate *dstState;
    };

    /**
     * FSM
     * FSM in FSM-based SADF graph
     */
    class FSM : public GraphComponent
    {
        public:
            // Constructor
            FSM(GraphComponent c);

            // Desctructor
            ~FSM();

            // Construct
            FSM *create(GraphComponent c) const;
            FSM *createCopy(GraphComponent c) const;
            FSM *clone(GraphComponent c) const;

            // Construct
            void constructFromXML(const CNodePtr fsmNode);

            // Convert
            void convertToXML(const CNodePtr fsmNode);

            // States
            FSMstate *getState(const CString &name) const;
            FSMstates &getStates()
            {
                return states;
            };
            FSMstate *getInitialState() const
            {
                return initialState;
            };
            void setInitialState(FSMstate *s)
            {
                initialState = s;
            };
            void addState(FSMstate *s)
            {
                states.push_back(s);
            };

            // Graph
            Graph *getGraph() const
            {
                return (Graph *)getParent();
            };

            // Print
            ostream &print(ostream &out);
            friend ostream &operator<<(ostream &out, FSM &f)
            {
                return f.print(out);
            };

        private:
            // Initial state
            FSMstate *initialState;

            // States
            FSMstates states;
    };

} // End namespace FSMSADF

#endif
