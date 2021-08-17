/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_parser.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   29 August 2006
 *
 *  Function        :   XML Parser for SADF Graphs
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

#include "sadf_parser.h"

// Functions to check inter-process matching of scenarios and cross-fix scenario identities

void SADF_FixScenarioIdentitiesSendToProcess(CString &GraphName, SADF_Process *Process)
{

    // Identify undefined scenarios in case process is controlled by single detector

    if (!Process->hasMultipleControlInputChannels())
    {

        SADF_Channel *Channel = Process->getControlInputChannels()[0];
        SADF_Process *Detector = Channel->getSource();

        // Check existance of process scenarios for any control tokens produced by detector and fix identities

        for (CId i = 0; i != Process->getNumberOfScenarios(); i++)
        {

            bool MatchFound = false;

            for (CId j = 0; j != Detector->getNumberOfSubScenarios(); j++)
                if (Channel->getProductionScenarioName(j) == Process->getScenarioName(i))
                {
                    MatchFound = true;
                    Channel->setProductionScenarioID(j, i);
                }

            if (!MatchFound)
                throw CException((CString)("Error: ") + ((Process->getType() == SADF_KERNEL) ? "Kernel '" : "Detector '") + Process->getName() + "' of SADF graph '" + GraphName
                                 + "' can operate in a scenario '" + Process->getScenarioName(i) + "', while detector '" + Detector->getName()
                                 + "' controlling it does have a subscenario in which that scenario is invoked.");
        }

        // Check inactive scenarios and fix new identities when possible

        for (CId i = 0; i != Detector->getNumberOfSubScenarios(); i++)
            if (Process->getScenarioID(Channel->getProductionScenarioName(i)) == SADF_UNDEFINED)
            {
                if (Process->getType() == SADF_KERNEL)
                {

                    CId ScenarioID = Process->addScenario(Channel->getProductionScenarioName(i));
                    Process->makeInactive(ScenarioID);

                    Process->addProfile(ScenarioID, new SADF_Profile());

                    for (CId j = i; j != Detector->getNumberOfSubScenarios(); j++)
                        if (Channel->getProductionScenarioName(i) == Channel->getProductionScenarioName(j))
                            Channel->setProductionScenarioID(j, ScenarioID);
                }
                else
                    throw CException((CString)("Error: Detector '") + Process->getName() + "' of SADF graph '" + GraphName
                                     + "' does not have a scenario '" + Channel->getProductionScenarioName(i) + "', while detector '" + Detector->getName()
                                     + "' controlling it does have a subscenario in which that scenario is invoked.");
            }
    }

    // Match scenarios if kernel is controlled by multiple detectors

    if (Process->hasMultipleControlInputChannels())
    {

        for (CId i = 0; i != Process->getControlInputChannels().size(); i++)
        {

            SADF_Channel *Channel = Process->getControlInputChannels()[i];
            SADF_Process *Detector = Channel->getSource();

            // Check existance of process control values for any control tokens produced by detector and fix identities

            for (CId j = 0; j != Process->getNumberOfScenarios(); j++)
            {

                bool MatchFound = false;

                for (CId k = 0; k != Detector->getNumberOfSubScenarios(); k++)
                    if (Channel->getProductionScenarioName(k) == Process->getControlValue(j, i))
                    {
                        MatchFound = true;
                        Channel->setProductionScenarioID(k, Process->getControlID(j, i));
                    }

                if (!MatchFound)
                    throw CException((CString)("Error: ") + ((Process->getType() == SADF_KERNEL) ? "Kernel '" : "Detector '") + Process->getName() + "' of SADF graph '"
                                     + GraphName + "' can operate in a scenario '" + Process->getScenarioName(j) + "' by receiving control value '" + Process->getControlValue(j, i)
                                     + "', while detector '" + Detector->getName() + "' controlling it does have a subscenario in which control tokens valued '"
                                     + Process->getControlValue(j, i) + "' are produced.");
            }
        }

        // Ensure that any inactive scenarios exist

        if (Process->getType() == SADF_KERNEL)
            throw CException((CString)("Error: Automatic construction of inactive scenarios for kernels with multiple control inputs is not yet implemented."));
        else
            throw CException((CString)("Error: Checking existance of scenarios for all control value combinations for a detector with multiple control inputs is not yet implemented."));
    }
}

// Construct SADF profile from XML

void SADF_ConstructProfile(SADF_Profile *Profile, const CNodePtr ProfileNode)
{

    if (CHasAttribute(ProfileNode, "execution_time"))
    {
        if ((CDouble)(CGetAttribute(ProfileNode, "execution_time")) < 0)
            throw  CException("Error: Negative execution time sepecified.");
        else
            Profile->setExecutionTime(CGetAttribute(ProfileNode, "execution_time"));
    }

    if (CHasAttribute(ProfileNode, "weight"))
        Profile->setWeight(CGetAttribute(ProfileNode, "weight"));
}

// Construct SADF channel from XML

void SADF_ConstructChannel(CString &GraphName, SADF_Channel *Channel, const CNodePtr ChannelPropertyNode)
{

    if (CHasAttribute(ChannelPropertyNode, "buffer_size"))
        Channel->setBufferSize(CGetAttribute(ChannelPropertyNode, "buffer_size"));

    if (Channel->getType() == SADF_DATA_CHANNEL)
        if (CHasAttribute(ChannelPropertyNode, "number_of_initial_tokens"))
            Channel->setNumberOfInitialTokens(CGetAttribute(ChannelPropertyNode, "number_of_initial_tokens"));

    if (CHasAttribute(ChannelPropertyNode, "token_size"))
        Channel->setTokenSize(CGetAttribute(ChannelPropertyNode, "token_size"));

    if (Channel->getType() == SADF_CONTROL_CHANNEL)
        if (CHasAttribute(ChannelPropertyNode, "initial_tokens"))
        {

            CString TokenSequence = CGetAttribute(ChannelPropertyNode, "initial_tokens").trim();

            // Parse sequence of initial control tokens

            while (TokenSequence.size() != 0)
            {

                char c;

                CString Element = "";

                do
                {
                    c = TokenSequence[0];
                    TokenSequence = TokenSequence.substr(1);

                    if (c == ',')
                        break;

                    Element += c;

                }
                while (TokenSequence.size() != 0);

                bool NumberFound = false;

                CString Tokens = "";

                do
                {
                    c = Element[0];
                    Element = Element.substr(1);

                    if (c == '*')
                    {
                        NumberFound = true;
                        break;
                    }

                    Tokens += c;

                }
                while (Element.size() != 0);

                if (NumberFound && Element.trim().size() == 0)
                    throw CException((CString)("Error: Initial token sequence for channel '") + Channel->getName() + "' of SADF graph '" + GraphName + "' is invalid.");

                CString ScenarioName = "";
                CId NumberOfTokens = 1;

                if (NumberFound)
                {
                    NumberOfTokens = strtol(Tokens, NULL, 10);
                    ScenarioName = Element;
                }
                else
                    ScenarioName = Tokens;

                ScenarioName.trim();

                if (!Channel->getDestination()->hasMultipleControlInputChannels())
                {

                    if (Channel->getDestination()->getScenarioID(ScenarioName) == SADF_UNDEFINED)
                        throw CException((CString)("Error: Initial token valued '") + ScenarioName + "' as specified for channel '" + Channel->getName()
                                         + "' of SADF graph '" + GraphName + "' is not a valid scenario for "
                                         + ((Channel->getDestination()->getType() == SADF_KERNEL) ? "kernel '" : "detector '") + Channel->getDestination()->getName() + "'.");

                    Channel->addInitialTokens(NumberOfTokens, Channel->getDestination()->getScenarioID(ScenarioName));

                }
                else
                {

                    bool ScenarioNotFound = true;

                    CId ChannelID = Channel->getDestination()->getControlInputChannelID(Channel->getName());

                    for (CId i = 0; ScenarioNotFound && i != Channel->getDestination()->getNumberOfScenarios(); i++)
                        if (Channel->getDestination()->getControlValue(i, ChannelID) == ScenarioName)
                        {
                            ScenarioNotFound = false;
                            Channel->addInitialTokens(NumberOfTokens, Channel->getDestination()->getControlID(i, ChannelID));
                        }

                    if (ScenarioNotFound)
                        throw CException((CString)("Error: Initial token valued '") + ScenarioName + "' as specified for channel '" + Channel->getName()
                                         + "' of SADF graph '" + GraphName + "' is not a valid control value for "
                                         + ((Channel->getDestination()->getType() == SADF_KERNEL) ? "kernel '" : "detector '") + Channel->getDestination()->getName() + "'.");
                }
            }
        }
}

