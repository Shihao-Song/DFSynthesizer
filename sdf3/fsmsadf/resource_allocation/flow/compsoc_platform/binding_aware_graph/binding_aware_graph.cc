/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   binding_aware_sdfg.cc
 *
 *  Author          :   Martijn Koedam (m.l.p.j.koedam@tue.nl)
 *
 *  Date            :   April 15, 2011
 *
 *  Function        :   Binding-aware SDFG
 *
 *  History         :
 *      15-04-11    :   Initial version.
 *
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

#include "binding_aware_graph.h"

namespace FSMSADF
{
    /**
     * BindingAwareGraph()
     * Constructor.
     */
    BindingAwareGraphCompSoCPlatform::BindingAwareGraphCompSoCPlatform(PlatformBinding *pb)
        : BindingAwareGraphBase(pb)
    {

    }

    /**
     * ~BindingAwareGraph()
     * Destructor.
     */
    BindingAwareGraphCompSoCPlatform::~BindingAwareGraphCompSoCPlatform()
    {
    }

    /**
     * Model the binding of an actor to a tile inside the binding-aware graph.
     * @param scenario s
     * @param actor a
     * @param processo p
     */
    void BindingAwareGraphCompSoCPlatform::modelActorToTileBinding(Actor *a, Processor *p,
            Scenario *s)
    {
        Channel *c;

        // Scenario graph
        ScenarioGraph *sg = a->getScenarioGraph();

        // Compute WCRT when using TDMA
        Time execTime = a->getExecutionTimeOfScenario(s, p->getType());
        Time contextSwitchTime = p->getContextSwitchOverhead();
        Time slotLength = p->getSlotLength();
        Time slotWithOverhead = slotLength + contextSwitchTime;
        Time tdmaTimeSlice = bindingAwareProperties[sg].getTimesliceMappedToProcessors(p);
        ASSERT(tdmaTimeSlice > 0, "tdmaTimeSlice cannot be <= 0");
        Time waitingTime = p->getWheelsize() - tdmaTimeSlice;
        int nrReservedSlots = (int) floor((double)tdmaTimeSlice / slotWithOverhead);
        Time wcrt = 0;

        while (execTime > 0)
        {
            wcrt = wcrt + waitingTime;

            if (execTime < nrReservedSlots * slotLength)
            {
                wcrt = wcrt + (Time)(ceil((double)execTime / slotLength) * slotWithOverhead);
                execTime = 0;
            }
            else
            {
                wcrt = wcrt + (nrReservedSlots * slotWithOverhead);
                execTime = execTime - (nrReservedSlots * slotLength);
            }
        }

        // Set the execution time of actor a on processor p as its worst-case
        // response time
        a->setExecutionTimeOfScenario(s, "wcrt", wcrt);
        a->setDefaultProcessorType("wcrt");

        // Add self-loop with one token
        c = a->getScenarioGraph()->createChannel(a, a);
        c->setName(a->getName() + "-selfedge");
        c->getSrcPort()->setRateOfScenario(s, 1);
        c->getDstPort()->setRateOfScenario(s, 1);
        c->setInitialTokens(1);
        c->setPersistentTokenNames(CString("PT-") + c->getName());
    }

