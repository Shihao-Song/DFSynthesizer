/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_TPS.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   29 September 2006
 *
 *  Function        :   SADF Timed Probabilistic System (TPS)
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

#include "sadf_tps.h"

// Functions for constructing detector states

void SADF_ConstructKernelStates(SADF_Graph *Graph, SADF_TPS *TPS, CId KernelID, SADF_KernelState *S)
{

    // Creation of control and start states;

    if (Graph->getKernel(KernelID)->hasControlInputChannels())
    {

        list<SADF_KernelState *> ControlStates;

        for (CId i = 0; i != Graph->getKernel(KernelID)->getNumberOfScenarios(); i++)
        {

            SADF_KernelState *ControlState = new SADF_KernelState(Graph->getKernel(KernelID), TPS->getKernelStates(KernelID).size(), i, SADF_CONTROL_STEP, SADF_MAX_DOUBLE);
            ControlStates.push_front(ControlState);
            TPS->getKernelStates(KernelID).push_front(ControlState);
            S->addTransition(ControlState, 1);      // Choice is determined by environment
        }

        for (list<SADF_KernelState *>::iterator i = ControlStates.begin(); i != ControlStates.end(); i++)
            for (CId j = 0; j != Graph->getKernel(KernelID)->getNumberOfProfiles((*i)->getScenario()); j++)
            {

                SADF_KernelState *StartState = new SADF_KernelState(Graph->getKernel(KernelID), TPS->getKernelStates(KernelID).size(), (*i)->getScenario(), SADF_START_STEP, Graph->getKernel(KernelID)->getProfile((*i)->getScenario(), j)->getExecutionTime());
                StartState->addTransition(S, 1);
                TPS->getKernelStates(KernelID).push_front(StartState);
                (*i)->addTransition(StartState, Graph->getKernel(KernelID)->getProfile((*i)->getScenario(), j)->getWeight());
            }

    }
    else
    {

        for (CId i = 0; i != Graph->getKernel(KernelID)->getNumberOfProfiles(0); i++)
        {

            SADF_KernelState *StartState = new SADF_KernelState(Graph->getKernel(KernelID), TPS->getKernelStates(KernelID).size(), 0, SADF_START_STEP, Graph->getKernel(KernelID)->getProfile(0, i)->getExecutionTime());
            StartState->addTransition(S, 1);
            TPS->getKernelStates(KernelID).push_front(StartState);
            S->addTransition(StartState, Graph->getKernel(KernelID)->getProfile(0, i)->getWeight());
        }
    }
}

SADF_DetectorState *SADF_DetectorState_InList(list<SADF_DetectorState *> List, SADF_DetectorState *S)
{

    for (list<SADF_DetectorState *>::iterator i = List.begin(); i != List.end(); i++)
        if (S->getType() == (*i)->getType() && S->getScenario() == (*i)->getScenario() && S->getSubScenario() == (*i)->getSubScenario() && S->getExecutionTime() == (*i)->getExecutionTime() && S->getMarkovChainStatus() == (*i)->getMarkovChainStatus())
            return (*i);

    return NULL;
}

