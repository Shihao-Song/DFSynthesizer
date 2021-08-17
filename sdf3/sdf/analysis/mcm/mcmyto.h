/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   mcmyto.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   March 28, 2011
 *
 *  Function        :   Compute the MCM for an HSDF graph using Young-
 *                      Tarjan-Orlin's algorithm.
 *
 *  Disclamer       :   This code is based the 'mmcycle' program which can be
 *                      found at 'http://elib.zib.de/pub/Packages/mathprog
 *                      /netopt/mmc-info'. The original code is written by
 *                      Georg Skorobohatyj (bzfskoro@zib.de) and is free
 *                      for redistribution.
 *
 *  History         :
 *      28-03-11    :   Initial version.
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

#ifndef SDF_ANALYSIS_MCM_MCMYTO_H_INCLUDED
#define SDF_ANALYSIS_MCM_MCMYTO_H_INCLUDED

#include "mcmgraph.h"
namespace SDF
{
    /**
     * mcmYoungTarjanOrlin ()
     * The function computes the maximum cycle mean of edge weight per edge of
     * an MCMgraph using Young-Tarjan-Orlin's algorithm.
     */
    CDouble maxCycleMeanYoungTarjanOrlin(MCMgraph *mcmGraph);

    /**
     * maxCycleMeanAndCriticalCycleYoungTarjanOrlin ()
     * The function computes the maximum cycle mean of edge weight of
     * an MCMgraph using Young-Tarjan-Orlin's algorithm.
     * It returns both the MCM and a critical cycle
     * The critical cycle is only returned if cycle and len are not NULL. Then *cycle points
     * to an array of *MCMEdges of the critical cycle and *len indicates the length of the cycle.
     * *cycle is a freshly allocated array and it is the caller's obligation to deallocate it
     * in due time.
     */
    CDouble maxCycleMeanAndCriticalCycleYoungTarjanOrlin(MCMgraph *mcmGraph, MCMedge *** cycle, uint *len);

    /**
     * maxCycleRatioYoungTarjanOrlin ()
     * The function computes the maximum cycle ratio of edge weight over delay of
     * an MCMgraph using Young-Tarjan-Orlin's algorithm.
     */
    CDouble maxCycleRatioYoungTarjanOrlin(MCMgraph *mcmGraph);

    /**
     * maxCycleRatioAndCriticalCycleYoungTarjanOrlin ()
     * The function computes the maximum cycle ratio of edge weight over delay of
     * an MCMgraph using Young-Tarjan-Orlin's algorithm.
     * It returns both the MCR and a critical cycle
     * The critical cycle is only returned if cycle and len are not NULL. Then *cycle points
     * to an array of *MCMEdges of the critical cycle and *len indicates the length of the cycle.
     * *cycle is a freshly allocated array and it is the caller's obligation to deallocate it
     * in due time.
     */
    CDouble maxCycleRatioAndCriticalCycleYoungTarjanOrlin(MCMgraph *mcmGraph, MCMedge *** cycle, uint *len);


    /**
     * minCycleRatioYoungTarjanOrlin ()
     * The function computes the minimum cycle ratio of edge weight over delay of
     * an MCMgraph using Young-Tarjan-Orlin's algorithm.
     */
    CDouble minCycleRatioYoungTarjanOrlin(MCMgraph *mcmGraph);

    /**
     * minCycleRatioAndCriticalCycleYoungTarjanOrlin ()
     * The function computes the minimum cycle ratio of edge weight over delay of
     * an MCMgraph using Young-Tarjan-Orlin's algorithm.
     * It returns both the MCR and a critical cycle
     * The critical cycle is only returned if cycle and len are not NULL. Then *cycle points
     * to an array of *MCMEdges of the critical cycle and *len indicates the length of the cycle.
     * *cycle is a freshly allocated array and it is the caller's obligation to deallocate it
     * in due time.
     */
    CDouble minCycleRatioAndCriticalCycleYoungTarjanOrlin(MCMgraph *mcmGraph, MCMedge *** cycle, uint *len);

}//namespace SDF
#endif

