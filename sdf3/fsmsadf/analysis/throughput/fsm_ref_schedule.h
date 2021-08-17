/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   fsm_ref_schedule.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 4, 2009
 *
 *  Function        :   FSM-based SADF throughput analysis algorithm using
 *                      scenario graph with reference schedule.
 *
 *  History         :
 *      04-05-09    :   Initial version.
 *
 * $Id: fsm_ref_schedule.h,v 1.1.2.2 2010-04-22 07:07:09 mgeilen Exp $
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

#ifndef FSMSADF_ANALYSIS_THROUGHPUT_FSM_REF_SCHEDULE_H_INCLUDED
#define FSMSADF_ANALYSIS_THROUGHPUT_FSM_REF_SCHEDULE_H_INCLUDED

#include "../../base/graph.h"

namespace FSMSADF
{

    /**
     * Throughput analysis using reference schedule
     * Compute the throughput of an FSM-based SDAF graph considering the FSM and a
     * reference schedule.
     */
    Throughput throughputAnalysisUsingRefSchedule(Graph *g);

}  // End namespace FSMSADF

#endif

