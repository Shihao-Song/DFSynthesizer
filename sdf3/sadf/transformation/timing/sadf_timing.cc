/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_timing.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   29 August 2006
 *
 *  Function        :   Transformations of SADF Graphs related to Execution Times
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

#include "sadf_timing.h"

// Function to average exection times

void SADF_Transform_AverageExecutionTimes(SADF_Graph *Graph)
{

    // Average execution times of kernels

    for (CId i = 0; i != Graph->getNumberOfKernels(); i++)
        for (CId j = 0; j != Graph->getKernel(i)->getNumberOfScenarios(); j++)
        {

            CDouble WeightedExecutionTime = 0;
            CDouble TotalWeight = 0;

            for (CId k = 0; k != Graph->getKernel(i)->getNumberOfProfiles(j); k++)
            {
                WeightedExecutionTime += Graph->getKernel(i)->getProfile(j, k)->getExecutionTime() * Graph->getKernel(i)->getProfile(j, k)->getWeight();
                TotalWeight += Graph->getKernel(i)->getProfile(j, k)->getWeight();
            }

            Graph->getKernel(i)->deleteProfiles(j);

            SADF_Profile *Profile = new SADF_Profile();
            Profile->setExecutionTime(WeightedExecutionTime / TotalWeight);

            Graph->getKernel(i)->addProfile(j, Profile);
        }

    // Average execution times of detectors

    for (CId i = 0; i != Graph->getNumberOfDetectors(); i++)
        for (CId j = 0; j != Graph->getDetector(i)->getNumberOfSubScenarios(); j++)
        {

            CDouble WeightedExecutionTime = 0;
            CDouble TotalWeight = 0;

            for (CId k = 0; k != Graph->getDetector(i)->getNumberOfProfiles(j); k++)
            {
                WeightedExecutionTime += Graph->getDetector(i)->getProfile(j, k)->getExecutionTime() * Graph->getDetector(i)->getProfile(j, k)->getWeight();
                TotalWeight += Graph->getDetector(i)->getProfile(j, k)->getWeight();
            }

            Graph->getDetector(i)->deleteProfiles(j);

            SADF_Profile *Profile = new SADF_Profile();
            Profile->setExecutionTime(WeightedExecutionTime / TotalWeight);

            Graph->getDetector(i)->addProfile(j, Profile);
        }
}

// Function to minimize execution times

void SADF_Transform_MinimizeExecutionTimes(SADF_Graph *Graph)
{

    // Minimize execution times of kernels

    for (CId i = 0; i != Graph->getNumberOfKernels(); i++)
        for (CId j = 0; j != Graph->getKernel(i)->getNumberOfScenarios(); j++)
        {

            CDouble MinimumExecutionTime = Graph->getKernel(i)->getProfile(j, 0)->getExecutionTime();

            for (CId k = 0; k != Graph->getKernel(i)->getNumberOfProfiles(j); k++)
                if (Graph->getKernel(i)->getProfile(j, k)->getExecutionTime() < MinimumExecutionTime)
                    MinimumExecutionTime = Graph->getKernel(i)->getProfile(j, k)->getExecutionTime();

            Graph->getKernel(i)->deleteProfiles(j);

            SADF_Profile *Profile = new SADF_Profile();
            Profile->setExecutionTime(MinimumExecutionTime);

            Graph->getKernel(i)->addProfile(j, Profile);
        }

    // Minimize execution times of detectors

    for (CId i = 0; i != Graph->getNumberOfDetectors(); i++)
        for (CId j = 0; j != Graph->getDetector(i)->getNumberOfSubScenarios(); j++)
        {

            CDouble MinimumExecutionTime = Graph->getDetector(i)->getProfile(j, 0)->getExecutionTime();

            for (CId k = 0; k != Graph->getDetector(i)->getNumberOfProfiles(j); k++)
                if (Graph->getDetector(i)->getProfile(j, k)->getExecutionTime() < MinimumExecutionTime)
                    MinimumExecutionTime = Graph->getDetector(i)->getProfile(j, k)->getExecutionTime();

            Graph->getDetector(i)->deleteProfiles(j);

            SADF_Profile *Profile = new SADF_Profile();
            Profile->setExecutionTime(MinimumExecutionTime);

            Graph->getDetector(i)->addProfile(j, Profile);
        }
}

// Function to maximize execution times

void SADF_Transform_MaximizeExecutionTimes(SADF_Graph *Graph)
{

    // Maximize execution times of kernels

    for (CId i = 0; i != Graph->getNumberOfKernels(); i++)
        for (CId j = 0; j != Graph->getKernel(i)->getNumberOfScenarios(); j++)
        {

            CDouble MaximumExecutionTime = Graph->getKernel(i)->getProfile(j, 0)->getExecutionTime();

            for (CId k = 0; k != Graph->getKernel(i)->getNumberOfProfiles(j); k++)
                if (Graph->getKernel(i)->getProfile(j, k)->getExecutionTime() > MaximumExecutionTime)
                    MaximumExecutionTime = Graph->getKernel(i)->getProfile(j, k)->getExecutionTime();

            Graph->getKernel(i)->deleteProfiles(j);

            SADF_Profile *Profile = new SADF_Profile();
            Profile->setExecutionTime(MaximumExecutionTime);

            Graph->getKernel(i)->addProfile(j, Profile);
        }

    // Maximize execution times of detectors

    for (CId i = 0; i != Graph->getNumberOfDetectors(); i++)
        for (CId j = 0; j != Graph->getDetector(i)->getNumberOfSubScenarios(); j++)
        {

            CDouble MaximumExecutionTime = Graph->getDetector(i)->getProfile(j, 0)->getExecutionTime();

            for (CId k = 0; k != Graph->getDetector(i)->getNumberOfProfiles(j); k++)
                if (Graph->getDetector(i)->getProfile(j, k)->getExecutionTime() > MaximumExecutionTime)
                    MaximumExecutionTime = Graph->getDetector(i)->getProfile(j, k)->getExecutionTime();

            Graph->getDetector(i)->deleteProfiles(j);

            SADF_Profile *Profile = new SADF_Profile();
            Profile->setExecutionTime(MaximumExecutionTime);

            Graph->getDetector(i)->addProfile(j, Profile);
        }
}
