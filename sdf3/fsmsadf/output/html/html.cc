/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   html.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   December 6, 2009
 *
 *  Function        :   Output FSM-based SADF in HTML format
 *
 *  History         :
 *      06-12-09    :   Initial version.
 *
 * $Id: html.cc,v 1.4 2010-02-15 08:23:07 sander Exp $
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

#include "html.h"
#include <sys/stat.h>

// For VS, the mkdir function is defined in direct.h
#ifdef _MSC_VER
#include <direct.h>
#endif

#include "../../analysis/base/repetition_vector.h"

namespace FSMSADF
{

    /**
     * OutputHTML()
     * Constructor.
     */
    OutputHTML::OutputHTML()
    {
        dirname = "html";
    }

    /**
     * ~OutputHTML()
     * Destructor.
     */
    OutputHTML::~OutputHTML()
    {
    }

    /**
     * outputAsHTML()
     * Output the flow as a set of HTML files.
     */
    void OutputHTML::outputAsHTML(SDF3FlowBase *flow)
    {
        // Application graph
        printApplicationGraph(flow);

        // Platform graph
        printPlatformGraph(flow);

        // Platform bindings
        printPlatformBindings(flow);
    }

    /**
     * printApplicationGraph()
     * Print application graph.
     */
    void OutputHTML::printApplicationGraph(SDF3FlowBase *flow)
    {
        ofstream ofile;

        // Get the application graph
        Graph *applicationGraph = flow->getApplicationGraph();

        // Create an output file for the application graph
        createOutputFileApplicationGraph(applicationGraph, ofile);

        // Print the file header
        printHeader(ofile, flow);
        ofile << "<h2>Application graph: " << applicationGraph->getName();
        ofile << "</h2>" << endl;

        // Print the FSM
        printFSM(applicationGraph->getFSM(), ofile);

        // Print list of scenarios
        ofile << "<p>Scenarios: ";
        for (Scenarios::iterator i = applicationGraph->getScenarios().begin();
             i != applicationGraph->getScenarios().end(); i++)
        {
            Scenario *s = *i;

            ofile << "<a href='" << getScenarioURL(s) << "'>" << s->getName();
            ofile << "</a> ";
        }

        // Print each scenario
        for (Scenarios::iterator i = applicationGraph->getScenarios().begin();
             i != applicationGraph->getScenarios().end(); i++)
        {
            Scenario *s = *i;

            ofile << "<hr/>" << endl;
            printScenario(s, ofile);
        }

        // Print the file footer
        printFooter(ofile, flow);
    }

    /**
     * printPlatformGraph()
     * Print platform graph.
     */
    void OutputHTML::printPlatformGraph(SDF3FlowBase *flow)
    {
        ofstream ofile;
        CString map;

        // Get the platform graph
        PlatformGraph *pg = flow->getPlatformGraph();

        // Create an output file for the platform graph
        createOutputFilePlatformGraph(pg, ofile);

        // Print the file header
        printHeader(ofile, flow);
        ofile << "<h2>Platform graph: " << pg->getName() << "</h2>" << endl;

        // Insert picture of platform graph
        convertPlatformGraphToPNG(pg, getPlatformGraphPicURL(pg), map);
        ofile << "<p class='center'><img src='";
        ofile << getPlatformGraphPicURL(pg);
        ofile << "' usemap='#platform_graph_" << pg->getName().replace("-", "_");
        ofile << "'/></p>" << endl;
        ofile << endl;
        ofile << map << endl;

        // Print list of all tiles
        ofile << "<p>Tiles: ";
        for (Tiles::iterator i = pg->getTiles().begin();
             i != pg->getTiles().end(); i++)
        {
            Tile *t = *i;
            ofile << "<a href='" << getTileURL(t) << "'>";
            ofile << t->getName() << "</a>" << endl;
        }
        ofile << "</p>" << endl;

        // Print list of all connections
        ofile << "<p>Connections: ";
        for (Connections::iterator i = pg->getConnections().begin();
             i != pg->getConnections().end(); i++)
        {
            Connection *c = *i;
            ofile << "<a href='" << getConnectionURL(c) << "'>";
            ofile << c->getName() << "</a>" << endl;
        }
        ofile << "</p>" << endl;

        // Tile properties
        for (Tiles::iterator i = pg->getTiles().begin();
             i != pg->getTiles().end(); i++)
        {
            Tile *t = *i;

            ofile << "<a name='" << getTileURL(t, true) << "'></a>" << endl;
            ofile << "<h4>Tile properties: " << t->getName() << "</h4>" << endl;

            // Processors
            ofile << "<table>" << endl;
            ofile << "<tr>" << endl;
            ofile << "<th>Processor</th>" << endl;
            ofile << "<th>Type</th>" << endl;
            ofile << "<th>Arbitration type</th>" << endl;
            ofile << "<th>Wheelsize</th>" << endl;
            ofile << "<th>Context switch overhead</th>" << endl;
            ofile << "<th>Preemption?</th>" << endl;
            ofile << "</tr>" << endl;
            for (Processors::iterator j = t->getProcessors().begin();
                 j != t->getProcessors().end(); j++)
            {
                Processor *p = *j;

                ofile << "<tr>" << endl;
                ofile << "<td><a name='" << getProcessorURL(p, true) << "'></a>";
                ofile << p->getName() << "</td>" << endl;
                ofile << "<td>" << p->getType() << "</td>" << endl;
                ofile << "<td>" << p->getArbitrationType() << "</td>" << endl;
                ofile << "<td>" << p->getWheelsize() << "</td>" << endl;
                ofile << "<td>" << p->getContextSwitchOverhead() << "</td>" << endl;
                if (p->hasSupportForPreemption())
                    ofile << "<td>true</td>" << endl;
                else
                    ofile << "<td>false</td>" << endl;
                ofile << "</tr>" << endl;
            }
            ofile << "</table>" << endl;
            ofile << "<br/>" << endl;

            // Memories
            ofile << "<table>" << endl;
            ofile << "<tr>" << endl;
            ofile << "<th>Memory</th>" << endl;
            ofile << "<th>Size</th>" << endl;
            ofile << "</tr>" << endl;
            for (Memories::iterator j = t->getMemories().begin();
                 j != t->getMemories().end(); j++)
            {
                Memory *m = *j;

                ofile << "<tr>" << endl;
                ofile << "<td><a name='" << getMemoryURL(m, true) << "'></a>";
                ofile << m->getName() << "</td>" << endl;
                ofile << "<td>" << m->getSize() << "</td>" << endl;
                ofile << "</tr>" << endl;
            }
            ofile << "</table>" << endl;
            ofile << "<br/>" << endl;

            // Network interfaces
            ofile << "<table>" << endl;
            ofile << "<tr>" << endl;
            ofile << "<th>Network interface</th>" << endl;
            ofile << "<th>#incoming</th>" << endl;
            ofile << "<th>Bandwidth in</th>" << endl;
            ofile << "<th>#outgoing</th>" << endl;
            ofile << "<th>Bandwidth out</th>" << endl;
            ofile << "</tr>" << endl;
            for (NetworkInterfaces::iterator j = t->getNetworkInterfaces().begin();
                 j != t->getNetworkInterfaces().end(); j++)
            {
                NetworkInterface *n = *j;

                ofile << "<tr>" << endl;
                ofile << "<td><a name='" << getNetworkInterfaceURL(n, true) << "'></a>";
                ofile << n->getName() << "</td>" << endl;
                ofile << "<td>" << n->getMaxNrInConnections() << "</td>" << endl;
                ofile << "<td>" << n->getInBandwidth() << "</td>" << endl;
                ofile << "<td>" << n->getMaxNrOutConnections() << "</td>" << endl;
                ofile << "<td>" << n->getOutBandwidth() << "</td>" << endl;
                ofile << "</tr>" << endl;
            }
            ofile << "</table>" << endl;
            ofile << "<br/>" << endl;
        }

        // Connection properties
        ofile << "<h4>Connection properties</h4>" << endl;
        ofile << "<table>" << endl;
        ofile << "<tr>" << endl;
        ofile << "<th>Connection</th>" << endl;
        ofile << "<th>Src tile</th>" << endl;
        ofile << "<th>Src ni</th>" << endl;
        ofile << "<th>Dst tile</th>" << endl;
        ofile << "<th>Dst ni</th>" << endl;
        ofile << "<th>Latency</th>" << endl;
        ofile << "</tr>" << endl;
        for (Connections::iterator i = pg->getConnections().begin();
             i != pg->getConnections().end(); i++)
        {
            Connection *c = *i;
            NetworkInterface *src = c->getSrcNetworkInterface();
            NetworkInterface *dst = c->getDstNetworkInterface();

            ofile << "<tr>" << endl;
            ofile << "<td><a name='" << getConnectionURL(c, true) << "'></a>";
            ofile << c->getName() << "</td>" << endl;
            ofile << "<td><a href='" << getTileURL(src->getTile()) << "'>";
            ofile << src->getTile()->getName() << "</a></td>" << endl;
            ofile << "<td><a href='" << getNetworkInterfaceURL(src) << "'>";
            ofile << src->getName() << "</a></td>" << endl;
            ofile << "<td><a href='" << getTileURL(dst->getTile()) << "'>";
            ofile << dst->getTile()->getName() << "</a></td>" << endl;
            ofile << "<td><a href='" << getNetworkInterfaceURL(dst) << "'>";
            ofile << dst->getName() << "</a></td>" << endl;
            ofile << "<td>" << c->getLatency() << "</td>" << endl;
            ofile << "</tr>" << endl;
        }
        ofile << "</table>" << endl;

        // Print the file footer
        printFooter(ofile, flow);
    }

