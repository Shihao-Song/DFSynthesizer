/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf2poosl_process.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   13 September 2006
 *
 *  Function        :   Output SADF graph in POOSL format (process class(es))
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

#include "sadf2poosl_process.h"

void SADF2POOSL_Kernel(SADF_Process *Kernel, ostream &out, bool PERT, bool p4r)
{

    bool Timed = false;

    for (CId i = 0; !Timed && i != Kernel->getNumberOfScenarios(); i++)
        for (CId j = 0; !Timed && j != Kernel->getNumberOfProfiles(i); j++)
            if (Kernel->getProfile(i, j)->getExecutionTime() != 0)
                Timed = true;

    out << "process class Kernel" << Kernel->getIdentity() << "(Monitor: Boolean, MonitorID: Integer, Trace: Boolean, Name: String)" << endl;
    out << "instance variables" << endl;

    if (Timed)
        out << "ExecutionTimes: Array, ";

    out << "Status: ProcessMonitor" << endl << endl;

    if (!p4r)
    {

        out << "port interface" << endl;

        if (Kernel->hasControlInputChannels())
            out << "CI" << Kernel->getControlInputChannels()[0]->getIdentity() << ", ";

        for (CId j = 0; j != Kernel->getDataInputChannels().size(); j++)
            out << "DI" << Kernel->getDataInputChannels()[j]->getIdentity() << ", ";

        for (CId j = 0; j != Kernel->getDataOutputChannels().size(); j++)
            out << "DO" << Kernel->getDataOutputChannels()[j]->getIdentity() << ", ";

        out << "Monitor" << endl << endl;
        out << "message interface" << endl;

        if (Kernel->hasControlInputChannels())
        {
            out << "CI" << Kernel->getControlInputChannels()[0]->getIdentity() << "!Inspect," << endl;
            out << "CI" << Kernel->getControlInputChannels()[0]->getIdentity() << "?Scenario(Integer)," << endl;
            out << "CI" << Kernel->getControlInputChannels()[0]->getIdentity() << "!Read," << endl;
        }

        for (CId j = 0; j != Kernel->getDataInputChannels().size(); j++)
        {
            out << "DI" << Kernel->getDataInputChannels()[j]->getIdentity() << "!Inspect(Integer)," << endl;
            out << "DI" << Kernel->getDataInputChannels()[j]->getIdentity() << "!Read," << endl;
        }

        for (CId j = 0; j != Kernel->getDataOutputChannels().size(); j++)
        {
            out << "DO" << Kernel->getDataOutputChannels()[j]->getIdentity() << "!Room(Integer)," << endl;
            out << "DO" << Kernel->getDataOutputChannels()[j]->getIdentity() << "!Reserve," << endl;
            out << "DO" << Kernel->getDataOutputChannels()[j]->getIdentity() << "!Write," << endl;
        }

        out << "Monitor?StopSimulation," << endl;
        out << "Monitor!Results(String)," << endl;
        out << "Monitor?Status(Integer)," << endl;
        out << "Monitor!AccuracyStatus(Integer, Boolean)" << endl;
    }

    out << endl;
    out << "initial method call" << endl;
    out << "Initialise()()" << endl << endl;
    out << "instance methods" << endl;

    out << "Initialise()()";

    if (Timed)
    {

        if (PERT)
            out << " |Distribution: PERT|" << endl << endl;
        else
            out << " |Distribution: GeneralDiscrete|" << endl << endl;

        if (p4r)
            out << "ExecutionTimes := new(Array) size(" << Kernel->getNumberOfScenarios() << ");" << endl;
        else
            out << "ExecutionTimes := new(Array) resize(" << Kernel->getNumberOfScenarios() << ");" << endl;

        for (CId i = 0; i != Kernel->getNumberOfScenarios(); i++)
        {

            if (PERT)
            {
                out << "Distribution := new(PERT)" << endl;

                SADF_Profile *Minimum = Kernel->getProfile(i, 0);
                SADF_Profile *Maximum = Kernel->getProfile(i, 0);
                SADF_Profile *Mode = Kernel->getProfile(i, 0);

                for (CId j = 0; j != Kernel->getNumberOfProfiles(i); j++)
                {
                    if (Kernel->getProfile(i, j)->getExecutionTime() < Minimum->getExecutionTime()) Minimum = Kernel->getProfile(i, j);
                    if (Kernel->getProfile(i, j)->getExecutionTime() > Maximum->getExecutionTime()) Maximum = Kernel->getProfile(i, j);
                }
                for (CId j = 0; j != Kernel->getNumberOfProfiles(i); j++)
                    if ((Kernel->getProfile(i, j) != Minimum) & (Kernel->getProfile(i, j) != Maximum))
                        Mode = Kernel->getProfile(i, j);

                out << " withParameters(";
                if (Minimum->getExecutionTime() != (CDouble)((unsigned long long)(Minimum->getExecutionTime())))
                    out << Minimum->getExecutionTime();
                else
                    out << ((unsigned long long)(Minimum->getExecutionTime()));
                out << ", ";
                if (Mode->getExecutionTime() != (CDouble)((unsigned long long)(Mode->getExecutionTime())))
                    out << Mode->getExecutionTime();
                else
                    out << ((unsigned long long)(Mode->getExecutionTime()));
                out << ", ";
                if (Maximum->getExecutionTime() != (CDouble)((unsigned long long)(Maximum->getExecutionTime())))
                    out << Maximum->getExecutionTime();
                else
                    out << ((unsigned long long)(Maximum->getExecutionTime()));
                out << ");" << endl;

            }
            else
            {

                out << "Distribution := new(GeneralDiscrete)" << endl;

                for (CId j = 0; j != Kernel->getNumberOfProfiles(i); j++)
                {

                    out << "   withParameters(";

                    if (Kernel->getProfile(i, j)->getExecutionTime() != (CDouble)((unsigned long long)(Kernel->getProfile(i, j)->getExecutionTime())))
                        out << Kernel->getProfile(i, j)->getExecutionTime();
                    else
                        out << ((unsigned long long)(Kernel->getProfile(i, j)->getExecutionTime()));

                    if (j < Kernel->getNumberOfProfiles(i) - 1)
                        out << ", " << Kernel->getProfile(i, j)->getWeight() << ")" << endl;
                    else
                        out << ", " << Kernel->getProfile(i, j)->getWeight() << ");" << endl;
                }
            }

            out << "ExecutionTimes put(" << i + 1 << ", Distribution);" << endl;
        }

    }
    else
        out << endl << endl;

    out << "Status := new(ProcessMonitor";
    if (PERT)
        out << "_PERT";
    out << ") init(Monitor, MonitorID, Trace, Name);" << endl;

    out << "abort" << endl;
    out << "   par Fire()() and if Monitor then CheckAccuracyStatus()() fi rap" << endl;
    out << "with Monitor?StopSimulation; if Monitor then Monitor!Results(Status getResults) fi." << endl << endl;

    out << "CheckAccuracyStatus()() |ID: Integer|" << endl << endl;
    out << "Monitor?Status(ID | ID = MonitorID);" << endl;
    out << "Monitor!AccuracyStatus(MonitorID, Status accurate);" << endl;
    out << "CheckAccuracyStatus()()." << endl << endl;

    out << "Fire()() |Scenario: Integer|" << endl << endl;

    if (Kernel->hasControlInputChannels())
    {
        out << "CI" << Kernel->getControlInputChannels()[0]->getIdentity() << "!Inspect;" << endl;
        out << "CI" << Kernel->getControlInputChannels()[0]->getIdentity() << "?Scenario(Scenario);" << endl;
    }
    else
        out << "Scenario := 0;" << endl;

    if (Kernel->getNumberOfScenarios() == 1)
    {

        for (CId j = 0; j != Kernel->getDataInputChannels().size(); j++)
            out << "DI" << Kernel->getDataInputChannels()[j]->getIdentity() << "!Inspect("
                << Kernel->getDataInputChannels()[j]->getConsumptionRate(0) << ");" << endl;

        for (CId j = 0; j != Kernel->getDataOutputChannels().size(); j++)
            out << "DO" << Kernel->getDataOutputChannels()[j]->getIdentity() << "!Room("
                << Kernel->getDataOutputChannels()[j]->getProductionRate(0) << ");" << endl;

        for (CId j = 0; j != Kernel->getDataOutputChannels().size(); j++)
            out << "DO" << Kernel->getDataOutputChannels()[j]->getIdentity() << "!Reserve;" << endl;

        out << "Status start(currentTime);" << endl;

        if (Timed)
        {
            if (p4r)
                out << "delay(ExecutionTimes get(Scenario + 1) sample);" << endl;
            else
                out << "delay(ExecutionTimes at(Scenario + 1) sample);" << endl;
        }

        out << "Status end(currentTime);" << endl;

        for (CId j = 0; j != Kernel->getDataInputChannels().size(); j++)
            out << "DI" << Kernel->getDataInputChannels()[j]->getIdentity() << "!Read;" << endl;

        for (CId j = 0; j != Kernel->getDataOutputChannels().size(); j++)
            out << "DO" << Kernel->getDataOutputChannels()[j]->getIdentity() << "!Write;" << endl;

    }
    else
    {

        for (CId i = 0; i != Kernel->getNumberOfScenarios(); i++)
        {

            bool TimedScenario = false;

            if (Timed)
                for (CId j = 0; !TimedScenario && j != Kernel->getNumberOfProfiles(i); j++)
                    if (Kernel->getProfile(i, j)->getExecutionTime() != 0)
                        TimedScenario = true;

            out << "if Scenario = " << i << " then" << endl;

            for (CId j = 0; j != Kernel->getDataInputChannels().size(); j++)
                if (Kernel->getDataInputChannels()[j]->getConsumptionRate(i) > 0)
                    out << "   DI" << Kernel->getDataInputChannels()[j]->getIdentity() << "!Inspect("
                        << Kernel->getDataInputChannels()[j]->getConsumptionRate(i) << ");" << endl;

            for (CId j = 0; j != Kernel->getDataOutputChannels().size(); j++)
                if (Kernel->getDataOutputChannels()[j]->getProductionRate(i) > 0)
                    out << "   DO" << Kernel->getDataOutputChannels()[j]->getIdentity() << "!Room("
                        << Kernel->getDataOutputChannels()[j]->getProductionRate(i) << ");" << endl;

            for (CId j = 0; j != Kernel->getDataOutputChannels().size(); j++)
                if (Kernel->getDataOutputChannels()[j]->getProductionRate(i) > 0)
                    out << "   DO" << Kernel->getDataOutputChannels()[j]->getIdentity() << "!Reserve;" << endl;

            out << "   Status start(currentTime);" << endl;

            if (TimedScenario)
            {
                if (p4r)
                    out << "   delay(ExecutionTimes get(Scenario + 1) sample);" << endl;
                else
                    out << "   delay(ExecutionTimes at(Scenario + 1) sample);" << endl;
            }

            out << "   Status end(currentTime)";

            for (CId j = 0; j != Kernel->getDataInputChannels().size(); j++)
                if (Kernel->getDataInputChannels()[j]->getConsumptionRate(i) > 0)
                    out << ";" << endl << "   DI" << Kernel->getDataInputChannels()[j]->getIdentity() << "!Read";

            for (CId j = 0; j != Kernel->getDataOutputChannels().size(); j++)
                if (Kernel->getDataOutputChannels()[j]->getProductionRate(i) > 0)
                    out << ";" << endl << "   DO" << Kernel->getDataOutputChannels()[j]->getIdentity() << "!Write";

            out << endl << "fi;" << endl;
        }
    }

    if (Kernel->hasControlInputChannels())
        out << "CI" << Kernel->getControlInputChannels()[0]->getIdentity() << "!Read;" << endl;

    out << "Fire()()." << endl << endl;
}

