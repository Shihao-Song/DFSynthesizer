/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   processor.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 12, 2009
 *
 *  Function        :   Tile processor.
 *
 *  History         :
 *      12-05-09    :   Initial version.
 *
 * $Id: processor.cc,v 1.1 2009-12-23 13:37:22 sander Exp $
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

#include "processor.h"
#include "graph.h"

namespace FSMSADF
{

    /**
     * ProcessorBinding()
     * Constructor.
     */
    ProcessorBinding::ProcessorBinding(GraphComponent c, Processor *p)
        :
        GraphComponent(c),
        processor(p),
        wheelsizeUsedForOtherGraphs(0)
    {
    }

    /**
     * ~ProcessorBinding()
     * Destructor.
     */
    ProcessorBinding::~ProcessorBinding()
    {
    }

    /**
     * clone()
     * Create a cloned copy of this processor binding.
     */
    ProcessorBinding *ProcessorBinding::clone(GraphComponent c)
    {
        ProcessorBinding *pb = new ProcessorBinding(c, getProcessor());

        // Actor bindings
        pb->setActorBindings(getActorBindings());

        // Static-order schedules
        pb->setStaticOrderSchedules(getStaticOrderSchedules());

        // TDMA wheelsize allocation
        pb->setWheelsizeAllocations(getWheelsizeAllocations());

        // TDMA wheelsize used for other graphs
        pb->setWheelsizeUsedForOtherGraphs(getWheelsizeUsedForOtherGraphs());

        return pb;
    }

    /**
     * constructFromXML()
     * This function creates the processor binding as specified by the
     * processorNode.
     */
    void ProcessorBinding::constructFromXML(Scenario *s,
                                            const CNodePtr processorNode)
    {
        // Application graph
        ScenarioGraph *sg = s->getScenarioGraph();

        // Iterate over the list of actors bound to this processor
        for (CNode *n = CGetChildNode(processorNode, "actor");
             n != NULL; n = CNextNode(processorNode, "actor"))
        {
            // Name specified for the actor?
            if (!CHasAttribute(n, "name"))
                throw CException("Mapped actor has no name identifier.");

            // Find actor in scenario graph
            Actor *a = sg->getActor(CGetAttribute(n, "name"));

            // Create binding of actor to processor
            addActorBinding(s, a);
        }

        // Arbitation
        if (CHasChildNode(processorNode, "arbitration"))
        {
            CNode *arbitrationNode = CGetChildNode(processorNode, "arbitration");

            // Wheel size
            allocateWheelsize(s, CGetAttribute(arbitrationNode, "timeslice"));

            // Static order schedule
            if (CHasChildNode(arbitrationNode, "schedule"))
            {
                // Create schedule
                StaticOrderSchedule so;
                so.constructFromXML(sg, CGetChildNode(arbitrationNode, "schedule"));

                // Set schedule of the processor
                setStaticOrderSchedule(s, so);
            }
        }
    }

    /**
     * constructFromXML()
     * This function sets the resource usgae of the processor.
     */
    void ProcessorBinding::constructResourceUsageFromXML(
        const CNodePtr processorNode)
    {
        // Arbitation
        if (CHasChildNode(processorNode, "arbitration"))
        {
            CNode *arbitrationNode = CGetChildNode(processorNode, "arbitration");

            // Wheel size
            setWheelsizeUsedForOtherGraphs(CGetAttribute(arbitrationNode,
                                           "timeslice"));
        }
    }

    /**
     * convertToXML()
     * This function converts the processor binding to an XML object.
     */
    void ProcessorBinding::convertToXML(Scenario *s, const CNodePtr processorNode)
    {
        map<Scenario *, Actors>::iterator scenActorIter;

        // Name
        CAddAttribute(processorNode, "name", getName());

        // Actor binding exists for this scenario?
        scenActorIter = actorBindings.find(s);
        if (scenActorIter != actorBindings.end())
        {
            // List of all actors bound to this processor
            for (Actors::iterator i = scenActorIter->second.begin();
                 i != scenActorIter->second.end(); i++)
            {
                CNode *n = CAddNode(processorNode, "actor");
                CAddAttribute(n, "name", (*i)->getName());
            }
        }

        // Arbitation
        CNode *arbitrationNode = CAddNode(processorNode, "arbitration");

        // Wheel size
        CAddAttribute(arbitrationNode, "timeslice", getAllocatedWheelsize(s));

        // Static order schedule
        if (hasStaticOrderSchedule(s))
        {
            getStaticOrderSchedule(s).convertToXML(CAddNode(arbitrationNode,
                                                   "schedule"));
        }

    }

    /**
     * convertToXML()
     * This function converts the resource usage of this processor binding
     * to an XML object.
     */
    void ProcessorBinding::convertResourceUsageToXML(const CNodePtr processorNode)
    {
        // Name
        CAddAttribute(processorNode, "name", getName());

        // Arbitation
        CNode *arbitrationNode = CAddNode(processorNode, "arbitration");

        // Wheel size
        CAddAttribute(arbitrationNode, "timeslice", getWheelsizeUsedForGraph()
                      + getWheelsizeUsedForOtherGraphs());
    }

