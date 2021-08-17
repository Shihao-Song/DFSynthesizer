/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   static_order_schedule.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 14, 2009
 *
 *  Function        :   Static-order schedule for FSM-based SADF graphs
 *
 *  History         :
 *      14-05-09    :   Initial version.
 *
 * $Id: static_order_schedule.cc,v 1.1 2009-12-23 13:37:23 sander Exp $
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

#include "static_order_schedule.h"
#include "../../../analysis/base/repetition_vector.h"

namespace FSMSADF
{

    /**
     * constructFromXML ()
     * Constructs the schedule from an XML representation.
     */
    void StaticOrderSchedule::constructFromXML(ScenarioGraph *g,
            const CNodePtr scheduleNode)
    {
        // Iterate over the state nodes in the schedule
        for (CNode *stateNode = CGetChildNode(scheduleNode, "state");
             stateNode != NULL; stateNode = CNextNode(scheduleNode, "state"))
        {
            // Actor?
            if (!CHasAttribute(stateNode, "actor"))
                throw CException("Static-order schedule state has no actor.");
            Actor *a = g->getActor(CGetAttribute(stateNode, "actor"));

            // Add actor to the schedule
            appendActor(a);

            // Is the actor the start of the periodic regime?
            if (CHasAttribute(stateNode, "startOfPeriodicRegime")
                && CGetAttribute(stateNode, "startOfPeriodicRegime") == "true")
            {
                setStartPeriodicSchedule(last());
            }
        }
    }

    /**
     * convertToXML ()
     * Convert the schedule to an XML representation.
     */
    void StaticOrderSchedule::convertToXML(const CNodePtr scheduleNode)
    {
        CNode *stateNode;

        // Create a state node for each schedule entry
        for (uint i = 0 ; i < size(); i++)
        {
            StaticOrderSchedule::const_iterator pos = getScheduleEntry(i);
            stateNode = CAddNode(scheduleNode, "state");

            // Check that the state is associated with an actor
            ASSERT(pos->actor != NULL, "State not associated with an actor.");

            // Set attributes on the node
            CAddAttribute(stateNode, "actor", pos->actor->getName());

            if (i == startPeriodicSchedule)
            {
                CAddAttribute(stateNode, "startOfPeriodicRegime", "true");
            }
        }
    }

    /**
     * minimize ()
     * The function returns a new static-order schedule in which the length of
     * the current schedule is reduced as much as possible. This new schedule
     * contains the smallest repeated part in the periodic phase of the schedule.
     * From the transient phase, all complete repetitions of the periodic phase are
     * removed. The schedule entries in the minimized schedule are assigned new
     * id's once the minimization is completed.
     */
    void StaticOrderSchedule::minimize()
    {
        uint periodicStart, periodicEnd, transientEnd, posPattern, posSchedule;
        StaticOrderSchedule newSchedule;
        bool startOfPattern = true;

        // Empty schedule?
        if (empty())
            return;

        // Start of the periodic schedule
        periodicStart = getStartPeriodicSchedule();
        ASSERT(periodicStart != UINT_MAX, "No periodic regime in the schedule");

        // Find smallest repeated part in the periodic schedule
        periodicEnd = periodicStart;
        posPattern = periodicStart;
        posSchedule = next(periodicEnd);

        while (posSchedule != periodicStart)
        {
            // Examining position in schedule (so no longer at start of pattern)
            startOfPattern = false;

            // Actor at posSchedule not equal to actor at posPattern?
            if (getScheduleEntry(posSchedule)->actor->getId()
                != getScheduleEntry(posPattern)->actor->getId())
            {
                // Add schedule part up-to and including posSchedule to the
                // pattern and restart the search starting from the next
                // position in the schedule
                periodicEnd = posSchedule;
                posPattern = periodicStart;
            }
            else
            {
                // Actor in schedule matches with actor in pattern, move one
                // position forward in pattern and in schedule (loop on pattern
                // if needed).
                posPattern = next(posPattern);
                if (posPattern == next(periodicEnd))
                {
                    posPattern = periodicStart;
                    startOfPattern = true;
                }
            }

            // Last element reached?
            if (isLast(posSchedule))
                break;

            // Next
            posSchedule = next(posSchedule);
        }

        // Is the exact end of the repeated part of the schedule not reached?
        if (!startOfPattern)
        {
            // No exact repetition of pattern
            periodicEnd = posSchedule;
        }

        // Remove actor from non-repeated part of the schedule
        // An part can be removed if (starting from the back), the
        // complete repeated pattern is found in the transient part.
        // If not, no further reduction of the transient part is
        // possible.
        transientEnd = previous(periodicStart);
        posPattern = periodicEnd;
        posSchedule = transientEnd;
        while (getScheduleEntry(posSchedule) != end())
        {
            if (getScheduleEntry(posSchedule)->actor->getId()
                != getScheduleEntry(posPattern)->actor->getId())
            {
                break;
            }
            else
            {
                posSchedule = previous(posSchedule);
                if (posPattern == periodicStart)
                {
                    posPattern = periodicEnd;
                    transientEnd = posSchedule;
                }
                else
                {
                    posPattern = previous(posPattern);
                }
            }
        }

        // Create a new schedule
        // Step 1: Is there a transient part?
        if (getScheduleEntry(transientEnd) != end())
        {
            // Add transient to the schedule
            newSchedule.insert(newSchedule.end(), begin(),
                               getScheduleEntry(++transientEnd));

            // Set the start of the periodic phase
            newSchedule.setStartPeriodicSchedule(transientEnd);
        }
        else
        {
            // Set the start of the periodic phase
            newSchedule.setStartPeriodicSchedule(0);
        }

        // Step 2: Add periodic part to the schedule
        newSchedule.insert(newSchedule.end(), getScheduleEntry(periodicStart),
                           getScheduleEntry(++periodicEnd));

        // Put new schedule in place of the old schedule
        *this = newSchedule;
    }