    /**
     * printPlatformBindings()
     * Print all platform bindings.
     */
    void OutputHTML::printPlatformBindings(SDF3FlowBase *flow)
    {
        PlatformBindings &platformBindings = flow->getPlatformBindings();
        PlatformGraph *pg = flow->getPlatformGraph();
        Graph *ag = flow->getApplicationGraph();
        ofstream ofile;

        // Compute repetition vector of each scenario
        map<Scenario *, RepetitionVector> repetitionVectors;
        for (Scenarios::iterator i = ag->getScenarios().begin();
             i != ag->getScenarios().end(); i++)
        {
            Scenario *s = *i;
            repetitionVectors[s] = computeRepetitionVector(s->getScenarioGraph(), s);
        }

        // Create an output file for the platform bindings overview
        createOutputFilePlatformBindings(ofile);

        // Print the file header
        printHeader(ofile, flow);
        ofile << "<h2>Platform bindings</h2>" << endl;

        ofile << "<p>The values printed in the table are the Pareto quantities ";
        ofile << "that are used at the end of the flow to determine Pareto ";
        ofile << "dominance. Please note that intermediate steps in the flow may ";
        ofile << "use different Pareto quantities to determine Pareto ";
        ofile << "dominance.</p>" << endl;

        // Print a list with all platform bindings
        ofile << "<table>" << endl;
        ofile << "<tr>" << endl;
        ofile << "<th>Binding</th>" << endl;
        for (Tiles::iterator i = pg->getTiles().begin();
             i != pg->getTiles().end(); i++)
        {
            Tile *t = *i;

            for (Processors::iterator j = t->getProcessors().begin();
                 j != t->getProcessors().end(); j++)
            {
                Processor *p = *j;

                ofile << "<th><a href='" << getTileURL(t) << "'>";
                ofile << t->getName() << "</a>.<a href='" << getProcessorURL(p);
                ofile << "'>" << p->getName() << "</a></th>" << endl;
            }
        }
        for (Tiles::iterator i = pg->getTiles().begin();
             i != pg->getTiles().end(); i++)
        {
            Tile *t = *i;

            for (Memories::iterator j = t->getMemories().begin();
                 j != t->getMemories().end(); j++)
            {
                Memory *m = *j;

                ofile << "<th><a href='" << getTileURL(t) << "'>";
                ofile << t->getName() << "</a>.<a href='" << getMemoryURL(m);
                ofile << "'>" << m->getName() << "</a></th>" << endl;
            }
        }
        for (Tiles::iterator i = pg->getTiles().begin();
             i != pg->getTiles().end(); i++)
        {
            Tile *t = *i;

            for (NetworkInterfaces::iterator j = t->getNetworkInterfaces().begin();
                 j != t->getNetworkInterfaces().end(); j++)
            {
                NetworkInterface *n = *j;

                ofile << "<th><a href='" << getTileURL(t) << "'>";
                ofile << t->getName() << "</a>.<a href='";
                ofile << getNetworkInterfaceURL(n);
                ofile << "'>" << n->getName() << "</a></th>" << endl;
            }
        }
        ofile << "</tr>" << endl;
        for (PlatformBindings::iterator i = platformBindings.begin();
             i != platformBindings.end(); i++)
        {
            PlatformBinding *pb = *i;

            // Pareto quantities
            GraphBindingBase gb = GraphBindingBase(pb);
            gb.computeParetoQuantities(repetitionVectors, true);
            const GraphBindingBase::ParetoQuantities pq = gb.getParetoQuantities();

            ofile << "<tr>" << endl;
            ofile << "<td><a href='" << getPlatformBindingURL(*i) << "'>";
            ofile << (*i)->getName() << "</a></td>" << endl;

            // Processors
            for (Tiles::iterator i = pg->getTiles().begin();
                 i != pg->getTiles().end(); i++)
            {
                Tile *t = *i;

                for (Processors::iterator j = t->getProcessors().begin();
                     j != t->getProcessors().end(); j++)
                {
                    Processor *p = *j;
                    map<Processor *, double>::const_iterator ppq;
                    ppq = pq.processingLoad.find(p);
                    // todo: provide nicer solution for Windows/Nix compatibility
#ifdef _MSC_VER
                    if (_isnan(ppq->second))
#else
                    if (isnan(ppq->second))
#endif
                    {
                        ofile << "<td>0</td>" << endl;
                    }
                    else
                    {
                        ofile << "<td>" << ppq->second;
                        ofile << "</td>" << endl;
                    }
                }
            }

            // Memories
            for (Tiles::iterator i = pg->getTiles().begin();
                 i != pg->getTiles().end(); i++)
            {
                Tile *t = *i;

                for (Memories::iterator j = t->getMemories().begin();
                     j != t->getMemories().end(); j++)
                {
                    Memory *m = *j;

                    ofile << "<td>" << pq.memoryLoad.find(m)->second << "</td>";
                    ofile << endl;
                }
            }

            // Communication load
            for (Tiles::iterator i = pg->getTiles().begin();
                 i != pg->getTiles().end(); i++)
            {
                Tile *t = *i;

                for (NetworkInterfaces::iterator j = t->getNetworkInterfaces().begin();
                     j != t->getNetworkInterfaces().end(); j++)
                {
                    NetworkInterface *n = *j;

                    ofile << "<td>" << pq.communicationLoad.find(n)->second;
                    ofile << "</td>" << endl;

                }

            }
            ofile << "</tr>" << endl;
        }
        ofile << "</table>" << endl;

        // Print the file footer
        printFooter(ofile, flow);

        // Print the individual platform bindings
        for (PlatformBindings::iterator i = platformBindings.begin();
             i != platformBindings.end(); i++)
        {
            printPlatformBinding(*i, flow);
        }
    }