// Construct SADF Scenario from XML

void SADF_ConstructScenario(CString &GraphName, SADF_Process *Process, CId ScenarioID, const CNodePtr ScenarioNode)
{

    // Parse Consumptions

    for (CNodePtr ConsumptionNode = CGetChildNode(ScenarioNode, "consume"); ConsumptionNode != NULL; ConsumptionNode = CNextNode(ConsumptionNode, "consume"))
    {

        if (!CHasAttribute(ConsumptionNode, "channel"))
            throw CException((CString)("Error: No channel specified for consumption in ") + ((Process->getType() == SADF_KERNEL) ? "scenario '" : "subscenario '")
                             + CGetAttribute(ScenarioNode, "name").trim() + "' for " + ((Process->getType() == SADF_KERNEL) ? "kernel '" : "detector '") + Process->getName()
                             + "' of SADF graph '" + GraphName + "'.");

        if (Process->getDataInputChannelID(CGetAttribute(ConsumptionNode, "channel").trim()) == SADF_UNDEFINED)
            throw CException((CString)("Error: Channel '") + CGetAttribute(ConsumptionNode, "channel").trim() + "' for which consumption is specified in "
                             + ((Process->getType() == SADF_KERNEL) ? "scenario '" : "subscenario '") + CGetAttribute(ScenarioNode, "name").trim() + "' is not an input channel for "
                             + ((Process->getType() == SADF_KERNEL) ? "kernel '" : "detector '") + Process->getName() + "' of SADF graph '" + GraphName + "'.");

        for (CNodePtr ConsumptionNode2 = CNextNode(ConsumptionNode, "consume"); ConsumptionNode2 != NULL; ConsumptionNode2 = CNextNode(ConsumptionNode2, "consume"))
            if (CHasAttribute(ConsumptionNode2, "channel"))
                if (CGetAttribute(ConsumptionNode, "channel").trim() == CGetAttribute(ConsumptionNode2, "channel").trim())
                    throw CException((CString)("Error: Multiple consumptions specified from channel '") + CGetAttribute(ConsumptionNode, "channel").trim() + "' in "
                                     + ((Process->getType() == SADF_KERNEL) ? "scenario '" : "subscenario '") + CGetAttribute(ScenarioNode, "name").trim() + "' for "
                                     + ((Process->getType() == SADF_KERNEL) ? "kernel '" : "detector '") + Process->getName() + "' of SADF graph '" + GraphName + "'.");

        if (!CHasAttribute(ConsumptionNode, "tokens"))
            throw CException((CString)("Error: No rate specified for consumption from channel '") + CGetAttribute(ConsumptionNode, "channel").trim() + "' in "
                             + ((Process->getType() == SADF_KERNEL) ? "scenario '" : "subscenario '") + CGetAttribute(ScenarioNode, "name").trim() + "' for "
                             + ((Process->getType() == SADF_KERNEL) ? "kernel '" : "detector '") + Process->getName() + "' of SADF graph '" + GraphName + "'.");

        Process->getDataInputChannels()[Process->getDataInputChannelID(CGetAttribute(ConsumptionNode, "channel").trim())]
        ->setConsumptionRate(ScenarioID, CGetAttribute(ConsumptionNode, "tokens"));
    }

    // Parse Productions

    for (CNodePtr ProductionNode = CGetChildNode(ScenarioNode, "produce"); ProductionNode != NULL; ProductionNode = CNextNode(ProductionNode, "produce"))
    {

        if (!CHasAttribute(ProductionNode, "channel"))
            throw CException((CString)("Error: No channel specified for production in ") + ((Process->getType() == SADF_KERNEL) ? "scenario '" : "subscenario '")
                             + CGetAttribute(ScenarioNode, "name").trim() + "' for " + ((Process->getType() == SADF_KERNEL) ? "kernel '" : "detector '") + Process->getName()
                             + "' of SADF graph '" + GraphName + "'.");

        if (Process->getDataOutputChannelID(CGetAttribute(ProductionNode, "channel").trim()) == SADF_UNDEFINED
            && Process->getControlOutputChannelID(CGetAttribute(ProductionNode, "channel").trim()) == SADF_UNDEFINED)
            throw CException((CString)("Error: Channel '") + CGetAttribute(ProductionNode, "channel").trim() + "' for which production is specified in "
                             + ((Process->getType() == SADF_KERNEL) ? "scenario '" : "subscenario '") + CGetAttribute(ScenarioNode, "name").trim() + "' is not an output channel for "
                             + ((Process->getType() == SADF_KERNEL) ? "kernel '" : "detector '") + Process->getName() + " of SADF graph '" + GraphName + "'.");

        for (CNodePtr ProductionNode2 = CNextNode(ProductionNode, "produce"); ProductionNode2 != NULL; ProductionNode2 = CNextNode(ProductionNode2, "produce"))
            if (CHasAttribute(ProductionNode2, "channel"))
                if (CGetAttribute(ProductionNode, "channel").trim() == CGetAttribute(ProductionNode2, "channel").trim())
                    throw CException((CString)("Error: Multiple productions specified to channel '") + CGetAttribute(ProductionNode, "channel").trim() + "' in "
                                     + ((Process->getType() == SADF_KERNEL) ? "scenario '" : "subscenario '") + CGetAttribute(ScenarioNode, "name").trim() + "' for "
                                     + ((Process->getType() == SADF_KERNEL) ? "kernel '" : "detector '") + Process->getName() + "' of SADF graph '" + GraphName + "'.");

        if (!CHasAttribute(ProductionNode, "tokens"))
            throw CException((CString)("Error: No rate specified for production to channel '") + CGetAttribute(ProductionNode, "channel").trim() + "' in "
                             + ((Process->getType() == SADF_KERNEL) ? "scenario '" : "subscenario '") + CGetAttribute(ScenarioNode, "name").trim() + "' for "
                             + ((Process->getType() == SADF_KERNEL) ? "kernel '" : "detector '") + Process->getName() + "' of SADF graph '" + GraphName + "'.");

        if (Process->getDataOutputChannelID(CGetAttribute(ProductionNode, "channel").trim()) == SADF_UNDEFINED)
        {

            // Process is a Detector

            Process->getControlOutputChannels()[Process->getControlOutputChannelID(CGetAttribute(ProductionNode, "channel").trim())]
            ->setProductionRate(ScenarioID, CGetAttribute(ProductionNode, "tokens"));

            if (CHasAttribute(ProductionNode, "value"))
                Process->getControlOutputChannels()[Process->getControlOutputChannelID(CGetAttribute(ProductionNode, "channel").trim())]
                ->setProductionScenarioName(ScenarioID, CGetAttribute(ProductionNode, "value").trim());
            else
                Process->getControlOutputChannels()[Process->getControlOutputChannelID(CGetAttribute(ProductionNode, "channel").trim())]
                ->setProductionScenarioName(ScenarioID, CGetAttribute(ScenarioNode, "name").trim());

        }
        else

            // Process is a Kernel or Detector

            Process->getDataOutputChannels()[Process->getDataOutputChannelID(CGetAttribute(ProductionNode, "channel").trim())]
            ->setProductionRate(ScenarioID, CGetAttribute(ProductionNode, "tokens"));
    }

    // Parse Profiles

    if (!CHasChildNode(ScenarioNode, "profile"))
    {

        Process->addProfile(ScenarioID, new SADF_Profile());

    }
    else
    {

        CDouble Total = 0;

        for (CNodePtr ProfileNode = CGetChildNode(ScenarioNode, "profile"); ProfileNode != NULL; ProfileNode = CNextNode(ProfileNode, "profile"))
        {
            SADF_Profile *Profile = new SADF_Profile();
            SADF_ConstructProfile(Profile, ProfileNode);
            Total += Profile->getWeight();
            Process->addProfile(ScenarioID, Profile);
        }

        for (CId i = 0; i != Process->getNumberOfProfiles(ScenarioID); i++)
            Process->getProfile(ScenarioID, i)->setWeight(Process->getProfile(ScenarioID, i)->getWeight() / Total);
    }
}

