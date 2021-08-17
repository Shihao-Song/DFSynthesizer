/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   csdf2sadf.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   13 September 2006
 *
 *  Function        :   Convert CSDF Graph in SADF Graph
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

#include "csdf2sadf.h"

SADF_Graph *CSDF2SADF(TimedCSDFgraph *CSDF)
{

    SADF_Graph *SADF = new SADF_Graph(CSDF->getName(), CSDF->getId());

    // Create Kernels and Detectors

    for (CSDFactorsIter i = CSDF->actorsBegin(); i != CSDF->actorsEnd(); i++)
        if ((*i)->sequenceLength() == 1)
            SADF->addKernel(new SADF_Process((*i)->getName(), SADF->getNumberOfKernels(), SADF_KERNEL));
        else
            SADF->addDetector(new SADF_Process((*i)->getName(), SADF->getNumberOfDetectors(), SADF_DETECTOR));

    // Create Channels

    for (CSDFchannelsIter i = CSDF->channelsBegin(); i != CSDF->channelsEnd(); i++)
    {

        TimedCSDFchannel *Channel = (TimedCSDFchannel *)(*i);
        SADF_Channel *DataChannel = new SADF_Channel(Channel->getName(), SADF->getNumberOfDataChannels(), SADF_DATA_CHANNEL);

        if (Channel->isConnected())
        {

            if (SADF->getKernel(Channel->getSrcActor()->getName()) != NULL)
                DataChannel->setSource(SADF->getKernel(Channel->getSrcActor()->getName()));
            else
                DataChannel->setSource(SADF->getDetector(Channel->getSrcActor()->getName()));

            if (SADF->getKernel(Channel->getDstActor()->getName()) != NULL)
                DataChannel->setDestination(SADF->getKernel(Channel->getDstActor()->getName()));
            else
                DataChannel->setDestination(SADF->getDetector(Channel->getDstActor()->getName()));

            DataChannel->getSource()->addDataOutputChannel(DataChannel);
            DataChannel->getDestination()->addDataInputChannel(DataChannel);

        }
        else
            throw CException((CString)("Channel '") + Channel->getName() + "' of CSDF graph '" + CSDF->getName() + "' is not connected.");

        if (Channel->getInitialTokens() != 0)
            DataChannel->setNumberOfInitialTokens((CId)(Channel->getInitialTokens()));
        if (Channel->getBufferSize().sz != -1)
            DataChannel->setBufferSize((CId)(Channel->getBufferSize().sz));
        if (Channel->getTokenSize() != -1)
            DataChannel->setTokenSize((CId)(Channel->getTokenSize()));

        SADF->addDataChannel(DataChannel);
    }

    // Finalise creation of Kernels

    for (CSDFactorsIter i = CSDF->actorsBegin(); i != CSDF->actorsEnd(); i++)
        if ((*i)->sequenceLength() == 1)
        {

            TimedCSDFactor *Actor = (TimedCSDFactor *)(*i);
            SADF_Process *Kernel = SADF->getKernel(Actor->getName());

            CId ScenarioID = Kernel->addScenario("default");

            for (CSDFportsIter j = Actor->portsBegin(); j != Actor->portsEnd(); j++)
            {

                if ((*j)->getType() == CSDFport::In)
                    SADF->getChannel((*j)->getChannel()->getName())->setConsumptionRate(0, (*j)->getRate()[0]);

                if ((*j)->getType() == CSDFport::Out)
                    SADF->getChannel((*j)->getChannel()->getName())->setProductionRate(0, (*j)->getRate()[0]);
            }

            SADF_Profile *Profile = new SADF_Profile();

            for (TimedCSDFactor::ProcessorsIter j = Actor->processorsBegin(); j != Actor->processorsEnd(); j++)
                if (Actor->getDefaultProcessor() == (*j)->type)
                    Profile->setExecutionTime((CDouble)((*j)->execTime[0]));

            Kernel->addProfile(ScenarioID, Profile);
        }

    // Finalise creation of Detectors

    for (CSDFactorsIter i = CSDF->actorsBegin(); i != CSDF->actorsEnd(); i++)
        if ((*i)->sequenceLength() != 1)
        {

            TimedCSDFactor *Actor = (TimedCSDFactor *)(*i);
            SADF_Process *Detector = SADF->getDetector(Actor->getName());

            CId ScenarioID = Detector->addScenario("default");

            SADF_MarkovChain *MarkovChain = new SADF_MarkovChain();

            for (CId j = 0; j != Actor->sequenceLength(); j++)
            {

                CId SubScenarioID = Detector->addSubScenario("Phase" + (CString)(j));
                MarkovChain->addState("Phase" + (CString)(j), SubScenarioID);

                for (CSDFportsIter k = Actor->portsBegin(); k != Actor->portsEnd(); k++)
                {

                    if ((*k)->getType() == CSDFport::In)
                        SADF->getChannel((*k)->getChannel()->getName())->setConsumptionRate(SubScenarioID, (*k)->getRate()[j]);

                    if ((*k)->getType() == CSDFport::Out)
                        SADF->getChannel((*k)->getChannel()->getName())->setProductionRate(SubScenarioID, (*k)->getRate()[j]);
                }

                SADF_Profile *Profile = new SADF_Profile();

                for (TimedCSDFactor::ProcessorsIter k = Actor->processorsBegin(); k != Actor->processorsEnd(); k++)
                    if (Actor->getDefaultProcessor() == (*k)->type)
                        Profile->setExecutionTime((CDouble)((*k)->execTime[j]));

                Detector->addProfile(SubScenarioID, Profile);
            }

            MarkovChain->setInitialState(Actor->sequenceLength() - 1);

            for (CId j = 0; j != Actor->sequenceLength() - 1 ; j++)
                MarkovChain->setTransitionProbability(j, j + 1, 1);

            MarkovChain->setTransitionProbability(Actor->sequenceLength() - 1, 0, 1);

            Detector->setMarkovChain(ScenarioID, MarkovChain);
        }

    return SADF;
}