    /**
     * printPlatformBinding()
     * Print platform binding.
     */
    void OutputHTML::printPlatformBinding(PlatformBinding *pb, SDF3FlowBase *flow)
    {
        ofstream ofile;
        CString map;

        // Application graph
        Graph *ag = pb->getApplicationGraph();
        PlatformGraph *pg = pb->getPlatformGraph();

        // Create an output file for the platform binding
        createOutputFilePlatformBinding(pb, ofile);

        // Print the file header
        printHeader(ofile, flow);
        ofile << "<h2>Platform bindings: " << pb->getName() << "</h2>" << endl;

        // General information
        ofile << "<p>Application graph: <a href='";
        ofile << getApplicationGraphURL(ag) << "'>";
        ofile << ag->getName() << "</a></p>" << endl;
        ofile << "<p>Platform graph: <a href='";
        ofile << getPlatformGraphURL(pg) << "'>";
        ofile << pg->getName() << "</a></p>" << endl;
        ofile << "<p>Scenarios: ";
        for (Scenarios::iterator i = ag->getScenarios().begin();
             i != ag->getScenarios().end(); i++)
        {
            Scenario *s = *i;

            ofile << "<a href='" << getScenarioPlatformBindingURL(s, pb);
            ofile << "'>" << s->getName() << "</a> ";
        }
        ofile << "</p>" << endl;

        // Resource usage
        ofile << "<h4>Resource usage</h4>" << endl;

        // Print resource usage of each processor
        ofile << "<table>" << endl;
        ofile << "<tr>" << endl;
        ofile << "<th>Processor</th>" << endl;
        ofile << "<th>Wheel usage</th>" << endl;
        ofile << "</tr>" << endl;
        for (TileBindings::iterator j = pb->getTileBindings().begin();
             j != pb->getTileBindings().end(); j++)
        {
            TileBinding *tb = *j;
            Tile *t = tb->getTile();

            // Processor bindings
            for (ProcessorBindings::iterator
                 k = tb->getProcessorBindings().begin();
                 k != tb->getProcessorBindings().end(); k++)
            {
                ProcessorBinding *pcb = *k;
                Processor *p = pcb->getProcessor();

                ofile << "<tr>" << endl;
                ofile << "<td><a href='" << getTileURL(t) << "'>";
                ofile << t->getName() << "</a>.<a href='";
                ofile << getProcessorURL(p) << "'>" << p->getName();
                ofile << "</a></td>" << endl;
                ofile << "<td>" << pcb->getWheelsizeUsedForGraph();
                ofile << " (";
                ofile << (100.0 * pcb->getWheelsizeUsedForGraph()
                          / p->getWheelsize());
                ofile << "%)" << "</td>" << endl;
                ofile << "</tr>" << endl;
            }
        }
        ofile << "</table>" << endl;
        ofile << "<br/>" << endl;

        // Print resource usage of each memory
        ofile << "<table>" << endl;
        ofile << "<tr>" << endl;
        ofile << "<th>Memory</th>" << endl;
        ofile << "<th>Usage</th>" << endl;
        ofile << "</tr>" << endl;

        // Memory bindings
        /// @todo reïmplement this with new code!
        /*
        for (TileBindings::iterator j = pb->getTileBindings().begin();
             j != pb->getTileBindings().end(); j++)
        {
            TileBinding *tb = *j;
            Tile *t = tb->getTile();

             for (MemoryBindings::iterator k = tb->getMemoryBindings().begin();
                    k != tb->getMemoryBindings().end(); k++)
            {
                MemoryBinding *mb = *k;
                Memory *m = mb->getMemory();

                ofile << "<tr>" << endl;
                ofile << "<td><a href='" << getTileURL(t) << "'>";
                ofile << t->getName() << "</a>.<a href='";
                ofile << getMemoryURL(m) << "'>" << m->getName();
                ofile << "</a></td>" << endl;
                ofile << "<td>" << mb->getMemorySizeUsedForGraph();
                ofile << " (";
                ofile << (100.0 * mb->getMemorySizeUsedForGraph() / m->getSize());
                ofile << "%)" << "</td>" << endl;
                ofile << "</tr>" << endl;
            }
        }
        * */
        ofile << "</table>" << endl;
        ofile << "<br/>" << endl;

        // Print resource usage of each network interface
        ofile << "<table>" << endl;
        ofile << "<tr>" << endl;
        ofile << "<th>Network interface</th>" << endl;
        ofile << "<th>#Incoming</th>" << endl;
        ofile << "<th>Bandwidth in</th>" << endl;
        ofile << "<th>#Outgoing</th>" << endl;
        ofile << "<th>Bandwidth out</th>" << endl;
        ofile << "</tr>" << endl;
        for (TileBindings::iterator j = pb->getTileBindings().begin();
             j != pb->getTileBindings().end(); j++)
        {
            TileBinding *tb = *j;
            Tile *t = tb->getTile();

            // Network interface bindings
            for (NetworkInterfaceBindings::iterator
                 k = tb->getNetworkInterfaceBindings().begin();
                 k != tb->getNetworkInterfaceBindings().end(); k++)
            {
                NetworkInterfaceBinding *nb = *k;
                NetworkInterface *n = nb->getNetworkInterface();

                ofile << "<tr>" << endl;
                ofile << "<td><a href='" << getTileURL(t) << "'>";
                ofile << t->getName() << "</a>.<a href='";
                ofile << getNetworkInterfaceURL(n) << "'>" << n->getName();
                ofile << "</a></td>" << endl;
                ofile << "<td>" << nb->getNrInConnectionsUsedForGraph();
                ofile << " (";
                ofile << (100.0 * nb->getNrInConnectionsUsedForGraph()
                          / n->getMaxNrInConnections());
                ofile << "%)" << "</td>" << endl;
                ofile << "<td>" << nb->getInBandwidthUsedForGraph();
                ofile << " (";
                ofile << (100.0 * nb->getInBandwidthUsedForGraph()
                          / n->getInBandwidth());
                ofile << "%)" << "</td>" << endl;
                ofile << "<td>" << nb->getNrOutConnectionsUsedForGraph();
                ofile << " (";
                ofile << (100.0 * nb->getNrOutConnectionsUsedForGraph()
                          / n->getMaxNrOutConnections());
                ofile << "%)" << "</td>" << endl;
                ofile << "<td>" << nb->getOutBandwidthUsedForGraph();
                ofile << " (";
                ofile << (100.0 * nb->getOutBandwidthUsedForGraph()
                          / n->getOutBandwidth());
                ofile << "%)" << "</td>" << endl;
                ofile << "</tr>" << endl;
            }
        }
        ofile << "</table>" << endl;
        ofile << "<br/>" << endl;

        // Scenarios
        for (Scenarios::iterator i = ag->getScenarios().begin();
             i != ag->getScenarios().end(); i++)
        {
            Scenario *s = *i;
            ScenarioGraph *sg = s->getScenarioGraph();
            bool hasBinding = false;

            // Scenario
            ofile << "<hr/>" << endl;
            ofile << "<a name='" << getScenarioPlatformBindingURL(s, pb);
            ofile << "'></a>" << endl;
            ofile << "<h4>Scenario: " << s->getName() << "</h4>" << endl;

            // Insert picture of scenario graph to platform graph binding
            convertPlatformBindingToPNG(s, pb, getPlatformBindingPicURL(s, pb), map);
            ofile << "<p class='center'><img src='";
            ofile << getPlatformBindingPicURL(s, pb);
            ofile << "' usemap='#platform_binding_" << s->getName() << "_";
            ofile << pb->getName().replace("-", "_");
            ofile << "'/></p>" << endl;
            ofile << endl;
            ofile << map << endl;

            // Print resource usage of each processor
            ofile << "<table>" << endl;
            ofile << "<tr>" << endl;
            ofile << "<th>Processor</th>" << endl;
            ofile << "<th>Wheel usage</th>" << endl;
            ofile << "<th>Schedule</th>" << endl;
            ofile << "</tr>" << endl;
            for (TileBindings::iterator j = pb->getTileBindings().begin();
                 j != pb->getTileBindings().end(); j++)
            {
                TileBinding *tb = *j;
                Tile *t = tb->getTile();

                // Processor bindings
                for (ProcessorBindings::iterator
                     k = tb->getProcessorBindings().begin();
                     k != tb->getProcessorBindings().end(); k++)
                {
                    ProcessorBinding *pcb = *k;
                    Processor *p = pcb->getProcessor();

                    ofile << "<tr>" << endl;
                    ofile << "<td><a href='" << getTileURL(t) << "'>";
                    ofile << t->getName() << "</a>.<a href='";
                    ofile << getProcessorURL(p) << "'>" << p->getName();
                    ofile << "</a></td>" << endl;
                    ofile << "<td>" << pcb->getAllocatedWheelsize(s);
                    ofile << " (";
                    ofile << (100.0 * pcb->getAllocatedWheelsize(s)
                              / p->getWheelsize());
                    ofile << "%)" << "</td>" << endl;
                    ofile << "<td>";
                    if (pcb->hasStaticOrderSchedule(s))
                    {
                        StaticOrderSchedule &so = pcb->getStaticOrderSchedule(s);
                        for (uint l = 0; l < so.size(); l++)
                        {
                            Actor *a = so[l].actor;

                            if (l == so.getStartPeriodicSchedule())
                                ofile << " (";

                            ofile << " <a href='" << getActorURL(s, a) << "'>";
                            ofile << a->getName() << "</a>";
                        }
                        if (so.size() > 0)
                            ofile << " )";
                    }
                    ofile << "</td>" << endl;
                    ofile << "</tr>" << endl;
                }
            }
            ofile << "</table>" << endl;
            ofile << "<br/>" << endl;

            // Print resource usage of each memory
            ofile << "<table>" << endl;
            ofile << "<tr>" << endl;
            ofile << "<th>Memory</th>" << endl;
            ofile << "<th>Usage</th>" << endl;
            ofile << "<th>Actors</th>" << endl;
            ofile << "<th>Channels</th>" << endl;
            ofile << "</tr>" << endl;
            // Memory bindings
            /*
            for (TileBindings::iterator j = pb->getTileBindings().begin();
                 j != pb->getTileBindings().end(); j++)
            {
                TileBinding *tb = *j;
                Tile *t = tb->getTile();

                for (MemoryBindings::iterator k = tb->getMemoryBindings().begin();
                        k != tb->getMemoryBindings().end(); k++)
                {
                    MemoryBinding *mb = *k;
                    Memory *m = mb->getMemory();

                    ofile << "<tr>" << endl;
                    ofile << "<td><a href='" << getTileURL(t) << "'>";
                    ofile << t->getName() << "</a>.<a href='";
                    ofile << getMemoryURL(m) << "'>" << m->getName();
                    ofile << "</a></td>" << endl;
                    ofile << "<td>" << mb->getAllocatedMemorySize(s);
                    ofile << " (";
                    ofile << (100.0 * mb->getAllocatedMemorySize(s) / m->getSize());
                    ofile << "%)" << "</td>" << endl;
                    ofile << "<td>";
                    for (Actors::iterator l = sg->getActors().begin();
                            l != sg->getActors().end(); l++)
                    {
                        Actor *a = *l;

                        // Actor bound to memory in scenario?
                        if (mb->hasActorBinding(s, a))
                        {
                            ofile << "<a href='" << getActorURL(s, a);
                            ofile << "'>" << a->getName() << "</a> " << endl;
                        }
                    }

                    ofile << "</td>" << endl;
                    ofile << "<td>";
                    for (Channels::iterator l = sg->getChannels().begin();
                            l != sg->getChannels().end(); l++)
                    {
                        Channel *c = *l;

                        // Channel bound to memory in scenario?
                        if (mb->hasChannelBinding(s, c))
                        {
                            ofile << "<a href='" << getChannelURL(s, c);
                            ofile << "'>" << c->getName() << "</a> " << endl;
                        }
                    }
                    ofile << "</td>" << endl;
                    ofile << "</tr>" << endl;
                }
            }
            * */
            ofile << "</table>" << endl;
            ofile << "<br/>" << endl;

            // Print resource usage of each network interface
            ofile << "<table>" << endl;
            ofile << "<tr>" << endl;
            ofile << "<th>Network interface</th>" << endl;
            ofile << "<th>#Incoming</th>" << endl;
            ofile << "<th>Bandwidth in</th>" << endl;
            ofile << "<th>Channels in</th>" << endl;
            ofile << "<th>#Outgoing</th>" << endl;
            ofile << "<th>Bandwidth out</th>" << endl;
            ofile << "<th>Channels out</th>" << endl;
            ofile << "</tr>" << endl;
            for (TileBindings::iterator j = pb->getTileBindings().begin();
                 j != pb->getTileBindings().end(); j++)
            {
                TileBinding *tb = *j;
                Tile *t = tb->getTile();

                // Network interface bindings
                for (NetworkInterfaceBindings::iterator
                     k = tb->getNetworkInterfaceBindings().begin();
                     k != tb->getNetworkInterfaceBindings().end(); k++)
                {
                    NetworkInterfaceBinding *nb = *k;
                    NetworkInterface *n = nb->getNetworkInterface();

                    ofile << "<tr>" << endl;
                    ofile << "<td><a href='" << getTileURL(t) << "'>";
                    ofile << t->getName() << "</a>.<a href='";
                    ofile << getNetworkInterfaceURL(n) << "'>" << n->getName();
                    ofile << "</a></td>" << endl;
                    ofile << "<td>" << nb->getAllocatedNrInConnections(s);
                    ofile << " (";
                    ofile << (100.0 * nb->getAllocatedNrInConnections(s)
                              / n->getMaxNrInConnections());
                    ofile << "%)" << "</td>" << endl;
                    ofile << "<td>" << nb->getAllocatedBandwidthInConnections(s);
                    ofile << " (";
                    ofile << (100.0 * nb->getAllocatedBandwidthInConnections(s)
                              / n->getInBandwidth());
                    ofile << "%)" << "</td>" << endl;
                    ofile << "<td>";
                    for (Channels::iterator l = sg->getChannels().begin();
                         l != sg->getChannels().end(); l++)
                    {
                        Channel *c = *l;

                        // Channel bound to network interface in scenario?
                        if (nb->hasInChannelBinding(s, c))
                        {
                            ofile << "<a href='" << getChannelURL(s, c);
                            ofile << "'>" << c->getName() << "</a> " << endl;
                        }
                    }
                    ofile << "</td>" << endl;
                    ofile << "<td>" << nb->getAllocatedNrOutConnections(s);
                    ofile << " (";
                    ofile << (100.0 * nb->getAllocatedNrOutConnections(s)
                              / n->getMaxNrOutConnections());
                    ofile << "%)" << "</td>" << endl;
                    ofile << "<td>" << nb->getAllocatedBandwidthOutConnections(s);
                    ofile << " (";
                    ofile << (100.0 * nb->getAllocatedBandwidthOutConnections(s)
                              / n->getOutBandwidth());
                    ofile << "%)" << "</td>" << endl;
                    ofile << "<td>";
                    for (Channels::iterator l = sg->getChannels().begin();
                         l != sg->getChannels().end(); l++)
                    {
                        Channel *c = *l;

                        // Channel bound to network interface in scenario?
                        if (nb->hasOutChannelBinding(s, c))
                        {
                            ofile << "<a href='" << getChannelURL(s, c);
                            ofile << "'>" << c->getName() << "</a> " << endl;
                        }
                    }
                    ofile << "</td>" << endl;
                    ofile << "</tr>" << endl;
                }
            }
            ofile << "</table>" << endl;
            ofile << "<br/>" << endl;

            // Print channel to connection bindings
            ofile << "<table>" << endl;
            ofile << "<tr>" << endl;
            ofile << "<th>Connection</th>" << endl;
            ofile << "<th>Channel</th>" << endl;
            ofile << "</tr>" << endl;
            for (ConnectionBindings::iterator j = pb->getConnectionBindings().begin();
                 j != pb->getConnectionBindings().end(); j++)
            {
                ConnectionBinding *cb = *j;

                // Iterate over all channels
                for (Channels::iterator k = sg->getChannels().begin();
                     k != sg->getChannels().end(); k++)
                {
                    Channel *c = *k;

                    // Channel bound to this connection in this scenario?
                    if (cb->hasChannelBinding(s, c))
                    {
                        ofile << "<tr>" << endl;
                        ofile << "<td><a href='";
                        ofile << getConnectionURL(cb->getConnection());
                        ofile << "'>" << cb->getConnection()->getName();
                        ofile << "</a></td>" << endl;
                        ofile << "<td><a href='" << getChannelURL(s, c);
                        ofile << "'>" << c->getName() << "</a></td>" << endl;
                        ofile << "</tr>" << endl;

                        // Found a binding
                        hasBinding = true;
                    }
                }
            }
            if (!hasBinding)
            {
                // Print message that no binding exists
                ofile << "<tr>" << endl;
                ofile << "<td>(no binding)</td>" << endl;
                ofile << "<td>(no binding)</td>" << endl;
                ofile << "</tr>" << endl;
            }
            ofile << "</table>" << endl;
        }

        // Print the file footer
        printFooter(ofile, flow);
    }

