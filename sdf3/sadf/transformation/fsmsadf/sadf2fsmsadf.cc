/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf2sdf.h
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   2 December 2009
 *
 *  Function        :   Abstract SADF Graph in FSM-based SADF Graph
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

#include "sadf2fsmsadf.h"

FSMSADF::Graph *SADF2FSMSADF(SADF_Graph *SADF)
{

    // Check whether conversion is possible

    list<SDFgraph *> Collection;

    if (SADF_Verify_SimpleStronglyConsistentSDFCollection(SADF, Collection))
    {

        for (CId i = 0; i != SADF->getNumberOfKernels(); i++)
            if (SADF->getKernel(i)->getProfile(0, 0)->getExecutionTime() != (CDouble)((unsigned long long)(SADF->getKernel(i)->getProfile(0, 0)->getExecutionTime())))
                throw CException((CString)("Error: Real-valued execution time for Kernel '") + SADF->getKernel(i)->getName() + "' not surported.");

        for (CId i = 0; i != SADF->getNumberOfDetectors(); i++)
            if (SADF->getDetector(i)->getProfile(0, 0)->getExecutionTime() != (CDouble)((unsigned long long)(SADF->getDetector(i)->getProfile(0, 0)->getExecutionTime())))
                throw CException((CString)("Error: Real-valued execution time for Detector '") + SADF->getDetector(i)->getName() + "' not surported.");

    }
    else
        throw CException((CString)("Error: SADF graph '") + SADF->getName() + "' is not a strongly consistent collection of SDF graphs.");

    // Check uniqueness of initial scenario name

    if (SADF->getNumberOfDetectors() == 1)
        for (CId i = 0; i != SADF->getDetector(0)->getNumberOfSubScenarios(); i++)
            if (SADF->getDetector(0)->getSubScenarioName(i) == (CString)("initial_") + SADF->getName())
                throw CException((CString)("Error: Please rename subscenario name '") + SADF->getDetector(0)->getSubScenarioName(i)
                                 + "' of Detector '" + SADF->getDetector(0)->getName() + "' of SADF graph '" + SADF->getName() + "'.");

    // Create FSM-based SADF graph

    FSMSADF::Graph *FS = new FSMSADF::Graph();
    FS->setName(SADF->getName());

    // Create Scenario Graphs

    for (list<SDFgraph *>::iterator i = Collection.begin(); i != Collection.end(); i++)
    {

        TimedSDFgraph *SDF = (TimedSDFgraph *)(*i);

        FSMSADF::Scenario *S = new FSMSADF::Scenario(FSMSADF::GraphComponent(FS, FS->getScenarios().size(), SDF->getName()));
        FS->addScenario(S);

        FSMSADF::ScenarioGraph *SG = new FSMSADF::ScenarioGraph(FSMSADF::GraphComponent(FS, FS->getScenarioGraphs().size(), SDF->getName()));
        FS->addScenarioGraph(SG);
        S->setScenarioGraph(SG);
        SG->setType(SDF->getName());

        // Create Actors

        for (SDFactors::iterator a = SDF->getActors().begin(); a != SDF->getActors().end(); a++)
        {

            FSMSADF::Actor *Actor = new FSMSADF::Actor(FSMSADF::GraphComponent(SG, SG->getActors().size(), (*a)->getName()));
            SG->addActor(Actor);
            Actor->setType((*a)->getName());
            Actor->setExecutionTimeOfScenario(S, ((TimedSDFactor *)(*a))->getDefaultProcessor(), ((TimedSDFactor *)(*a))->getExecutionTime());
            Actor->setDefaultProcessorType(((TimedSDFactor *)(*a))->getDefaultProcessor());

            for (SDFports::iterator p = (*a)->portsBegin(); p != (*a)->portsEnd(); p++)
            {

                FSMSADF::Port *Port = new FSMSADF::Port(FSMSADF::GraphComponent(Actor, Actor->getPorts().size(), (*p)->getName()));
                Port->setType((*p)->getTypeAsString());
                Actor->addPort(Port);
                Port->setRateOfScenario(S, (*p)->getRate());
            }
        }

        // Create Channels

        for (SDFchannels::iterator c = SDF->channelsBegin(); c != SDF->channelsEnd(); c++)
        {

            FSMSADF::Channel *Channel = new FSMSADF::Channel(FSMSADF::GraphComponent(SG, SG->getChannels().size(), (*c)->getName()));
            SG->addChannel(Channel);
            Channel->setInitialTokens((*c)->getInitialTokens());
            Channel->setTokenSizeOfScenario(S, ((TimedSDFchannel *)(*c))->getTokenSize());
            Channel->connectSrc(SG->getActor((*c)->getSrcActor()->getName())->getPort((*c)->getSrcPort()->getName()));
            Channel->connectDst(SG->getActor((*c)->getDstActor()->getName())->getPort((*c)->getDstPort()->getName()));

            if ((*c)->getInitialTokens() > 0)
            {
                CString PersistentNames;
                for (CId i = 0; i != (*c)->getInitialTokens(); i++)
                {
                    CString TokenName = (*c)->getName();
                    if (i != 0)
                        PersistentNames = PersistentNames + ",";
                    PersistentNames = PersistentNames + TokenName + (CString)(i);
                }
                Channel->setPersistentTokenNames(PersistentNames);
            }
        }
    }

    if (SADF->getNumberOfDetectors() == 0)
    {

        // Create FSM

        FSMSADF::FSM *FSM = new FSMSADF::FSM(FSMSADF::GraphComponent(FS, 0, "default"));
        FS->setFSM(FSM);

        FSMSADF::FSMstate *InitialState = new FSMSADF::FSMstate(FSMSADF::GraphComponent(FSM, FSM->getStates().size(), "default"));
        FSM->addState(InitialState);
        FSM->setInitialState(InitialState);
        InitialState->setScenario(FS->getScenario(Collection.front()->getName()));

        FSMSADF::FSMtransition *Transition = new FSMSADF::FSMtransition(FSMSADF::GraphComponent(FSM, 0));
        Transition->connect(FSM->getInitialState(), FSM->getInitialState());
    }

    if (SADF->getNumberOfDetectors() == 1)
    {

        // Create Initial Scenario

        FSMSADF::Scenario *InitialScenario = new FSMSADF::Scenario(FSMSADF::GraphComponent(FS, FS->getScenarios().size(), (CString)("initial_") + SADF->getName()));
        FS->addScenario(InitialScenario);

        FSMSADF::ScenarioGraph *SG = new FSMSADF::ScenarioGraph(FSMSADF::GraphComponent(FS, FS->getScenarioGraphs().size(), (CString)("initial_") + SADF->getName()));
        FS->addScenarioGraph(SG);
        InitialScenario->setScenarioGraph(SG);
        SG->setType((CString)("initial_") + SADF->getName());

        FSMSADF::Actor *Actor = new FSMSADF::Actor(FSMSADF::GraphComponent(SG, SG->getActors().size(), (CString)("initial_") + SADF->getName()));
        SG->addActor(Actor);
        Actor->setType((CString)("initial_") + SADF->getName());
        Actor->setExecutionTimeOfScenario(InitialScenario, (CString)("initial_") + SADF->getName(), 0);
        Actor->setDefaultProcessorType((CString)("initial_") + SADF->getName());

        for (CId i = 0; i != SADF->getNumberOfDataChannels(); i++)
            if (SADF->getDataChannel(i)->getNumberOfInitialTokens() > 0)
            {

                CString ChannelName = SADF->getDataChannel(i)->getName();

                FSMSADF::Port *InPort = new FSMSADF::Port(FSMSADF::GraphComponent(Actor, Actor->getPorts().size(), (CString)("In_") + ChannelName));
                Actor->addPort(InPort);
                InPort->setType(FSMSADF::Port::In);
                InPort->setRateOfScenario(InitialScenario, 1);

                FSMSADF::Port *OutPort = new FSMSADF::Port(FSMSADF::GraphComponent(Actor, Actor->getPorts().size(), (CString)("Out") + ChannelName));
                Actor->addPort(OutPort);
                OutPort->setType(FSMSADF::Port::Out);
                OutPort->setRateOfScenario(InitialScenario, 1);

                FSMSADF::Channel *SelfChannel = new FSMSADF::Channel(FSMSADF::GraphComponent(SG, SG->getChannels().size(), ChannelName));
                SG->addChannel(SelfChannel);
                SelfChannel->connectSrc(OutPort);
                SelfChannel->connectDst(InPort);
                SelfChannel->setInitialTokens(SADF->getDataChannel(i)->getNumberOfInitialTokens());
                SelfChannel->setTokenSizeOfScenario(InitialScenario, 1);

                CString PersistentNames;
                for (CId j = 0; j != SADF->getDataChannel(i)->getNumberOfInitialTokens(); j++)
                {
                    if (j != 0)
                        PersistentNames = PersistentNames + ",";
                    PersistentNames = PersistentNames + ChannelName + (CString)(j);
                }
                SelfChannel->setPersistentTokenNames(PersistentNames);
            }

        for (CId i = 0; i != SADF->getNumberOfControlChannels(); i++)
            if (SADF->getControlChannel(i)->getNumberOfInitialTokens() > 0)
            {

                CString ChannelName = SADF->getControlChannel(i)->getName();

                FSMSADF::Port *InPort = new FSMSADF::Port(FSMSADF::GraphComponent(Actor, Actor->getPorts().size(), (CString)("In_") + ChannelName));
                Actor->addPort(InPort);
                InPort->setType(FSMSADF::Port::In);
                InPort->setRateOfScenario(InitialScenario, 1);

                FSMSADF::Port *OutPort = new FSMSADF::Port(FSMSADF::GraphComponent(Actor, Actor->getPorts().size(), (CString)("Out_") + ChannelName));
                Actor->addPort(OutPort);
                OutPort->setType(FSMSADF::Port::Out);
                OutPort->setRateOfScenario(InitialScenario, 1);

                FSMSADF::Channel *SelfChannel = new FSMSADF::Channel(FSMSADF::GraphComponent(SG, SG->getChannels().size(), ChannelName));
                SG->addChannel(SelfChannel);
                SelfChannel->connectSrc(OutPort);
                SelfChannel->connectDst(InPort);
                SelfChannel->setInitialTokens(SADF->getControlChannel(i)->getNumberOfInitialTokens());
                SelfChannel->setTokenSizeOfScenario(InitialScenario, 1);

                CString PersistentNames;
                for (CId j = 0; j != SADF->getControlChannel(i)->getNumberOfInitialTokens(); j++)
                {
                    if (j != 0)
                        PersistentNames = PersistentNames + ",";
                    PersistentNames = PersistentNames + ChannelName + (CString)(j);
                }
                SelfChannel->setPersistentTokenNames(PersistentNames);
            }

        for (CId i = 0; i != SADF->getNumberOfKernels(); i++)
        {

            CString ChannelName = SADF->getKernel(i)->getName();

            FSMSADF::Port *InPort = new FSMSADF::Port(FSMSADF::GraphComponent(Actor, Actor->getPorts().size(), (CString)("In_") + ChannelName));
            Actor->addPort(InPort);
            InPort->setType(FSMSADF::Port::In);
            InPort->setRateOfScenario(InitialScenario, 1);

            FSMSADF::Port *OutPort = new FSMSADF::Port(FSMSADF::GraphComponent(Actor, Actor->getPorts().size(), (CString)("Out_") + ChannelName));
            Actor->addPort(OutPort);
            OutPort->setType(FSMSADF::Port::Out);
            OutPort->setRateOfScenario(InitialScenario, 1);

            FSMSADF::Channel *SelfChannel = new FSMSADF::Channel(FSMSADF::GraphComponent(SG, SG->getChannels().size(), ChannelName));
            SG->addChannel(SelfChannel);
            SelfChannel->connectSrc(OutPort);
            SelfChannel->connectDst(InPort);
            SelfChannel->setInitialTokens(1);
            SelfChannel->setTokenSizeOfScenario(InitialScenario, 1);
            SelfChannel->setPersistentTokenNames((CString)("Self_") + ChannelName + "0");
        }

        for (CId i = 0; i != SADF->getNumberOfDetectors(); i++)
        {

            CString ChannelName = SADF->getDetector(i)->getName();

            FSMSADF::Port *InPort = new FSMSADF::Port(FSMSADF::GraphComponent(Actor, Actor->getPorts().size(), (CString)("In_") + ChannelName));
            Actor->addPort(InPort);
            InPort->setType(FSMSADF::Port::In);
            InPort->setRateOfScenario(InitialScenario, 1);

            FSMSADF::Port *OutPort = new FSMSADF::Port(FSMSADF::GraphComponent(Actor, Actor->getPorts().size(), (CString)("Out_") + ChannelName));
            Actor->addPort(OutPort);
            OutPort->setType(FSMSADF::Port::Out);
            OutPort->setRateOfScenario(InitialScenario, 1);

            FSMSADF::Channel *SelfChannel = new FSMSADF::Channel(FSMSADF::GraphComponent(SG, SG->getChannels().size(), ChannelName));
            SG->addChannel(SelfChannel);
            SelfChannel->connectSrc(OutPort);
            SelfChannel->connectDst(InPort);
            SelfChannel->setInitialTokens(1);
            SelfChannel->setTokenSizeOfScenario(InitialScenario, 1);
            SelfChannel->setPersistentTokenNames((CString)("Self_") + ChannelName + "0");
        }

        // Create FSM

        FSMSADF::FSM *FSM = new FSMSADF::FSM(FSMSADF::GraphComponent(FS, 0, SADF->getDetector(0)->getScenarioName(0)));
        FS->setFSM(FSM);

        SADF_MarkovChain *MarkovChain = SADF->getDetector(0)->getMarkovChain(0);

        for (CId i = 0; i != MarkovChain->getNumberOfStates(); i++)
        {

            FSMSADF::FSMstate *State = new FSMSADF::FSMstate(FSMSADF::GraphComponent(FSM, FSM->getStates().size(), MarkovChain->getStateName(i)));
            FSM->addState(State);
            State->setScenario(FS->getScenario(SADF->getDetector(0)->getSubScenarioName(MarkovChain->getSubScenario(i))));
        }

        for (CId i = 0; i != MarkovChain->getNumberOfStates(); i++)
            for (CId j = 0; j != MarkovChain->getNumberOfStates(); j++)
                if (MarkovChain->getTransitionProbability(i, j) > 0)
                {

                    FSMSADF::FSMtransition *Transition = new FSMSADF::FSMtransition(FSMSADF::GraphComponent(FSM, 0));
                    Transition->connect(FSM->getState(MarkovChain->getStateName(i)), FSM->getState(MarkovChain->getStateName(j)));
                }

        FSMSADF::FSMstate *InitialState = new FSMSADF::FSMstate(FSMSADF::GraphComponent(FSM, FSM->getStates().size(), (CString)("initial_") + SADF->getName()));
        FSM->addState(InitialState);
        FSM->setInitialState(InitialState);
        InitialState->setScenario(InitialScenario);

        for (CId i = 0; i != MarkovChain->getNumberOfStates(); i++)
            if (MarkovChain->getTransitionProbability(MarkovChain->getInitialState(), i) > 0)
            {

                FSMSADF::FSMtransition *Transition = new FSMSADF::FSMtransition(FSMSADF::GraphComponent(FSM, 0));
                Transition->connect(InitialState, FSM->getState(MarkovChain->getStateName(i)));
            }
    }

    return FS;
}
