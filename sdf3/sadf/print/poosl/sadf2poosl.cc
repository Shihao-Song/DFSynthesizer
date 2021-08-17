/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf2poosl.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   13 September 2006
 *
 *  Function        :   Convert SADF Graph into POOSL Model
 *
 *  History         :
 *      13-09-06    :   Initial version.
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

#include "sadf2poosl.h"

void SADF2POOSL(SADF_Graph *Graph, SADF_SimulationSettings *Settings, CString &LogFileName, ostream &out, bool PERT, bool p4r)
{

    // Check Monitoring & Tracing

    bool ProductionOfResults = false;

    for (CId i = 0; !ProductionOfResults && i != Graph->getNumberOfKernels(); i++)
        if (Settings->getKernelSettings(i)->getMonitor() | Settings->getKernelSettings(i)->getTrace())
            ProductionOfResults = true;

    for (CId i = 0; !ProductionOfResults && i != Graph->getNumberOfDetectors(); i++)
        if (Settings->getDetectorSettings(i)->getMonitor() | Settings->getDetectorSettings(i)->getTrace())
            ProductionOfResults = true;

    for (CId i = 0; !ProductionOfResults && i != Graph->getNumberOfDataChannels(); i++)
        if (Settings->getDataChannelSettings(i)->getMonitor() | Settings->getDataChannelSettings(i)->getTrace())
            ProductionOfResults = true;

    for (CId i = 0; !ProductionOfResults && i != Graph->getNumberOfControlChannels(); i++)
        if (Settings->getControlChannelSettings(i)->getMonitor() | Settings->getControlChannelSettings(i)->getTrace())
            ProductionOfResults = true;

    if (!ProductionOfResults)
        throw CException("Error: No monitoring or tracing specified");

    for (CId i = 0; i != Graph->getNumberOfKernels(); i++)
        if (Graph->getKernel(i)->hasMultipleControlInputChannels())
            throw CException("Error: Conversion to POOSL does not yet support multiple control inputs");

    for (CId i = 0; i != Graph->getNumberOfDetectors(); i++)
        if (Graph->getDetector(i)->hasMultipleControlInputChannels())
            throw CException("Error: Conversion to POOSL does not yet support multiple control inputs");

    // Validate PERT Distributions

    if (PERT)
    {

        for (CId i = 0; i != Graph->getNumberOfKernels(); i++)
            for (CId j = 0; j != Graph->getKernel(i)->getNumberOfScenarios(); j++)
            {
                if (Graph->getKernel(i)->getNumberOfProfiles(j) == 0)
                {
                    Graph->getKernel(i)->addProfile(j, new SADF_Profile());
                    Graph->getKernel(i)->addProfile(j, new SADF_Profile());
                    Graph->getKernel(i)->addProfile(j, new SADF_Profile());
                }
                else if (Graph->getKernel(i)->getNumberOfProfiles(j) == 1)
                {
                    SADF_Profile *Maximum = new SADF_Profile;
                    Maximum->setExecutionTime(Graph->getKernel(i)->getProfile(j, 0)->getExecutionTime());
                    Maximum->setWeight(Graph->getKernel(i)->getProfile(j, 0)->getWeight());
                    Graph->getKernel(i)->addProfile(j, Maximum);
                    SADF_Profile *Mode = new SADF_Profile;
                    Mode->setExecutionTime(Graph->getKernel(i)->getProfile(j, 0)->getExecutionTime());
                    Mode->setWeight(2 * Graph->getKernel(i)->getProfile(j, 0)->getWeight());
                    Graph->getKernel(i)->addProfile(j, Mode);
                }
                else if (Graph->getKernel(i)->getNumberOfProfiles(j) == 3)
                {
                    SADF_Profile *Minimum = Graph->getKernel(i)->getProfile(j, 0);
                    SADF_Profile *Mode = Graph->getKernel(i)->getProfile(j, 0);
                    SADF_Profile *Maximum = Graph->getKernel(i)->getProfile(j, 0);
                    for (CId k = 0; k != Graph->getKernel(i)->getNumberOfProfiles(j); k++)
                    {
                        if (Graph->getKernel(i)->getProfile(j, k)->getExecutionTime() < Minimum->getExecutionTime()) Minimum = Graph->getKernel(i)->getProfile(j, k);
                        if (Graph->getKernel(i)->getProfile(j, k)->getExecutionTime() > Maximum->getExecutionTime()) Maximum = Graph->getKernel(i)->getProfile(j, k);
                    }
                    for (CId k = 0; k != Graph->getKernel(i)->getNumberOfProfiles(j); k++)
                        if ((Graph->getKernel(i)->getProfile(j, k) != Minimum) & (Graph->getKernel(i)->getProfile(j, k) != Maximum))
                            Mode = Graph->getKernel(i)->getProfile(j, k);
                    if (Minimum->getExecutionTime() != Maximum->getExecutionTime())
                    {
                        if ((Minimum->getExecutionTime() == Mode->getExecutionTime()) | (Maximum->getExecutionTime() == Mode->getExecutionTime()))
                            throw CException((CString)("Error: Mode execution time for Scenario of Kernel '") + Graph->getKernel(i)->getName() + "' equals Minimum or Maximum execution time.");
                    }
                }
                else if (Graph->getKernel(i)->getNumberOfProfiles(j) > 3)
                    throw CException((CString)("Error: More than 3 profiles specified for Scenario of Kernel '") + Graph->getKernel(i)->getName() + "'.");
            }

        for (CId i = 0; i != Graph->getNumberOfDetectors(); i++)
            for (CId j = 0; j != Graph->getDetector(i)->getNumberOfSubScenarios(); j++)
            {
                if (Graph->getDetector(i)->getNumberOfProfiles(j) == 0)
                {
                    Graph->getDetector(i)->addProfile(j, new SADF_Profile());
                    Graph->getDetector(i)->addProfile(j, new SADF_Profile());
                    Graph->getDetector(i)->addProfile(j, new SADF_Profile());
                }
                else if (Graph->getDetector(i)->getNumberOfProfiles(j) == 1)
                {
                    SADF_Profile *Maximum = new SADF_Profile;
                    Maximum->setExecutionTime(Graph->getDetector(i)->getProfile(j, 0)->getExecutionTime());
                    Maximum->setWeight(Graph->getDetector(i)->getProfile(j, 0)->getWeight());
                    Graph->getDetector(i)->addProfile(j, Maximum);
                    SADF_Profile *Mode = new SADF_Profile;
                    Mode->setExecutionTime(Graph->getDetector(i)->getProfile(j, 0)->getExecutionTime());
                    Mode->setWeight(2 * Graph->getDetector(i)->getProfile(j, 0)->getWeight());
                    Graph->getDetector(i)->addProfile(j, Mode);
                }
                else if (Graph->getDetector(i)->getNumberOfProfiles(j) == 3)
                {
                    SADF_Profile *Minimum = Graph->getDetector(i)->getProfile(j, 0);
                    SADF_Profile *Mode = Graph->getDetector(i)->getProfile(j, 0);
                    SADF_Profile *Maximum = Graph->getDetector(i)->getProfile(j, 0);
                    for (CId k = 0; k != Graph->getDetector(i)->getNumberOfProfiles(j); k++)
                    {
                        if (Graph->getDetector(i)->getProfile(j, k)->getExecutionTime() < Minimum->getExecutionTime()) Minimum = Graph->getDetector(i)->getProfile(j, k);
                        if (Graph->getDetector(i)->getProfile(j, k)->getExecutionTime() > Maximum->getExecutionTime()) Maximum = Graph->getDetector(i)->getProfile(j, k);
                    }
                    for (CId k = 0; k != Graph->getDetector(i)->getNumberOfProfiles(j); k++)
                        if ((Graph->getDetector(i)->getProfile(j, k) != Minimum) & (Graph->getDetector(i)->getProfile(j, k) != Maximum))
                            Mode = Graph->getDetector(i)->getProfile(j, k);
                    if (Minimum->getExecutionTime() != Maximum->getExecutionTime())
                    {
                        if ((Minimum->getExecutionTime() == Mode->getExecutionTime()) | (Maximum->getExecutionTime() == Mode->getExecutionTime()))
                            throw CException((CString)("Error: Mode execution time for SubScenario of Detector '") + Graph->getDetector(i)->getName() + "' equals Minimum or Maximum execution time.");
                    }
                }
                else if (Graph->getDetector(i)->getNumberOfProfiles(j) > 3)
                    throw CException((CString)("Error: More than 3 profiles specified for SubScenario of Detector '") + Graph->getDetector(i)->getName() + "'.");
            }
    }

    // Start the real stuff

    if (p4r)
    {
        out << "system specification SADF" << endl;
        out << "behaviour specification" << endl;
        SADF2POOSL_Cluster_p4r(Graph, Settings, LogFileName, out);
    }
    else
    {
        out << "system" << endl;
        SADF2POOSL_Cluster(Graph, Settings, LogFileName, out);
    }

    SADF2POOSL_Process(Graph, Settings, out, PERT, p4r);
    SADF2POOSL_Data(out, PERT, p4r);
}