    /**
     * createOutputFileApplicationGraph()
     * Create a file to output the application graph.
     */
    void OutputHTML::createOutputFileApplicationGraph(Graph *applicationGraph,
            ofstream &out)
    {
        createOutputFile(getApplicationGraphURL(applicationGraph), out);
    }

    /**
     * createOutputFilePlatformGraph()
     * Create a file to output the platform graph.
     */
    void OutputHTML::createOutputFilePlatformGraph(PlatformGraph *platformGraph,
            ofstream &out)
    {
        createOutputFile(getPlatformGraphURL(platformGraph), out);
    }

    /**
     * createOutputFilePlatformBinding()
     * Create a file to output the platform binding.
     */
    void OutputHTML::createOutputFilePlatformBinding(
        PlatformBinding *platformBinding, ofstream &out)
    {
        createOutputFile(getPlatformBindingURL(platformBinding), out);
    }

    /**
     * createOutputFilePlatformBindings()
     * Create a file to output the platform bindings overview.
     */
    void OutputHTML::createOutputFilePlatformBindings(ofstream &out)
    {
        createOutputFile(getPlatformBindingsURL(), out);
    }

    /**
     * createOutputFile()
     * Create an output file with the supplied name.
     */
    void OutputHTML::createOutputFile(const CString &name, ofstream &out)
    {
        CString filename = getDirname() + "/" + name;

        // Create output directory
        createOutputDirectory();

        // Create output file
        out.open(filename);
        if (!out.is_open())
            throw CException("[ERROR] Failed creating '" + filename + "'");
    }

    /**
     * createOutputDirectory()
     * Create directory for HTML outpu
     */
    void OutputHTML::createOutputDirectory()
    {
#ifdef _MSC_VER
        _mkdir(dirname.c_str());
#else
        mkdir(dirname.c_str(), S_IRUSR | S_IWUSR | S_IXUSR);
#endif

    }

