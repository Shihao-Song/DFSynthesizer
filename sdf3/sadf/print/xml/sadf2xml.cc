/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf2xml.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   13 September 2006
 *
 *  Function        :   Output SADF Graph in xml Format
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

#include "sadf2xml.h"

void SADF2XML(SADF_Graph *Graph, ostream &out)
{

    // Write preamble

    out << "<?xml version='1.0' encoding='UTF-8'?>" << endl;
    out << "<sdf3 type='sadf' version='1.0' xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' xmlns='uri:sadf' xsi:schemaLocation='uri:sadf http://www.es.ele.tue.nl/sadf/sdf3-sadf.xsd'>" << endl << endl;
    out << "<sadf name='" << Graph->getName() << "'>" << endl << endl;

    // Write Structure

    out << "<structure>" << endl;

    for (CId i = 0; i != Graph->getNumberOfKernels(); i++)
        out << "  <kernel name='" << Graph->getKernel(i)->getName() << "'/>" << endl;

    for (CId i = 0; i != Graph->getNumberOfDetectors(); i++)
        out << "  <detector name='" << Graph->getDetector(i)->getName() << "'/>" << endl;

    for (CId i = 0; i != Graph->getNumberOfDataChannels(); i++)
        out << "  <channel name='" << Graph->getDataChannel(i)->getName() << "' type='data' source='" << Graph->getDataChannel(i)->getSource()->getName()
            << "' destination='" << Graph->getDataChannel(i)->getDestination()->getName() << "'/>" << endl;

    for (CId i = 0; i != Graph->getNumberOfControlChannels(); i++)
        out << "  <channel name='" << Graph->getControlChannel(i)->getName() << "' type='control' source='" << Graph->getControlChannel(i)->getSource()->getName()
            << "' destination='" << Graph->getControlChannel(i)->getDestination()->getName() << "'/>" << endl;

    out << "</structure>" << endl << endl;

    // Write Properties

    out << "<properties>" << endl;

    for (CId i = 0; i != Graph->getNumberOfKernels(); i++)
    {

        SADF_Process *Kernel = Graph->getKernel(i);

        out << "  <kernel_properties kernel='" << Kernel->getName() << "'>" << endl;

        for (CId j = 0; j != Kernel->getNumberOfScenarios(); j++)
            if (Kernel->isActive(j) || Kernel->hasMultipleControlInputChannels() || Kernel->getNumberOfProfiles(j) > 1 ||
                (Kernel->getNumberOfProfiles(j) == 1 && Kernel->getProfile(j, 0)->getExecutionTime() > 0))
            {

                out << "    <scenario name='" << Kernel->getScenarioName(j) << "'>" << endl;

                if (Kernel->hasMultipleControlInputChannels())
                    for (CId k = 0; k != Kernel->getControlInputChannels().size(); k++)
                        out << "      <control channel='" << Kernel->getControlInputChannels()[k]->getName() << "' value='" << Kernel->getControlValue(j, k) << "'/>" << endl;

                for (CId k = 0; k != Kernel->getDataInputChannels().size(); k++)
                    if (Kernel->getDataInputChannels()[k]->getConsumptionRate(j) != 0)
                        out << "      <consume channel='" << Kernel->getDataInputChannels()[k]->getName() << "' tokens='"
                            << Kernel->getDataInputChannels()[k]->getConsumptionRate(j) << "'/>" << endl;

                for (CId k = 0; k != Kernel->getDataOutputChannels().size(); k++)
                    if (Kernel->getDataOutputChannels()[k]->getProductionRate(j) != 0)
                        out << "      <produce channel='" << Kernel->getDataOutputChannels()[k]->getName() << "' tokens='"
                            << Kernel->getDataOutputChannels()[k]->getProductionRate(j) << "'/>" << endl;

                for (CId k = 0; k != Kernel->getNumberOfProfiles(j); k++)
                    if (Kernel->getProfile(j, k)->getExecutionTime() != 0)
                    {

                        if (Kernel->getProfile(j, k)->getExecutionTime() != (CDouble)((unsigned long long)(Kernel->getProfile(j, k)->getExecutionTime())))
                            out << "      <profile execution_time='" << Kernel->getProfile(j, k)->getExecutionTime() << "'";
                        else
                            out << "      <profile execution_time='" << ((unsigned long long)(Kernel->getProfile(j, k)->getExecutionTime())) << "'";

                        if (Kernel->getProfile(j, k)->getWeight() != 1)
                            out << " weight='" << Kernel->getProfile(j, k)->getWeight() << "'";

                        out << "/>" << endl;
                    }

                out << "    </scenario>" << endl;
            }

        out << "  </kernel_properties>" << endl << endl;
    }

    for (CId i = 0; i != Graph->getNumberOfDetectors(); i++)
    {

        SADF_Process *Detector = Graph->getDetector(i);

        out << "  <detector_properties detector='" << Detector->getName() << "'>" << endl;

        if (Detector->hasMultipleControlInputChannels())
            for (CId j = 0; j != Detector->getNumberOfScenarios(); j++)
            {

                out << "    <scenario name='" << Detector->getScenarioName(j) << "'>" << endl;

                for (CId k = 0; k != Detector->getControlInputChannels().size(); k++)
                    out << "      <control channel='" << Detector->getControlInputChannels()[k]->getName() << "' value'" << Detector->getControlValue(j, k) << "'/>" << endl;

                out << "    </scenario>" << endl;
            }

        for (CId j = 0; j != Detector->getNumberOfSubScenarios(); j++)
        {

            out << "    <subscenario name='" << Detector->getSubScenarioName(j) << "'>" << endl;

            for (CId k = 0; k != Detector->getDataInputChannels().size(); k++)
                if (Detector->getDataInputChannels()[k]->getConsumptionRate(j) != 0)
                    out << "      <consume channel='" << Detector->getDataInputChannels()[k]->getName() << "' tokens='"
                        << Detector->getDataInputChannels()[k]->getConsumptionRate(j) << "'/>" << endl;

            for (CId k = 0; k != Detector->getDataOutputChannels().size(); k++)
                if (Detector->getDataOutputChannels()[k]->getProductionRate(j) != 0)
                    out << "      <produce channel='" << Detector->getDataOutputChannels()[k]->getName() << "' tokens='" <<
                        Detector->getDataOutputChannels()[k]->getProductionRate(j) << "'/>" << endl;

            for (CId k = 0; k != Detector->getControlOutputChannels().size(); k++)
            {

                out << "      <produce channel='" << Detector->getControlOutputChannels()[k]->getName() << "' tokens='" <<
                    Detector->getControlOutputChannels()[k]->getProductionRate(j) << "'";

                if (Detector->getControlOutputChannels()[k]->getProductionScenarioName(j) != Detector->getSubScenarioName(j))
                    out << " value='" << Detector->getControlOutputChannels()[k]->getProductionScenarioName(j) << "'";

                out << "/>" << endl;
            }

            for (CId k = 0; k != Detector->getNumberOfProfiles(j); k++)
                if (Detector->getProfile(j, k)->getExecutionTime() != 0)
                {

                    if (Detector->getProfile(j, k)->getExecutionTime() != (CDouble)((unsigned long long)(Detector->getProfile(j, k)->getExecutionTime())))
                        out << "      <profile execution_time='" << Detector->getProfile(j, k)->getExecutionTime() << "'";
                    else
                        out << "      <profile execution_time='" << ((unsigned long long)(Detector->getProfile(j, k)->getExecutionTime())) << "'";

                    if (Detector->getProfile(j, k)->getWeight() != 1)
                        out << " weight='" << Detector->getProfile(j, k)->getWeight() << "'";

                    out << "/>" << endl;
                }

            out << "    </subscenario>" << endl;
        }

        for (CId j = 0; j != Detector->getNumberOfScenarios(); j++)
        {

            out << "    <markov_chain ";

            if (Detector->hasControlInputChannels())
                out << "scenario='" << Detector->getScenarioName(j) << "' ";

            out << "initial_state='" << Detector->getMarkovChain(j)->getStateName(Detector->getMarkovChain(j)->getInitialState()) << "'>" << endl;

            for (CId k = 0; k != Detector->getMarkovChain(j)->getNumberOfStates(); k++)
            {

                out << "      <state name='" << Detector->getMarkovChain(j)->getStateName(k) << "' subscenario='"
                    << Detector->getSubScenarioName(Detector->getMarkovChain(j)->getSubScenario(k)) << "'>" << endl;

                for (CId n = 0; n != Detector->getMarkovChain(j)->getNumberOfStates(); n++)
                    if (Detector->getMarkovChain(j)->getTransitionProbability(k, n) != 0)
                        out << "        <transition destination='" << Detector->getMarkovChain(j)->getStateName(n) << "' weight='"
                            << Detector->getMarkovChain(j)->getTransitionProbability(k, n) << "'/>" << endl;

                out << "      </state>" << endl;
            }

            out << "    </markov_chain>" << endl;
        }

        out << "  </detector_properties>" << endl << endl;
    }

    for (CId i = 0; i != Graph->getNumberOfDataChannels(); i++)
        if (Graph->getDataChannel(i)->getNumberOfInitialTokens() != 0 || Graph->getDataChannel(i)->getBufferSize() != SADF_UNBOUNDED
            || Graph->getDataChannel(i)->getTokenSize() != 1)
        {

            out << "  <channel_properties channel='" << Graph->getDataChannel(i)->getName() << "'";

            if (Graph->getDataChannel(i)->getNumberOfInitialTokens() != 0)
                out << " number_of_initial_tokens='" << Graph->getDataChannel(i)->getNumberOfInitialTokens() << "'";

            if (Graph->getDataChannel(i)->getBufferSize() != SADF_UNBOUNDED)
                out << " buffer_size='" << Graph->getDataChannel(i)->getBufferSize() << "'" << endl;

            if (Graph->getDataChannel(i)->getTokenSize() != 1)
                out << " token_size='" << Graph->getDataChannel(i)->getTokenSize() << "'" << endl;

            out << "/>" << endl;
        }

    for (CId i = 0; i != Graph->getNumberOfControlChannels(); i++)
        if (Graph->getControlChannel(i)->getNumberOfInitialTokens() != 0 || Graph->getControlChannel(i)->getBufferSize() != SADF_UNBOUNDED
            || Graph->getControlChannel(i)->getTokenSize() != 1)
        {

            SADF_Channel *Channel = Graph->getControlChannel(i);

            out << "  <channel_properties channel='" << Channel->getName() << "'";

            if (Channel->getBufferSize() != SADF_UNBOUNDED)
                out << " buffer_size='" << Channel->getBufferSize() << "'" << endl;

            if (Channel->getTokenSize() != 1)
                out << " token_size='" << Channel->getTokenSize() << "'" << endl;

            if (Channel->getNumberOfInitialTokens() != 0)
            {

                CQueue NumbersQueue(Channel->getNumbersQueue());
                CQueue ContentQueue(Channel->getContentQueue());

                out << " initial_tokens='";

                while (!NumbersQueue.empty())
                {

                    CId ID = ContentQueue.front();
                    CString TokenValue;

                    if (Channel->getDestination()->hasMultipleControlInputChannels())
                    {

                        bool ScenarioNotFound = true;

                        CId ChannelID = Channel->getDestination()->getControlInputChannelID(Channel->getName());

                        for (CId j = 0; ScenarioNotFound && j != Channel->getDestination()->getNumberOfScenarios(); j++)
                            if (Channel->getDestination()->getControlID(j, ChannelID) == ID)
                            {
                                ScenarioNotFound = false;
                                TokenValue = Channel->getDestination()->getControlValue(j, ChannelID);
                            }

                    }
                    else
                        TokenValue = Channel->getDestination()->getScenarioName(ID);

                    if (NumbersQueue.front() == 1)
                        out << TokenValue;
                    else
                        out << NumbersQueue.front() << "*" << TokenValue;

                    if (NumbersQueue.size() != 1)
                        out << ", ";

                    NumbersQueue.pop();
                    ContentQueue.pop();
                }

                out << "'";
            }

            out << "/>" << endl;
        }

    out << "</properties>" << endl << endl;

    // Write postamble

    out << "</sadf>" << endl;
    out << "</sdf3>" << endl;
}