// Construct SADF Markov Chain from XML

void SADF_ConstructMarkovChain(CString &GraphName, SADF_Process *Detector, SADF_MarkovChain *MarkovChain, const CNodePtr MarkovChainNode)
{

    // Determine state space

    if (!CHasChildNode(MarkovChainNode, "state"))
        throw CException((CString)("Error: No states specified for Markov chain")
                         + ((Detector->hasControlInputChannels()) ? " regarding scenario '" + CGetAttribute(MarkovChainNode, "scenario").trim() + "' " : " ")
                         + "of detector '" + Detector->getName() + "' of SADF graph '" + GraphName + "'.");

    for (CNodePtr StateNode = CGetChildNode(MarkovChainNode, "state"); StateNode != NULL; StateNode = CNextNode(StateNode, "state"))
    {

        if (!CHasAttribute(StateNode, "name"))
            throw CException((CString)("Error: No name specified for state of Markov chain")
                             + ((Detector->hasControlInputChannels()) ? " for scenario '" + CGetAttribute(MarkovChainNode, "scenario").trim() + "' " : " ")
                             + "of detector '" + Detector->getName() + "' of SADF graph '" + GraphName + "'.");

        for (CNodePtr StateNode2 = CNextNode(StateNode, "state"); StateNode2 != NULL; StateNode2 = CNextNode(StateNode2, "state"))
            if (CHasAttribute(StateNode2, "name"))
                if (CGetAttribute(StateNode, "name").trim() == CGetAttribute(StateNode2, "name").trim())
                    throw CException((CString)("Error: Multiple states named '") + CGetAttribute(StateNode, "name").trim() + "' specified for Markov chain"
                                     + ((Detector->hasControlInputChannels()) ? " for scenario '" + CGetAttribute(MarkovChainNode, "scenario").trim() + "' " : " ")
                                     + "of detector '" + Detector->getName() + "' of SADF graph '" + GraphName + "'.");

        if (!CHasAttribute(StateNode, "subscenario"))
            throw CException((CString)("Error: No subscenario specified for state '") + CGetAttribute(StateNode, "name").trim() + "' of Markov chain"
                             + ((Detector->hasControlInputChannels()) ? " for scenario '" + CGetAttribute(MarkovChainNode, "scenario").trim() + "' " : " ")
                             + "of detector '" + Detector->getName() + "' of SADF graph '" + GraphName + "'.");

        if (Detector->getSubScenarioID(CGetAttribute(StateNode, "subscenario").trim()) == SADF_UNDEFINED)
            throw CException((CString)("Error: Subscenario specified for state '") + CGetAttribute(StateNode, "name").trim() + "' of Markov chain"
                             + ((Detector->hasControlInputChannels()) ? " for scenario '" + CGetAttribute(MarkovChainNode, "scenario").trim() + "' " : " ")
                             + "of detector '" + Detector->getName() + "' of SADF graph '" + GraphName + "' does not exist.");

        MarkovChain->addState(CGetAttribute(StateNode, "name").trim(), Detector->getSubScenarioID(CGetAttribute(StateNode, "subscenario").trim()));
    }

    // Determinde initial state

    CId InitialState = MarkovChain->getStateID(CGetAttribute(MarkovChainNode, "initial_state").trim());

    if (InitialState != SADF_UNDEFINED)
        MarkovChain->setInitialState(InitialState);
    else
        throw CException((CString)("Error: Initial state '") + CGetAttribute(MarkovChainNode, "initial_state").trim() + "' of Markov chain"
                         + ((Detector->hasControlInputChannels()) ? " for scenario '" + CGetAttribute(MarkovChainNode, "scenario").trim() + "' " : " ")
                         + "of detector '" + Detector->getName() + "' of SADF graph '" + GraphName + "' does not exist.");

    // Determine transition matrix

    for (CNodePtr StateNode = CGetChildNode(MarkovChainNode, "state"); StateNode != NULL; StateNode = CNextNode(StateNode, "state"))
    {

        CId SourceID = MarkovChain->getStateID(CGetAttribute(StateNode, "name").trim());

        if (!CHasChildNode(StateNode, "transition"))
            throw CException((CString)("Error: No transitions specified from state '") + CGetAttribute(StateNode, "name").trim() + "' of Markov chain"
                             + ((Detector->hasControlInputChannels()) ? " for scenario '" + CGetAttribute(MarkovChainNode, "scenario").trim() + "' " : " ")
                             + "of detector '" + Detector->getName() + "' of SADF graph '" + GraphName + "'.");

        CDouble Total = 0;

        for (CNodePtr TransitionNode = CGetChildNode(StateNode, "transition"); TransitionNode != NULL; TransitionNode = CNextNode(TransitionNode, "transition"))
        {

            if (!CHasAttribute(TransitionNode, "destination"))
                throw CException((CString)("Error: No destination specified for transition from state '") + CGetAttribute(StateNode, "name").trim() + "' of Markov chain"
                                 + ((Detector->hasControlInputChannels()) ? " for scenario '" + CGetAttribute(MarkovChainNode, "scenario").trim() + "' " : " ")
                                 + "of detector '" + Detector->getName() + "' of SADF graph '" + GraphName + "'.");

            if (MarkovChain->getStateID(CGetAttribute(TransitionNode, "destination").trim()) == SADF_UNDEFINED)
                throw CException((CString)("Error: Destination state '") + CGetAttribute(TransitionNode, "destination").trim()
                                 + "' does not exist for transition of Markov chain" + ((Detector->hasControlInputChannels()) ? " for scenario '"
                                         + CGetAttribute(MarkovChainNode, "scenario").trim() + "' " : " ") + "of detector '" + Detector->getName() + "' of SADF graph '" + GraphName + "'.");

            for (CNodePtr TransitionNode2 = CNextNode(TransitionNode, "transition"); TransitionNode2 != NULL; TransitionNode2 = CNextNode(TransitionNode2, "transition"))
                if (CHasAttribute(TransitionNode2, "destination"))
                    if (CGetAttribute(TransitionNode, "destination").trim() == CGetAttribute(TransitionNode2, "destination").trim())
                        throw CException((CString)("Error: Multiple transitions to destination '") + CGetAttribute(TransitionNode, "destination").trim()
                                         + "' specified from state '" + CGetAttribute(StateNode, "name").trim() + "' of Markov chain"
                                         + ((Detector->hasControlInputChannels()) ? " for scenario '" + CGetAttribute(MarkovChainNode, "scenario").trim() + "' " : " ")
                                         + "of detector '" + Detector->getName() + "' of SADF graph '" + GraphName + "'.");

            CDouble Probability = 1;

            if (CHasAttribute(TransitionNode, "weight"))
                Probability = CGetAttribute(TransitionNode, "weight");

            MarkovChain->setTransitionProbability(SourceID, MarkovChain->getStateID(CGetAttribute(TransitionNode, "destination").trim()), Probability);

            Total = Total + Probability;
        }

        for (CId i = 0; i != MarkovChain->getNumberOfStates(); i++)
            MarkovChain->setTransitionProbability(SourceID, i, (MarkovChain->getTransitionProbability(SourceID, i) / Total));
    }

    if (!MarkovChain->isSingleComponent())
        throw CException((CString)("Error: Markov chain") + ((Detector->hasControlInputChannels()) ? " for scenario '" + CGetAttribute(MarkovChainNode, "scenario").trim()
                         + "' " : " ") + "of detector '" + Detector->getName() + "' of SADF graph '" + GraphName + "' does not consist of a single component.");
}