    /**
     * printScenario()
     * Print the scenario in HTML format to the supplied output stream.
     */
    void OutputHTML::printScenario(Scenario *s, ostream &ofile)
    {
        CString map;

        // Scenario graph
        ScenarioGraph *sg = s->getScenarioGraph();

        // Section heading
        ofile << "<a name='" << getScenarioURL(s, true) << "'></a>" << endl;
        ofile << "<h3>Scenario: " << s->getName() << "</h3>" << endl;

        // Scenario graph
        ofile << "<p>Scenario graph: " << sg->getName() << endl;

        // Insert picture of scenario graph
        convertScenarioGraphToPNG(s, sg, getScenarioGraphPicURL(s), map);
        ofile << "<p class='center'><img src='";
        ofile << getScenarioGraphPicURL(s);
        ofile << "' usemap='#scenario_graph_" << s->getName() << "'/></p>" << endl;
        ofile << endl;
        ofile << map << endl;

        // Print list of all actors
        ofile << "<p>Actors: ";
        for (Actors::iterator i = sg->getActors().begin();
             i != sg->getActors().end(); i++)
        {
            Actor *a = *i;
            ofile << "<a href='" << getActorURL(s, a) << "'>";
            ofile << a->getName() << "</a>" << endl;
        }
        ofile << "</p>" << endl;

        // Print list of all channels
        ofile << "<p>Channels: ";
        for (Channels::iterator i = sg->getChannels().begin();
             i != sg->getChannels().end(); i++)
        {
            Channel *c = *i;
            ofile << "<a href='" << getChannelURL(s, c) << "'>";
            ofile << c->getName() << "</a>" << endl;
        }
        ofile << "</p>" << endl;

        ofile << "<h4>Actor properties</h4>" << endl;
        ofile << "<table>" << endl;
        ofile << "<tr>";
        ofile << "<th>Actor</th>" << endl;
        ofile << "<th>Processor type</th>" << endl;
        ofile << "<th>Execution time</th>" << endl;
        ofile << "<th>State size</th>" << endl;
        ofile << "<th>Default?</th>" << endl;
        ofile << "</tr>" << endl;

        // Print properties of all actors
        for (Actors::iterator i = sg->getActors().begin();
             i != sg->getActors().end(); i++)
        {
            Actor *a = *i;

            for (Actor::ProcessorTypes::const_iterator j = a->getProcessorTypes().begin();
                 j != a->getProcessorTypes().end(); j++)
            {
                const Actor::ProcessorType &t = *j;

                ofile << "<tr>" << endl;
                ofile << "<td>";
                ofile << "<a name='" << getActorURL(s, a, true) << "'></a>";
                ofile << a->getName() << "</td>" << endl;
                ofile << "<td>" << t.type << "</td>" << endl;
                ofile << "<td>" << a->getExecutionTimeOfScenario(s, t.type) << "</td>" << endl;
                /// @todo: print out memory size (instead of state size)
                ofile << "<td/>" << endl;
                //ofile << "<td>" << a->getStateSizeOfScenario(s, t.type) << "</td>" << endl;
                if (t.type == a->getDefaultProcessorType())
                    ofile << "<td>true</td>" << endl;
                else
                    ofile << "<td>false</td>" << endl;
                ofile << "</tr>" << endl;
            }
        }

        ofile << "</table>" << endl;
        ofile << "<h4>Channel properties</h4>" << endl;
        ofile << "<table>" << endl;
        ofile << "<tr>";
        ofile << "<th>Channel</th>" << endl;
        ofile << "<th>Src actor</th>" << endl;
        ofile << "<th>Src port</th>" << endl;
        ofile << "<th>Src rate</th>" << endl;
        ofile << "<th>Dst actor</th>" << endl;
        ofile << "<th>Dst port</th>" << endl;
        ofile << "<th>Dst rate</th>" << endl;
        ofile << "<th>Initial tokens</th>" << endl;
        ofile << "<th>Token size</th>" << endl;
        ofile << "</tr>" << endl;

        // Print properties of all channels
        for (Channels::iterator i = sg->getChannels().begin();
             i != sg->getChannels().end(); i++)
        {
            Channel *c = *i;

            // Properties
            ofile << "<tr>" << endl;
            ofile << "<td>";
            ofile << "<a name='" << getChannelURL(s, c, true) << "'></a>";
            ofile << c->getName();
            ofile << "</td>" << endl;
            ofile << "<td><a href='" << getActorURL(s, c->getSrcActor());
            ofile << "'>" << c->getSrcActor()->getName() << "</a></td>" << endl;
            ofile << "<td><a href='" << getActorURL(s, c->getSrcActor());
            ofile << "'>" << c->getSrcPort()->getName() << "</a></td>" << endl;
            ofile << "<td>" << c->getSrcPort()->getRateOfScenario(s) << "</td>" << endl;
            ofile << "<td><a href='" << getActorURL(s, c->getDstActor());
            ofile << "'>" << c->getDstActor()->getName() << "</a></td>" << endl;
            ofile << "<td><a href='" << getActorURL(s, c->getDstActor());
            ofile << "'>" << c->getDstPort()->getName() << "</a></td>" << endl;
            ofile << "<td>" << c->getDstPort()->getRateOfScenario(s) << "</td>" << endl;
            ofile << "<td>" << c->getInitialTokens() << "</td>" << endl;
            ofile << "<td>" << c->getTokenSizeOfScenario(s) << "</td>" << endl;
            ofile << "</tr>" << endl;
        }
        ofile << "</table>" << endl;
    }

    /**
     * printFSM()
     * Print the FSM to the supplied stream.
     */
    void OutputHTML::printFSM(FSM *fsm, ostream &ofile)
    {
        CString map;

        ofile << "<h4>FSM</h4>" << endl;
        convertFSMToPNG(fsm, getFSMPicURL(), map);
        ofile << "<p class='center'><img src='" << getFSMPicURL();
        ofile << "' usemap='#fsm'/></p>" << endl;
        ofile << map << endl;
    }

    /**
     * printHeader()
     * Print the HTML header to the supplied output stream.
     */
    void OutputHTML::printHeader(ostream &ofile, SDF3FlowBase *flow)
    {
        ofile << "<!DOCTYPE html PUBLIC '-//W3C//DTD XHTML 1.0 Strict//EN' ";
        ofile << "'http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd'>" << endl;
        ofile << "<html xmlns='http://www.w3.org/1999/xhtml'>" << endl;
        ofile << "<head>" << endl;
        ofile << "    <title>SDF3</title>" << endl;
        ofile << "    <meta http-equiv='Content-type' content='text/html;charset=iso-8859-1' />" << endl;
        ofile << "    <style>" << endl;
        printStylesheet(ofile);
        ofile << "    </style>" << endl;
        ofile << "</head>" << endl;
        ofile << "<body>" << endl;
        ofile << "<a name='top'></a>" << endl;
        ofile << "<div id='mainone'>" << endl;
        ofile << endl;
        ofile << "    <!-- header -->" << endl;
        ofile << "    <div id='header'>" << endl;
        ofile << "        <a href='http://www.es.ele.tue.nl/sdf3'>SDF3</a>" << endl;
        ofile << "    </div>" << endl;
        ofile << endl;
        ofile << "    <!-- navigation -->" << endl;
        ofile << "    <div id='nav-main'>" << endl;
        ofile << "        <ul id='nav-main-list'>" << endl;
        ofile << "            <li><a href='";
        ofile << getApplicationGraphURL(flow->getApplicationGraph());
        ofile << "'>Application Graph</a></li>" << endl;
        ofile << "            <li><a href='";
        ofile << getPlatformGraphURL(flow->getPlatformGraph());
        ofile << "'>Platform Graph</a></li>" << endl;
        ofile << "            <li><a href='";
        ofile << getPlatformBindingsURL();
        ofile << "'>Platform Bindings</a></li>" << endl;
        ofile << "        </ul>" << endl;
        ofile << "    </div>" << endl;
        ofile << endl;
        ofile << "    <div id='content'>";
    }

    /**
     * printFooter()
     * Print the HTML footer to the supplied output stream.
     */
    void OutputHTML::printFooter(ostream &ofile, SDF3FlowBase *flow)
    {
        ofile << "    </div>" << endl;
        ofile << "</div>" << endl;
        ofile << "</body>" << endl;
        ofile << "</html>" << endl;
    }