    /**
     * changeActorAssociations ()
     * Change the associated actors in the schedule.
     */
    void StaticOrderSchedule::changeActorAssociations(ScenarioGraph *newGraph)
    {
        StaticOrderSchedule::iterator pos = begin();

        while (pos != end())
        {
            if (newGraph->getActor(pos->actor->getName()) != NULL)
            {
                pos->actor = newGraph->getActor(pos->actor->getName());

                // Next
                pos++;
            }
            else
            {
                // Entry at pos inside the transient?
                if (pos < begin() + startPeriodicSchedule)
                    startPeriodicSchedule--;

                pos = erase(pos);
            }
        }
    }

    /**
     * isPeriodic()
     * The function returns true when the static-order schedule is a periodic
     * schedule. Otherwise it returns false.
     */
    bool StaticOrderSchedule::isPeriodic() const
    {
        if (startPeriodicSchedule == 0)
            return true;
        return false;
    }

    /**
     * getActorCount()
     * The function returns the number of schedule entries of each actor which
     * appears in the schedule.
     */
    map<Actor *, uint> StaticOrderSchedule::getActorCount() const
    {
        map<Actor *, uint> cnt;

        for (StaticOrderSchedule::const_iterator pos = begin();
             pos != end(); pos++)
        {
            if (cnt.find(pos->actor) == cnt.end())
                cnt[pos->actor] = 1;
            else
                cnt[pos->actor]++;
        }

        return cnt;
    }

    /**
     * getIterationLength()
     * The function returns the iteration length of the schedule considering this
     * schedule and the scenario graph g under the scenario s.
     */
    uint StaticOrderSchedule::getIterationLength(ScenarioGraph *g,
            Scenario *s) const
    {
        uint repVecLength = 0;
        set<Actor *> actors;

        // Length of the schedule
        uint schLength = last() - getStartPeriodicSchedule() + 1;

        // Number of actor firings
        RepetitionVector repVec = computeRepetitionVector(g, s);
        for (StaticOrderSchedule::const_iterator pos = begin();
             pos != end(); pos++)
        {
            // Actor not seen before in the schedule?
            if (actors.find(pos->actor) == actors.end())
            {
                repVecLength += repVec[pos->actor->getId()];
                actors.insert(pos->actor);
            }
        }

        // The iteration length is smallest denominator of the ratio between the
        // repetition vector length and schedule length
        CFraction iterLength(repVecLength, schLength);
        iterLength = iterLength.lowestTerm();
        return (uint) iterLength.denominator();
    }

} // End namespace FSMSADF

