/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   edf.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   June 2, 2009
 *
 *  Function        :   Earliest deadline first scheduling
 *
 *  History         :
 *      02-06-09    :   Initial version.
 *
 * $Id: edf.h,v 1.1 2009-12-23 13:37:23 sander Exp $
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

#ifndef FSMSADF_RESOURCE_ALLOCATION_SCHEDULING_EARLIEST_DEADLINE_FIRST_EDF_H_INCLUDED
#define FSMSADF_RESOURCE_ALLOCATION_SCHEDULING_EARLIEST_DEADLINE_FIRST_EDF_H_INCLUDED

#include "precedence_graph.h"
#include "../../flow/base_platform/binding_aware_graph/binding_aware_graph.h"

namespace FSMSADF
{

    /**
     * Earliest deadline first scheduling algorithm.
     */
    class EarliestDeadLineFirstScheduling
    {
        public:
            // Constructor
            EarliestDeadLineFirstScheduling() { };

            // Destructor
            ~EarliestDeadLineFirstScheduling() { };

            // Create EDF for all scenario graphs
            void schedule(PlatformBinding *pb, BindingAwareGraphBase *g);

        private:
            // Schedule binding-aware graph in scenario s
            void scheduleScenario(PlatformBinding *b, BindingAwareGraphBase *g,
                                  Scenario *s);
    };

} // End namspace FSMSADF

#endif