void SADF_ConstructDetectorStates(SADF_Graph *Graph, SADF_TPS *TPS, CId DetectorID, SADF_DetectorState *S)
{

    // Create new detect states

    list<SADF_DetectorState *> NewDetectStates;

    for (CId i = 0; i != Graph->getDetector(DetectorID)->getNumberOfScenarios(); i++)
        for (CId j = 0; j != Graph->getDetector(DetectorID)->getMarkovChain(i)->getNumberOfStates(); j++)
            if (Graph->getDetector(DetectorID)->getMarkovChain(i)->getTransitionProbability(S->getStateOfMarkovChain(i), j) > 0)
            {

                vector<CId> NewMarkovChainStatus(S->getMarkovChainStatus());
                NewMarkovChainStatus[i] = j;

                SADF_DetectorState *DetectState = new SADF_DetectorState(Graph->getDetector(DetectorID), TPS->getDetectorStates(DetectorID).size(), i, Graph->getDetector(DetectorID)->getMarkovChain(i)->getSubScenario(j), NewMarkovChainStatus, SADF_DETECT_STEP, SADF_MAX_DOUBLE);
                SADF_DetectorState *Test = SADF_DetectorState_InList(TPS->getDetectorStates(DetectorID), DetectState);

                if (Test == NULL)
                {
                    NewDetectStates.push_front(DetectState);
                    S->addTransition(DetectState, Graph->getDetector(DetectorID)->getMarkovChain(i)->getTransitionProbability(S->getStateOfMarkovChain(i), j));
                    TPS->getDetectorStates(DetectorID).push_front(DetectState);
                }
                else
                {
                    S->addTransition(Test, Graph->getDetector(DetectorID)->getMarkovChain(i)->getTransitionProbability(S->getStateOfMarkovChain(i), j));
                    delete DetectState;
                }
            }

    // Create states corresponding to each new detect state

    list<SADF_DetectorState *> NewEndStates;

    for (list<SADF_DetectorState *>::iterator i = NewDetectStates.begin(); i != NewDetectStates.end(); i++)
    {

        SADF_DetectorState *DetectState = (*i);

        // Create end state

        SADF_DetectorState *EndState = new SADF_DetectorState(Graph->getDetector(DetectorID), TPS->getDetectorStates(DetectorID).size(), SADF_UNDEFINED, SADF_UNDEFINED, DetectState->getMarkovChainStatus(), SADF_END_STEP, SADF_MAX_DOUBLE);
        SADF_DetectorState *Test = SADF_DetectorState_InList(TPS->getDetectorStates(DetectorID), EndState);

        if (Test == NULL)
        {
            TPS->getDetectorStates(DetectorID).push_front(EndState);
            NewEndStates.push_front(EndState);
        }
        else
        {
            delete EndState;
            EndState = Test;
        }

        // Create start states

        for (CId j = 0; j != Graph->getDetector(DetectorID)->getNumberOfProfiles(DetectState->getSubScenario()); j++)
        {

            SADF_DetectorState *StartState = new SADF_DetectorState(Graph->getDetector(DetectorID), TPS->getDetectorStates(DetectorID).size(), DetectState->getScenario(), DetectState->getSubScenario(), DetectState->getMarkovChainStatus(), SADF_START_STEP, Graph->getDetector(DetectorID)->getProfile(DetectState->getSubScenario(), j)->getExecutionTime());
            StartState->addTransition(EndState, 1);
            TPS->getDetectorStates(DetectorID).push_front(StartState);
            DetectState->addTransition(StartState, Graph->getDetector(DetectorID)->getProfile(DetectState->getSubScenario(), j)->getWeight());
        }
    }

    // Recursively construct states from new end states

    for (list<SADF_DetectorState *>::iterator i = NewEndStates.begin(); i != NewEndStates.end(); i++)
        SADF_ConstructDetectorStates(Graph, TPS, DetectorID, (*i));
}

// Constructor

SADF_TPS::SADF_TPS(SADF_Graph *Graph)
{

    NumberOfConfigurations = 0;

    KernelStates.resize(Graph->getNumberOfKernels());
    DetectorStates.resize(Graph->getNumberOfDetectors());
    InitialKernelStates.resize(Graph->getNumberOfKernels());
    InitialDetectorStates.resize(Graph->getNumberOfDetectors());

    // Create Kernel States

    for (CId i = 0; i != Graph->getNumberOfKernels(); i++)
    {

        // Creation of initial state

        SADF_KernelState *InitialState;

        if (Graph->getKernel(i)->hasControlInputChannels())
            InitialState = new SADF_KernelState(Graph->getKernel(i), KernelStates[i].size(), SADF_UNDEFINED, SADF_END_STEP, SADF_MAX_DOUBLE);
        else
            InitialState = new SADF_KernelState(Graph->getKernel(i), KernelStates[i].size(), 0, SADF_END_STEP, SADF_MAX_DOUBLE);

        InitialKernelStates[i] = InitialState;
        KernelStates[i].push_front(InitialState);

        SADF_ConstructKernelStates(Graph, this, i, InitialState);
    }

    // Create Detector States

    for (CId i = 0; i != Graph->getNumberOfDetectors(); i++)
    {

        vector<CId> MarkovChainStatus(Graph->getDetector(i)->getNumberOfScenarios());

        for (CId j = 0; j != Graph->getDetector(i)->getNumberOfScenarios(); j++)
            MarkovChainStatus[j] = Graph->getDetector(i)->getMarkovChain(j)->getInitialState();

        SADF_DetectorState *InitialState = new SADF_DetectorState(Graph->getDetector(i), DetectorStates[i].size(), SADF_UNDEFINED, SADF_UNDEFINED, MarkovChainStatus, SADF_END_STEP, SADF_MAX_DOUBLE);
        InitialDetectorStates[i] = InitialState;
        DetectorStates[i].push_front(InitialState);

        SADF_ConstructDetectorStates(Graph, this, i, InitialState);
    }

    InitialConfiguration = new SADF_Configuration(Graph, this, SADF_END_STEP);
}

