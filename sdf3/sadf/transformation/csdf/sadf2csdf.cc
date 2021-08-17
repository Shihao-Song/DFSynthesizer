/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf2csdf.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   13 September 2006
 *
 *  Function        :   Convert SADF Graph in CSDF Graph
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

#include "sadf2csdf.h"

TimedCSDFgraph *SADF2CSDF(SADF_Graph *SADF)
{

    // Disallow SADF graphs with kernels that have multiple controls for now

    for (CId i = 0; i != SADF->getNumberOfKernels(); i++)
        if (SADF->getKernel(i)->hasMultipleControlInputChannels())
            throw CException("Error: Conversion not yet supported for SADF graphs with kernels that have multiple control inputs.");

    // Disallow SADF graphs with detectors that have controls for now

    for (CId i = 0; i != SADF->getNumberOfDetectors(); i++)
        if (SADF->getDetector(i)->hasControlInputChannels())
            throw CException("Error: Conversion not yet supported for SADF graphs with detectors that have control inputs.");

    // Check whether conversion is possible

    if (!SADF_Verify_CSDF(SADF))
        throw CException((CString)("Error: SADF Graph '") + SADF->getName() + "' does not represent a CSDF Graph.");

    for (CId i = 0; i != SADF->getNumberOfKernels(); i++)
        for (CId j = 0; j != SADF->getKernel(i)->getNumberOfScenarios(); j++)
        {

            if (SADF->getKernel(i)->getProfile(j, 0)->getExecutionTime() !=
                (CDouble)((unsigned long long)(SADF->getKernel(i)->getProfile(j, 0)->getExecutionTime())))
                throw CException((CString)("Error: Real-valued execution time for Kernel '") + SADF->getKernel(i)->getName() + "' not surported.");

            if (SADF->getKernel(i)->getProfile(j, 0)->getExecutionTime() != (CDouble)((CSDFtime)(SADF->getKernel(i)->getProfile(j, 0)->getExecutionTime())))
                throw CException((CString)("Error: Execution time for Kernel '") + SADF->getKernel(i)->getName() + "' is too large to be supported.");
        }

    for (CId i = 0; i != SADF->getNumberOfDetectors(); i++)
        for (CId j = 0; j != SADF->getDetector(i)->getNumberOfSubScenarios(); j++)
        {

            if (SADF->getDetector(i)->getProfile(j, 0)->getExecutionTime() !=
                (CDouble)((unsigned long long)(SADF->getDetector(i)->getProfile(j, 0)->getExecutionTime())))
                throw CException((CString)("Error: Real-valued execution time for Detector '") + SADF->getDetector(i)->getName() + "' not surported.");

            if (SADF->getDetector(i)->getProfile(j, 0)->getExecutionTime() !=
                (CDouble)((CSDFtime)(SADF->getDetector(i)->getProfile(j, 0)->getExecutionTime())))
                throw CException((CString)("Error: Execution time for Detector '") + SADF->getDetector(i)->getName() + "' is too large to be supported.");
        }

    // Determine scenario sequences

    vector<CId> KernelSequenceLengths(SADF->getNumberOfKernels());
    vector<CId> DetectorSequenceLengths(SADF->getNumberOfDetectors());
    vector< vector<CId> > KernelScenarioSequences(SADF->getNumberOfKernels());
    vector< vector<CId> > DetectorSubScenarioSequences(SADF->getNumberOfDetectors());

    for (CId i = 0; i != SADF->getNumberOfDetectors(); i++)
    {

        SADF_Process *Detector = SADF->getDetector(i);
        SADF_MarkovChain *MarkovChain = Detector->getMarkovChain(0);

        DetectorSequenceLengths[i] = MarkovChain->getNumberOfStates();
        vector<CId> SubScenarioSequence(DetectorSequenceLengths[i]);

        CId State = MarkovChain->getInitialState();

        for (CId j = 0; j != MarkovChain->getNumberOfStates(); j++)
        {

            bool NextStateFound = false;

            for (CId k = 0; !NextStateFound && k != MarkovChain->getNumberOfStates(); k++)
                if (MarkovChain->getTransitionProbability(State, k) == 1)
                {
                    NextStateFound = true;
                    State = k;
                }

            SubScenarioSequence[j] = MarkovChain->getSubScenario(State);

        }

        DetectorSubScenarioSequences[i] = SubScenarioSequence;
    }

    for (CId i = 0; i != SADF->getNumberOfKernels(); i++)
        if (!SADF->getKernel(i)->hasControlInputChannels())
        {

            KernelSequenceLengths[i] = 1;
            vector<CId> ScenarioSequence(1, 0);
            KernelScenarioSequences[i] = ScenarioSequence;

        }
        else
        {

            SADF_Process *Kernel = SADF->getKernel(i);
            SADF_Process *Detector = Kernel->getControlInputChannels()[0]->getSource();

            KernelSequenceLengths[i] = DetectorSequenceLengths[Detector->getIdentity()];
            vector<CId> ScenarioSequence(KernelSequenceLengths[i]);

            for (CId j = 0; j != KernelSequenceLengths[i]; j++)
                ScenarioSequence[j] = Kernel->getControlInputChannels()[0]->getProductionScenarioID(DetectorSubScenarioSequences[Detector->getIdentity()][j]);

            KernelScenarioSequences[i] = ScenarioSequence;
        }

    // Create CSDF graph

    CSDFcomponent Component = new CSDFcomponent(NULL, SADF->getIdentity());
    TimedCSDFgraph *CSDF = new TimedCSDFgraph(Component);
    CSDF->setName(SADF->getName());
    CSDF->setType("csdf");

    // Create Actors

    for (CId i = 0; i != SADF->getNumberOfKernels(); i++)
    {

        TimedCSDFactor *Actor = CSDF->createActor();
        Actor->setName(SADF->getKernel(i)->getName());
        Actor->setType(SADF->getKernel(i)->getName());

        // Create Input Ports

        CSDFport *Port;

        for (CId j = 0; j != SADF->getKernel(i)->getDataInputChannels().size(); j++)
        {

            CSDFrate RateSequence;
            RateSequence.resize(KernelSequenceLengths[i]);

            for (CId k = 0; k != KernelSequenceLengths[i]; k++)
                RateSequence[k] = SADF->getKernel(i)->getDataInputChannels()[j]->getConsumptionRate(KernelScenarioSequences[i][k]);

            Port = Actor->createPort("in", RateSequence);
            Port->setName((CString)("In_") + SADF->getKernel(i)->getDataInputChannels()[j]->getName());

            if (SADF->getKernel(i)->getDataInputChannels()[j]->getBufferSize() != SADF_UNBOUNDED)
            {
                Port = Actor->createPort("out", RateSequence);
                Port->setName((CString)("Reverse_Out_") + SADF->getKernel(i)->getDataInputChannels()[j]->getName());
            }
        }

        for (CId j = 0; j != SADF->getKernel(i)->getControlInputChannels().size(); j++)
        {

            CSDFrate RateSequence(KernelSequenceLengths[i], 1);

            Port = Actor->createPort("in", RateSequence);
            Port->setName((CString)("In_") + SADF->getKernel(i)->getControlInputChannels()[j]->getName());

            if (SADF->getKernel(i)->getControlInputChannels()[j]->getBufferSize() != SADF_UNBOUNDED)
            {
                Port = Actor->createPort("out", RateSequence);
                Port->setName((CString)("Reverse_Out_") + SADF->getKernel(i)->getControlInputChannels()[j]->getName());
            }
        }

        // Create Output Ports

        for (CId j = 0; j != SADF->getKernel(i)->getDataOutputChannels().size(); j++)
        {

            CSDFrate RateSequence;
            RateSequence.resize(KernelSequenceLengths[i]);

            for (CId k = 0; k != KernelSequenceLengths[i]; k++)
                RateSequence[k] = SADF->getKernel(i)->getDataOutputChannels()[j]->getProductionRate(KernelScenarioSequences[i][k]);

            Port = Actor->createPort("out", RateSequence);
            Port->setName((CString)("Out_") + SADF->getKernel(i)->getDataOutputChannels()[j]->getName());

            if (SADF->getKernel(i)->getDataOutputChannels()[j]->getBufferSize() != SADF_UNBOUNDED)
            {
                Port = Actor->createPort("in", RateSequence);
                Port->setName((CString)("Reverse_In_") + SADF->getKernel(i)->getDataOutputChannels()[j]->getName());
            }
        }

        // Create Ports for Self-Edge

        CSDFrate RateSequence(KernelSequenceLengths[i], 1);

        Port = Actor->createPort("out", RateSequence);
        Port->setName("Self_Out");

        Port = Actor->createPort("in", RateSequence);
        Port->setName("Self_In");

        // Create Processor for Execution Time

        TimedCSDFactor::Processor *Processor = Actor->addProcessor(SADF->getKernel(i)->getName());

        CSDFtimeSequence ExecutionTimeSequence;
        ExecutionTimeSequence.resize(KernelSequenceLengths[i]);

        for (CId j = 0; j != KernelSequenceLengths[i]; j++)
            ExecutionTimeSequence[j] = (CSDFtime) SADF->getKernel(i)->getProfile(KernelScenarioSequences[i][j], 0)->getExecutionTime();

        Processor->execTime = ExecutionTimeSequence;
        Actor->setDefaultProcessor(SADF->getKernel(i)->getName());
    }

    for (CId i = 0; i != SADF->getNumberOfDetectors(); i++)
    {

        TimedCSDFactor *Actor = CSDF->createActor();
        Actor->setName(SADF->getDetector(i)->getName());
        Actor->setType(SADF->getDetector(i)->getName());

        // Create Input Ports

        CSDFport *Port;

        for (CId j = 0; j != SADF->getDetector(i)->getDataInputChannels().size(); j++)
        {

            CSDFrate RateSequence;
            RateSequence.resize(DetectorSequenceLengths[i]);

            for (CId k = 0; k != DetectorSequenceLengths[i]; k++)
                RateSequence[k] = SADF->getDetector(i)->getDataInputChannels()[j]->getConsumptionRate(DetectorSubScenarioSequences[i][k]);

            Port = Actor->createPort("in", RateSequence);
            Port->setName((CString)("In_") + SADF->getDetector(i)->getDataInputChannels()[j]->getName());

            if (SADF->getDetector(i)->getDataInputChannels()[j]->getBufferSize() != SADF_UNBOUNDED)
            {
                Port = Actor->createPort("out", RateSequence);
                Port->setName((CString)("Reverse_Out_") + SADF->getDetector(i)->getDataInputChannels()[j]->getName());
            }
        }

        // Create Output Ports

        for (CId j = 0; j != SADF->getDetector(i)->getDataOutputChannels().size(); j++)
        {

            CSDFrate RateSequence;
            RateSequence.resize(DetectorSequenceLengths[i]);

            for (CId k = 0; k != DetectorSequenceLengths[i]; k++)
                RateSequence[k] = SADF->getDetector(i)->getDataOutputChannels()[j]->getProductionRate(DetectorSubScenarioSequences[i][k]);

            Port = Actor->createPort("out", RateSequence);
            Port->setName((CString)("Out_") + SADF->getDetector(i)->getDataOutputChannels()[j]->getName());

            if (SADF->getDetector(i)->getDataOutputChannels()[j]->getBufferSize() != SADF_UNBOUNDED)
            {
                Port = Actor->createPort("in", RateSequence);
                Port->setName((CString)("Reverse_In_") + SADF->getDetector(i)->getDataOutputChannels()[j]->getName());
            }
        }

        for (CId j = 0; j != SADF->getDetector(i)->getControlOutputChannels().size(); j++)
        {

            CSDFrate RateSequence;
            RateSequence.resize(DetectorSequenceLengths[i]);

            for (CId k = 0; k != DetectorSequenceLengths[i]; k++)
                RateSequence[k] = SADF->getDetector(i)->getControlOutputChannels()[j]->getProductionRate(DetectorSubScenarioSequences[i][k]);

            Port = Actor->createPort("out", RateSequence);
            Port->setName((CString)("Out_") + SADF->getDetector(i)->getControlOutputChannels()[j]->getName());

            if (SADF->getDetector(i)->getControlOutputChannels()[j]->getBufferSize() != SADF_UNBOUNDED)
            {
                Port = Actor->createPort("in", RateSequence);
                Port->setName((CString)("Reverse_In_") + SADF->getDetector(i)->getControlOutputChannels()[j]->getName());
            }
        }

        // Create Ports for Self-Edge

        CSDFrate RateSequence(DetectorSequenceLengths[i], 1);

        Port = Actor->createPort("out", RateSequence);
        Port->setName("Self_Out");

        Port = Actor->createPort("in", RateSequence);
        Port->setName("Self_In");

        // Create Processor for Execution Time

        TimedCSDFactor::Processor *Processor = Actor->addProcessor(SADF->getDetector(i)->getName());

        CSDFtimeSequence ExecutionTimeSequence;
        ExecutionTimeSequence.resize(DetectorSequenceLengths[i]);

        for (CId j = 0; j != DetectorSequenceLengths[i]; j++)
            ExecutionTimeSequence[j] = (CSDFtime) SADF->getDetector(i)->getProfile(DetectorSubScenarioSequences[i][j], 0)->getExecutionTime();

        Processor->execTime = ExecutionTimeSequence;
        Actor->setDefaultProcessor(SADF->getDetector(i)->getName());
    }

    // Create Channels

    for (CId i = 0; i != SADF->getNumberOfDataChannels(); i++)
    {

        CSDFcomponent Component = new CSDFcomponent(CSDF, CSDF->nrChannels());
        TimedCSDFchannel *Channel = new TimedCSDFchannel(Component);
        Channel->setName(SADF->getDataChannel(i)->getName());
        Channel->connectSrc(CSDF->getActor(SADF->getDataChannel(i)->getSource()->getName())->getPort((CString)("Out_")
                            + SADF->getDataChannel(i)->getName()));
        Channel->connectDst(CSDF->getActor(SADF->getDataChannel(i)->getDestination()->getName())->getPort((CString)("In_")
                            + SADF->getDataChannel(i)->getName()));

        if (SADF->getDataChannel(i)->getNumberOfInitialTokens() != 0)
            Channel->setInitialTokens(SADF->getDataChannel(i)->getNumberOfInitialTokens());

        Channel->setTokenSize(SADF->getDataChannel(i)->getTokenSize());

        CSDF->addChannel(Channel);

        if (SADF->getDataChannel(i)->getBufferSize() != SADF_UNBOUNDED)
        {
            CSDFcomponent Component = new CSDFcomponent(CSDF, CSDF->nrChannels());
            TimedCSDFchannel *ReverseChannel = new TimedCSDFchannel(Component);
            ReverseChannel->setName((CString)("Reverse_") + SADF->getDataChannel(i)->getName());
            ReverseChannel->connectSrc(CSDF->getActor(SADF->getDataChannel(i)->getDestination()->getName())->getPort((CString)("Reverse_Out_")
                                       + SADF->getDataChannel(i)->getName()));
            ReverseChannel->connectDst(CSDF->getActor(SADF->getDataChannel(i)->getSource()->getName())->getPort((CString)("Reverse_In_")
                                       + SADF->getDataChannel(i)->getName()));
            ReverseChannel->setInitialTokens((uint)(SADF->getDataChannel(i)->getBufferSize() - (CSize) SADF->getDataChannel(i)->getNumberOfInitialTokens()));
            ReverseChannel->setTokenSize(SADF->getDataChannel(i)->getTokenSize());
            CSDF->addChannel(ReverseChannel);
        }
    }

    for (CId i = 0; i != SADF->getNumberOfControlChannels(); i++)
    {

        CSDFcomponent Component = new CSDFcomponent(CSDF, CSDF->nrChannels());
        TimedCSDFchannel *Channel = new TimedCSDFchannel(Component);
        Channel->setName(SADF->getControlChannel(i)->getName());
        Channel->connectSrc(CSDF->getActor(SADF->getControlChannel(i)->getSource()->getName())->getPort((CString)("Out_")
                            + SADF->getControlChannel(i)->getName()));
        Channel->connectDst(CSDF->getActor(SADF->getControlChannel(i)->getDestination()->getName())->getPort((CString)("In_")
                            + SADF->getControlChannel(i)->getName()));

        Channel->setTokenSize(SADF->getControlChannel(i)->getTokenSize());

        CSDF->addChannel(Channel);

        if (SADF->getControlChannel(i)->getBufferSize() != SADF_UNBOUNDED)
        {
            CSDFcomponent Component = new CSDFcomponent(CSDF, CSDF->nrChannels());
            TimedCSDFchannel *ReverseChannel = new TimedCSDFchannel(Component);
            ReverseChannel->setName((CString)("Reverse_") + SADF->getControlChannel(i)->getName());
            ReverseChannel->connectSrc(CSDF->getActor(SADF->getControlChannel(i)->getDestination()->getName())->getPort((CString)("Reverse_Out_")
                                       + SADF->getControlChannel(i)->getName()));
            ReverseChannel->connectDst(CSDF->getActor(SADF->getControlChannel(i)->getSource()->getName())->getPort((CString)("Reverse_In_")
                                       + SADF->getControlChannel(i)->getName()));
            ReverseChannel->setTokenSize(SADF->getControlChannel(i)->getTokenSize());
            CSDF->addChannel(ReverseChannel);
        }
    }

    for (CId i = 0; i != SADF->getNumberOfKernels(); i++)
    {
        CSDFcomponent Component = new CSDFcomponent(CSDF, CSDF->nrChannels());
        TimedCSDFchannel *SelfChannel = new TimedCSDFchannel(Component);
        SelfChannel->setName((CString)("Self_") + SADF->getKernel(i)->getName());
        SelfChannel->connectSrc(CSDF->getActor(SADF->getKernel(i)->getName())->getPort("Self_Out"));
        SelfChannel->connectDst(CSDF->getActor(SADF->getKernel(i)->getName())->getPort("Self_In"));
        SelfChannel->setInitialTokens(1);
        SelfChannel->setTokenSize(1);
        CSDF->addChannel(SelfChannel);
    }

    for (CId i = 0; i != SADF->getNumberOfDetectors(); i++)
    {
        CSDFcomponent Component = new CSDFcomponent(CSDF, CSDF->nrChannels());
        TimedCSDFchannel *SelfChannel = new TimedCSDFchannel(Component);
        SelfChannel->setName((CString)("Self_") + SADF->getDetector(i)->getName());
        SelfChannel->connectSrc(CSDF->getActor(SADF->getDetector(i)->getName())->getPort("Self_Out"));
        SelfChannel->connectDst(CSDF->getActor(SADF->getDetector(i)->getName())->getPort("Self_In"));
        SelfChannel->setInitialTokens(1);
        SelfChannel->setTokenSize(1);
        CSDF->addChannel(SelfChannel);
    }

    return CSDF;
}