    /**
     * printStylesheet ()
     * Write the sylesheet to the supplied output stream.
     */
    void OutputHTML::printStylesheet(ostream &ofile)
    {
        ofile << "        #header a {" << endl;
        ofile << "            text-decoration: none;" << endl;
        ofile << "            color: #6889b5;" << endl;
        ofile << "            font-size: 20pt;" << endl;
        ofile << "            font-weight: bold;" << endl;
        ofile << "        }" << endl;
        ofile << "        " << endl;
        ofile << "        #nav-main {" << endl;
        ofile << "            background-color: #4f6889;" << endl;
        ofile << "            width: 100%;" << endl;
        ofile << "            font-weight: bold;" << endl;
        ofile << "            font-family: 'Luxi Sans', verdana, helvetica, arial, sans-serif;" << endl;
        ofile << "            font-size: x-small;" << endl;
        ofile << "            margin-top: 5px;" << endl;
        ofile << "        }" << endl;
        ofile << "        " << endl;
        ofile << "        #nav-main-list {" << endl;
        ofile << "            height: 20px; /* size of navigation bar */" << endl;
        ofile << "            list-style: none;" << endl;
        ofile << "            margin: 0px;" << endl;
        ofile << "            padding: 0px;" << endl;
        ofile << "        }" << endl;
        ofile << "        " << endl;
        ofile << "        #nav-main-list li {" << endl;
        ofile << "            float: left;" << endl;
        ofile << "            display: inline;" << endl;
        ofile << "        }" << endl;
        ofile << "        " << endl;
        ofile << "        #nav-main-list a {" << endl;
        ofile << "            color: #fff;" << endl;
        ofile << "            text-decoration: none;" << endl;
        ofile << "            vertical-align: middle;" << endl;
        ofile << "            padding-left: 10px;" << endl;
        ofile << "            padding-right: 10px;" << endl;
        ofile << "            line-height: 20px;" << endl;
        ofile << "            display: block;" << endl;
        ofile << "        }" << endl;
        ofile << "         " << endl;
        ofile << "        #nav-main-list a:hover {" << endl;
        ofile << "            background-color: #7ca3d6;" << endl;
        ofile << "        } " << endl;
        ofile << "        " << endl;
        ofile << "        #nav-main-list li strong{" << endl;
        ofile << "            background-color: #6889b5;" << endl;
        ofile << "            display: block;" << endl;
        ofile << "        } " << endl;
        ofile << "        " << endl;
        ofile << "        * html .nav-corner-br {" << endl;
        ofile << "          margin-top: -12px;" << endl;
        ofile << "        }" << endl;
        ofile << "        " << endl;
        ofile << "        * html .nav-corner-bl {" << endl;
        ofile << "          margin-top: -12px;" << endl;
        ofile << "        }" << endl;
        ofile << "        " << endl;
        ofile << "        body {" << endl;
        ofile << "          margin: 0;" << endl;
        ofile << "          padding: 0;" << endl;
        ofile << "          text-align: center;" << endl;
        ofile << "          font-family: 'Luxi Sans', 'Bitstream Vera Sans', ";
        ofile << "'Lucida Grande', 'Trebuchet MS', helvetica, verdana, arial, sans-serif;" << endl;
        ofile << "          font-size: small;" << endl;
        ofile << "          color: #333;" << endl;
        ofile << "        }" << endl;
        ofile << "        " << endl;
        ofile << "        a {" << endl;
        ofile << "          text-decoration: none;" << endl;
        ofile << "          color: #333;" << endl;
        ofile << "        }" << endl;
        ofile << "        " << endl;
        ofile << "        a:hover {" << endl;
        ofile << "          text-decoration: underline;" << endl;
        ofile << "        }" << endl;
        ofile << "        " << endl;
        ofile << "        #mainone {" << endl;
        ofile << "            min-width: 720px;" << endl;
        ofile << "            padding-left: 30px;" << endl;
        ofile << "            padding-right: 30px;" << endl;
        ofile << "            margin-left: auto;" << endl;
        ofile << "            margin-right: auto;" << endl;
        ofile << "            text-align: left;" << endl;
        ofile << "        }" << endl;
        ofile << "        " << endl;
        ofile << "        #site-left {" << endl;
        ofile << "            width: 120px;" << endl;
        ofile << "            float: left;" << endl;
        ofile << "            margin-right: 20px;" << endl;
        ofile << "        }" << endl;
        ofile << "        " << endl;
        ofile << "        #site-middle-two {" << endl;
        ofile << "            margin-left: 140px;  /*width of site-left block */" << endl;
        ofile << "        }" << endl;
        ofile << "        " << endl;
        ofile << "        #content {" << endl;
        ofile << "            margin: 10px;" << endl;
        ofile << "        }" << endl;
        ofile << "        " << endl;
        ofile << "        img {" << endl;
        ofile << "            border: 0px;" << endl;
        ofile << "        }" << endl;
        ofile << "        " << endl;
        ofile << "        div.center {" << endl;
        ofile << "            text-align: center;" << endl;
        ofile << "        }" << endl;
        ofile << "        " << endl;
        ofile << "        .command {" << endl;
        ofile << "            font-family: 'Courier';" << endl;
        ofile << "        }" << endl;
        ofile << "        " << endl;
        ofile << "        div.command {" << endl;
        ofile << "            margin-top: 10px;" << endl;
        ofile << "            margin-bottom: 10px;" << endl;
        ofile << "            white-space: pre;" << endl;
        ofile << "        }" << endl;
        ofile << "        " << endl;
        ofile << "        div.example {" << endl;
        ofile << "          display: block;" << endl;
        ofile << "          padding: 10px;" << endl;
        ofile << "          border: 1px solid #6666cc;" << endl;
        ofile << "          color: #000;" << endl;
        ofile << "          overflow: auto;" << endl;
        ofile << "          margin: 1em 2em;" << endl;
        ofile << "        }" << endl;
        ofile << "        " << endl;
        ofile << "        code.screen, pre.screen {" << endl;
        ofile << "            font-family: 'Courier';" << endl;
        ofile << "          display: block;" << endl;
        ofile << "          padding: 10px;" << endl;
        ofile << "          border: 1px solid #bbb;" << endl;
        ofile << "          background-color: #eee;" << endl;
        ofile << "          color: #000;" << endl;
        ofile << "          overflow: auto;" << endl;
        ofile << "          margin: 1em 2em;" << endl;
        ofile << "        }" << endl;
        ofile << "        " << endl;
        ofile << "        pre {" << endl;
        ofile << "            font-size: 10pt;" << endl;
        ofile << "        }" << endl;
        ofile << "        " << endl;
        ofile << "        img.right {" << endl;
        ofile << "            display: block;" << endl;
        ofile << "            float: right;" << endl;
        ofile << "            margin: 10px;" << endl;
        ofile << "        }" << endl;
        ofile << "        " << endl;
        ofile << "        img.left {" << endl;
        ofile << "            display: block;" << endl;
        ofile << "            float: left;" << endl;
        ofile << "            margin: 10px;" << endl;
        ofile << "        }" << endl;
        ofile << "        " << endl;
        ofile << "        p.right {" << endl;
        ofile << "            text-align: right;" << endl;
        ofile << "        }" << endl;
        ofile << "        " << endl;
        ofile << "        div.caution {" << endl;
        ofile << "          display: block;" << endl;
        ofile << "          padding: 10px;" << endl;
        ofile << "          border: 1px solid #6666cc;" << endl;
        ofile << "          background-color: #ccccff;" << endl;
        ofile << "          color: #000;" << endl;
        ofile << "          overflow: auto;" << endl;
        ofile << "          margin: 1em 2em;" << endl;
        ofile << "        }" << endl;
        ofile << "        " << endl;
        ofile << "        div.info {" << endl;
        ofile << "          display: block;" << endl;
        ofile << "          padding: 10px;" << endl;
        ofile << "          border: 1px solid #6666cc;" << endl;
        ofile << "          background-color: #ccccff;" << endl;
        ofile << "          color: #000;" << endl;
        ofile << "          overflow: auto;" << endl;
        ofile << "          margin: 1em 2em;" << endl;
        ofile << "        }" << endl;
        ofile << "        " << endl;
        ofile << "        th {" << endl;
        ofile << "            color: #4f6889;" << endl;
        ofile << "        }" << endl;
        ofile << "        " << endl;
        ofile << "        .header {" << endl;
        ofile << "            color: #4f6889;" << endl;
        ofile << "        }" << endl;
        ofile << "        " << endl;
        ofile << "        ul.separate li {" << endl;
        ofile << "            margin-bottom: 10px;" << endl;
        ofile << "        }" << endl;
        ofile << "        " << endl;
        ofile << "        ol.separate li {" << endl;
        ofile << "            margin-bottom: 10px;" << endl;
        ofile << "        }" << endl;
        ofile << "        " << endl;
        ofile << "        li.obsolete {" << endl;
        ofile << "            color: #aaa;" << endl;
        ofile << "        }" << endl;
        ofile << "        " << endl;
        ofile << "        div.tabs {" << endl;
        ofile << "            width: 100%;" << endl;
        ofile << "            height: 20px; /* size of navigation bar */" << endl;
        ofile << "            list-style: none;" << endl;
        ofile << "            margin: 0px;" << endl;
        ofile << "            padding: 0px;" << endl;
        ofile << "            " << endl;
        ofile << "        }" << endl;
        ofile << "        " << endl;
        ofile << "        div.tabs ul {" << endl;
        ofile << "            margin: 0px;" << endl;
        ofile << "            padding: 0px;" << endl;
        ofile << "        }" << endl;
        ofile << "        " << endl;
        ofile << "        div.tabs li {" << endl;
        ofile << "            float: left;" << endl;
        ofile << "            display: inline;" << endl;
        ofile << "        }" << endl;
        ofile << "        " << endl;
        ofile << "        div.tabs a {" << endl;
        ofile << "            color: #6f6f6f;" << endl;
        ofile << "            text-decoration: none;" << endl;
        ofile << "            padding-left: 0px;" << endl;
        ofile << "            padding-right: 10px;" << endl;
        ofile << "            line-height: 20px;" << endl;
        ofile << "            display: block;" << endl;
        ofile << "        }" << endl;
        ofile << "         " << endl;
        ofile << "        div.tabs a:hover {" << endl;
        ofile << "            text-decoration: underline;" << endl;
        ofile << "        } " << endl;
        ofile << "        " << endl;
        ofile << "        #current{" << endl;
        ofile << "            font-weight: bold;" << endl;
        ofile << "            display: block;" << endl;
        ofile << "        } " << endl;
        ofile << "        " << endl;
        ofile << "        .center{" << endl;
        ofile << "            text-align: center;" << endl;
        ofile << "        } " << endl;
        ofile << "        td {" << endl;
        ofile << "            padding-right: 30px;" << endl;
        ofile << "        }" << endl;
        ofile << "        " << endl;
        ofile << "        th {" << endl;
        ofile << "            padding-right: 30px;" << endl;
        ofile << "        }" << endl;
        ofile << "        .footer {" << endl;
        ofile << "          font-size: x-small;" << endl;
        ofile << "            color: #777;" << endl;
        ofile << "            padding-top: 50px;" << endl;
        ofile << "            padding-bottom: 30px;" << endl;
        ofile << "            clear: both;" << endl;
        ofile << "        }" << endl;
        ofile << "        " << endl;
        ofile << "        a.footer {" << endl;
        ofile << "            text-decoration: none;" << endl;
        ofile << "            color: #666699;" << endl;
        ofile << "        }" << endl;
        ofile << "        " << endl;
        ofile << "        a.footer:visited {" << endl;
        ofile << "            color: #666699;" << endl;
        ofile << "        }" << endl;
        ofile << "        " << endl;
        ofile << "        a.footer:hover {" << endl;
        ofile << "            text-decoration: underline;" << endl;
        ofile << "            color: #666699;" << endl;
        ofile << "        }" << endl;
    }

