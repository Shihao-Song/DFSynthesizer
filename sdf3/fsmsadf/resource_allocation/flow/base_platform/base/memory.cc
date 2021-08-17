/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   memory.cc
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
 * $Id: memory.cc,v 1.1 2009-12-23 13:37:21 sander Exp $
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
     * computeStorageDist ()
     * Compute storage distributions for the application graph.
     */
    void SDF3FlowBase::computeStorageDist()
    {
        // Output current state of the flow
        logInfo("Compute storage distributions.");

        if (!memoryDimAlgo->computeStorageDist())
            setNextStateOfFlow(FlowFailed);
        else
            setNextStateOfFlow(FlowSelectStorageDist);
    }

    /**
     * selectStorageDist ()
     * Select storage distribution.
     */
    void SDF3FlowBase::selectStorageDist()
    {
        // Output current state of the flow
        logInfo("Select storage distribution.");

        if (!memoryDimAlgo->selectStorageDist())
            setNextStateOfFlow(FlowFailed);
        else
            setNextStateOfFlow(FlowEstimateStorageDist);
    }

    /**
     * estimateStorageDist ()
     * Estimate storage distibution
     */
    void SDF3FlowBase::estimateStorageDist()
    {
        // Output current state of the flow
        logInfo("Estimate storage distribution.");

        if (!memoryDimAlgo->estimateStorageDist())
            setNextStateOfFlow(FlowFailed);
        else
            setNextStateOfFlow(FlowEstimateBandwidthConstraint);
    }

} // End namespace FSMSADF