// Destructor

SADF_TPS::~SADF_TPS()
{

    for (CId i = 0; i != KernelStates.size(); i++)
        for (list<SADF_KernelState *>::iterator j = KernelStates[i].begin(); j != KernelStates[i].end(); j++)
            delete(*j);

    for (CId i = 0; i != DetectorStates.size(); i++)
        for (list<SADF_DetectorState *>::iterator j = DetectorStates[i].begin(); j != DetectorStates[i].end(); j++)
            delete(*j);

    for (SADF_HashedListOfConfigurations::iterator i = ConfigurationSpace.begin(); i != ConfigurationSpace.end(); i++)
        for (SADF_ListOfConfigurations::iterator j = (*i).begin(); j != (*i).end(); j++)
            delete *j;
}

// Access to instance variables

void SADF_TPS::addConfiguration(SADF_Configuration *C)
{

    C->setIdentity((CId) NumberOfConfigurations);

    bool MatchFound = false;

    for (SADF_HashedListOfConfigurations::iterator i = ConfigurationSpace.begin(); !MatchFound && i != ConfigurationSpace.end(); i++)
        if ((*i).front()->getHashKey() == C->getHashKey())
        {
            (*i).push_front(C);
            MatchFound = true;
        }

    if (!MatchFound)
    {
        SADF_ListOfConfigurations NewListOfConfigurations;
        NewListOfConfigurations.push_front(C);
        ConfigurationSpace.push_front(NewListOfConfigurations);
    }

    NumberOfConfigurations++;
}

SADF_Configuration *SADF_TPS::inConfigurationSpace(SADF_Configuration *C)
{

    for (SADF_HashedListOfConfigurations::iterator i = ConfigurationSpace.begin(); i != ConfigurationSpace.end(); i++)
        if ((*i).front()->getHashKey() == C->getHashKey())
            for (SADF_ListOfConfigurations::iterator j = (*i).begin(); j != (*i).end(); j++)
                if ((*j)->equal(C))
                    return *j;

    return NULL;
}

void SADF_TPS::deleteContentOfConfigurations()
{

    for (SADF_HashedListOfConfigurations::iterator i = ConfigurationSpace.begin(); i != ConfigurationSpace.end(); i++)
        for (SADF_ListOfConfigurations::iterator j = (*i).begin(); j != (*i).end(); j++)
            (*j)->deleteContent();
}


// Functions to determine graph properties

void SADF_MarkReachableConfigurations(SADF_Configuration *Source)
{

    Source->setMarking(true);

    for (list<SADF_Transition *>::iterator i = Source->getTransitions().begin(); i != Source->getTransitions().end(); i++)
        if (!(*i)->getDestination()->isMarked())
            SADF_MarkReachableConfigurations((*i)->getDestination());
}

bool SADF_TPS::isSingleStronglyConnectedComponent()
{

    // Precondition: all configurations in configuration space must not be marked

    bool SingleStronglyConnectedComponent = true;

    SADF_MarkReachableConfigurations(ConfigurationSpace.front().front());

    for (SADF_HashedListOfConfigurations::iterator i = ConfigurationSpace.begin(); SingleStronglyConnectedComponent && i != ConfigurationSpace.end(); i++)
        for (SADF_ListOfConfigurations::iterator j = (*i).begin(); SingleStronglyConnectedComponent && j != (*i).end(); j++)
            if (!(*j)->isMarked())
                SingleStronglyConnectedComponent = false;

    return SingleStronglyConnectedComponent;
}