// Construct SADF Kernel from XML

void SADF_ConstructKernel(CString &GraphName, SADF_Process *Kernel, const CNodePtr KernelPropertyNode)
{

    // Construct scenarios

    if (!CHasChildNode(KernelPropertyNode, "scenario"))
        throw CException((CString)("Error: No scenario(s) speficied for kernel '") + Kernel->getName() + "' of SADF graph '" + GraphName + "'.");

    if (!Kernel->hasControlInputChannels() && (CGetNumberOfChildNodes(KernelPropertyNode, "scenario") > 1))
        throw CException((CString)("Error: Multiple scenarios defined for kernel '") + Kernel->getName() + "' without control inputs for SADF graph '" + GraphName + "'.");

    for (CNodePtr ScenarioNode = CGetChildNode(KernelPropertyNode, "scenario"); ScenarioNode != NULL; ScenarioNode = CNextNode(ScenarioNode, "scenario"))
    {

        if (!CHasAttribute(ScenarioNode, "name"))
            throw CException((CString)("Error: No name specified for scenario of ") + "kernel '" + Kernel->getName() + "' of SADF graph '" + GraphName + "'.");

        for (CNodePtr ScenarioNode2 = CNextNode(ScenarioNode, "scenario"); ScenarioNode2 != NULL; ScenarioNode2 = CNextNode(ScenarioNode2, "scenario"))
            if (CHasAttribute(ScenarioNode2, "name"))
                if (CGetAttribute(ScenarioNode, "name").trim() == CGetAttribute(ScenarioNode2, "name").trim())
                    throw CException((CString)("Error: Multiple scenarios '") + CGetAttribute(ScenarioNode, "name").trim() + "' specified for kernel '"
                                     + Kernel->getName() + "' of SADF graph '" + GraphName + "'.");

        CId ScenarioID = Kernel->addScenario(CGetAttribute(ScenarioNode, "name").trim());

        SADF_ConstructScenario(GraphName, Kernel, ScenarioID, ScenarioNode);

        if (!Kernel->checkActivity(ScenarioID))
            Kernel->makeInactive(ScenarioID);

        // Construct scenario definition in case of multiple control channels

        if (Kernel->hasMultipleControlInputChannels())
        {

            if (!CHasChildNode(ScenarioNode, "control"))
                throw CException((CString)("Error: Kernel '") + Kernel->getName() + "' requires specification of controls that define scenario '"
                                 + CGetAttribute(ScenarioNode, "name").trim() + "' for SADF graph '" + GraphName + "'.");

            for (CNodePtr ControlNode = CGetChildNode(ScenarioNode, "control"); ControlNode != NULL; ControlNode = CNextNode(ControlNode, "control"))
            {

                if (!CHasAttribute(ControlNode, "channel"))
                    throw CException((CString)("Error: No channel specified for control that defines scenario '") + CGetAttribute(ScenarioNode, "name").trim()
                                     + "' for kernel '" + Kernel->getName() + "' of SADF graph '" + GraphName + "'.");

                if (Kernel->getControlInputChannelID(CGetAttribute(ControlNode, "channel").trim()) == SADF_UNDEFINED)
                    throw CException((CString)("Error: Channel '") + CGetAttribute(ControlNode, "channel").trim() + "' for which control is specified in scenario '"
                                     + CGetAttribute(ScenarioNode, "name").trim() + "' is not a control channel to kernel '" + Kernel->getName() + "' of SADF graph '" + GraphName + "'.");

                for (CNodePtr ControlNode2 = CNextNode(ControlNode, "control"); ControlNode2 != NULL; ControlNode2 = CNextNode(ControlNode2, "control"))
                    if (CHasAttribute(ControlNode2, "channel"))
                        if (CGetAttribute(ControlNode, "channel").trim() == CGetAttribute(ControlNode2, "channel").trim())
                            throw CException((CString)("Error: Multiple controls specified for channel '") + CGetAttribute(ControlNode, "channel").trim()
                                             + "' that define scneario '" + CGetAttribute(ScenarioNode, "name").trim() + "' for kernel '" + Kernel->getName() + "' of SADF graph '"
                                             + GraphName + "'.");

                if (!CHasAttribute(ControlNode, "value"))
                    throw CException((CString)("Error: No value specified for control from channel '") + CGetAttribute(ControlNode, "channel").trim() + "' in scenario '"
                                     + CGetAttribute(ScenarioNode, "name").trim() + "' for kernel '" + Kernel->getName() + "' of SADF graph '" + GraphName + "'.");

                Kernel->setControlValue(ScenarioID, Kernel->getControlInputChannelID(CGetAttribute(ControlNode, "channel").trim()), CGetAttribute(ControlNode, "value").trim());
            }

            // Check existance of control definition for all control input channels

            for (CId i = 0; i != Kernel->getControlInputChannels().size(); i++)
                if (Kernel->getControlValue(ScenarioID, i).empty())
                    throw CException((CString)("Error: No control value specified for control channel '") + Kernel->getControlInputChannels()[i]->getName() + "' to kernel '"
                                     + Kernel->getName() + "' of SADF graph '" + GraphName + "' for scenario '" + CGetAttribute(ScenarioNode, "name").trim() + "'.");
        }
    }

    // Validate existance of postive Production/Consumption Rates

    for (CId i = 0; i != Kernel->getDataInputChannels().size(); i++)
    {

        bool ConsumptionFound = false;

        for (CId j = 0; !ConsumptionFound && j != Kernel->getNumberOfScenarios(); j++)
            if (Kernel->getDataInputChannels()[i]->getConsumptionRate(j) != 0)
                ConsumptionFound = true;

        if (!ConsumptionFound)
            throw CException((CString)("Error: No consumption from channel '") + Kernel->getDataInputChannels()[i]->getName() + "' in any scenario of kernel '"
                             + Kernel->getName() + "' of SADF graph '" + GraphName + "'.");
    }

    for (CId i = 0; i != Kernel->getDataOutputChannels().size(); i++)
    {

        bool ProductionFound = false;

        for (CId j = 0; !ProductionFound && j != Kernel->getNumberOfScenarios(); j++)
            if (Kernel->getDataOutputChannels()[i]->getProductionRate(j) != 0)
                ProductionFound = true;

        if (!ProductionFound)
            throw CException((CString)("Error: No production to channel '") + Kernel->getDataOutputChannels()[i]->getName() + "' in any scenario of kernel '"
                             + Kernel->getName() + "' of SADF graph '" + GraphName + "'.");
    }

    // Fix ControlIDs

    if (Kernel->hasMultipleControlInputChannels())
        for (CId i = 0; i != Kernel->getControlInputChannels().size(); i++)
        {

            CId ControlID = 0;

            for (CId j = 0; j != Kernel->getNumberOfScenarios(); j++)
                if (Kernel->getControlID(j, i) == SADF_UNDEFINED)
                {

                    Kernel->setControlID(j, i, ControlID);

                    for (CId k = j + 1; k != Kernel->getNumberOfScenarios(); k++)
                        if (Kernel->getControlValue(j, i) == Kernel->getControlValue(k, i))
                            Kernel->setControlID(k, i, ControlID);

                    ControlID++;
                }
        }
}

