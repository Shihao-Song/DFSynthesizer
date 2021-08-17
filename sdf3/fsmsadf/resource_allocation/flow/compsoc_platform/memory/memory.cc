/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   memory.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 22, 2009
 *
 *  Function        :   Memory dimensioning
 *
 *  History         :
 *     08-09-11    :   Initial version.
 *
 * $Id: memory.cc,v 1.1 2009-12-23 13:37:22 sander Exp $
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

#include "memory.h"

namespace FSMSADF
{

    /**
     * estimateStorageDist ()
     * Set the storage space constraints of the channels based on the
     * selected storage distribution.
     */
    bool MemoryDimAlgoCompSoCPlatform::estimateStorageDist()
    {
        StorageDistribution maxSz, minSz;
        map<CString, Rate> maxSrcRate, maxDstRate;
        map<CString, uint> maxInitialToken;

        // Remove the existing platform bindings(except the initial binding)
        for (PlatformBindings::iterator i = platformBindings->begin();
             i != platformBindings->end();)
        {
            PlatformBinding *pb = *i;

            if (pb->isInitialBinding())
                i++;
            else
                platformBindings->erase(i++);
        }

        // Create a new platform binding based on the initial binding
        PlatformBinding *initPb = getInitialPlatformBinding();
        PlatformBinding *pb = initPb->clone(*initPb);
        pb->setName("0");
        platformBindings->push_back(pb);

        // Iterate over the scenarios (compute max buffer size)
        for (map<Scenario *, set<StorageDistribution>::iterator>::iterator
             i = selectedStorageDistribution.begin();
             i != selectedStorageDistribution.end(); i++)
        {
            Scenario *s = i->first;
            ScenarioGraph *sg = s->getScenarioGraph();
            StorageDistribution d = *(i->second);

            // Iterate over all channels
            for (Channels::iterator j = sg->getChannels().begin();
                 j != sg->getChannels().end(); j++)
            {
                Channel *c = *j;

                maxSz[c] = max(maxSz[c], d[c]);
            }
        }

        // Iterate over the scenario graphs (compute min buffer size)
        for (Scenarios::iterator i = applicationGraph->getScenarios().begin();
             i != applicationGraph->getScenarios().end(); i++)
        {
            Scenario *s = *i;
            ScenarioGraph *sg = s->getScenarioGraph();
            StorageDistributions ds = sg->getStorageDistributionsOfScenario(s);

            // Find first storage distribution with non-zero throughput
            for (StorageDistributions::iterator j = ds.begin(); j != ds.end(); j++)
            {
                StorageDistribution d = *j;

                // Throughput larger then zero?
                if (d.getThroughput() > 0)
                {
                    // Iterate over all channels
                    for (Channels::iterator k = sg->getChannels().begin();
                         k != sg->getChannels().end(); k++)
                    {
                        Channel *c = *k;
                        minSz[c] = max(minSz[c], d[c]);
                    }
                    break;
                }
            }
        }

        // Iterate over the scenario graphs (compute maximum rate and initial token)
        for (Scenarios::iterator i = applicationGraph->getScenarios().begin();
             i != applicationGraph->getScenarios().end(); i++)
        {
            Scenario *s = *i;
            ScenarioGraph *sg = s->getScenarioGraph();

            // Iterate over all channels
            for (Channels::iterator j = sg->getChannels().begin();
                 j != sg->getChannels().end(); j++)
            {
                Channel *c = *j;

                maxSrcRate[c->getName()] = max(maxSrcRate[c->getName()],
                                               c->getSrcPort()->getRateOfScenario(s));
                maxDstRate[c->getName()] = max(maxDstRate[c->getName()],
                                               c->getDstPort()->getRateOfScenario(s));
                maxInitialToken[c->getName()] = max(maxInitialToken[c->getName()],
                                                    c->getInitialTokens());
            }
        }

        // Iterate over the scenario graphs
        for (Scenarios::iterator i = applicationGraph->getScenarios().begin();
             i != applicationGraph->getScenarios().end(); i++)
        {
            Scenario *s = *i;
            ScenarioGraph *sg = s->getScenarioGraph();
            GraphBindingConstraints *gb = pb->getGraphBindingConstraints();

            // Iterate over all channels
            for (Channels::iterator j = sg->getChannels().begin();
                 j != sg->getChannels().end(); j++)
            {
                Channel *c = *j;
                BufferSize sz;

                // We place all the memory in the fifo (compsoc will place this at the consumer.
                // The src buffer in compsoc is always the max rate.
                // the dst buffer in compsoc is merged with the fifo buffer. (output convert will make
                // fifo buffer =dstbuffer+fifo size

                // Select minimal deadlock free buffersize for channels mapped
                // to memory (connected actors are fired sequentially anyhow
                if ((maxSrcRate[c->getName()] + maxDstRate[c->getName()]) < maxSz[c])
                {
                    sz[BufferLocation::Mem] = max(maxSz[c] - maxSrcRate[c->getName()] - maxDstRate[c->getName()],
                                                  minSz[c]);
                }
                else
                {
                    sz[BufferLocation::Mem] = minSz[c];
                }

                // Compute source and destination buffer sizes
                sz[BufferLocation::Src] = maxSrcRate[c->getName()];
                sz[BufferLocation::Dst] = maxDstRate[c->getName()];

                // Correct buffer sizes for constraints
                if (sz[BufferLocation::Mem] < maxInitialToken[c->getName()])
                    sz[BufferLocation::Mem] = maxInitialToken[c->getName()];


                // Set the buffer size of this channel in scenario s
                gb->getConstraintsOfScenario(s)->getConstraintsOfChannel(c)
                ->setBufferSize(sz);

                logMsg("Storage space of channel '" + c->getName()
                       + "' in scenario graph '" + sg->getName()
                       + "' (mem/src/dst): "
                       + CString(sz[BufferLocation::Mem]) + " / "
                       + CString(sz[BufferLocation::Src]) + " / "
                       + CString(sz[BufferLocation::Dst]) + " tokens");
            }
        }

        return true;
    }

} // namespace