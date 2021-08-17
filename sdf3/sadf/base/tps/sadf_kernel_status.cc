/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_kernel_status.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   29 September 2006
 *
 *  Function        :   SADF Kernel Status
 *
 *  History         :
 *      29-09-06    :   Initial version.
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

#include "sadf_kernel_status.h"
#include "sadf_tps.h"

// Constructors

SADF_KernelState::SADF_KernelState(SADF_Process *K, CId ID, CId ScenarioID, CId ActionType, CDouble Time) : SADF_Component()
{

    setIdentity(ID);

    Kernel = K;

    setType(ActionType);
    Scenario = ScenarioID;
    ExecutionTime = Time;
}

SADF_KernelStatus::SADF_KernelStatus(SADF_Configuration *C, SADF_KernelState *S, CDouble T)
{

    Configuration = C;

    State = S;
    RemainingExecutionTime = T;
}

SADF_KernelStatus::SADF_KernelStatus(SADF_Configuration *C, SADF_KernelStatus *S)
{

    Configuration = C;

    State = S->getState();
    RemainingExecutionTime = S->getRemainingExecutionTime();
}

// Access to instance variables

void SADF_KernelState::addTransition(SADF_KernelState *K, CDouble P)
{

    CId NumberOfTransitions = Destinations.size() + 1;

    Destinations.resize(NumberOfTransitions);
    Probabilities.resize(NumberOfTransitions);

    Destinations[NumberOfTransitions - 1] = K;
    Probabilities[NumberOfTransitions - 1] = P;
}

// Functions to determine current status

bool SADF_KernelStatus::isReadyToFire()
{

    // Only check for controlled kernels in which case the scenario is not specified after end step

    bool ReadyToFire = State->getType() == SADF_END_STEP && State->getScenario() == SADF_UNDEFINED;

    for (CId i = 0; ReadyToFire && i != State->getKernel()->getControlInputChannels().size(); i++)
        if (Configuration->getControlStatus(State->getKernel()->getControlInputChannels()[i]->getIdentity())->getAvailableTokens() == 0)
            ReadyToFire = false;

    return ReadyToFire;
}

bool SADF_KernelStatus::isReadyToStart()
{

    // Determine Scenario

    CId Scenario = State->getScenario();

    bool ReadyToStart = State->getType() == SADF_CONTROL_STEP || (State->getType() == SADF_END_STEP && Scenario != SADF_UNDEFINED);

    // Check availability of tokens and space

    if (ReadyToStart && State->getKernel()->isActive(Scenario))
    {

        for (CId i = 0; ReadyToStart && i != State->getKernel()->getDataInputChannels().size(); i++)
            if (Configuration->getChannelStatus(State->getKernel()->getDataInputChannels()[i]->getIdentity())->getAvailableTokens()
                < State->getKernel()->getDataInputChannels()[i]->getConsumptionRate(Scenario))
                ReadyToStart = false;

        for (CId i = 0; ReadyToStart && i != State->getKernel()->getDataOutputChannels().size(); i++)
            if (State->getKernel()->getDataOutputChannels()[i]->getBufferSize() != SADF_UNBOUNDED)
                if (Configuration->getChannelStatus(State->getKernel()->getDataOutputChannels()[i]->getIdentity())->getAvailableTokens()
                    + State->getKernel()->getDataOutputChannels()[i]->getProductionRate(Scenario)
                    > State->getKernel()->getDataOutputChannels()[i]->getBufferSize())
                    ReadyToStart = false;
    }

    return ReadyToStart;
}

bool SADF_KernelStatus::isReadyToEnd()
{

    return State->getType() == SADF_START_STEP && RemainingExecutionTime <= 0;  // <= is used to circumvent rounding errors
}

// Functions to change status

SADF_ListOfConfigurations SADF_KernelStatus::control(SADF_Graph *Graph, SADF_TPS *TPS, bool RelevantStep)
{

    // the control is only used in case Kernel has control input channels

    SADF_ListOfConfigurations NewConfigurations;

    SADF_Configuration *NewConfiguration = new SADF_Configuration(Graph, Configuration, SADF_CONTROL_STEP, 0);

    // Update status of kernel in new configuration

    SADF_Process *Kernel = State->getKernel();

    bool ScenarioNotFound = true;

    for (CId i = 0; ScenarioNotFound && i != State->getNumberOfTransitions(); i++)
    {

        CId Scenario = State->getDestination(i)->getScenario();

        if (Kernel->hasMultipleControlInputChannels())
        {

            bool MatchingControls = true;

            for (CId j = 0; MatchingControls && j != Kernel->getControlInputChannels().size(); j++)
                if (Configuration->getControlStatus(Kernel->getControlInputChannels()[j]->getIdentity())->inspect() != Kernel->getControlID(Scenario, j))
                    MatchingControls = false;

            if (MatchingControls)
            {
                NewConfiguration->getKernelStatus(Kernel->getIdentity())->setState(State->getDestination(i));
                ScenarioNotFound = false;
            }

        }
        else if (Configuration->getControlStatus(Kernel->getControlInputChannels().front()->getIdentity())->inspect() == Scenario)
        {

            NewConfiguration->getKernelStatus(Kernel->getIdentity())->setState(State->getDestination(i));
            ScenarioNotFound = false;
        }
    }

    // Check existance of new configuration if relevant step

    if (RelevantStep)
    {

        NewConfiguration->computeHashKey();
        SADF_Configuration *Test = TPS->inConfigurationSpace(NewConfiguration);

        if (Test != NULL)
        {
            Configuration->addTransition(Test, 1, 0);
            delete NewConfiguration;
        }
        else
        {
            NewConfiguration->setRelevance(true);
            Configuration->addTransition(NewConfiguration, 1, 0);
            TPS->addConfiguration(NewConfiguration);
            NewConfigurations.push_front(NewConfiguration);
        }

    }
    else
    {

        Configuration->addTransition(NewConfiguration, 1, 0);
        NewConfigurations.push_front(NewConfiguration);
    }

    return NewConfigurations;
}

