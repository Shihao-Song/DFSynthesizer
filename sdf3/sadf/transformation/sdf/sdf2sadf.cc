/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdf2sadf.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   13 September 2006
 *
 *  Function        :   Convert SDF Graph in SADF Graph
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

#include "sdf2sadf.h"

SADF_Graph *SDF2SADF(TimedSDFgraph *SDF)
{

    SADF_Graph *SADF = new SADF_Graph(SDF->getName(), SDF->getId());

    // Create Kernels

    for (SDFactorsIter i = SDF->actorsBegin(); i != SDF->actorsEnd(); i++)
        SADF->addKernel(new SADF_Process((*i)->getName(), SADF->getNumberOfKernels(), SADF_KERNEL));

    // Create Channels

    for (SDFchannelsIter i = SDF->channelsBegin(); i != SDF->channelsEnd(); i++)
    {

        TimedSDFchannel *Channel = (TimedSDFchannel *)(*i);
        SADF_Channel *DataChannel = new SADF_Channel(Channel->getName(), SADF->getNumberOfDataChannels(), SADF_DATA_CHANNEL);

        if (Channel->isConnected())
        {

            DataChannel->setSource(SADF->getKernel(Channel->getSrcActor()->getName()));
            DataChannel->setDestination(SADF->getKernel(Channel->getDstActor()->getName()));

            DataChannel->getSource()->addDataOutputChannel(DataChannel);
            DataChannel->getDestination()->addDataInputChannel(DataChannel);

        }
        else
            throw CException((CString)("Channel '") + Channel->getName() + "' of SDF graph '" + SDF->getName() + "' is not connected.");

        if (Channel->getInitialTokens() != 0)
            DataChannel->setNumberOfInitialTokens((CId)(Channel->getInitialTokens()));
        if (Channel->getBufferSize().sz != -1)
            DataChannel->setBufferSize((CId)(Channel->getBufferSize().sz));
        if (Channel->getTokenSize() != -1)
            DataChannel->setTokenSize((CId)(Channel->getTokenSize()));

        SADF->addDataChannel(DataChannel);
    }

    // Finalise Creation of Kernels

    for (SDFactorsIter i = SDF->actorsBegin(); i != SDF->actorsEnd(); i++)
    {

        TimedSDFactor *Actor = (TimedSDFactor *)(*i);
        SADF_Process *Kernel = SADF->getKernel(Actor->getName());

        CId ScenarioID = Kernel->addScenario("default");

        SADF_Profile *Profile = new SADF_Profile();

        for (TimedSDFactor::ProcessorsIter j = Actor->processorsBegin(); j != Actor->processorsEnd(); j++)
            if (Actor->getDefaultProcessor() == (*j)->type)
                Profile->setExecutionTime((CDouble)((*j)->execTime));

        Kernel->addProfile(ScenarioID, Profile);
    }

    // Finalise Creation of Channels

    for (SDFchannelsIter i = SDF->channelsBegin(); i != SDF->channelsEnd(); i++)
    {

        TimedSDFchannel *Channel = (TimedSDFchannel *)(*i);
        SADF_Channel *DataChannel = SADF->getChannel(Channel->getName());

        DataChannel->setConsumptionRate(0, Channel->getDstPort()->getRate());
        DataChannel->setProductionRate(0, Channel->getSrcPort()->getRate());
    }

    return SADF;
}