    /**
     * getApplicationGraphURL()
     * Get an URL for the application graph.
     */
    CString OutputHTML::getApplicationGraphURL(Graph *applicationGraph)
    {
        return CString("application-graph-" + applicationGraph->getName() + ".html");
    }

    /**
     * getScenarioURL()
     * Get an URL for the scenario.
     */
    CString OutputHTML::getScenarioURL(Scenario *s, bool onlyName)
    {
        if (onlyName)
            return "sc-" + s->getName();

        return getApplicationGraphURL(s->getGraph()) + "#sc-" + s->getName();
    }

    /**
     * getActorURL()
     * Get an URL for the actor.
     */
    CString OutputHTML::getActorURL(Scenario *s, Actor *a, bool onlyName)
    {
        return getScenarioURL(s, onlyName) + "-ac-" + a->getName();
    }

    /**
     * getChannelURL()
     * Get an URL for the channel.
     */
    CString OutputHTML::getChannelURL(Scenario *s, Channel *c, bool onlyName)
    {
        return getScenarioURL(s, onlyName) + "-ch-" + c->getName();
    }

    /**
     * getPlatformGraphURL()
     * Get an URL for the platform graph.
     */
    CString OutputHTML::getPlatformGraphURL(PlatformGraph *platformGraph)
    {
        return CString("platform-graph-" + platformGraph->getName() + ".html");
    }

    /**
     * getTileURL()
     * Get an URL for the tile.
     */
    CString OutputHTML::getTileURL(Tile *t, bool onlyName)
    {
        if (onlyName)
            return "tile-" + t->getName();

        return getPlatformGraphURL((PlatformGraph *)(t->getParent()))
               + "#tile-" + t->getName();
    }

    /**
     * getNetworkInterfaceURL()
     * Get an URL for the network interface.
     */
    CString OutputHTML::getNetworkInterfaceURL(NetworkInterface *n, bool onlyName)
    {
        return getTileURL(n->getTile(), onlyName) + "-ni-" + n->getName();
    }

    /**
     * getProcessorURL()
     * Get an URL for the processor.
     */
    CString OutputHTML::getProcessorURL(Processor *p, bool onlyName)
    {
        return getTileURL(p->getTile(), onlyName) + "-proc-" + p->getName();
    }

    /**
     * getMemoryURL()
     * Get an URL for the memory.
     */
    CString OutputHTML::getMemoryURL(Memory *m, bool onlyName)
    {
        return getTileURL(m->getTile(), onlyName) + "-mem-" + m->getName();
    }

    /**
     * getConnectionURL()
     * Get an URL for the connection.
     */
    CString OutputHTML::getConnectionURL(Connection *c, bool onlyName)
    {
        if (onlyName)
            return "connection-" + c->getName();

        return getPlatformGraphURL(c->getGraph()) + "#connection-" + c->getName();
    }

    /**
     * getPlatformBindingURL()
     * Get an URL for the platform binding.
     */
    CString OutputHTML::getPlatformBindingURL(PlatformBinding *platformBinding)
    {
        return CString("platform-binding-" + platformBinding->getName() + ".html");
    }

    /**
     * getPlatformBindingsURL()
     * Get an URL for the platform bindings.
     */
    CString OutputHTML::getPlatformBindingsURL()
    {
        return CString("platform-bindings.html");
    }

    /**
     * getScenarioPlatformBindingURL()
     * Get an URL for the platform binding of scenario s.
     */
    CString OutputHTML::getScenarioPlatformBindingURL(Scenario *s,
            PlatformBinding *pb, bool onlyName)
    {
        if (onlyName)
            return CString("platform-binding-") + pb->getName() + "-" + s->getName();

        return getPlatformBindingURL(pb) + "#platform-binding-" + pb->getName()
               + "-" + s->getName();
    }

    /**
     * getScenarioGraphPicURL()
     * Get an URL for the scenario graph picture.
     */
    CString OutputHTML::getScenarioGraphPicURL(Scenario *s)
    {
        return CString("scenario-graph-" + s->getName() + ".png");
    }

    /**
     * getPlatformGraphPicURL()
     * Get an URL for the platform graph picture.
     */
    CString OutputHTML::getPlatformGraphPicURL(PlatformGraph *pg)
    {
        return CString("platform-graph-" + pg->getName() + ".png");
    }

    /**
     * getFSMPicURL()
     * Get an URL for the FSM picture.
     */
    CString OutputHTML::getFSMPicURL()
    {
        return CString("fsm.png");
    }

    /**
     * getPlatformBindingPicURL()
     * Get an URL for the platform binding picture.
     */
    CString OutputHTML::getPlatformBindingPicURL(Scenario *s, PlatformBinding *pb)
    {
        return CString("platform-binding-" + s->getName() + "-"
                       + pb->getName() + ".png");
    }

    /**
     * convertGraphToPNG ()
     * Convert the graph using dot to a PNG image. The resulting picture is stored
     * in a file with the supplied filename. The function also creates an image
     * map in XHTML format. This image map is returned through the map variable.
     * The function returns true on success. Otherwise it returns false.
     */
    bool OutputHTML::convertGraphToPNG(CString &graph, CString &filename,
                                       CString &map)
    {
        CString tmpFilename;
        ofstream tmpFile;
        ifstream mapFile;

        // Temporary file
        tmpFilename = tempFileName("", "sdf3");
        tmpFile.open(tmpFilename);

        // Store graph in file
        tmpFile << graph;

        // Close the temporary file
        tmpFile.close();

        // Run dot to convert the picture
        CString cmd = "dot -Tpng -o " + getDirname() + "/"
                      + filename + " " + tmpFilename;
        if (system(cmd.c_str()) != 0)
        {
            return false;
        }

        // Run dot to convert the picture
        if (system("dot -Tcmapx -o " + tmpFilename + ".map " + tmpFilename) != 0)
        {
            return false;
        }

        // Read the image map from file
        mapFile.open(tmpFilename + ".map");

        // Clear map
        map = "";

        // Read map from file
        while (!mapFile.eof())
        {
            char c;
            mapFile.get(c);

            if (!mapFile.eof())
                map += c;
        }

        // Close the map
        mapFile.close();

        // Remove temporary file
        remove(tmpFilename);
        remove(tmpFilename + ".map");

        // Done
        return true;
    }