void SADF2POOSL_Detector(SADF_Process *Detector, ostream &out, bool PERT, bool p4r)
{

    bool Timed = false;

    for (CId i = 0; !Timed && i != Detector->getNumberOfSubScenarios(); i++)
        for (CId j = 0; !Timed && j != Detector->getNumberOfProfiles(i); j++)
            if (Detector->getProfile(i, j)->getExecutionTime() != 0)
                Timed = true;

    out << "process class Detector" << Detector->getIdentity() << "(Monitor: Boolean, MonitorID: Integer, Trace: Boolean, Name: String)" << endl;
    out << "instance variables" << endl;

    if (Timed)
        out << "ExecutionTimes: Array, ";

    out << "Status: ProcessMonitor, MarkovChains: Array" << endl << endl;

    if (!p4r)
    {
        out << "port interface" << endl;

        if (Detector->hasControlInputChannels())
            out << "CI" << Detector->getControlInputChannels()[0]->getIdentity() << ", ";

        for (CId j = 0; j != Detector->getDataInputChannels().size(); j++)
            out << "DI" << Detector->getDataInputChannels()[j]->getIdentity() << ", ";

        for (CId j = 0; j != Detector->getDataOutputChannels().size(); j++)
            out << "DO" << Detector->getDataOutputChannels()[j]->getIdentity() << ", ";

        for (CId j = 0; j != Detector->getControlOutputChannels().size(); j++)
            out << "CO" << Detector->getControlOutputChannels()[j]->getIdentity() << ", ";

        out << "Monitor" << endl << endl;
        out << "message interface" << endl;

        if (Detector->hasControlInputChannels())
        {
            out << "CI" << Detector->getControlInputChannels()[0]->getIdentity() << "!Inspect," << endl;
            out << "CI" << Detector->getControlInputChannels()[0]->getIdentity() << "?Scenario(Integer)," << endl;
            out << "CI" << Detector->getControlInputChannels()[0]->getIdentity() << "!Read," << endl;
        }

        for (CId j = 0; j != Detector->getDataInputChannels().size(); j++)
        {
            out << "DI" << Detector->getDataInputChannels()[j]->getIdentity() << "!Inspect(Integer)," << endl;
            out << "DI" << Detector->getDataInputChannels()[j]->getIdentity() << "!Read," << endl;
        }

        for (CId j = 0; j != Detector->getDataOutputChannels().size(); j++)
        {
            out << "DO" << Detector->getDataOutputChannels()[j]->getIdentity() << "!Room(Integer)," << endl;
            out << "DO" << Detector->getDataOutputChannels()[j]->getIdentity() << "!Reserve," << endl;
            out << "DO" << Detector->getDataOutputChannels()[j]->getIdentity() << "!Write," << endl;
        }

        for (CId j = 0; j != Detector->getControlOutputChannels().size(); j++)
        {
            out << "CO" << Detector->getControlOutputChannels()[j]->getIdentity() << "!Room(Integer)," << endl;
            out << "CO" << Detector->getControlOutputChannels()[j]->getIdentity() << "!Reserve," << endl;
            out << "CO" << Detector->getControlOutputChannels()[j]->getIdentity() << "!Write(Integer)," << endl;
        }

        out << "Monitor?StopSimulation," << endl;
        out << "Monitor!Results(String)," << endl;
        out << "Monitor?Status(Integer)," << endl;
        out << "Monitor!AccuracyStatus(Integer, Boolean)" << endl;
    }

    out << "initial method call" << endl;
    out << "Initialise()()" << endl << endl;
    out << "instance methods" << endl;

    out << "Initialise()() |MarkovChain: MarkovChain";

    if (Timed)
    {

        if (PERT)
            out << ", Distribution: PERT|" << endl << endl;
        else
            out << ", Distribution: GeneralDiscrete|" << endl << endl;

        if (p4r)
            out << "ExecutionTimes := new(Array) size(" << Detector->getNumberOfSubScenarios() << ");" << endl;
        else
            out << "ExecutionTimes := new(Array) resize(" << Detector->getNumberOfSubScenarios() << ");" << endl;

        for (CId i = 0; i != Detector->getNumberOfSubScenarios(); i++)
        {

            if (PERT)
            {

                out << "Distribution := new(PERT)" << endl;

                SADF_Profile *Minimum = Detector->getProfile(i, 0);
                SADF_Profile *Maximum = Detector->getProfile(i, 0);
                SADF_Profile *Mode = Detector->getProfile(i, 0);

                for (CId j = 0; j != Detector->getNumberOfProfiles(i); j++)
                {
                    if (Detector->getProfile(i, j)->getExecutionTime() < Minimum->getExecutionTime()) Minimum = Detector->getProfile(i, j);
                    if (Detector->getProfile(i, j)->getExecutionTime() > Maximum->getExecutionTime()) Maximum = Detector->getProfile(i, j);
                }
                for (CId j = 0; j != Detector->getNumberOfProfiles(i); j++)
                    if ((Detector->getProfile(i, j) != Minimum) & (Detector->getProfile(i, j) != Maximum))
                        Mode = Detector->getProfile(i, j);

                out << " withParameters(";
                if (Minimum->getExecutionTime() != (CDouble)((unsigned long long)(Minimum->getExecutionTime())))
                    out << Minimum->getExecutionTime();
                else
                    out << ((unsigned long long)(Minimum->getExecutionTime()));
                out << ", ";
                if (Mode->getExecutionTime() != (CDouble)((unsigned long long)(Mode->getExecutionTime())))
                    out << Mode->getExecutionTime();
                else
                    out << ((unsigned long long)(Mode->getExecutionTime()));
                out << ", ";
                if (Maximum->getExecutionTime() != (CDouble)((unsigned long long)(Maximum->getExecutionTime())))
                    out << Maximum->getExecutionTime();
                else
                    out << ((unsigned long long)(Maximum->getExecutionTime()));
                out << ");" << endl;

            }
            else
            {

                out << "Distribution := new(GeneralDiscrete)" << endl;

                for (CId j = 0; j != Detector->getNumberOfProfiles(i); j++)
                {

                    out << "   withParameters(";

                    if (Detector->getProfile(i, j)->getExecutionTime() != (CDouble)((unsigned long long)(Detector->getProfile(i, j)->getExecutionTime())))
                        out << Detector->getProfile(i, j)->getExecutionTime();
                    else
                        out << ((unsigned long long)(Detector->getProfile(i, j)->getExecutionTime()));

                    if (j < Detector->getNumberOfProfiles(i) - 1)
                        out << ", " << Detector->getProfile(i, j)->getWeight() << ")" << endl;
                    else
                        out << ", " << Detector->getProfile(i, j)->getWeight() << ");" << endl;
                }
            }

            out << "ExecutionTimes put(" << i + 1 << ", Distribution);" << endl;
        }

    }
    else
        out << "|" << endl << endl;

    if (p4r)
        out << "MarkovChains := new(Array) size(" << Detector->getNumberOfScenarios() << ");" << endl;
    else
        out << "MarkovChains := new(Array) resize(" << Detector->getNumberOfScenarios() << ");" << endl;

    for (CId s = 0; s != Detector->getNumberOfScenarios(); s++)
    {

        out << "MarkovChain := new(MarkovChain) init" << endl;

        for (CId i = 0; i != Detector->getMarkovChain(s)->getNumberOfStates(); i++)
        {

            out << "   addState(\"" << Detector->getMarkovChain(s)->getStateName(i) << "\", "
                << Detector->getMarkovChain(s)->getSubScenario(i) << ")" << endl;

            CDouble CumulativeWeight = 0;

            for (CId j = 0; j != Detector->getMarkovChain(s)->getNumberOfStates(); j++)
                if (Detector->getMarkovChain(s)->getTransitionProbability(i, j) != 0)
                {

                    out << "      addTransition(\"" << Detector->getMarkovChain(s)->getStateName(i) << "\", \""
                        << Detector->getMarkovChain(s)->getStateName(j) << "\", " << CumulativeWeight << ", "
                        << CumulativeWeight + Detector->getMarkovChain(s)->getTransitionProbability(i, j) << ")" << endl;

                    CumulativeWeight += Detector->getMarkovChain(s)->getTransitionProbability(i, j);
                }
        }

        out << "   setInitialState(\"" << Detector->getMarkovChain(s)->getStateName(Detector->getMarkovChain(s)->getInitialState()) << "\");" << endl;
        out << "MarkovChains put(" << s + 1 << ", MarkovChain);" << endl;
    }

    out << "Status := new(ProcessMonitor";
    if (PERT)
        out << "_PERT";
    out << ") init(Monitor, MonitorID, Trace, Name);" << endl;
    out << "abort" << endl;
    out << "   par Fire()() and if Monitor then CheckAccuracyStatus()() fi rap" << endl;
    out << "with Monitor?StopSimulation; if Monitor then Monitor!Results(Status getResults) fi." << endl << endl;

    out << "CheckAccuracyStatus()() |ID: Integer|" << endl << endl;
    out << "Monitor?Status(ID | ID = MonitorID);" << endl;
    out << "Monitor!AccuracyStatus(MonitorID, Status accurate);" << endl;
    out << "CheckAccuracyStatus()()." << endl << endl;

    out << "Fire()() |Scenario, SubScenario: Integer|" << endl << endl;

    if (Detector->hasControlInputChannels())
    {
        out << "CI" << Detector->getControlInputChannels()[0]->getIdentity() << "!Inspect;" << endl;
        out << "CI" << Detector->getControlInputChannels()[0]->getIdentity() << "?Scenario(Scenario);" << endl;
    }
    else
        out << "Scenario := 0;" << endl;

    if (p4r)
        out << "SubScenario := MarkovChains get(Scenario + 1) getNextSubScenario;" << endl;
    else
        out << "SubScenario := MarkovChains at(Scenario + 1) getNextSubScenario;" << endl;

    if (Detector->getNumberOfSubScenarios() == 1)
    {

        for (CId j = 0; j != Detector->getDataInputChannels().size(); j++)
            out << "DI" << Detector->getDataInputChannels()[j]->getIdentity() << "!Inspect("
                << Detector->getDataInputChannels()[j]->getConsumptionRate(0) << ");" << endl;

        for (CId j = 0; j != Detector->getDataOutputChannels().size(); j++)
            out << "DO" << Detector->getDataOutputChannels()[j]->getIdentity() << "!Room("
                << Detector->getDataOutputChannels()[j]->getProductionRate(0) << ");" << endl;

        for (CId j = 0; j != Detector->getControlOutputChannels().size(); j++)
            out << "CO" << Detector->getControlOutputChannels()[j]->getIdentity() << "!Room("
                << Detector->getControlOutputChannels()[j]->getProductionRate(0) << ");" << endl;

        for (CId j = 0; j != Detector->getDataOutputChannels().size(); j++)
            out << "DO" << Detector->getDataOutputChannels()[j]->getIdentity() << "!Reserve;" << endl;

        for (CId j = 0; j != Detector->getControlOutputChannels().size(); j++)
            out << "CO" << Detector->getControlOutputChannels()[j]->getIdentity() << "!Reserve;" << endl;

        out << "Status start(currentTime);" << endl;

        if (Timed)
        {
            if (p4r)
                out << "delay(ExecutionTimes get(SubScenario + 1) sample);" << endl;
            else
                out << "delay(ExecutionTimes at(SubScenario + 1) sample);" << endl;
        }

        out << "Status end(currentTime);" << endl;

        for (CId j = 0; j != Detector->getDataInputChannels().size(); j++)
            out << "DI" << Detector->getDataInputChannels()[j]->getIdentity() << "!Read;" << endl;

        for (CId j = 0; j != Detector->getDataOutputChannels().size(); j++)
            out << "DO" << Detector->getDataOutputChannels()[j]->getIdentity() << "!Write;" << endl;

        for (CId j = 0; j != Detector->getControlOutputChannels().size(); j++)
            out << "CO" << Detector->getControlOutputChannels()[j]->getIdentity() << "!Write("
                << Detector->getControlOutputChannels()[j]->getProductionScenarioID(0) << ");" << endl;
    }
    else
    {

        for (CId i = 0; i != Detector->getNumberOfSubScenarios(); i++)
        {

            bool TimedScenario = false;

            if (Timed)
                for (CId j = 0; !TimedScenario && j != Detector->getNumberOfProfiles(i); j++)
                    if (Detector->getProfile(i, j)->getExecutionTime() != 0)
                        TimedScenario = true;

            out << "if SubScenario = " << i << " then" << endl;

            for (CId j = 0; j != Detector->getDataInputChannels().size(); j++)
                if (Detector->getDataInputChannels()[j]->getConsumptionRate(i) > 0)
                    out << "   DI" << Detector->getDataInputChannels()[j]->getIdentity() << "!Inspect("
                        << Detector->getDataInputChannels()[j]->getConsumptionRate(i) << ");" << endl;

            for (CId j = 0; j != Detector->getDataOutputChannels().size(); j++)
                if (Detector->getDataOutputChannels()[j]->getProductionRate(i) > 0)
                    out << "   DO" << Detector->getDataOutputChannels()[j]->getIdentity() << "!Room("
                        << Detector->getDataOutputChannels()[j]->getProductionRate(i) << ");" << endl;

            for (CId j = 0; j != Detector->getControlOutputChannels().size(); j++)
                out << "   CO" << Detector->getControlOutputChannels()[j]->getIdentity() << "!Room("
                    << Detector->getControlOutputChannels()[j]->getProductionRate(i) << ");" << endl;

            for (CId j = 0; j != Detector->getDataOutputChannels().size(); j++)
                if (Detector->getDataOutputChannels()[j]->getProductionRate(i) > 0)
                    out << "   DO" << Detector->getDataOutputChannels()[j]->getIdentity() << "!Reserve;" << endl;

            for (CId j = 0; j != Detector->getControlOutputChannels().size(); j++)
                out << "   CO" << Detector->getControlOutputChannels()[j]->getIdentity() << "!Reserve;" << endl;

            out << "   Status start(currentTime);" << endl;

            if (TimedScenario)
            {
                if (p4r)
                    out << "   delay(ExecutionTimes get(SubScenario + 1) sample);" << endl;
                else
                    out << "   delay(ExecutionTimes at(SubScenario + 1) sample);" << endl;
            }

            out << "   Status end(currentTime)";

            for (CId j = 0; j != Detector->getDataInputChannels().size(); j++)
                if (Detector->getDataInputChannels()[j]->getConsumptionRate(i) > 0)
                    out << ";" << endl << "   DI" << Detector->getDataInputChannels()[j]->getIdentity() << "!Read";

            for (CId j = 0; j != Detector->getDataOutputChannels().size(); j++)
                if (Detector->getDataOutputChannels()[j]->getProductionRate(i) > 0)
                    out << ";" << endl << "   DO" << Detector->getDataOutputChannels()[j]->getIdentity() << "!Write";

            for (CId j = 0; j != Detector->getControlOutputChannels().size(); j++)
                out << ";" << endl << "   CO" << Detector->getControlOutputChannels()[j]->getIdentity() << "!Write("
                    << Detector->getControlOutputChannels()[j]->getProductionScenarioID(i) << ")";

            out << endl << "fi;" << endl;
        }
    }

    if (Detector->hasControlInputChannels())
        out << "CI" << Detector->getControlInputChannels()[0]->getIdentity() << "!Read;" << endl;

    out << "Fire()()." << endl << endl;
}

