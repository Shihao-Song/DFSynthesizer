/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   throughput.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 4, 2009
 *
 *  Function        :   FSM-based SADF throughput analysis algorithms
 *
 *  History         :
 *      04-05-09    :   Initial version.
 *
 * $Id: throughput.h,v 1.1.2.6 2010-05-07 20:43:32 mgeilen Exp $
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

#ifndef FSMSADF_ANALYSIS_THROUGHPUT_THROUGHPUT_H_INCLUDED
#define FSMSADF_ANALYSIS_THROUGHPUT_THROUGHPUT_H_INCLUDED

/**
 * Throughput analysis
 * Compute the throughput of an FSM-based SDAF graph while ignoring the FSM.
 *
 * Algorithm:
 * 1. Determine for each scenario s the eigenvector v_s and period T_s.
 * 2. Compute a suitable 'reference schedule'. This can be done by for example,
 *    taking the max over all eigenvectors. An alternative could be to use the
 *    ALAP eigenvector. Currently, the first option is used.
 * 3. Compute the delay for each scenario s using the reference schedule and the
 *    period of scenario s.
 * 4. The throughput is equal to max_s (d_s + T_s).
 */
#include "scenariograph.h"

/**
 * Throughput analysis using reference schedule
 * Compute the throughput of an FSM-based SDAF graph considering the FSM and a
 * reference schedule.
 *
 * Algorithm:
 * 1. Determine for each scenario s the eigenvector v_s and period T_s.
 * 2. Compute a suitable 'reference schedule'. This can be done by for example,
 *    taking the max over all eigenvectors. An alternative could be to use the
 *    ALAP eigenvector. Currently, the first option is used.
 * 3. Compute the delay for each scenario s using the reference schedule and the
 *    period of scenario s.
 * 4. The throughput is equal to the MCM of the FSM where the weight of a
 *    node which corresponds to scenario s is equal to (d_s + T_s).
 */
#include "fsm_ref_schedule.h"

/**
 * Throughput analysis considering scenario transitions
 * Compute the throughput of an FSM-based SDAF graph considering the FSM the
 * schedules of each source and destination scenario in the FSM.
 *
 * Algorithm:
 * 1. Determine for each scenario s the eigenvector v_s and period T_s.
 * 2. The throughput is equal to the MCM of the FSM where the weight of an edge
 *    which represents a scenario transition from scenario s to scenario t is
 *    equal to: d_st = exploreDelayForSchedule(v_s, v_t, T_t).
 */
#include "fsm_scenario_transitions.h"

/**
 * Throughput analysis of an FSM-based SDAF graph through a state-space
 * traversal.
 */
#include "statespace.h"

/**
 * Throughput analysis of an FSM-based SDAF graph through a maxplus
 * automaton.
 */
#include "maxplusautomaton.h"


#endif