SADF_ListOfConfigurations SADF_KernelStatus::start(SADF_Graph *Graph, SADF_TPS *TPS, bool RelevantStep)
{

    SADF_ListOfConfigurations NewConfigurations;

    SADF_Process *Kernel = State->getKernel();

    for (CId i = 0; i != State->getNumberOfTransitions(); i++)
    {

        SADF_Configuration *NewConfiguration = new SADF_Configuration(Graph, Configuration, SADF_START_STEP, 0);

        // Update status of kernel in new configuration

        if ((State->getType() == SADF_END_STEP || State->getType() == SADF_UNDEFINED) && Kernel->isActive(0))
            for (CId j = 0; j != Kernel->getDataOutputChannels().size(); j++)
                NewConfiguration->getChannelStatus(Kernel->getDataOutputChannels()[j]->getIdentity())->reserve(Kernel->getDataOutputChannels()[j]->getProductionRate(0));

        if (State->getType() == SADF_CONTROL_STEP && Kernel->isActive(State->getScenario()))
            for (CId j = 0; j != Kernel->getDataOutputChannels().size(); j++)
                NewConfiguration->getChannelStatus(Kernel->getDataOutputChannels()[j]->getIdentity())->reserve(Kernel->getDataOutputChannels()[j]->getProductionRate(State->getScenario()));

        NewConfiguration->getKernelStatus(Kernel->getIdentity())->setState(State->getDestination(i));

        CDouble ExecutionTime = NewConfiguration->getKernelStatus(Kernel->getIdentity())->getRemainingExecutionTime();

        if (ExecutionTime > 0 && ExecutionTime < Configuration->getMinimalRemainingExecutionTime())
            NewConfiguration->setMinimalRemainingExecutionTime(ExecutionTime);

        // Check existance of new configuration if relevant step

        if (RelevantStep)
        {

            NewConfiguration->computeHashKey();
            SADF_Configuration *Test = TPS->inConfigurationSpace(NewConfiguration);

            if (Test != NULL)
            {
                Configuration->addTransition(Test, State->getProbability(i), 0);
                delete NewConfiguration;
            }
            else
            {
                NewConfiguration->setRelevance(true);
                Configuration->addTransition(NewConfiguration, State->getProbability(i), 0);
                TPS->addConfiguration(NewConfiguration);
                NewConfigurations.push_front(NewConfiguration);
            }

        }
        else
        {

            Configuration->addTransition(NewConfiguration, State->getProbability(i), 0);
            NewConfigurations.push_front(NewConfiguration);
        }
    }

    return NewConfigurations;
}

SADF_ListOfConfigurations SADF_KernelStatus::end(SADF_Graph *Graph, SADF_TPS *TPS, bool RelevantStep)
{

    SADF_ListOfConfigurations NewConfigurations;

    SADF_Configuration *NewConfiguration = new SADF_Configuration(Graph, Configuration, SADF_END_STEP, 0);

    // Update status of kernel in new configuration

    SADF_Process *Kernel = State->getKernel();

    for (CId i = 0; i != Kernel->getControlInputChannels().size(); i++)
        NewConfiguration->getControlStatus(Kernel->getControlInputChannels()[i]->getIdentity())->remove();

    if (Kernel->isActive(State->getScenario()))
    {

        for (CId i = 0; i != Kernel->getDataInputChannels().size(); i++)
            NewConfiguration->getChannelStatus(Kernel->getDataInputChannels()[i]->getIdentity())->remove(Kernel->getDataInputChannels()[i]->getConsumptionRate(State->getScenario()));

        for (CId i = 0; i != Kernel->getDataOutputChannels().size(); i++)
            NewConfiguration->getChannelStatus(Kernel->getDataOutputChannels()[i]->getIdentity())->write(Kernel->getDataOutputChannels()[i]->getProductionRate(State->getScenario()));
    }

    NewConfiguration->getKernelStatus(Kernel->getIdentity())->setState(State->getDestination(0));

    // Check existance of new configuration if relevant step

    if (RelevantStep)
    {

        NewConfiguration->computeHashKey();
        SADF_Configuration *Test = TPS->inConfigurationSpace(NewConfiguration);

        if (Test != NULL)
        {
            Configuration->addTransition(Test, 1, 0);
            delete NewConfiguration;
        }
        else
        {
            NewConfiguration->setRelevance(true);
            Configuration->addTransition(NewConfiguration, 1, 0);
            TPS->addConfiguration(NewConfiguration);
            NewConfigurations.push_front(NewConfiguration);
        }

    }
    else
    {

        Configuration->addTransition(NewConfiguration, 1, 0);
        NewConfigurations.push_front(NewConfiguration);
    }

    return NewConfigurations;
}

// Equality operators

bool SADF_KernelStatus::equal(SADF_KernelStatus *S)
{

    return State->getIdentity() == S->getState()->getIdentity() && RemainingExecutionTime == S->getRemainingExecutionTime();
}
