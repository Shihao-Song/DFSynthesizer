/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_process.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   29 August 2006
 *
 *  Function        :   SADF Process
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

#include "sadf_process.h"

// Constructor

SADF_Process::SADF_Process(const CString &N, const CId ID, const CId T) : SADF_Component()
{

    this->setName(N);
    this->setIdentity(ID);
    this->setType(T);
}

// Destructor

SADF_Process::~SADF_Process()
{

    for (CId i = 0; i != Profiles.size(); i++)
        for (CId j = 0; j != Profiles[i].size(); j++)
            delete Profiles[i][j];

    for (CId i = 0; i != MarkovChains.size(); i++)
        delete MarkovChains[i];
}

// Access to Channels

CId SADF_Process::getDataInputChannelID(const CString &ChannelName)
{

    for (CId i = 0; i != DataInputChannels.size(); i++)
        if (DataInputChannels[i]->getName() == ChannelName)
            return i;

    return SADF_UNDEFINED;
}

CId SADF_Process::getDataOutputChannelID(const CString &ChannelName)
{

    for (CId i = 0; i != DataOutputChannels.size(); i++)
        if (DataOutputChannels[i]->getName() == ChannelName)
            return i;

    return SADF_UNDEFINED;
}

CId SADF_Process::getControlInputChannelID(const CString &ChannelName)
{

    for (CId i = 0; i != ControlInputChannels.size(); i++)
        if (ControlInputChannels[i]->getName() == ChannelName)
            return i;

    return SADF_UNDEFINED;
}

CId SADF_Process::getControlOutputChannelID(const CString &ChannelName)
{

    for (CId i = 0; i != ControlOutputChannels.size(); i++)
        if (ControlOutputChannels[i]->getName() == ChannelName)
            return i;

    return SADF_UNDEFINED;
}

// Access to Profiles

void SADF_Process::deleteProfiles(const CId ScenarioID)
{

    for (CId i = 0; i != Profiles[ScenarioID].size(); i++)
        delete Profiles[ScenarioID][i];

    Profiles[ScenarioID].resize(0);
}

// Access to Scenarios and SubScenarios

CId SADF_Process::addScenario(const CString &ScenarioName)
{

    Scenarios.push_back(ScenarioName);

    if (this->getType() == SADF_KERNEL)
    {

        ActiveScenario.push_back(true);

        for (CId i = 0; i != DataInputChannels.size(); i++)
            DataInputChannels[i]->addConsumption();

        for (CId i = 0; i != DataOutputChannels.size(); i++)
            DataOutputChannels[i]->addProduction();

        for (CId i = 0; i != ControlInputChannels.size(); i++)
            ControlInputChannels[i]->addConsumption();

        vector<SADF_Profile *> ProfileVector;
        Profiles.push_back(ProfileVector);
    }

    if (this->getType() == SADF_DETECTOR)
        MarkovChains.resize(MarkovChains.size() + 1);

    return Scenarios.size() - 1;
}

CId SADF_Process::addSubScenario(const CString &SubScenarioName)
{

    SubScenarios.push_back(SubScenarioName);

    vector<SADF_Profile *> ProfileVector;
    Profiles.push_back(ProfileVector);

    for (CId i = 0; i != DataInputChannels.size(); i++)
        DataInputChannels[i]->addConsumption();

    for (CId i = 0; i != DataOutputChannels.size(); i++)
        DataOutputChannels[i]->addProduction();

    for (CId i = 0; i != ControlInputChannels.size(); i++)
        ControlInputChannels[i]->addConsumption();

    for (CId i = 0; i != ControlOutputChannels.size(); i++)
        ControlOutputChannels[i]->addProduction();

    return SubScenarios.size() - 1;
}

CId SADF_Process::getScenarioID(const CString &ScenarioName)
{

    for (CId i = 0; i != Scenarios.size(); i++)
        if (Scenarios[i] == ScenarioName)
            return i;

    return SADF_UNDEFINED;
}

CId SADF_Process::getSubScenarioID(const CString &SubScenarioName)
{

    for (CId i = 0; i != SubScenarios.size(); i++)
        if (SubScenarios[i] == SubScenarioName)
            return i;

    return SADF_UNDEFINED;
}

// Functions

bool SADF_Process::checkActivity(const CId ScenarioID)
{

    for (CId i = 0; i != DataInputChannels.size(); i++)
        if (DataInputChannels[i]->getConsumptionRate(ScenarioID) != 0)
            return true;

    for (CId i = 0; i != DataOutputChannels.size(); i++)
        if (DataOutputChannels[i]->getProductionRate(ScenarioID) != 0)
            return true;

    return false;
}