    /**
     * addActorBinding()
     * The function binds actor a in scenario s to this processor. The function
     * returns true on success. Otherwise it returns false.
     */
    bool ProcessorBinding::addActorBinding(Scenario *s, Actor *a)
    {
        // No binding exists yet?
        if (!hasActorBinding(s, a))
            actorBindings[s].push_back(a);

        return true;
    }

    /**
     * removeActorBinding()
     * The function removes the binding of actor a in scenario s to this
     * processor.
     */
    void ProcessorBinding::removeActorBinding(Scenario *s, const Actor *a)
    {
        for (Actors::iterator i = actorBindings[s].begin();
             i != actorBindings[s].end(); i++)
        {
            if (*i == a)
            {
                actorBindings[s].erase(i);
                break;
            }
        }
    }

    /**
     * hasActorBinding()
     * The function returns true when a binding of actor a in scenario s to this
     * processor exists. Otherwise it returns false.
     */
    bool ProcessorBinding::hasActorBinding(Scenario *s, const Actor *a) const
    {
        map<Scenario *, Actors>::const_iterator scenIter = actorBindings.find(s);

        if (scenIter == actorBindings.end())
            return false;

        for (Actors::const_iterator i = scenIter->second.begin();
             i != scenIter->second.end(); i++)
        {
            if (*i == a)
                return true;
        }

        return false;
    }

    /**
     * setStaticOrderSchedule()
     * The function associates the static order schedule so with scenario s.
     */
    StaticOrderSchedule &ProcessorBinding::getStaticOrderSchedule(Scenario *s)
    {
        if (hasStaticOrderSchedule(s))
            return staticOrderSchedules.find(s)->second;

        throw CException("Scenario '" + s->getName() + "' has no schedule.");
    }

    /**
     * setStaticOrderSchedule()
     * The function associates the static order schedule so with scenario s.
     */
    void ProcessorBinding::setStaticOrderSchedule(Scenario *s,
            StaticOrderSchedule &so)
    {
        staticOrderSchedules[s] = so;
    }

    /**
     * removeStaticOrderSchedule()
     * The function removes the static-order schedule of scenario s.
     */
    void ProcessorBinding::removeStaticOrderSchedule(Scenario *s)
    {
        if (hasStaticOrderSchedule(s))
            staticOrderSchedules.erase(s);
    }

    /**
     * hasStaticOrderSchedule()
     * The function returns true when scenario s has a static order schedule.
     */
    bool ProcessorBinding::hasStaticOrderSchedule(Scenario *s) const
    {
        if (staticOrderSchedules.find(s) != staticOrderSchedules.end())
            return true;

        return false;
    }

    /**
     * hasResourcesWheelSize()
     * The function checks wether the size of timewheel which is allocated to the
     * application can be extended to time t.
     */
    bool ProcessorBinding::hasResourcesWheelSize(Scenario *s, Time t) const
    {
        if (getAvailableWheelsize(s) + getAllocatedWheelsize(s) >= t)
            return true;

        return false;
    }

    /**
     * allocateWheelsize()
     * The function changes the TMDA timewheel allocation of the application graph
     * on this processor to time t.
     */
    bool ProcessorBinding::allocateWheelsize(Scenario *s, Time t)
    {
        // Sufficient resource available?
        if (hasResourcesWheelSize(s, t))
        {
            wheelsize[s] = t;
            return true;
        }

        return false;
    }

    /**
     * getAvailableWheelsize()
     * The function returns the wheelsize which is available on this processor in
     * scenario s.
     */
    Time ProcessorBinding::getAvailableWheelsize(Scenario *s) const
    {
        return (processor->getWheelsize()
                - getAllocatedWheelsize(s)
                - wheelsizeUsedForOtherGraphs);
    }

    /**
     * getAllocatedWheelsize()
     * The function returns the wheelsize allocated to the application graph in
     * scenario s.
     */
    Time ProcessorBinding::getAllocatedWheelsize(Scenario *s) const
    {
        map<Scenario *, Time>::const_iterator i = wheelsize.find(s);

        if (i != wheelsize.end())
            return i->second;

        return 0;
    }

    /**
     * getWheelsizeUsedForGraph()
     * The function returns the maximal wheel size which has been
     * allocated to the application in any of its scenarios.
     */
    Time ProcessorBinding::getWheelsizeUsedForGraph() const
    {
        Time n = 0;
        Graph *g = getTileBinding()->getPlatformBinding()->getApplicationGraph();

        for (Scenarios::iterator i = g->getScenarios().begin();
             i != g->getScenarios().end(); i++)
        {
            n = MAX(n, getAllocatedWheelsize(*i));
        }

        return n;
    }

} // End namespace FSMSADF

