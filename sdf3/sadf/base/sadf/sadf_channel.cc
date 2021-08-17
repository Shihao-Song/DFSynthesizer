/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_channel.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   29 August 2006
 *
 *  Function        :   SADF Channel
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

#include "sadf_channel.h"

// Constructor

SADF_Channel::SADF_Channel(const CString &N, const CId ID, const CId T) : SADF_Component()
{

    this->setName(N);
    this->setIdentity(ID);
    this->setType(T);

    BufferSize = SADF_UNBOUNDED;
    NumberOfInitialTokens = 0;
    TokenSize = 1;
}

// Access to Productions and Consumptions

void SADF_Channel::addConsumption()
{

    if (this->getType() == SADF_CONTROL_CHANNEL)
    {
        ConsumptionRates.push_back(1);
        ConsumptionScenarioIDs.push_back(SADF_UNDEFINED);
        ConsumptionScenarioNames.resize(ConsumptionScenarioNames.size() + 1);
    }
    else
        ConsumptionRates.push_back(0);
}

void SADF_Channel::addProduction()
{

    ProductionRates.push_back(0);

    if (this->getType() == SADF_CONTROL_CHANNEL)
    {
        ProductionScenarioIDs.push_back(SADF_UNDEFINED);
        ProductionScenarioNames.resize(ProductionScenarioNames.size() + 1);
    }
}

// Access to Properties

void SADF_Channel::addInitialTokens(const CId Number, const CId ScenarioID)
{

    NumbersQueue.push(Number);
    ContentQueue.push(ScenarioID);
    NumberOfInitialTokens += Number;
}