// Functions to remove transient configurations

void SADF_FindRecurrencePoints(SADF_Configuration *Source)
{

    Source->setMarking(true);

    for (list<SADF_Transition *>::iterator i = Source->getTransitions().begin(); i != Source->getTransitions().end(); i++)
        if (!(*i)->getDestination()->isMarked())
            SADF_FindRecurrencePoints((*i)->getDestination());
        else if ((*i)->getDestination()->isRelevant())
            (*i)->getDestination()->setRelevance(false);
}

void SADF_RemoveTransitionsUntilRecurrencePoints(SADF_Configuration *Source)
{

    bool Remove = false;

    for (list<SADF_Transition *>::iterator i = Source->getTransitions().begin(); !Remove && i != Source->getTransitions().end(); i++)
        if (!(*i)->getDestination()->isRelevant())
            Remove = true;

    if (!Remove)
        for (list<SADF_Transition *>::iterator i = Source->getTransitions().begin(); i != Source->getTransitions().end(); i++)
            SADF_RemoveTransitionsUntilRecurrencePoints((*i)->getDestination());

    Source->removeAllTransitions();
}

void SADF_TPS::removeTransientConfigurations()
{

    // Precondition: all configurations in configuration space must not be marked and relevant

    SADF_FindRecurrencePoints(InitialConfiguration);
    SADF_RemoveTransitionsUntilRecurrencePoints(InitialConfiguration);

    for (SADF_HashedListOfConfigurations::iterator i = ConfigurationSpace.begin(); i != ConfigurationSpace.end();)
    {

        SADF_HashedListOfConfigurations::iterator n = i;
        i++;

        for (SADF_ListOfConfigurations::iterator j = (*n).begin(); j != (*n).end();)
        {

            SADF_ListOfConfigurations::iterator m = j;
            j++;

            if ((*m)->getTransitions().empty())
            {
                NumberOfConfigurations--;
                delete *m;
                (*n).erase(m);
            }
            else
            {
                (*m)->setMarking(false);
                (*m)->setRelevance(true);
            }
        }

        if ((*n).empty())
            ConfigurationSpace.erase(n);
    }

    // Reassign identities

    CId ID = 0;

    for (SADF_HashedListOfConfigurations::iterator i = ConfigurationSpace.begin(); i != ConfigurationSpace.end(); i++)
        for (SADF_ListOfConfigurations::iterator j = (*i).begin(); j != (*i).end(); j++)
        {
            (*j)->setIdentity(ID);
            ID++;
        }
}

// Functions to compute equilibrium distribution

vector<CDouble> SADF_TPS::computeEquilibriumDistribution()
{

    // Initialise augmented matrix

    SparseMatrix *Matrix = new SparseMatrix((CId) NumberOfConfigurations);

    for (SADF_HashedListOfConfigurations::iterator i = ConfigurationSpace.begin(); i != ConfigurationSpace.end(); i++)
        for (SADF_ListOfConfigurations::iterator j = (*i).begin(); j != (*i).end(); j++)
            for (list<SADF_Transition *>::iterator k = (*j)->getTransitions().begin(); k != (*j)->getTransitions().end(); k++)
                if ((*k)->getDestination()->getIdentity() < NumberOfConfigurations - 1)
                    Matrix->set((*k)->getDestination()->getIdentity(), (*j)->getIdentity(), (*k)->getProbability());

    for (CId i = 0; i != NumberOfConfigurations + 1; i++)
    {

        Matrix->set((CId) NumberOfConfigurations - 1, i, 1);

        if (i < NumberOfConfigurations - 1)
            Matrix->set(i, i, Matrix->get(i, i) - 1);
    }

    vector<CDouble> EquilibriumDistribution = Matrix->computeEigenVector();

    delete Matrix;

    return EquilibriumDistribution;
}

