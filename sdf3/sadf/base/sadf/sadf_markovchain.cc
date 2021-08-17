/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_markovchain.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   29 August 2006
 *
 *  Function        :   SADF Markov Chain
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

#include "sadf_markovchain.h"

// Access to instance variables

CId SADF_MarkovChain::getStateID(const CString &StateName)
{

    for (CId i = 0; i != StateSpace.size(); i++)
        if (StateSpace[i] == StateName)
            return i;

    return SADF_UNDEFINED;
}

void SADF_MarkovChain::addState(const CString &Name, const CId SubScenarioID)
{

    CId NumberOfStates = StateSpace.size() + 1;

    StateSpace.resize(NumberOfStates);
    SubScenarios.resize(NumberOfStates);

    StateSpace[NumberOfStates - 1] = Name;
    SubScenarios[NumberOfStates - 1] = SubScenarioID;

    TransitionMatrix.resize(NumberOfStates);

    vector<CDouble> Transitions(NumberOfStates, 0);
    TransitionMatrix[NumberOfStates - 1] = Transitions;

    for (CId i = 0; i != NumberOfStates - 1; i++)
    {
        TransitionMatrix[i].resize(NumberOfStates);
        TransitionMatrix[i][NumberOfStates - 1] = 0;
    }
}

// Functions to determine whether Markov chain consists of single component

void SADF_MarkovChain::colorConnectedStates(CId StateID, vector<CId> &Color)
{

    Color[StateID] = 1;

    for (CId i = 0; i != TransitionMatrix.size(); i++)
        if (TransitionMatrix[StateID][i] > 0 && Color[i] == 0)
            colorConnectedStates(i, Color);
}

bool SADF_MarkovChain::isSingleComponent()
{

    vector<CId> Color(StateSpace.size(), 0);

    colorConnectedStates(0, Color);

    for (CId i = 0; i != StateSpace.size(); i++)
        if (Color[i] == 0)
            return false;

    return true;
}

// Functions to determine whether Markov chain is strongly connected

void SADF_MarkovChain::colorReverselyConnectedStates(CId StateID, vector<CId> &Color)
{

    Color[StateID] = 1;

    for (CId i = 0; i != TransitionMatrix.size(); i++)
        if (TransitionMatrix[i][StateID] > 0 && Color[i] == 0)
            colorReverselyConnectedStates(i, Color);
}

bool SADF_MarkovChain::isSingleStronglyConnectedComponent()
{

    // Markov Chain is assumed to be a single component

    vector<CId> Color(StateSpace.size(), 0);

    colorReverselyConnectedStates(0, Color);

    for (CId i = 0; i != StateSpace.size(); i++)
        if (Color[i] == 0)
            return false;

    return true;
}

// Functions to determine whether Markov chain is single simple cyclic strongly connected component

bool SADF_MarkovChain::isDeterministicCycle()
{

    bool Deterministic = isSingleStronglyConnectedComponent();

    for (CId i = 0; Deterministic && i != TransitionMatrix.size(); i++)
        for (CId j = 0; j != TransitionMatrix.size(); j++)
            if (TransitionMatrix[i][j] > 0)
                if (TransitionMatrix[i][j] < 1)
                    return false;

    return true;
}