    /**
     * convertScenarioGraphToPNG()
     * The function converts the supplied scenario graph to a PNG image and store
     * it in the supplied filename. An XHTML image map is returned through the map
     * variable.
     */
    void OutputHTML::convertScenarioGraphToPNG(Scenario *s, ScenarioGraph *sg,
            CString filename, CString &map)
    {
        CString graph;

        // Header
        graph = "digraph scenario_graph_" + s->getName().replace("-", "_") + " {\n";
        graph += "    size=\"7,10\";\n";

        // Actors
        for (Actors::iterator i = sg->getActors().begin();
             i != sg->getActors().end(); i++)
        {
            Actor *a = *i;

            graph += "    " + a->getName().replace("-", "_");
            graph += " [ label=\"" + a->getName();
            graph += "\", href=\"" + getActorURL(s, a) + "\" ];\n";
        }

        // Channels
        for (Channels::iterator i = sg->getChannels().begin();
             i != sg->getChannels().end(); i++)
        {
            Channel *c = *i;

            Actor *srcA = c->getSrcActor();
            Actor *dstA = c->getDstActor();

            graph += "    " + srcA->getName().replace("-", "_");
            graph += " -> " + dstA->getName().replace("-", "_");
            graph += " [ label=\"" + c->getName() + "(";
            graph += CString(c->getInitialTokens()) + ")\"";
            graph += " href=\"" + getChannelURL(s, c);
            graph += "\" ];\n";
        }

        // Footer
        graph += "}\n";

        // Convert graph to PNG
        convertGraphToPNG(graph, filename, map);
    }

    /**
     * convertFSMToPNG()
     * The function converts the supplied FSM to a PNG image and stores
     * it in the supplied filename. An XHTML image map is returned through the map
     * variable.
     */
    void OutputHTML::convertFSMToPNG(FSM *fsm, CString filename, CString &map)
    {
        CString graph;

        // Header
        graph = "digraph fsm {\n";
        graph += "    size=\"7,10\";\n";

        // States
        for (FSMstates::iterator i = fsm->getStates().begin();
             i != fsm->getStates().end(); i++)
        {
            FSMstate *s = *i;

            graph += "    " + s->getName().replace("-", "_");
            graph += " [ label=\"" + s->getName() + "(";
            graph += s->getScenario()->getName() + ")";
            graph += "\", href=\"" + getScenarioURL(s->getScenario()) + "\" ";

            if (s == fsm->getInitialState())
                graph += " shape=\"doublecircle\" ";
            else
                graph += " shape=\"circle\" ";

            graph += "];\n";
        }

        // Transitions
        for (FSMstates::iterator i = fsm->getStates().begin();
             i != fsm->getStates().end(); i++)
        {
            FSMstate *s = *i;

            for (FSMtransitionsCIter j = s->getTransitions().begin();
                 j != s->getTransitions().end(); j++)
            {
                FSMtransition *t = *j;

                graph += "    " + s->getName().replace("-", "_");
                graph += " -> " + t->getDstState()->getName().replace("-", "_");
                graph += " [ label=\"" + t->getName() + "\" ];\n";
            }
        }

        // Footer
        graph += "}\n";

        // Convert graph to PNG
        convertGraphToPNG(graph, filename, map);
    }

    /**
     * convertPlatformGraphToPNG()
     * The function converts the supplied platform graph to a PNG image and store
     * it in the supplied filename. An XHTML image map is returned through the map
     * variable.
     */
    void OutputHTML::convertPlatformGraphToPNG(PlatformGraph *pg,
            CString filename, CString &map)
    {
        CString graph;

        // Header
        graph = "digraph platform_graph_" + pg->getName().replace("-", "_") + " {\n";
        graph += "    size=\"7,10\";\n";

        // Tiles
        for (Tiles::iterator i = pg->getTiles().begin();
             i != pg->getTiles().end(); i++)
        {
            Tile *t = *i;

            graph += "    " + t->getName().replace("-", "_");
            graph += " [ label=\"" + t->getName();
            graph += "\", href=\"" + getTileURL(t) + "\" ";
            graph += " shape=\"box\" ];\n";
        }

        // Connections
        for (Connections::iterator i = pg->getConnections().begin();
             i != pg->getConnections().end(); i++)
        {
            Connection *c = *i;

            Tile *src = c->getSrcNetworkInterface()->getTile();
            Tile *dst = c->getDstNetworkInterface()->getTile();

            graph += "    " + src->getName().replace("-", "_");
            graph += " -> " + dst->getName().replace("-", "_");
            graph += " [ label=\"" + c->getName() + "\"";
            graph += " href=\"" + getConnectionURL(c) + "\" ];\n";
        }

        // Footer
        graph += "}\n";

        // Convert graph to PNG
        convertGraphToPNG(graph, filename, map);
    }

    /**
     * convertPlatformBindingToPNG()
     * The function converts the supplied platform binding to a PNG image and store
     * it in the supplied filename. An XHTML image map is returned through the map
     * variable.
     */
    void OutputHTML::convertPlatformBindingToPNG(Scenario *s, PlatformBinding *pb,
            CString filename, CString &map)
    {
        CString graph;

        // Scenario graph and platform graph
        ScenarioGraph *sg = s->getScenarioGraph();
        PlatformGraph *pg = pb->getPlatformGraph();

        // Header
        graph = "digraph platform_binding_" + s->getName() + "_";
        graph += pb->getName().replace("-", "_");
        graph += + " {\n";
        graph += "    size=\"7,10\";\n";

        // Actors
        for (Actors::iterator i = sg->getActors().begin();
             i != sg->getActors().end(); i++)
        {
            Actor *a = *i;

            graph += "    " + a->getName().replace("-", "_");
            graph += " [ label=\"" + a->getName();
            graph += "\", href=\"" + getActorURL(s, a) + "\" ];\n";
        }

        // Channels
        for (Channels::iterator i = sg->getChannels().begin();
             i != sg->getChannels().end(); i++)
        {
            Channel *c = *i;

            Actor *srcA = c->getSrcActor();
            Actor *dstA = c->getDstActor();

            graph += "    " + srcA->getName().replace("-", "_");
            graph += " -> " + dstA->getName().replace("-", "_");
            graph += " [ label=\"" + c->getName() + "(";
            graph += CString(c->getInitialTokens()) + ")\"";
            graph += " href=\"" + getChannelURL(s, c);
            graph += "\" ];\n";
        }

        // Tiles
        for (Tiles::iterator i = pg->getTiles().begin();
             i != pg->getTiles().end(); i++)
        {
            Tile *t = *i;

            graph += "    " + t->getName().replace("-", "_");
            graph += " [ label=\"" + t->getName();
            graph += "\", href=\"" + getTileURL(t) + "\" ";
            graph += " shape=\"box\" ];\n";
        }

        // Connections
        for (Connections::iterator i = pg->getConnections().begin();
             i != pg->getConnections().end(); i++)
        {
            Connection *c = *i;

            Tile *src = c->getSrcNetworkInterface()->getTile();
            Tile *dst = c->getDstNetworkInterface()->getTile();

            graph += "    " + src->getName().replace("-", "_");
            graph += " -> " + dst->getName().replace("-", "_");
            graph += " [ label=\"" + c->getName() + "\"";
            graph += " href=\"" + getConnectionURL(c) + "\" ];\n";
        }

        // Bindings from actors to tiles
        for (TileBindings::iterator i = pb->getTileBindings().begin();
             i != pb->getTileBindings().end(); i++)
        {
            TileBinding *tb = *i;
            Tile *t = tb->getTile();

            // Iterate over all processor bindings inside tile
            for (ProcessorBindings::iterator j = tb->getProcessorBindings().begin();
                 j != tb->getProcessorBindings().end(); j++)
            {
                ProcessorBinding *pcb = *j;

                // Iterate over all actors
                for (Actors::iterator k = sg->getActors().begin();
                     k != sg->getActors().end(); k++)
                {
                    Actor *a = *k;

                    // Actor bound to processor in scenario?
                    if (pcb->hasActorBinding(s, a))
                    {
                        graph += "    " + a->getName().replace("-", "_");
                        graph += " -> " + t->getName().replace("-", "_");
                        graph += " [ style=\"dashed\" ];\n";
                    }
                }
            }
        }

        // Footer
        graph += "}\n";

        // Convert graph to PNG
        convertGraphToPNG(graph, filename, map);
    }

} // End namespace FSMSADF