    /**
     * The function models a channel mapped to a connection.
     *
     */
    void BindingAwareGraphCompSoCPlatform::modelChannelToConnectionBinding(Channel *c,
            ChannelBindingConstraints *bc, Connection *co, Scenario *s)
    {
        CString persistentTokenNames;

        // Scenario graph
        ScenarioGraph *sg = c->getScenarioGraph();

        // Buffer size of channel c
        BufferSize b = bc->getBufferSize();

        // Processor to which destination actor of channel c is bound
        //Processor *pd = bindingAwareProperties[sg].getProcessorMappedToActor(c->getDstActor());

        // Execution times of the actors acol, acor, ad, and al
        Time latency = co->getLatency();
        /// @todo We need a better way to calculate the bandwidth constraint.
        /// currently it becomes equal to the constraint. so
        /// the final equation looks like rate+latency < rate.
        Time rate = 1; //(Time)((c->getTokenSizeOfScenario(s) / bc->getBandwidth()));

        /// @todo the following parameters are hardcoded for the network interface.
        /// Times in ns for the xilinx2tile3dma setup. These change when network
        /// changes.
        Time t_dmastart = (Time) 2e3;
        Time t_dmasend = (Time) 2e3;
        Time t_dmaend = (Time) 6e3;

        unsigned int src_rate = c->getSrcPort()->getRateOfScenario(s);
        unsigned int fact = (src_rate * 2 + 1);

        // The memory inside the DMA (defined in the arch_bufdim.xml)
        unsigned int buf_dma = 3;
        // The memory of the network. (dependent on size?)
        unsigned int buf_network = 5;
        // The number of 'packages' one token is split into.
        /// @todo setting q(dma_token_split) > 1 causes the time spend in
        /// PrecedenceGraph::updateDeadline to explode.
        unsigned int dma_token_split = 1;

        Actor *fake_src = sg->createActor();
        fake_src->setName(c->getName() + "_fakesrc");
        fake_src->setExecutionTimeOfScenario(s, "fakesrc", 0);
        fake_src->setDefaultProcessorType("fakesrc");

        Actor *dma_start = sg->createActor();
        dma_start->setName(c->getName() + "_dmastart");
        // Assume time unit in ns
        dma_start->setExecutionTimeOfScenario(s, "dmastart", t_dmastart);
        dma_start->setDefaultProcessorType("dmastart");

        Actor *dma_send = sg->createActor();
        dma_send->setName(c->getName() + "_dmasend");
        // Assume time unit in ns
        dma_send->setExecutionTimeOfScenario(s, "dmasend", t_dmasend);
        dma_send->setDefaultProcessorType("dmasend");


        Actor *dma_end = sg->createActor();
        dma_end->setName(c->getName() + "_dmaend");
        // Assume time unit in ns
        dma_end->setExecutionTimeOfScenario(s, "dmaend", t_dmaend);
        dma_end->setDefaultProcessorType("dmaend");

        Actor *fake_dma_end = sg->createActor();
        fake_dma_end->setName(c->getName() + "_fakedmaend");
        fake_dma_end->setExecutionTimeOfScenario(s, "fakedmaend", 0);
        fake_dma_end->setDefaultProcessorType("fakedmaend");

        // Create an actor acor which models the connection rate
        Actor *acor = sg->createActor();
        acor->setName(c->getName() + "_rate");
        acor->setExecutionTimeOfScenario(s, "rate", rate);
        acor->setDefaultProcessorType("rate");

        // Create an actor acol which models the connection latency
        Actor *acol = sg->createActor();
        acol->setName(c->getName() + "_latency");
        acol->setExecutionTimeOfScenario(s, "latency", latency);
        acol->setDefaultProcessorType("latency");

        Actor *fake_dst = sg->createActor();
        fake_dst->setName(c->getName() + "_fakedst");
        fake_dst->setExecutionTimeOfScenario(s, "fakedst", 0);
        fake_dst->setDefaultProcessorType("fakedst");

        Channel *ch_src_fake_src = sg->createChannel(c->getSrcActor(), fake_src);
        ch_src_fake_src->setName(c->getName() + "-srcfakesrc");
        ch_src_fake_src->getSrcPort()->setRateOfScenario(s, src_rate);
        ch_src_fake_src->getDstPort()->setRateOfScenario(s, src_rate);
        ch_src_fake_src->setInitialTokens(c->getInitialTokens());
        persistentTokenNames = "";
        for (uint i = 0; i < ch_src_fake_src->getInitialTokens(); i++)
        {
            if (!persistentTokenNames.empty())
                persistentTokenNames += ",";

            persistentTokenNames = persistentTokenNames + CString("PT-") + CString(i);
            persistentTokenNames = persistentTokenNames + CString("-");
            persistentTokenNames = persistentTokenNames + ch_src_fake_src->getName();
        }
        ch_src_fake_src->setPersistentTokenNames(persistentTokenNames);

        Channel *ch_fake_src_src = sg->createChannel(fake_src, c->getSrcActor());
        ch_fake_src_src->setName(c->getName() + "-srcsrcfake");
        ch_fake_src_src->getSrcPort()->setRateOfScenario(s, 1);
        ch_fake_src_src->getDstPort()->setRateOfScenario(s, 1);
        ASSERT(b[BufferLocation::Src] >= c->getInitialTokens(),
               "There is not enough space in the input buffer to fit the initial tokens");
        ch_fake_src_src->setInitialTokens(b[BufferLocation::Src] - c->getInitialTokens());
        persistentTokenNames = "";
        for (uint i = 0; i < ch_fake_src_src->getInitialTokens(); i++)
        {
            if (!persistentTokenNames.empty())
                persistentTokenNames += ",";

            persistentTokenNames = persistentTokenNames + CString("PT-") + CString(i);
            persistentTokenNames = persistentTokenNames + CString("-");
            persistentTokenNames = persistentTokenNames + ch_fake_src_src->getName();
        }
        ch_fake_src_src->setPersistentTokenNames(persistentTokenNames);

        // src->dma_start
        Channel *ch_src_dma_start = sg->createChannel(fake_src, dma_start);
        ch_src_dma_start->setName(c->getName() + "-srcdmastart");
        ch_src_dma_start->getSrcPort()->setRateOfScenario(s, fact);
        ch_src_dma_start->getDstPort()->setRateOfScenario(s, 1);
        ch_src_dma_start->setInitialTokens(0);

        // dma_start->dma_send
        Channel *ch_dma_start_dma_send = sg->createChannel(dma_start, dma_send);
        ch_dma_start_dma_send->setName(c->getName() + "-dmastartdmasend");
        ch_dma_start_dma_send->getSrcPort()->setRateOfScenario(s, dma_token_split);
        ch_dma_start_dma_send->getDstPort()->setRateOfScenario(s, 1);
        ch_dma_start_dma_send->setInitialTokens(0);

        // dma_send->dma_end
        Channel *ch_dma_send_dma_end = sg->createChannel(dma_send, dma_end);
        ch_dma_send_dma_end->setName(c->getName() + "-dmasenddmaend");
        ch_dma_send_dma_end->getSrcPort()->setRateOfScenario(s, 1);
        ch_dma_send_dma_end->getDstPort()->setRateOfScenario(s, dma_token_split);
        ch_dma_send_dma_end->setInitialTokens(0);

        // dma_end -> dma_start
        Channel *ch_dma_end_dma_start = sg->createChannel(dma_end, dma_start);
        ch_dma_end_dma_start->setName(c->getName() + "-dmaenddmastart");
        ch_dma_end_dma_start->getSrcPort()->setRateOfScenario(s, 1);
        ch_dma_end_dma_start->getDstPort()->setRateOfScenario(s, 1);
        ch_dma_end_dma_start->setInitialTokens(1);
        ch_dma_end_dma_start->setPersistentTokenNames(CString("PT-") + c->getName());

        // dma_end -> fake_dma_end
        Channel *ch_dma_end_src = sg->createChannel(dma_end, fake_dma_end);
        ch_dma_end_src->setName(c->getName() + "-dmaendfakedmaend");
        ch_dma_end_src->getSrcPort()->setRateOfScenario(s, 1);
        ch_dma_end_src->getDstPort()->setRateOfScenario(s, fact);
        ch_dma_end_src->setInitialTokens(0);

        Channel *ch_dma_end_fake_src_fake = sg->createChannel(fake_dma_end, fake_src);
        ch_dma_end_fake_src_fake->setName(c->getName() + "-dmaendfakesrcfake");
        ch_dma_end_fake_src_fake->getSrcPort()->setRateOfScenario(s, 1);
        ch_dma_end_fake_src_fake->getDstPort()->setRateOfScenario(s, 1);
        ch_dma_end_fake_src_fake->setInitialTokens(1);
        ch_dma_end_fake_src_fake->setPersistentTokenNames(CString("PT-") + c->getName());

        //dma_send -> rate
        Channel *ch_dma_send_rate = sg->createChannel(dma_send, acor);
        ch_dma_send_rate->setName(c->getName() + "-dmasendrate");
        ch_dma_send_rate->getSrcPort()->setRateOfScenario(s, 1);
        ch_dma_send_rate->getDstPort()->setRateOfScenario(s, 1);
        ch_dma_send_rate->setInitialTokens(0);

        // rate -> dma_send
        Channel *ch_rate_dma_send = sg->createChannel(acor, dma_send);
        ch_rate_dma_send->setName(c->getName() + "-ratedmasend");
        ch_rate_dma_send->getSrcPort()->setRateOfScenario(s, 1);
        ch_rate_dma_send->getDstPort()->setRateOfScenario(s, 1);
        ch_rate_dma_send->setInitialTokens(buf_dma);
        persistentTokenNames = "";
        for (uint i = 0; i < ch_rate_dma_send->getInitialTokens(); i++)
        {
            if (!persistentTokenNames.empty())
                persistentTokenNames += ",";

            persistentTokenNames = persistentTokenNames + CString("PT-") + CString(i);
            persistentTokenNames = persistentTokenNames + CString("-");
            persistentTokenNames = persistentTokenNames + ch_rate_dma_send->getName();
        }
        ch_rate_dma_send->setPersistentTokenNames(persistentTokenNames);

        // rate -> rate
        Channel *ch_rate_rate = sg->createChannel(acor, acor);
        ch_rate_rate->setName(c->getName() + "-raterate");
        ch_rate_rate->getSrcPort()->setRateOfScenario(s, 1);
        ch_rate_rate->getDstPort()->setRateOfScenario(s, 1);
        ch_rate_rate->setInitialTokens(1);
        ch_rate_rate->setPersistentTokenNames(CString("PT-") + c->getName());

        // rate->latency
        Channel *ch_rate_latency = sg->createChannel(acor, acol);
        ch_rate_latency->setName(c->getName() + "-ratelatency");
        ch_rate_latency->getSrcPort()->setRateOfScenario(s, 1);
        ch_rate_latency->getDstPort()->setRateOfScenario(s, 1);
        ch_rate_latency->setInitialTokens(0);

        // latency->dst
        Channel *ch_latency_dst = sg->createChannel(acol, fake_dst);
        ch_latency_dst->setName(c->getName() + "-latencydst");
        ch_latency_dst->getSrcPort()->setRateOfScenario(s, 1);
        ch_latency_dst->getDstPort()->setRateOfScenario(s, dma_token_split * fact);
        ch_latency_dst->setInitialTokens(0);

        // fake_dst -> dst
        Channel *ch_fakedstdst = sg->createChannel(fake_dst, c->getDstActor());
        ch_fakedstdst->setName(c->getName() + "-fakedstdst");
        ch_fakedstdst->getSrcPort()->setRateOfScenario(s, src_rate);
        ch_fakedstdst->getDstPort()->setRateOfScenario(s, c->getDstPort()->getRateOfScenario(s));
        ch_fakedstdst->setInitialTokens(0);

        // back edge. releasing buffers in B
        Actor *be_dma_start = sg->createActor();
        be_dma_start->setName(c->getName() + "_be_dmastart");
        // Assume time unit in ns
        be_dma_start->setExecutionTimeOfScenario(s, "be_dmastart", t_dmastart);
        be_dma_start->setDefaultProcessorType("be_dmastart");

        Actor *be_dma_send = sg->createActor();
        be_dma_send->setName(c->getName() + "_be_dmasend");
        // Assume time unit in ns
        be_dma_send->setExecutionTimeOfScenario(s, "be_dmasend", t_dmasend);
        be_dma_send->setDefaultProcessorType("be_dmasend");


        Actor *be_dma_end = sg->createActor();
        be_dma_end->setName(c->getName() + "_be_dmaend");
        // Assume time unit in ns
        be_dma_end->setExecutionTimeOfScenario(s, "dmaend", t_dmaend);
        be_dma_end->setDefaultProcessorType("dmaend");

        // Create an actor acol which models the connection latency
        Actor *be_latency = sg->createActor();
        be_latency->setName(c->getName() + "_be_latency");
        be_latency->setExecutionTimeOfScenario(s, "be_latency", latency);
        be_latency->setDefaultProcessorType("be_latency");

        // Create an actor acor which models the connection rate
        Actor *be_rate = sg->createActor();
        be_rate->setName(c->getName() + "_be_rate");
        be_rate->setExecutionTimeOfScenario(s, "be_rate", rate);
        be_rate->setDefaultProcessorType("be_rate");

        // dst->be_dma_start
        Channel *ch_be_dst_dma_start = sg->createChannel(c->getDstActor(), be_dma_start);
        ch_be_dst_dma_start->setName(c->getName() + "-be_dstdmastart");
        ch_be_dst_dma_start->getSrcPort()->setRateOfScenario(s, 1);
        ch_be_dst_dma_start->getDstPort()->setRateOfScenario(s, 1);
        ch_be_dst_dma_start->setInitialTokens(0);

        // be_dma_start->be_dma_send
        Channel *ch_be_dma_start_send = sg->createChannel(be_dma_start, be_dma_send);
        ch_be_dma_start_send->setName(c->getName() + "-be_dmastartsend");
        ch_be_dma_start_send->getSrcPort()->setRateOfScenario(s, 1);
        ch_be_dma_start_send->getDstPort()->setRateOfScenario(s, 1);
        ch_be_dma_start_send->setInitialTokens(0);

        // be_dma_send ->be_dma_end
        Channel *ch_be_dma_send_end = sg->createChannel(be_dma_send, be_dma_end);
        ch_be_dma_send_end->setName(c->getName() + "-be_dmasendend");
        ch_be_dma_send_end->getSrcPort()->setRateOfScenario(s, 1);
        ch_be_dma_send_end->getDstPort()->setRateOfScenario(s, 1);
        ch_be_dma_send_end->setInitialTokens(0);

        // be_dma_end->dst
        Channel *ch_be_dma_end_dst = sg->createChannel(be_dma_end, c->getDstActor());
        ch_be_dma_end_dst->setName(c->getName() + "-be_dmaenddst");
        ch_be_dma_end_dst->getSrcPort()->setRateOfScenario(s, 1);
        ch_be_dma_end_dst->getDstPort()->setRateOfScenario(s, 1);
        ch_be_dma_end_dst->setInitialTokens(1);
        ch_be_dma_end_dst->setPersistentTokenNames(CString("PT-") + c->getName());

        // be_dma_end->be_dma_start
        Channel *ch_be_dma_end_start = sg->createChannel(be_dma_end, be_dma_start);
        ch_be_dma_end_start->setName(c->getName() + "-be_dmaendstart");
        ch_be_dma_end_start->getSrcPort()->setRateOfScenario(s, 1);
        ch_be_dma_end_start->getDstPort()->setRateOfScenario(s, 1);
        ch_be_dma_end_start->setInitialTokens(1);
        ch_be_dma_end_start->setPersistentTokenNames(CString("PT-") + c->getName());

        // be_dma_send -> be_rate
        Channel *ch_be_dma_send_rate = sg->createChannel(be_dma_send, be_rate);
        ch_be_dma_send_rate->setName(c->getName() + "-be_dmasendrate");
        ch_be_dma_send_rate->getSrcPort()->setRateOfScenario(s, 1);
        ch_be_dma_send_rate->getDstPort()->setRateOfScenario(s, 1);
        ch_be_dma_send_rate->setInitialTokens(0);

        // be_rate->be_dma_send
        Channel *ch_be_rate_send = sg->createChannel(be_rate, be_dma_send);
        ch_be_rate_send->setName(c->getName() + "-be_ratedmasend");
        ch_be_rate_send->getSrcPort()->setRateOfScenario(s, 1);
        ch_be_rate_send->getDstPort()->setRateOfScenario(s, 1);
        ch_be_rate_send->setInitialTokens(buf_dma);
        persistentTokenNames = "";
        for (uint i = 0; i < ch_be_rate_send->getInitialTokens(); i++)
        {
            if (!persistentTokenNames.empty())
                persistentTokenNames += ",";

            persistentTokenNames = persistentTokenNames + CString("PT-") + CString(i);
            persistentTokenNames = persistentTokenNames + CString("-");
            persistentTokenNames = persistentTokenNames + ch_be_rate_send->getName();
        }
        ch_be_rate_send->setPersistentTokenNames(persistentTokenNames);

        // be_rate -> be_latency
        Channel *ch_be_rate_latency = sg->createChannel(be_rate, be_latency);
        ch_be_rate_latency->setName(c->getName() + "-be_ratelatency");
        ch_be_rate_latency->getSrcPort()->setRateOfScenario(s, 1);
        ch_be_rate_latency->getDstPort()->setRateOfScenario(s, 1);
        ch_be_rate_latency->setInitialTokens(0);

        // be_rate -> be_rate
        Channel *ch_be_rate_rate = sg->createChannel(be_rate, be_rate);
        ch_be_rate_rate->setName(c->getName() + "-be_raterate");
        ch_be_rate_rate->getSrcPort()->setRateOfScenario(s, 1);
        ch_be_rate_rate->getDstPort()->setRateOfScenario(s, 1);
        ch_be_rate_rate->setInitialTokens(1);
        ch_be_rate_rate->setPersistentTokenNames(CString("PT-") + c->getName());
        persistentTokenNames = "";
        for (uint i = 0; i < ch_be_rate_rate->getInitialTokens(); i++)
        {
            if (!persistentTokenNames.empty())
                persistentTokenNames += ",";

            persistentTokenNames = persistentTokenNames + CString("PT-") + CString(i);
            persistentTokenNames = persistentTokenNames + CString("-");
            persistentTokenNames = persistentTokenNames + ch_be_rate_rate->getName();
        }
        ch_be_rate_rate->setPersistentTokenNames(persistentTokenNames);

        // be_latency -> src
        Channel *ch_be_latency_src = sg->createChannel(be_latency, c->getSrcActor());
        ch_be_latency_src->setName(c->getName() + "-be_latencysrc");
        ch_be_latency_src->getSrcPort()->setRateOfScenario(s, 1);
        ch_be_latency_src->getDstPort()->setRateOfScenario(s, 1);
        ch_be_latency_src->setInitialTokens(buf_network + b[BufferLocation::Dst]);
        persistentTokenNames = "";
        for (uint i = 0; i < ch_be_latency_src->getInitialTokens(); i++)
        {
            if (!persistentTokenNames.empty())
                persistentTokenNames += ",";

            persistentTokenNames = persistentTokenNames + CString("PT-") + CString(i);
            persistentTokenNames = persistentTokenNames + CString("-");
            persistentTokenNames = persistentTokenNames + ch_be_latency_src->getName();
        }
        ch_be_latency_src->setPersistentTokenNames(persistentTokenNames);
    }

} // End namespace FSMSADF

