/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   processor.h
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
 * $Id: processor.h,v 1.1 2009-12-23 13:37:22 sander Exp $
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

#ifndef FSMSADF_RESOURCE_ALLOCATION_PLATFORM_BINDING_PROCESSOR_H_INCLUDED
#define FSMSADF_RESOURCE_ALLOCATION_PLATFORM_BINDING_PROCESSOR_H_INCLUDED

#include "../platform_graph/graph.h"
#include "../../base/graph.h"
#include "../scheduling/static_order/static_order_schedule.h"

namespace FSMSADF
{

    // Forward class definition
    class ProcessorBinding;
    class TileBinding;

    // List of processor binding pointers
    typedef list<ProcessorBinding *> ProcessorBindings;

    /**
     * ProcessorBinding
     * Container for processor binding.
     */
    class ProcessorBinding : public GraphComponent
    {
        public:

            // Constructor
            ProcessorBinding(GraphComponent c, Processor *p);

            // Destructor
            ~ProcessorBinding();

            // Construct
            ProcessorBinding *clone(GraphComponent c);

            // Construct
            void constructFromXML(Scenario *s, const CNodePtr processorNode);
            void constructResourceUsageFromXML(const CNodePtr processorNode);

            // Convert
            void convertToXML(Scenario *s, const CNodePtr processorNode);
            void convertResourceUsageToXML(const CNodePtr processorNode);

            // Processor
            Processor *getProcessor() const
            {
                return processor;
            };

            // Actor bindings
            map<Scenario *, Actors> &getActorBindings()
            {
                return actorBindings;
            };
            void setActorBindings(const map<Scenario *, Actors> &a)
            {
                actorBindings = a;
            };
            bool addActorBinding(Scenario *s, Actor *a);
            void removeActorBinding(Scenario *s, const Actor *a);
            bool hasActorBinding(Scenario *s, const Actor *a) const;

            // Tile binding
            TileBinding *getTileBinding() const
            {
                return (TileBinding *)(getParent());
            };

            // Static order schedules
            map<Scenario *, StaticOrderSchedule> &getStaticOrderSchedules()
            {
                return staticOrderSchedules;
            };
            void setStaticOrderSchedules(const map<Scenario *, StaticOrderSchedule> &s)
            {
                staticOrderSchedules = s;
            };
            StaticOrderSchedule &getStaticOrderSchedule(Scenario *s);
            void setStaticOrderSchedule(Scenario *s, StaticOrderSchedule &so);
            void removeStaticOrderSchedule(Scenario *s);
            bool hasStaticOrderSchedule(Scenario *s) const;

            // Resource allocation
            map<Scenario *, Time> &getWheelsizeAllocations()
            {
                return wheelsize;
            };
            void setWheelsizeAllocations(const map<Scenario *, Time> &w)
            {
                wheelsize = w;
            };
            bool hasResourcesWheelSize(Scenario *s, Time t) const;
            bool allocateWheelsize(Scenario *s, Time t);

            // Resource occupancy
            Time getAvailableWheelsize(Scenario *s) const;
            Time getAllocatedWheelsize(Scenario *s) const;

            // Resource occupancy of application graph
            Time getWheelsizeUsedForGraph() const;

            // Initial resource occupancy
            Time getWheelsizeUsedForOtherGraphs() const
            {
                return wheelsizeUsedForOtherGraphs;
            };
            void setWheelsizeUsedForOtherGraphs(Time sz)
            {
                wheelsizeUsedForOtherGraphs = sz;
            };

        private:
            // Processor
            Processor *processor;

            // Actor bindings
            map<Scenario *, Actors> actorBindings;

            // Static-order schedules
            map<Scenario *, StaticOrderSchedule> staticOrderSchedules;

            // TDMA wheelsize allocation
            map<Scenario *, Time> wheelsize;

            // TDMA wheelsize used for other graphs
            Time wheelsizeUsedForOtherGraphs;
    };

} // End namespace FSMSADF

#endif

