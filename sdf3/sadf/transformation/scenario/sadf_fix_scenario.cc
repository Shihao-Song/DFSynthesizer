/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_fix_scenario.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   29 August 2006
 *
 *  Function        :   Transformations of SADF Graphs related to (Sub)Scenarios
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

#include "sadf_fix_scenario.h"

SADF_Graph *SADF_FixScenarios(SADF_Graph *Graph, vector<CId> SubScenarios, bool RemoveChannelsWithOneZeroRate)
{

    SADF_Graph *NewGraph = new SADF_Graph(CString(Graph->getName()), Graph->getIdentity());

    // Identify scenarios for kernels

    vector<CId> Scenarios(Graph->getNumberOfKernels(), 0);

    for (CId i = 0; i != Graph->getNumberOfKernels(); i++)
        if (Graph->getKernel(i)->hasControlInputChannels())
        {
            if (Graph->getKernel(i)->hasMultipleControlInputChannels())
            {

                bool ScenarioNotFound = true;

                for (CId j = 0; ScenarioNotFound && j != Graph->getKernel(i)->getNumberOfScenarios(); j++)
                {

                    bool MatchingControls = true;

                    for (CId k = 0; MatchingControls && k != Graph->getKernel(i)->getControlInputChannels().size(); k++)
                    {

                        SADF_Channel *Channel = Graph->getKernel(i)->getControlInputChannels()[k];

                        if (Channel->getProductionScenarioID(SubScenarios[Channel->getSource()->getIdentity()]) !=
                            Graph->getKernel(i)->getControlID(SubScenarios[Channel->getSource()->getIdentity()] , k))
                            MatchingControls = false;
                    }

                    if (MatchingControls)
                    {
                        ScenarioNotFound = false;
                        Scenarios[i] = j;
                    }
                }

                if (ScenarioNotFound)
                    throw CException((CString)("Error: Requested combination of subscenarios does not yield valid behaviour for kernel '")
                                     + Graph->getKernel(i)->getName() + "'.");

            }
            else
            {

                SADF_Channel *ControlChannel = Graph->getKernel(i)->getControlInputChannels()[0];

                Scenarios[i] = ControlChannel->getProductionScenarioID(SubScenarios[ControlChannel->getSource()->getIdentity()]);
            }
        }

    // Create kernels

    for (CId i = 0; i != Graph->getNumberOfKernels(); i++)
        NewGraph->addKernel(new SADF_Process(CString(Graph->getKernel(i)->getName()), NewGraph->getNumberOfKernels(), SADF_KERNEL));

    for (CId i = 0; i != Graph->getNumberOfDetectors(); i++)
        NewGraph->addKernel(new SADF_Process(CString(Graph->getDetector(i)->getName()), NewGraph->getNumberOfKernels(), SADF_KERNEL));

    // Create channels

    CId NumberOfActiveDataChannels = 0;
    vector<bool> ActivityOnDataChannel(Graph->getNumberOfDataChannels(), true);

    for (CId i = 0; i != Graph->getNumberOfDataChannels(); i++)
    {

        SADF_Process *Source = Graph->getDataChannel(i)->getSource();
        SADF_Process *Destination = Graph->getDataChannel(i)->getDestination();

        CId ConsumptionRate = 0;
        CId ProductionRate = 0;

        if (Source->getType() == SADF_KERNEL)
            ProductionRate = Graph->getDataChannel(i)->getProductionRate(Scenarios[Source->getIdentity()]);
        else
            ProductionRate = Graph->getDataChannel(i)->getProductionRate(SubScenarios[Source->getIdentity()]);

        if (Destination->getType() == SADF_KERNEL)
            ConsumptionRate = Graph->getDataChannel(i)->getConsumptionRate(Scenarios[Destination->getIdentity()]);
        else
            ConsumptionRate = Graph->getDataChannel(i)->getConsumptionRate(SubScenarios[Destination->getIdentity()]);

        if (ProductionRate != 0 && ConsumptionRate != 0)
            NumberOfActiveDataChannels++;
        else if (ProductionRate == 0 && ConsumptionRate == 0)
            ActivityOnDataChannel[i] = false;
        else if (RemoveChannelsWithOneZeroRate)
            ActivityOnDataChannel[i] = false;
    }

    for (CId i = 0; i != Graph->getNumberOfDataChannels(); i++)
        if (ActivityOnDataChannel[i])
        {

            SADF_Channel *Channel = new SADF_Channel(CString(Graph->getDataChannel(i)->getName()), NewGraph->getNumberOfDataChannels(), SADF_DATA_CHANNEL);
            NewGraph->addDataChannel(Channel);

            SADF_Process *Source = NewGraph->getKernel(Graph->getDataChannel(i)->getSource()->getName());
            SADF_Process *Destination = NewGraph->getKernel(Graph->getDataChannel(i)->getDestination()->getName());

            Channel->setSource(Source);
            Channel->setDestination(Destination);
            Source->addDataOutputChannel(Channel);
            Destination->addDataInputChannel(Channel);

            Channel->setBufferSize((CId) Graph->getDataChannel(i)->getBufferSize());
            Channel->setNumberOfInitialTokens(Graph->getDataChannel(i)->getNumberOfInitialTokens());
            Channel->setTokenSize(Graph->getDataChannel(i)->getTokenSize());
        }

    for (CId i = 0; i != Graph->getNumberOfControlChannels(); i++)
    {

        SADF_Channel *Channel = new SADF_Channel(CString(Graph->getControlChannel(i)->getName()), NewGraph->getNumberOfDataChannels(), SADF_DATA_CHANNEL);
        NewGraph->addDataChannel(Channel);

        SADF_Process *Source = NewGraph->getKernel(Graph->getControlChannel(i)->getSource()->getName());
        SADF_Process *Destination = NewGraph->getKernel(Graph->getControlChannel(i)->getDestination()->getName());

        Channel->setSource(Source);
        Channel->setDestination(Destination);
        Source->addDataOutputChannel(Channel);
        Destination->addDataInputChannel(Channel);

        Channel->setBufferSize((CId) Graph->getControlChannel(i)->getBufferSize());
        Channel->setNumberOfInitialTokens(Graph->getControlChannel(i)->getNumberOfInitialTokens());
        Channel->setTokenSize(Graph->getControlChannel(i)->getTokenSize());
    }

    // Construct profiles

    for (CId i = 0; i != Graph->getNumberOfKernels(); i++)
        NewGraph->getKernel(Graph->getKernel(i)->getName())->addScenario(CString(Graph->getKernel(i)->getScenarioName(Scenarios[i])));

    for (CId i = 0; i != Graph->getNumberOfDetectors(); i++)
        NewGraph->getKernel(Graph->getDetector(i)->getName())->addScenario(CString(Graph->getDetector(i)->getSubScenarioName(SubScenarios[i])));

    for (CId i = 0; i != Graph->getNumberOfKernels(); i++)
        for (CId j = 0; j != Graph->getKernel(i)->getNumberOfProfiles(Scenarios[i]); j++)
        {

            SADF_Profile *Profile = new SADF_Profile();
            Profile->setExecutionTime(Graph->getKernel(i)->getProfile(Scenarios[i], j)->getExecutionTime());
            Profile->setWeight(Graph->getKernel(i)->getProfile(Scenarios[i], j)->getWeight());

            NewGraph->getKernel(Graph->getKernel(i)->getName())->addProfile(0, Profile);
        }

    for (CId i = 0; i != Graph->getNumberOfDetectors(); i++)
        for (CId j = 0; j != Graph->getDetector(i)->getNumberOfProfiles(SubScenarios[i]); j++)
        {

            SADF_Profile *Profile = new SADF_Profile();
            Profile->setExecutionTime(Graph->getDetector(i)->getProfile(SubScenarios[i], j)->getExecutionTime());
            Profile->setWeight(Graph->getDetector(i)->getProfile(SubScenarios[i], j)->getWeight());

            NewGraph->getKernel(Graph->getDetector(i)->getName())->addProfile(0, Profile);
        }

    // Construct rates

    for (CId i = 0; i != Graph->getNumberOfDataChannels(); i++)
        if (ActivityOnDataChannel[i])
        {

            if (Graph->getDataChannel(i)->getDestination()->getType() == SADF_KERNEL)
                NewGraph->getChannel(Graph->getDataChannel(i)->getName())->setConsumptionRate(0,
                        Graph->getDataChannel(i)->getConsumptionRate(Scenarios[Graph->getDataChannel(i)->getDestination()->getIdentity()]));
            else
                NewGraph->getChannel(Graph->getDataChannel(i)->getName())->setConsumptionRate(0,
                        Graph->getDataChannel(i)->getConsumptionRate(SubScenarios[Graph->getDataChannel(i)->getDestination()->getIdentity()]));

            if (Graph->getDataChannel(i)->getSource()->getType() == SADF_KERNEL)
                NewGraph->getChannel(Graph->getDataChannel(i)->getName())->setProductionRate(0,
                        Graph->getDataChannel(i)->getProductionRate(Scenarios[Graph->getDataChannel(i)->getSource()->getIdentity()]));
            else
                NewGraph->getChannel(Graph->getDataChannel(i)->getName())->setProductionRate(0,
                        Graph->getDataChannel(i)->getProductionRate(SubScenarios[Graph->getDataChannel(i)->getSource()->getIdentity()]));
        }

    for (CId i = 0; i != Graph->getNumberOfControlChannels(); i++)
    {
        NewGraph->getChannel(Graph->getControlChannel(i)->getName())->setConsumptionRate(0, 1);
        NewGraph->getChannel(Graph->getControlChannel(i)->getName())->setProductionRate(0,
                Graph->getControlChannel(i)->getProductionRate(SubScenarios[Graph->getControlChannel(i)->getSource()->getIdentity()]));
    }

    return NewGraph;
}