void SADF_ConstructDetector(CString &GraphName, SADF_Process *Detector, const CNodePtr DetectorPropertyNode)
{

    // Construct subscenarios

    if (!CHasChildNode(DetectorPropertyNode, "subscenario"))
        throw CException((CString)("Error: No subscenario(s) speficied for ") + "detector '" + Detector->getName() + "' of SADF graph '" + GraphName + "'.");

    for (CNodePtr SubScenarioNode = CGetChildNode(DetectorPropertyNode, "subscenario"); SubScenarioNode != NULL; SubScenarioNode = CNextNode(SubScenarioNode, "subscenario"))
    {

        if (!CHasAttribute(SubScenarioNode, "name"))
            throw CException((CString)("Error: No name specified for subscenario of detector'") + Detector->getName() + "' of SADF graph '" + GraphName + "'.");

        for (CNodePtr SubScenarioNode2 = CNextNode(SubScenarioNode, "subscenario"); SubScenarioNode2 != NULL; SubScenarioNode2 = CNextNode(SubScenarioNode2, "subscenario"))
            if (CHasAttribute(SubScenarioNode2, "name"))
                if (CGetAttribute(SubScenarioNode, "name").trim() == CGetAttribute(SubScenarioNode2, "name").trim())
                    throw CException((CString)("Error: Multiple subscenarios '") + CGetAttribute(SubScenarioNode, "name").trim() + "' specified for detector '"
                                     + Detector->getName() + "' of SADF graph '" + GraphName + "'.");

        CId SubScenarioID = Detector->addSubScenario(CGetAttribute(SubScenarioNode, "name").trim());

        SADF_ConstructScenario(GraphName, Detector, SubScenarioID, SubScenarioNode);

        // Check productions to all control channels

        for (CId i = 0; i != Detector->getControlOutputChannels().size(); i++)
            if (Detector->getControlOutputChannels()[i]->getProductionRate(SubScenarioID) == 0)
                throw CException((CString)("Error: No production specified to output channel '") + Detector->getControlOutputChannels()[i]->getName() + "' of detector '"
                                 + Detector->getName() + "' of SADF graph '" + GraphName + "' for subscenario '" + CGetAttribute(SubScenarioNode, "name").trim() + "'.");
    }

    // Validate existance of postive Production/Consumption Rates

    for (CId i = 0; i != Detector->getDataInputChannels().size(); i++)
    {

        bool ConsumptionFound = false;

        for (CId j = 0; !ConsumptionFound && j != Detector->getNumberOfSubScenarios(); j++)
            if (Detector->getDataInputChannels()[i]->getConsumptionRate(j) != 0)
                ConsumptionFound = true;

        if (!ConsumptionFound)
            throw CException((CString)("Error: No consumption from channel '") + Detector->getDataInputChannels()[i]->getName() + "' in any subscenario of detector '"
                             + Detector->getName() + "' of SADF graph '" + GraphName + "'.");
    }

    for (CId i = 0; i != Detector->getDataOutputChannels().size(); i++)
    {

        bool ProductionFound = false;

        for (CId j = 0; !ProductionFound && j != Detector->getNumberOfSubScenarios(); j++)
            if (Detector->getDataOutputChannels()[i]->getProductionRate(j) != 0)
                ProductionFound = true;

        if (!ProductionFound)
            throw CException((CString)("Error: No production to channel '") + Detector->getDataOutputChannels()[i]->getName() + "' in any subscenario of detector '"
                             + Detector->getName() + "' of SADF graph '" + GraphName + "'.");
    }

    // Construct scenario definitions

    if (!CHasChildNode(DetectorPropertyNode, "markov_chain"))
        throw CException((CString)("Error: No Markov chain(s) specified for detector '") + Detector->getName() + "' of SADF graph '" + GraphName + "'.");

    if (!Detector->hasControlInputChannels())
    {

        Detector->addScenario("default");

        if (CGetNumberOfChildNodes(DetectorPropertyNode, "markov_chain") != 1)
            throw CException((CString)("Error: Multiple Markov chains specified for uncontrolled detector '") + Detector->getName() + "' of SADF graph '" + GraphName + "'.");
    }

    if (Detector->hasControlInputChannels())
    {
        if (!Detector->hasMultipleControlInputChannels())
        {

            for (CNodePtr MarkovChainNode = CGetChildNode(DetectorPropertyNode, "markov_chain"); MarkovChainNode != NULL;
                 MarkovChainNode = CNextNode(MarkovChainNode, "markov_chain"))
            {

                if (!CHasAttribute(MarkovChainNode, "scenario"))
                    throw CException((CString)("Error: No scenario specified for Markov chain of detector '") + Detector->getName() + "' of SADF graph '" + GraphName + "'.");

                for (CNodePtr MarkovChainNode2 = CNextNode(MarkovChainNode, "markov_chain"); MarkovChainNode2 != NULL;
                     MarkovChainNode2 = CNextNode(MarkovChainNode2, "markov_chain"))
                    if (CHasAttribute(MarkovChainNode2, "scenario"))
                        if (CGetAttribute(MarkovChainNode, "scenario").trim() == CGetAttribute(MarkovChainNode2, "scenario").trim())
                            throw CException((CString)("Error: Multiple Markov chains specified for scenario '") + CGetAttribute(MarkovChainNode, "scenario").trim()
                                             + "' for detector '" + Detector->getName() + "' of SADF graph '" + GraphName + "'.");

                Detector->addScenario(CGetAttribute(MarkovChainNode, "scenario"));
            }

        }
        else
        {

            if (!CHasChildNode(DetectorPropertyNode, "scenario"))
                throw CException((CString)("Error: Detector '") + Detector->getName() + "' for SADF graph '" + GraphName
                                 + "' requires specification of scenarios for combinations of input values on control channels.");

            for (CNodePtr ScenarioNode = CGetChildNode(DetectorPropertyNode, "scenario"); ScenarioNode != NULL; ScenarioNode = CNextNode(ScenarioNode, "scenario"))
            {

                if (!CHasAttribute(ScenarioNode, "name"))
                    throw CException((CString)("Error: No name specified for scenario of detector '") + Detector->getName() + "' of SADF graph '" + GraphName + "'.");

                for (CNodePtr ScenarioNode2 = CNextNode(ScenarioNode, "scenario"); ScenarioNode2 != NULL; ScenarioNode2 = CNextNode(ScenarioNode2, "scenario"))
                    if (CHasAttribute(ScenarioNode2, "name"))
                        if (CGetAttribute(ScenarioNode, "name").trim() == CGetAttribute(ScenarioNode2, "name").trim())
                            throw CException((CString)("Error: Multiple scenarios '") + CGetAttribute(ScenarioNode, "name").trim() + "' specified for detector '"
                                             + Detector->getName() + "' of SADF graph '" + GraphName + "'.");

                CId ScenarioID = Detector->addScenario(CGetAttribute(ScenarioNode, "name").trim());

                if (!CHasChildNode(ScenarioNode, "control"))
                    throw CException((CString)("Error: Detector '") + Detector->getName() + "' requires specification of controls that define scenario '"
                                     + CGetAttribute(ScenarioNode, "name") + "' for SADF graph '" + GraphName + "'.");

                for (CNodePtr ControlNode = CGetChildNode(ScenarioNode, "control"); ControlNode != NULL; ControlNode = CNextNode(ControlNode, "control"))
                {

                    if (!CHasAttribute(ControlNode, "channel"))
                        throw CException((CString)("Error: No channel specified for control that defines scenario '") + CGetAttribute(ScenarioNode, "name").trim()
                                         + "' for detector '" + Detector->getName() + "' of SADF graph '" + GraphName + "'.");

                    if (Detector->getControlInputChannelID(CGetAttribute(ControlNode, "channel").trim()) == SADF_UNDEFINED)
                        throw CException((CString)("Error: Channel '") + CGetAttribute(ControlNode, "channel").trim() + "' for which control is specified in scenario '"
                                         + CGetAttribute(ScenarioNode, "name").trim() + "' is not a control channel to detector '" + Detector->getName() + "' of SADF graph '"
                                         + GraphName + "'.");

                    for (CNodePtr ControlNode2 = CNextNode(ControlNode, "control"); ControlNode2 != NULL; ControlNode2 = CNextNode(ControlNode2, "control"))
                        if (CHasAttribute(ControlNode2, "channel"))
                            if (CGetAttribute(ControlNode, "channel").trim() == CGetAttribute(ControlNode2, "channel").trim())
                                throw CException((CString)("Error: Multiple controls specified for channel '") + CGetAttribute(ControlNode, "channel").trim()
                                                 + "' that define scneario '" + CGetAttribute(ScenarioNode, "name").trim() + "' for detector '" + Detector->getName()
                                                 + "' of SADF graph '" + GraphName + "'.");

                    if (!CHasAttribute(ControlNode, "value"))
                        throw CException((CString)("Error: No value specified for control from channel '") + CGetAttribute(ControlNode, "channel").trim() + "' in scenario '"
                                         + CGetAttribute(ScenarioNode, "name").trim() + "' for detector '" + Detector->getName() + "' of SADF graph '" + GraphName + "'.");

                    Detector->setControlValue(ScenarioID, Detector->getControlInputChannelID(CGetAttribute(ControlNode, "channel").trim()),
                                              CGetAttribute(ControlNode, "value").trim());
                }

                // Check existance of control definition for all control input channels

                for (CId i = 0; i != Detector->getControlInputChannels().size(); i++)
                    if (Detector->getControlValue(ScenarioID, i).empty())
                        throw CException((CString)("Error: No control value specified for control channel '") + Detector->getControlInputChannels()[i]->getName()
                                         + "' to detector '" + Detector->getName() + "' of SADF graph '" + GraphName + "' for scenario '" + CGetAttribute(ScenarioNode, "name").trim()
                                         + "'.");
            }

            // Fix ControlIDs

            for (CId i = 0; i != Detector->getControlInputChannels().size(); i++)
            {

                CId ControlID = 0;

                for (CId j = 0; j != Detector->getNumberOfScenarios(); j++)
                    if (Detector->getControlID(j, i) == SADF_UNDEFINED)
                    {

                        Detector->setControlID(j, i, ControlID);

                        for (CId k = j + 1; k != Detector->getNumberOfScenarios(); k++)
                            if (Detector->getControlValue(j, i) == Detector->getControlValue(k, i))
                                Detector->setControlID(k, i, ControlID);

                        ControlID++;
                    }
            }
        }
    }

    // Construct Markov chain(s)

    for (CNodePtr MarkovChainNode = CGetChildNode(DetectorPropertyNode, "markov_chain"); MarkovChainNode != NULL;
         MarkovChainNode = CNextNode(MarkovChainNode, "markov_chain"))
    {

        if (!CHasAttribute(MarkovChainNode, "initial_state"))
            throw CException((CString)("Error: No initial state specified for Markov chain")
                             + ((Detector->hasControlInputChannels()) ? " for scenario '" + CGetAttribute(MarkovChainNode, "scenario").trim() + "' " : " ")
                             + "of detector '" + Detector->getName() + "' of SADF graph '" + GraphName + "'.");

        if (Detector->hasMultipleControlInputChannels())
        {

            if (!CHasAttribute(MarkovChainNode, "scenario"))
                throw CException((CString)("Error: No scenario specified for Markov chain of detector '") + Detector->getName() + "' of SADF graph '" + GraphName + "'.");

            for (CNodePtr MarkovChainNode2 = CNextNode(MarkovChainNode, "markov_chain"); MarkovChainNode2 != NULL;
                 MarkovChainNode2 = CNextNode(MarkovChainNode2, "markov_chain"))
                if (CHasAttribute(MarkovChainNode2, "scenario"))
                    if (CGetAttribute(MarkovChainNode, "scenario").trim() == CGetAttribute(MarkovChainNode2, "scenario").trim())
                        throw CException((CString)("Error: Multiple Markov chains specified for scenario '") + CGetAttribute(MarkovChainNode, "scenario").trim()
                                         + "' for detector '" + Detector->getName() + "' of SADF graph '" + GraphName + "'.");
        }

        CId ScenarioID = 0;

        if (Detector->hasControlInputChannels())
        {

            ScenarioID = Detector->getScenarioID(CGetAttribute(MarkovChainNode, "scenario"));

            if (ScenarioID == SADF_UNDEFINED)
                throw CException((CString)("Error: Markov chain for unknown scenario '") + CGetAttribute(MarkovChainNode, "scenario") + "' specified for detector '"
                                 + Detector->getName() + "' of SADF graph '" + GraphName + "'.");
        }

        SADF_MarkovChain *MarkovChain = new SADF_MarkovChain();

        SADF_ConstructMarkovChain(GraphName, Detector, MarkovChain, MarkovChainNode);

        Detector->setMarkovChain(ScenarioID, MarkovChain);
    }

    for (CId i = 0; i != Detector->getNumberOfScenarios(); i++)
        if (Detector->getMarkovChain(i) == NULL)
            throw CException((CString)("Error: No Markov chain specified for '") + Detector->getScenarioName(i) + "' of detector '" + Detector->getName()
                             + "' of SADF graph '" + GraphName + "'.");
}

