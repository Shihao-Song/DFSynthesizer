/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_consistency.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   2 December 2009
 *
 *  Function        :   Consistency of SADF Graphs
 *
 *  History         :
 *      29-08-06    :   Initial version.
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

#include "sadf_consistency.h"

// Function to determine whether SADF is strongly consistent

bool SADF_Verify_StrongConsistency(SADF_Graph *Graph)
{

    if (Graph->getNumberOfDetectors() == 0)
        if (isSDFgraphConsistent(SADF2UntimedSDF(Graph)))
            return true;

    if (Graph->getNumberOfDetectors() == 1)
    {

        bool Consistent = true;

        for (CId i = 0; Consistent && i != Graph->getDetector(0)->getNumberOfSubScenarios(); i++)
        {

            vector<CId> SubScenarios(1, i);

            SADF_Graph *ScenarioGraph = SADF_FixScenarios(Graph, SubScenarios, false);

            for (CId j = 0; Consistent && j != ScenarioGraph->getNumberOfDataChannels(); j++)
                if (ScenarioGraph->getDataChannel(j)->getConsumptionRate(0) == 0 || ScenarioGraph->getDataChannel(j)->getProductionRate(0) == 0)
                    Consistent = false;

            if (Consistent)
            {

                ScenarioGraph->setName(Graph->getDetector(0)->getSubScenarioName(i));

                SDFgraph *SDF = SADF2UntimedSDF(ScenarioGraph);

                if (isSDFgraphConsistent(SDF))
                {

                    RepetitionVector Vector = computeRepetitionVector(SDF);

                    if (Vector[SDF->getActor(Graph->getDetector(0)->getName())->getId()] != 1)
                        Consistent = false;
                }
            }
        }

        return Consistent;
    }

    // Case of more than one detectors is not yet covered

    return false;
}
