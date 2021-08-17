/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   fsm.cc
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
 * $Id: fsm.cc,v 1.1.2.3 2010-05-08 23:59:38 mgeilen Exp $
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
#include "graph.h"

namespace FSMSADF
{

    /**
     * FSMstate()
     * Constructor.
     */
    FSMstate::FSMstate(GraphComponent c)
        :
        GraphComponent(c),
        scenario(NULL)
    {
    }

    // Desctructor
    FSMstate::~FSMstate()
    {
        // Transitions
        for (FSMtransitions::iterator i = transitions.begin();
             i != transitions.end(); i++)
        {
            delete *i;
        }
    }

    /**
     * create ()
     * Create a new FSMstate object.
     */
    FSMstate *FSMstate::create(GraphComponent c) const
    {
        return new FSMstate(c);
    }

    /**
     * createCopy ()
     * Create a copy of this FSMstate component.
     */
    FSMstate *FSMstate::createCopy(GraphComponent c) const
    {
        FSMstate *s = create(c);

        // Properties
        s->setName(getName());

        return s;
    }

    /**
     * clone ()
     * Create a cloned copy of this object.
     */
    FSMstate *FSMstate::clone(GraphComponent c) const
    {
        FSMstate *s = createCopy(c);

        // Get the FSM associated with the graph component c, which is the parent
        // of the new state s
        FSM *fsm = dynamic_cast<FSM *>(c.getParent());
        ASSERT(fsm != NULL, "Parent of FSMstate should be an FSM.");

        // Insert the state into the FSM
        fsm->addState(s);

        // Scenario
        s->setScenario(fsm->getGraph()->getScenario(getScenario()->getName()));

        // Transitions
        for (FSMtransitions::const_iterator i = transitions.begin();
             i != transitions.end(); i++)
        {
            FSMtransition *t = *i;

            // Clone transition (trigers cloning of destination state, when needed)
            t->clone(GraphComponent(s, t->getId()));
        }

        return s;
    }

    /**
     * print()
     * Print the FSM state to the supplied output stream.
     */
    ostream &FSMstate::print(ostream &out)
    {
        out << "state: " << getName() << endl;
        out << "scenario: " << scenario->getName() << endl;

        for (FSMtransitions::iterator i = transitions.begin();
             i != transitions.end(); i++)
        {
            out << "destination: " << (*i)->getDstState()->getName() << endl;
        }

        return out;
    }


    /**
     * FSMtransition ()
     * Constructor.
     */
    FSMtransition::FSMtransition(GraphComponent c)
        :
        GraphComponent(c),
        srcState(NULL),
        dstState(NULL)
    {
    }

    /**
     * ~FSMtransition ()
     * Desctructor.
     */
    FSMtransition::~FSMtransition()
    {
    }

    /**
     * create ()
     * Create a new FSMtransition object.
     */
    FSMtransition *FSMtransition::create(GraphComponent c) const
    {
        return new FSMtransition(c);
    }

    /**
     * createCopy ()
     * Create a copy of this FSMtransition component.
     */
    FSMtransition *FSMtransition::createCopy(GraphComponent c) const
    {
        FSMtransition *t = create(c);

        // Properties
        t->setName(getName());

        return t;
    }

    /**
     * clone ()
     * Create a cloned copy of this object.
     */
    FSMtransition *FSMtransition::clone(GraphComponent c) const
    {
        FSMtransition *t = createCopy(c);

        // Get the FSMstate associated with the graph component c, which is the
        // parent of the new transition t. This state is also the source of the
        // transition.
        FSMstate *src = dynamic_cast<FSMstate *>(c.getParent());
        ASSERT(src != NULL, "Parent of FSMtransition should be an FSMstate.");

        // Get the FSM associated with the source state.
        FSM *fsm = src->getFSM();

        // Locate the destination state in the FSM
        FSMstate *dst = NULL;
        for (FSMstates::const_iterator i = fsm->getStates().begin();
             i != fsm->getStates().end(); i++)
        {
            if ((*i)->getName() == getDstState()->getName())
            {
                dst = *i;
                break;
            }
        }

        // No destination state exists?
        if (dst == NULL)
        {
            dst = getDstState()->clone(GraphComponent(fsm, getDstState()->getId()));
        }

        // Connect the transition to the source and destination state
        t->connect(src, dst);

        return t;
    }

    /**
     * connect ()
     * Connect the FSM transition to the source state.
     */
    void FSMtransition::connect(FSMstate *src, FSMstate *dst)
    {
        srcState = src;
        dstState = dst;
        src->connectTransition(this);
    }

    /**
     * print()
     * Print the FSM transition to the supplied output stream.
     */
    ostream &FSMtransition::print(ostream &out)
    {
        out << "transition src: " << srcState->getName() << endl;
        out << "transition dst: " << dstState->getName() << endl;

        return out;
    }

