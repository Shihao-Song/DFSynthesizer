/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   static_order_schedule.h
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
 * $Id: static_order_schedule.h,v 1.1 2009-12-23 13:37:23 sander Exp $
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

#ifndef FSMSADF_RESOURCE_ALLOCATION_SCHEDULING_STATIC_ORDER_STATIC_ORDER_SCHEDULE_H_INCLUDED
#define FSMSADF_RESOURCE_ALLOCATION_SCHEDULING_STATIC_ORDER_STATIC_ORDER_SCHEDULE_H_INCLUDED

#include "../../../base/graph.h"

namespace FSMSADF
{

    /**
     * Static-order schedule entry
     * One position in a static-order schedule.
     **/
    class StaticOrderScheduleEntry
    {
        public:
            // Constructor
            StaticOrderScheduleEntry(Actor *a = NULL)
                : actor(a) {};

            // Destructor
            ~StaticOrderScheduleEntry() {};

            // Actor scheduled with this entry
            Actor *actor;
    };

    /**
     * Static-order schedule
     * A static-order schedule provides a sequence of actor firings. The schedule
     * may contain a cycle which makes it effectively an infinite schedule.
     **/
    class StaticOrderSchedule : public vector<StaticOrderScheduleEntry>
    {
        public:
            // Constructor
            StaticOrderSchedule()
            {
                startPeriodicSchedule = UINT_MAX;
            };

            // Destructor
            ~StaticOrderSchedule() {};

            // Construct
            void constructFromXML(ScenarioGraph *g, const CNodePtr scheduleNode);

            // Convert
            void convertToXML(const CNodePtr scheduleNode);

            // Append actor to the schedule
            void appendActor(Actor *a)
            {
                push_back(StaticOrderScheduleEntry(a));
            };

            // Append schedule entry to the schedule
            void appendScheduleEntry(StaticOrderScheduleEntry &s)
            {
                push_back(s);
            };

            // Insert actor befere the element at position into the schedule and return
            // iterator to newly inserted element
            StaticOrderSchedule::iterator insertActor(
                StaticOrderSchedule::iterator position, Actor *a)
            {
                // Update start of periodic schedule when new element is in transient
                // and a periodic regime has already been defined
                if (startPeriodicSchedule != UINT_MAX && begin() + startPeriodicSchedule > position)
                {
                    startPeriodicSchedule++;
                }

                // Insert actor
                return insert(position, StaticOrderScheduleEntry(a));
            };

            // Access schedule entry
            StaticOrderSchedule::iterator getScheduleEntry(const uint i)
            {
                if (i >= size())
                    return end();
                return (begin() + i);
            };

            // Loop-back in schedule
            uint getStartPeriodicSchedule() const
            {
                return startPeriodicSchedule;
            };
            void setStartPeriodicSchedule(const uint i)
            {
                startPeriodicSchedule = i;
            };

            // Next entry in the schedule
            uint next(const uint current) const
            {
                if (isLast(current))
                    return getStartPeriodicSchedule();
                return (current + 1);
            };

            // Previous entry in the schedule
            uint previous(const uint current) const
            {
                if (current == 0)
                    return UINT_MAX;
                return (current - 1);
            };

            // Last entry in the schedule
            uint last() const
            {
                return size() - 1;
            }

            // Last entry before the loop-back
            bool isLast(const uint current) const
            {
                if (current == last())
                    return true;
                return false;
            }

            // Compute a minimized version of the schedule
            void minimize();

            // Change the associated actors in the schedule
            void changeActorAssociations(ScenarioGraph *newGraph);

            // Is the schedule periodic?
            bool isPeriodic() const;

            // Number of schedule entries for actors in the schedule
            map<Actor *, uint> getActorCount() const;

            // Iteration length of the schedule for scenario graph g under scenario s
            uint getIterationLength(ScenarioGraph *g, Scenario *s) const;

        private:
            uint startPeriodicSchedule;
    };

} // End namespace FSMSADF

#endif

