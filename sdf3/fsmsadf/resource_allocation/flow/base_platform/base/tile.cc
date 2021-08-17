/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   tile.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 14, 2009
 *
 *  Function        :   Graph mapping to MP-SoC
 *
 *  History         :
 *      29-05-09    :   Initial version.
 *
 * $Id: tile.cc,v 1.1 2009-12-23 13:37:21 sander Exp $
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

#include "flow.h"

namespace FSMSADF
{

    /**
     * estimateBandwidthConstraint ()
     * Estimate bandwidth requirement of the channels when mapped to a connection.
     */
    void SDF3FlowBase::estimateBandwidthConstraint()
    {
        // Output current state of the flow
        logInfo("Estimate bandwidth constraints.");

        if (!tileBindingAlgo->estimateBandwidthConstraint())
            setNextStateOfFlow(FlowFailed);
        else
            setNextStateOfFlow(FlowBindGraphtoTile);
    }

    /**
     * bindGraphtoTiles ()
     * Bind graph to the tile resources.
     */
    void SDF3FlowBase::bindGraphtoTiles()
    {
        // Output current state of the flow
        logInfo("Bind graph to tiles.");

        if (!tileBindingAlgo->bindGraphtoTiles(getMaxNrBindingsTileBindingAlgo()))
            setNextStateOfFlow(FlowFailed);
        else
            setNextStateOfFlow(FlowEstimateConnectionDelay);
    }

    /**
     * estimateConnectionDelay ()
     * Estimaet delay constraints of channels when bound to a connection.
     */
    void SDF3FlowBase::estimateConnectionDelay()
    {
        // Output current state of the flow
        logInfo("Estimate connection delay.");

        if (!tileBindingAlgo->estimateConnectionDelay())
            setNextStateOfFlow(FlowFailed);
        else
            setNextStateOfFlow(FlowConstructTileSchedules);
    }

    /**
     * constructTileSchedules ()
     * Construct a static-order and TDMA schedule for every tile to which actors
     * are bound.
     */
    void SDF3FlowBase::constructTileSchedules()
    {
        // Output current state of the flow
        logInfo("Construct schedules per tile.");

        if (!tileBindingAlgo->constructTileSchedules(getMaxNrBindingsTileBindingAlgo()))
            setNextStateOfFlow(FlowSelectStorageDist);
        else
            setNextStateOfFlow(FlowCompleted);
    }

} // End namespace FSMSADF