    /**
     * FSM ()
     * Constrcutro
     */
    FSM::FSM(GraphComponent c)
        :
        GraphComponent(c),
        initialState(NULL)
    {
    }

    /**
     * ~FSM ()
     * Destrcutor.
     */
    FSM::~FSM()
    {
        // States
        for (FSMstates::iterator i = states.begin(); i != states.end(); i++)
            delete *i;
    }

    /**
     * create ()
     * Create a new FSM object.
     */
    FSM *FSM::create(GraphComponent c) const
    {
        return new FSM(c);
    }

    /**
     * createCopy ()
     * Create a copy of this FSM component.
     */
    FSM *FSM::createCopy(GraphComponent c) const
    {
        FSM *fsm = create(c);

        // Properties
        fsm->setName(getName());

        return fsm;
    }

    /**
     * clone ()
     * Create a cloned copy of this object.
     */
    FSM *FSM::clone(GraphComponent c) const
    {
        FSM *fsm = createCopy(c);

        // Clone the initial state (this trigers cloning of all reachable states)
        fsm->setInitialState(getInitialState()->clone(GraphComponent(fsm,
                             getInitialState()->getId())));

        return fsm;
    }

    /**
     * getState()
     * The function returns a pointer to the statewith the supplied name.
     */
    FSMstate *FSM::getState(const CString &name) const
    {
        for (FSMstates::const_iterator i = states.begin();
             i != states.end(); i++)
        {
            if ((*i)->getName() == name)
                return (*i);
        }

        throw CException("FSM does not contain a state with name '" + name + "'.");
    }

    /**
     * constructFromXML ()
     * Construct the FSM from the XML node.
     */
    void FSM::constructFromXML(const CNodePtr fsmNode)
    {
        CString name;
        Scenario *s;
        FSMstate *state;
        FSMtransition *transition;

        // States
        for (CNode *stateNode = CGetChildNode(fsmNode, "state");
             stateNode != NULL; stateNode = CNextNode(stateNode, "state"))
        {
            // Name
            if (!CHasAttribute(stateNode, "name"))
                throw CException("FSM state has no name.");
            name = CGetAttribute(stateNode, "name");

            // Scenario
            if (!CHasAttribute(stateNode, "scenario"))
                throw CException("FSM state has no scenario.");
            s = getGraph()->getScenario(CGetAttribute(stateNode, "scenario"));

            // Create a new state
            state = new FSMstate(GraphComponent(this, getStates().size(), name));
            state->setScenario(s);
            addState(state);
        }

        // Transitions
        for (CNode *stateNode = CGetChildNode(fsmNode, "state");
             stateNode != NULL; stateNode = CNextNode(stateNode, "state"))
        {
            // Name
            if (!CHasAttribute(stateNode, "name"))
                throw CException("FSM state has no name.");
            name = CGetAttribute(stateNode, "name");

            // Get the state
            state = getState(name);

            for (CNode *transNode = CGetChildNode(stateNode, "transition");
                 transNode != NULL;
                 transNode = CNextNode(transNode, "transition"))
            {
                // Destination state
                if (!CHasAttribute(transNode, "destination"))
                    throw CException("FSM transition has no destination.");

                // Create a new transition
                transition = new FSMtransition(GraphComponent(this, 0));
                transition->connect(state,
                                    getState(CGetAttribute(transNode, "destination")));
            }
        }

        // Initial state
        if (!CHasAttribute(fsmNode, "initialstate"))
            throw CException("FSM has no initial state.");
        setInitialState(getState(CGetAttribute(fsmNode, "initialstate")));
    }

    /**
     * convertToXML ()
     * Convert the FSM to an XML node.
     */
    void FSM::convertToXML(const CNodePtr fsmNode)
    {
        // Initial state
        CAddAttribute(fsmNode, "initialstate", getInitialState()->getName());

        // States
        for (FSMstates::iterator i = states.begin(); i != states.end(); i++)
        {
            CNode *stateNode = CAddNode(fsmNode, "state");
            FSMstate *s = *i;

            // Name
            CAddAttribute(stateNode, "name", s->getName());

            // Scenario
            CAddAttribute(stateNode, "scenario", s->getScenario()->getName());

            // Transitions
            for (FSMtransitionsCIter j = s->getTransitions().begin();
                 j != s->getTransitions().end(); j++)
            {
                FSMtransition *t = *j;

                CNode *transitionNode = CAddNode(stateNode, "transition");
                CAddAttribute(transitionNode, "destination",
                              t->getDstState()->getName());
            }
        }
    }

    /**
     * print ()
     * Print the FSM to the supplied output stream.
     */
    ostream &FSM::print(ostream &out)
    {
        out << "FSM" << endl;
        out << "initial state: " << initialState->getScenario()->getName() << endl;

        // States
        for (FSMstates::iterator i = states.begin(); i != states.end(); i++)
            (*i)->print(out);

        out << endl;

        return out;
    }

} // End namespace FSMSADF