// Construct SADF graph from XML

SADF_Graph *SADF_ConstructGraph(const CNodePtr ApplicationNode, const CId ApplicationNumber)
{

    if (!CHasAttribute(ApplicationNode, "name"))
        throw CException("Error: No name specified for SADF graph.");

    SADF_Graph *Graph = new SADF_Graph(CGetAttribute(ApplicationNode, "name").trim(), ApplicationNumber);

    // Create graph structure

    if (!CHasChildNode(ApplicationNode, "structure"))
        throw CException((CString)("Error: No structure speficied for SADF graph ") + Graph->getName() + ".");

    CNodePtr Structure = CGetChildNode(ApplicationNode, "structure");

    // Create kernels

    for (CNodePtr KernelNode = CGetChildNode(Structure, "kernel"); KernelNode != NULL; KernelNode = CNextNode(KernelNode, "kernel"))
    {

        if (!CHasAttribute(KernelNode, "name"))
            throw CException((CString)("Error: No name specified for kernel of SADF graph '") + Graph->getName() + "'.");

        for (CNodePtr KernelNode2 = CNextNode(KernelNode, "kernel"); KernelNode2 != NULL; KernelNode2 = CNextNode(KernelNode2, "kernel"))
            if (CHasAttribute(KernelNode2, "name"))
                if (CGetAttribute(KernelNode, "name").trim() == CGetAttribute(KernelNode2, "name").trim())
                    throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' contains multiple kernels named '"
                                     + CGetAttribute(KernelNode, "name").trim() + "'.");

        Graph->addKernel(new SADF_Process(CGetAttribute(KernelNode, "name").trim(), Graph->getNumberOfKernels(), SADF_KERNEL));
    }

    // Create detectors

    for (CNodePtr DetectorNode = CGetChildNode(Structure, "detector"); DetectorNode != NULL; DetectorNode = CNextNode(DetectorNode, "detector"))
    {

        if (!CHasAttribute(DetectorNode, "name"))
            throw CException((CString)("Error: No name specified for detector of SADF graph '") + Graph->getName() + "'.");

        for (CNodePtr DetectorNode2 = CNextNode(DetectorNode, "detector"); DetectorNode2 != NULL; DetectorNode2 = CNextNode(DetectorNode2, "detector"))
            if (CHasAttribute(DetectorNode2, "name"))
                if (CGetAttribute(DetectorNode, "name").trim() == CGetAttribute(DetectorNode2, "name").trim())
                    throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' contains multiple detectors named '"
                                     + CGetAttribute(DetectorNode, "name").trim() + "'.");

        Graph->addDetector(new SADF_Process(CGetAttribute(DetectorNode, "name").trim(), Graph->getNumberOfDetectors(), SADF_DETECTOR));
    }

    // Validate naming uniqueness of kernels versus detectors

    for (CId i = 0; i != Graph->getNumberOfKernels(); i++)
        for (CId j = 0; j != Graph->getNumberOfDetectors(); j++)
            if (Graph->getKernel(i)->getName() == Graph->getDetector(j)->getName())
                throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' contains kernel and detector with the same name '"
                                 + Graph->getKernel(i)->getName() + "'.");

    // Create Channels

    for (CNodePtr ChannelNode = CGetChildNode(Structure, "channel"); ChannelNode != NULL; ChannelNode = CNextNode(ChannelNode, "channel"))
    {

        if (!CHasAttribute(ChannelNode, "name"))
            throw CException((CString)("Error: No name specified for channel of SADF graph '") + Graph->getName() + "'.");

        if (!CHasAttribute(ChannelNode, "source"))
            throw CException((CString)("Error: No source specified for channel '") + CGetAttribute(ChannelNode, "name").trim() + "' of SADF graph '" + Graph->getName() + "'.");

        if (!CHasAttribute(ChannelNode, "destination"))
            throw CException((CString)("Error: No destination specified for channel '") + CGetAttribute(ChannelNode, "name").trim() + "' of SADF graph '"
                             + Graph->getName() + "'.");

        if (!CHasAttribute(ChannelNode, "type"))
            throw CException((CString)("Error: No type specified for channel '") + CGetAttribute(ChannelNode, "name").trim() + "' of SADF graph '" + Graph->getName() + "'.");

        for (CNodePtr ChannelNode2 = CNextNode(ChannelNode, "channel"); ChannelNode2 != NULL; ChannelNode2 = CNextNode(ChannelNode2, "channel"))
            if (CHasAttribute(ChannelNode2, "name"))
                if (CGetAttribute(ChannelNode, "name").trim() == CGetAttribute(ChannelNode2, "name").trim())
                    throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' contains multiple channels named '"
                                     + CGetAttribute(ChannelNode, "name").trim() + "'.");

        SADF_Channel *Channel = NULL;

        if (CGetAttribute(ChannelNode, "type").trim() == "data")
            Channel = new SADF_Channel(CGetAttribute(ChannelNode, "name").trim(), Graph->getNumberOfDataChannels(), SADF_DATA_CHANNEL);
        else if (CGetAttribute(ChannelNode, "type").trim() == "control")
            Channel = new SADF_Channel(CGetAttribute(ChannelNode, "name").trim(), Graph->getNumberOfControlChannels(), SADF_CONTROL_CHANNEL);
        else
            throw CException((CString)("Error: Invalid type specified for channel '") + CGetAttribute(ChannelNode, "name").trim() + "' of SADF graph '"
                             + Graph->getName() + "'.");

        if (Channel->getType() == SADF_DATA_CHANNEL)
            Graph->addDataChannel(Channel);
        else
            Graph->addControlChannel(Channel);

        // Validate connectivity

        SADF_Process *Kernel = Graph->getKernel(CGetAttribute(ChannelNode, "source").trim());
        SADF_Process *Detector = Graph->getDetector(CGetAttribute(ChannelNode, "source").trim());

        if (Kernel != NULL)
        {

            if (Channel->getType() == SADF_CONTROL_CHANNEL)
                throw CException((CString)("Error: Source of control channel '") + Channel->getName() + "' is kernel '" + Kernel->getName()
                                 + "' instead of a detector in SADF graph '" + Graph->getName() + "'.");

            Channel->setSource(Kernel);
            Kernel->addDataOutputChannel(Channel);

        }
        else
        {

            if (Detector != NULL)
            {

                Channel->setSource(Detector);

                if (Channel->getType() == SADF_DATA_CHANNEL)
                    Detector->addDataOutputChannel(Channel);
                else
                    Detector->addControlOutputChannel(Channel);

            }
            else
                throw CException((CString)("Error: No matching kernel or detector for source '") + CGetAttribute(ChannelNode, "source").trim() + "' of channel '"
                                 + Channel->getName() + "' of SADF graph '" + Graph->getName() + "'.");
        }

        Kernel = Graph->getKernel(CGetAttribute(ChannelNode, "destination").trim());
        Detector = Graph->getDetector(CGetAttribute(ChannelNode, "destination").trim());

        if (Kernel != NULL)
        {

            Channel->setDestination(Kernel);

            if (Channel->getType() == SADF_DATA_CHANNEL)
                Kernel->addDataInputChannel(Channel);
            else
                Kernel->addControlInputChannel(Channel);

        }
        else
        {

            if (Detector != NULL)
            {

                Channel->setDestination(Detector);

                if (Channel->getType() == SADF_DATA_CHANNEL)
                    Detector->addDataInputChannel(Channel);
                else
                    Detector->addControlInputChannel(Channel);

            }
            else
                throw CException((CString)("Error: No matching kernel or detector for destination '") + CGetAttribute(ChannelNode, "destination").trim() + "' of channel '"
                                 + Channel->getName() + "' of SADF graph '" + Graph->getName() + "'.");
        }
    }

    // Annotate processes with their properties

    if (!CHasChildNode(ApplicationNode, "properties"))
        throw CException((CString)("Error: No properties specified for SADF graph '") + Graph->getName() + "'.");

    CNodePtr Properties = CGetChildNode(ApplicationNode, "properties");

    for (CNodePtr KernelPropertyNode = CGetChildNode(Properties, "kernel_properties"); KernelPropertyNode != NULL;
         KernelPropertyNode = CNextNode(KernelPropertyNode, "kernel_properties"))
    {

        if (!CHasAttribute(KernelPropertyNode, "kernel"))
            throw CException((CString)("Error: No kernel name specified for kernel properties in SADF graph '") + Graph->getName() + "'.");

        SADF_Process *Kernel = Graph->getKernel(CGetAttribute(KernelPropertyNode, "kernel").trim());

        if (Kernel != NULL)
            SADF_ConstructKernel(Graph->getName(), Kernel, KernelPropertyNode);
        else
            throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' has no kernel named '" + CGetAttribute(KernelPropertyNode, "kernel").trim() + "'.");
    }

    for (CNodePtr DetectorPropertyNode = CGetChildNode(Properties, "detector_properties"); DetectorPropertyNode != NULL;
         DetectorPropertyNode = CNextNode(DetectorPropertyNode, "detector_properties"))
    {

        if (!CHasAttribute(DetectorPropertyNode, "detector"))
            throw CException((CString)("Error: No detector name specified for detector properties in SADF graph '") + Graph->getName() + "'.");

        SADF_Process *Detector = Graph->getDetector(CGetAttribute(DetectorPropertyNode, "detector").trim());

        if (Detector != NULL)
            SADF_ConstructDetector(Graph->getName(), Detector, DetectorPropertyNode);
        else
            throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' has no detector named '" + CGetAttribute(DetectorPropertyNode, "detector").trim() + "'.");
    }

    // Create inactive scenarios and fix scenario identities

    for (CId i = 0; i != Graph->getNumberOfKernels(); i++)
        if (Graph->getKernel(i)->hasControlInputChannels())
            SADF_FixScenarioIdentitiesSendToProcess(Graph->getName(), Graph->getKernel(i));

    for (CId i = 0; i != Graph->getNumberOfDetectors(); i++)
        if (Graph->getDetector(i)->hasControlInputChannels())
            SADF_FixScenarioIdentitiesSendToProcess(Graph->getName(), Graph->getDetector(i));

    // Annotate channels with their properties

    for (CNodePtr ChannelPropertyNode = CGetChildNode(Properties, "channel_properties"); ChannelPropertyNode != NULL;
         ChannelPropertyNode = CNextNode(ChannelPropertyNode, "channel_properties"))
    {

        if (!CHasAttribute(ChannelPropertyNode, "channel"))
            throw CException((CString)("Error: No channel name specified for channel properties in SADF graph '") + Graph->getName() + "'.");

        SADF_Channel *Channel = Graph->getChannel(CGetAttribute(ChannelPropertyNode, "channel").trim());

        if (Channel != NULL)
            SADF_ConstructChannel(Graph->getName(), Channel, ChannelPropertyNode);
        else
            throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' has no channel named '" + CGetAttribute(ChannelPropertyNode, "channel").trim() + "'.");
    }

    // Return the constructed SADF graph

    return Graph;
}
