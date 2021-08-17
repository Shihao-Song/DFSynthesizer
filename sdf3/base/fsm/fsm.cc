/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   fsm.cc
 *
 *  Author          :   Marc Geilen (m.c.w.geilen@tue.nl)
 *
 *  Date            :   March 23, 2009
 *
 *  Function        :   finite state machines
 *
 *  History         :
 *      23-03-09    :   Initial version.
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


#include "fsm.h"

namespace FSM
{

    namespace StateStringLabeled
    {

        void FiniteStateMachine::addStateLabeled(const CString &sl)
        {
            StateStringLabeled::State *s = new StateStringLabeled::State(sl);
            this->addState(s);
        }


        void FiniteStateMachine::addEdgeLabeled(const CString &src, const CString &dst)
        {
            Labeled::State<CString, char> *ssrc = this->getStateLabeled(src);
            Labeled::State<CString, char> *sdst = this->getStateLabeled(dst);
            Labeled::FiniteStateMachine<CString, char>::addEdge(ssrc, 'X', sdst);
        }

        SetOfStates *FiniteStateMachine::reachableStates(void)
        {
            return (SetOfStates *) Labeled::FiniteStateMachine<CString, char>::reachableStates();
        }

        void FiniteStateMachine::setInitialStateLabeled(const CString &sl)
        {
            this->setInitialState(this->getStateLabeled(sl));
        }


    }

    namespace Product
    {

        State *FiniteStateMachine::getInitialState()
        {
            return new State(this->fsma->getInitialState(), this->fsmb->getInitialState(), this);
        }

        Abstract::SetOfEdges *State::getOutgoingEdges()
        {
            if (!outgoingEdgesDone)
            {
                // compute outgoing edges
                const Abstract::SetOfEdges *oea = this->sa->getOutgoingEdges();
                const Abstract::SetOfEdges *oeb = this->sb->getOutgoingEdges();
                Abstract::SetOfEdges::const_iterator i = oea->begin();
                Abstract::SetOfEdges::const_iterator j = oeb->begin();
                while (i != oea->end())
                {
                    while (j != oeb->end())
                    {
                        if (this->fsm->matchEdges(*i, *j))
                        {
                            Abstract::Edge *e = this->fsm->ensureEdge(*i, *j);
                            this->outgoingEdges->insert(e);
                        }
                        j++;
                    }
                    i++;
                }
                outgoingEdgesDone = true;
            }
            return this->outgoingEdges;
        }
    }

}