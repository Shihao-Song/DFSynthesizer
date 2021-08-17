/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf2dot.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   13 September 2006
 *
 *  Function        :   Output SADF Graph in DOT Format
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

#include "sadf2dot.h"

void SADF2DOT(SADF_Graph *Graph, ostream &out)
{

    CId i;

    out << "digraph " << Graph->getName() << " {" << endl;

    out << endl;

    // Output all kernels

    for (i = 0; i != Graph->getNumberOfKernels(); i++)
        out << "    K" << Graph->getKernel(i)->getIdentity() << " [ label=\"" << Graph->getKernel(i)->getName() << "\" ];" << endl;

    out << endl;

    // Output all detectors

    for (i = 0; i != Graph->getNumberOfDetectors(); i++)
        out << "    D" << Graph->getDetector(i)->getIdentity() << " [ label=\"" << Graph->getDetector(i)->getName() << "\", style=dotted ];" << endl;

    out << endl;

    // Output all data channels

    for (i = 0; i != Graph->getNumberOfDataChannels(); i++)
    {

        bool FixedProductionRate = true;
        bool FixedConsumptionRate = true;
        CId ProductionRate = SADF_UNDEFINED;
        CId ConsumptionRate = SADF_UNDEFINED;

        SADF_Channel *Channel = Graph->getDataChannel(i);
        SADF_Process *Process = Channel->getSource();

        if (Process->getType() == SADF_KERNEL)
            for (CId n = 0; FixedProductionRate && n != Process->getNumberOfScenarios(); n++)
                if (Channel->getProductionRate(n) != 0 && ProductionRate == SADF_UNDEFINED)
                    ProductionRate = Channel->getProductionRate(n);
                else if (Channel->getProductionRate(n) != ProductionRate)
                    FixedProductionRate = false;

        if (Process->getType() == SADF_DETECTOR)
            for (CId n = 0; FixedProductionRate && n != Process->getNumberOfSubScenarios(); n++)
                if (Channel->getProductionRate(n) != 0 && ProductionRate == SADF_UNDEFINED)
                    ProductionRate = Channel->getProductionRate(n);
                else if (Channel->getProductionRate(n) != ProductionRate)
                    FixedProductionRate = false;

        Process = Channel->getDestination();

        if (Process->getType() == SADF_KERNEL)
            for (CId n = 0; FixedConsumptionRate && n != Process->getNumberOfScenarios(); n++)
                if (Channel->getConsumptionRate(n) != 0 && ConsumptionRate == SADF_UNDEFINED)
                    ConsumptionRate = Channel->getConsumptionRate(n);
                else if (Channel->getConsumptionRate(n) != ConsumptionRate)
                    FixedConsumptionRate = false;

        if (Process->getType() == SADF_DETECTOR)
            for (CId n = 0; FixedConsumptionRate && n != Process->getNumberOfSubScenarios(); n++)
                if (Channel->getConsumptionRate(n) != 0 && ConsumptionRate == SADF_UNDEFINED)
                    ConsumptionRate = Channel->getConsumptionRate(n);
                else if (Channel->getConsumptionRate(n) != ConsumptionRate)
                    FixedConsumptionRate = false;

        if (Channel->getSource()->getType() == SADF_KERNEL)
            out << "    K";
        else
            out << "    D";
        out << Channel->getSource()->getIdentity() << " -> ";

        if (Channel->getNumberOfInitialTokens() != 0)
        {

#ifdef __DOT_EXPLICIT_INITIAL_TOKENS

            out << "DC" << Channel->getIdentity() << " [ label=\"" << Channel->getName() << "\"";

            if (FixedProductionRate)
                out << ", taillabel=\"" << ProductionRate << "\"";

            out << ", arrowhead=none ];" << endl;

            out << "    DC" << Channel->getIdentity() << " [ label=\"\", height=0.1, width=0.1, style=filled, color=black ];" << endl;

            out << "    DC" << Channel->getIdentity() << " -> ";

            if (Channel->getDestination()->getType() == SADF_KERNEL)
                out << "K";
            else
                out << "D";
            out << Channel->getDestination()->getIdentity();

            out << " [ taillabel=\"" << Channel->getNumberOfInitialTokens() << "\"";

            if (FixedConsumptionRate)
                out << ", headlabel=\"" << ConsumptionRate << "\"";

            out << " ];" << endl;

#else // __DOT_EXPLICIT_INITIAL_TOKENS

            if (Channel->getDestination()->getType() == SADF_KERNEL)
                out << "K";
            else
                out << "D";
            out << Channel->getDestination()->getIdentity() << " [ label=\"" << Channel->getName() << "(" << Channel->getNumberOfInitialTokens() << ")\"";

            if (FixedProductionRate)
                out << ", taillabel=\"" << ProductionRate << "\"";

            if (FixedConsumptionRate)
                out << ", headlabel=\"" << ConsumptionRate << "\"";

            out << " ];" << endl;

#endif // __DOT_EXPLICIT_INITIAL_TOKENS

        }
        else
        {
            if (Channel->getDestination()->getType() == SADF_KERNEL)
                out << "K";
            else
                out << "D";

            out << Channel->getDestination()->getIdentity() << " [ label=\"" << Channel->getName() << "\"";

            if (FixedProductionRate)
                out << ", taillabel=\"" << ProductionRate << "\"";

            if (FixedConsumptionRate)
                out << ", headlabel=\"" << ConsumptionRate << "\"";

            out << " ];" << endl;
        }
    }

    out << endl;

    // Output all control channels

    for (i = 0; i != Graph->getNumberOfControlChannels(); i++)
    {

        SADF_Channel *Channel = Graph->getControlChannel(i);
        SADF_Process *Detector = Channel->getSource();

        bool FixedRate = true;
        CId Rate = SADF_UNDEFINED;

        for (CId n = 0; FixedRate && n != Detector->getNumberOfSubScenarios(); n++)
            if (Channel->getProductionRate(n) != 0 && Rate == SADF_UNDEFINED)
                Rate = Channel->getProductionRate(n);
            else if (Channel->getProductionRate(n) != Rate)
                FixedRate = false;

        out << "    D" << Detector->getIdentity() << " -> ";

        if (Channel->getNumberOfInitialTokens() != 0)
        {

#ifdef __DOT_EXPLICIT_INITIAL_TOKENS

            out << "CC" << Channel->getIdentity() << " [ label=\"" << Channel->getName() << "\"";

            if (FixedRate)
                out << ", taillabel=\"" << Rate << "\"";

            out << ", style=dotted, arrowhead=none ];" << endl;

            out << "    CC" << Channel->getIdentity() << " [ label=\"\", height=0.1, width=0.1, style=filled, color=black ];" << endl;

            out << "    CC" << Channel->getIdentity() << " -> ";

            if (Channel->getDestination()->getType() == SADF_KERNEL)
                out << "K";
            else
                out << "D";

            out << Channel->getDestination()->getIdentity();

            out << " [ taillabel=\"";

            CQueue NumbersQueue(Channel->getNumbersQueue());
            CQueue ContentQueue(Channel->getContentQueue());

            while (!NumbersQueue.empty())
            {

                CId ID = ContentQueue.front();
                CString TokenValue;

                SADF_Process *Destination = Channel->getDestination();

                if (Destination->hasMultipleControlInputChannels())
                {

                    bool ScenarioNotFound = true;

                    CId ChannelID = Destination->getControlInputChannelID(Channel->getName());

                    for (CId j = 0; ScenarioNotFound && j != Destination->getNumberOfScenarios(); j++)
                        if (Destination->getControlID(j, ChannelID) == ID)
                        {
                            ScenarioNotFound = false;
                            TokenValue = Destination->getControlValue(j, ChannelID);
                        }

                }
                else
                    TokenValue = Destination->getScenarioName(ID);

                if (NumbersQueue.front() == 1)
                    out << TokenValue;
                else
                    out << NumbersQueue.front() << "*" << TokenValue;

                if (NumbersQueue.size() != 1)
                    out << ", ";

                NumbersQueue.pop();
                ContentQueue.pop();
            }

            out << "\", headlabel=\"1\", style=dotted ];" << endl;

#else // __DOT_EXPLICIT_INITIAL_TOKENS

            if (Channel->getDestination()->getType() == SADF_KERNEL)
                out << "K";
            else
                out << "D";

            out << Channel->getDestination()->getIdentity() << " [ label=\"" << Channel->getName() << "(";

            CQueue NumbersQueue(Channel->getNumbersQueue());
            CQueue ContentQueue(Channel->getContentQueue());

            while (!NumbersQueue.empty())
            {

                CId ID = ContentQueue.front();
                CString TokenValue;

                SADF_Process *Destination = Channel->getDestination();

                if (Destination->hasMultipleControlInputChannels())
                {

                    bool ScenarioNotFound = true;

                    CId ChannelID = Destination->getControlInputChannelID(Channel->getName());

                    for (CId j = 0; ScenarioNotFound && j != Destination->getNumberOfScenarios(); j++)
                        if (Destination->getControlID(j, ChannelID) == ID)
                        {
                            ScenarioNotFound = false;
                            TokenValue = Destination->getControlValue(j, ChannelID);
                        }

                }
                else
                    TokenValue = Destination->getScenarioName(ID);

                if (NumbersQueue.front() == 1)
                    out << TokenValue;
                else
                    out << NumbersQueue.front() << "*" << TokenValue;

                if (NumbersQueue.size() != 1)
                    out << ", ";

                NumbersQueue.pop();
                ContentQueue.pop();
            }

            out << ")\"";

            if (FixedRate)
                out << ", taillabel=\"" << Rate << "\"";

            out << ", headlabel=\"1\", style=dotted ];" << endl;

#endif // __DOT_EXPLICIT_INITIAL_TOKENS

        }
        else
        {
            if (Channel->getDestination()->getType() == SADF_KERNEL)
                out << "K";
            else
                out << "D";
            out << Channel->getDestination()->getIdentity() << " [ label=\"" << Channel->getName() << "\"";

            if (FixedRate)
                out << ", taillabel=\"" << Rate << "\"";

            out << ", headlabel=\"1\", style=dotted ];" << endl;
        }
    }

    out << "}" << endl;
}