void SADF2POOSL_Process(SADF_Graph *Graph, SADF_SimulationSettings *Settings, ostream &out, bool PERT, bool p4r)
{

    for (CId i = 0; i != Graph->getNumberOfKernels(); i++)
        SADF2POOSL_Kernel(Graph->getKernel(i), out, PERT, p4r);

    for (CId i = 0; i != Graph->getNumberOfDetectors(); i++)
        SADF2POOSL_Detector(Graph->getDetector(i), out, PERT, p4r);

    // Generate Process Class SimulationController

    out << "process class SimulationController(NumberOfMonitors: Integer, LogFile: String)" << endl;
    out << "instance variables" << endl;
    out << "Status: MonitorStatus" << endl << endl;

    if (!p4r)
    {
        out << "port interface" << endl;
        out << "Monitor" << endl << endl;
        out << "message interface" << endl;
        out << "Monitor?Results(String)," << endl;
        out << "Monitor!Status(Integer)," << endl;
        out << "Monitor?AccuracyStatus(Integer, Boolean)," << endl;
        out << "Monitor!StopSimulation" << endl;
    }

    out << "initial method call" << endl;
    out << "Initialise()()" << endl << endl;
    out << "instance methods" << endl;

    out << "Initialise()() |MonitorID: Integer, Results: String|" << endl << endl;
    out << "Status := new(MonitorStatus) init(NumberOfMonitors, LogFile);" << endl;

    if (Settings->getMaximumModelTime() != SADF_UNDEFINED)
    {
        if (Settings->getMaximumModelTime() != (CDouble)((unsigned long long)(Settings->getMaximumModelTime())))
            out << "abort CheckAccuracyStatus()() with delay(" << Settings->getMaximumModelTime() << ");" << endl;
        else
            out << "abort CheckAccuracyStatus()() with delay(" << ((unsigned long long)(Settings->getMaximumModelTime())) << ");" << endl;
    }
    else
        out << "CheckAccuracyStatus()();" << endl;

    out << "abort while true do Monitor!StopSimulation od with delay(1.0e-10);" << endl;

    out << "MonitorID := 1;" << endl;
    out << "while MonitorID <= NumberOfMonitors do" << endl;
    out << "   Monitor?Results(Results){Status append(Results)};" << endl;
    out << "   MonitorID := MonitorID + 1" << endl;
    out << "od;" << endl;
    out << "Status log." << endl << endl;

    out << "CheckAccuracyStatus()() |MonitorID: Integer, Accurate: Boolean|" << endl << endl;

    CDouble MaximumTimeStep = 0;
    CDouble MinimumTimeStep = -1;

    for (CId i = 0; i != Graph->getNumberOfKernels(); i++)
        for (CId j = 0; j != Graph->getKernel(i)->getNumberOfScenarios(); j++)
            for (CId k = 0; k != Graph->getKernel(i)->getNumberOfProfiles(j); k++)
            {

                if (Graph->getKernel(i)->getProfile(j, k)->getExecutionTime() > MaximumTimeStep)
                    MaximumTimeStep = Graph->getKernel(i)->getProfile(j, k)->getExecutionTime();

                if (MinimumTimeStep == -1)
                    MinimumTimeStep = Graph->getKernel(i)->getProfile(j, k)->getExecutionTime();
                else if (Graph->getKernel(i)->getProfile(j, k)->getExecutionTime() < MinimumTimeStep)
                    MinimumTimeStep = Graph->getKernel(i)->getProfile(j, k)->getExecutionTime();
            }

    for (CId i = 0; i != Graph->getNumberOfDetectors(); i++)
        for (CId j = 0; j != Graph->getDetector(i)->getNumberOfSubScenarios(); j++)
            for (CId k = 0; k != Graph->getDetector(i)->getNumberOfProfiles(j); k++)
            {

                if (Graph->getDetector(i)->getProfile(j, k)->getExecutionTime() > MaximumTimeStep)
                    MaximumTimeStep = Graph->getDetector(i)->getProfile(j, k)->getExecutionTime();

                if (MinimumTimeStep == -1)
                    MinimumTimeStep = Graph->getDetector(i)->getProfile(j, k)->getExecutionTime();
                else if (Graph->getDetector(i)->getProfile(j, k)->getExecutionTime() < MinimumTimeStep)
                    MinimumTimeStep = Graph->getDetector(i)->getProfile(j, k)->getExecutionTime();
            }

    CDouble TimeStep = MaximumTimeStep - MinimumTimeStep;

    if (TimeStep != 0)
    {
        if (TimeStep != (CDouble)((CId)(TimeStep)))
            out << "delay(" << (50 * TimeStep) << ");" << endl;
        else
            out << "delay(" << (50 * ((CId)(TimeStep))) << ");" << endl;
    }
    else
    {
        if (MinimumTimeStep != (CDouble)((CId)(MinimumTimeStep)))
            out << "delay(" << (50 * MinimumTimeStep) << ");" << endl;
        else
            out << "delay(" << (50 * ((CId)(MinimumTimeStep))) << ");" << endl;
    }

    out << "MonitorID := 1;" << endl;
    out << "while MonitorID <= NumberOfMonitors do" << endl;
    out << "   Monitor!Status(MonitorID);" << endl;
    out << "   Monitor?AccuracyStatus(MonitorID, Accurate){Status register(MonitorID, Accurate)};" << endl;
    out << "   MonitorID := MonitorID + 1" << endl;
    out << "od;" << endl;
    out << "if (Status accurate not) then CheckAccuracyStatus()() fi." << endl << endl;

    // Generate Process Class DataBuffer

    out << "process class DataBuffer(BufferSize: Integer, InitialTokens: Integer, TokenSize: Integer, Monitor: Boolean, MonitorID: Integer, Trace: Boolean, Name: String)" << endl;
    out << "instance variables" << endl;
    out << "Status: DataBufferMonitor" << endl << endl;

    if (!p4r)
    {
        out << "port interface" << endl;
        out << "Monitor, In, Out" << endl << endl;
        out << "message interface" << endl;
        out << "In?Room(Integer)," << endl;
        out << "In?Reserve," << endl;
        out << "In?Write," << endl;
        out << "Out?Inspect(Integer)," << endl;
        out << "Out?Read," << endl;
        out << "Monitor!Results(String)," << endl;
        out << "Monitor?Status(Integer)," << endl;
        out << "Monitor!AccuracyStatus(Integer, Boolean)," << endl;
        out << "Monitor?StopSimulation" << endl;
    }

    out << "initial method call" << endl;
    out << "Initialise()()" << endl << endl;
    out << "instance methods" << endl;

    out << "Initialise()()" << endl << endl;
    out << "Status := new(DataBufferMonitor) init(BufferSize, InitialTokens, TokenSize, Monitor, MonitorID, Trace, Name);" << endl;
    out << "abort" << endl;
    out << "   par HandleInput()() and HandleOutput()() and if Monitor then CheckAccuracyStatus()() fi rap" << endl;
    out << "with Monitor?StopSimulation; if Monitor then Monitor!Results(Status getResults) fi." << endl << endl;

    out << "CheckAccuracyStatus()() |ID: Integer|" << endl << endl;
    out << "Monitor?Status(ID | ID = MonitorID);" << endl;
    out << "Monitor!AccuracyStatus(MonitorID, Status accurate);" << endl;
    out << "CheckAccuracyStatus()()." << endl << endl;

    out << "HandleInput()() |NumberOfTokens: Integer|" << endl << endl;
    out << "In?Room(NumberOfTokens | Status room(NumberOfTokens));" << endl;
    out << "In?Reserve{Status reserve(NumberOfTokens, currentTime)};" << endl;
    out << "In?Write{Status write(NumberOfTokens)};" << endl;
    out << "HandleInput()()." << endl << endl;

    out << "HandleOutput()() |NumberOfTokens: Integer|" << endl << endl;
    out << "Out?Inspect(NumberOfTokens | Status available(NumberOfTokens));" << endl;
    out << "Out?Read{Status remove(NumberOfTokens, currentTime)};" << endl;
    out << "HandleOutput()()." << endl << endl;

    // Generate Process Class ControlBuffer

    out << "process class ControlBuffer(BufferSize: Integer, NumbersInitialTokens: Queue, ContentInitialTokens: Queue, TokenSize: Integer, Monitor: Boolean, MonitorID: Integer, Trace: Boolean, Name: String)" << endl;
    out << "instance variables" << endl;
    out << "Status: ControlBufferMonitor" << endl << endl;

    if (!p4r)
    {
        out << "port interface" << endl;
        out << "Monitor, In, Out" << endl << endl;
        out << "message interface" << endl;
        out << "In?Room(Integer)," << endl;
        out << "In?Reserve," << endl;
        out << "In?Write(Integer)," << endl;
        out << "Out?Inspect," << endl;
        out << "Out!Scenario(Integer)," << endl;
        out << "Out?Read," << endl;
        out << "Monitor!Results(String)," << endl;
        out << "Monitor?Status(Integer)," << endl;
        out << "Monitor!AccuracyStatus(Integer, Boolean)," << endl;
        out << "Monitor?StopSimulation" << endl;
    }

    out << "initial method call" << endl;
    out << "Initialise()()" << endl << endl;
    out << "instance methods" << endl;

    out << "Initialise()()" << endl << endl;
    out << "Status := new(ControlBufferMonitor) init(BufferSize, NumbersInitialTokens, ContentInitialTokens, TokenSize, Monitor, MonitorID, Trace, Name);" << endl;
    out << "abort" << endl;
    out << "   par HandleInput()() and HandleOutput()() and if Monitor then CheckAccuracyStatus()() fi rap" << endl;
    out << "with Monitor?StopSimulation; if Monitor then Monitor!Results(Status getResults) fi." << endl << endl;

    out << "CheckAccuracyStatus()() |ID: Integer|" << endl << endl;
    out << "Monitor?Status(ID | ID = MonitorID);" << endl;
    out << "Monitor!AccuracyStatus(MonitorID, Status accurate);" << endl;
    out << "CheckAccuracyStatus()()." << endl << endl;

    out << "HandleInput()() |NumberOfTokens: Integer, Scenario: Integer|" << endl << endl;
    out << "In?Room(NumberOfTokens | Status room(NumberOfTokens));" << endl;
    out << "In?Reserve{Status reserve(NumberOfTokens, currentTime)};" << endl;
    out << "In?Write(Scenario){Status write(NumberOfTokens, Scenario)};" << endl;
    out << "HandleInput()()." << endl << endl;

    out << "HandleOutput()()" << endl << endl;
    out << "[Status available] Out?Inspect;" << endl;
    out << "Out!Scenario(Status inspect);" << endl;
    out << "Out?Read{Status remove(currentTime)};" << endl;
    out << "HandleOutput()()." << endl << endl;
}
