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
 *      22-05-09    :   Initial version.
 *      19-04-11    :   Add accessors (M. Koedam)
 *                      Virtual functions, rename to SDF3FlowBase
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
     * initialize()
     * The function initializes the memory dimensioning algorithm.
     */
    void MemoryDimAlgoBase::init()
    {
        // Has application graph?
        if (applicationGraph == NULL)
            throw CException("No application graph given.");

        // Application graph has no isolated scenarios?
        if (!applicationGraph->hasIsolatedScenarios())
            throw CException("Graph has no isolated scenarios.");

        // Has platform graph?
        if (platformGraph == NULL)
            throw CException("No platform graph given.");

        // Has list of platform bindings?
        if (platformBindings == NULL)
            throw CException("No list of platform bindings given.");

        // No platform bindings given?
        if (platformBindings->size() == 0)
        {
            // Create an empty initial platform binding
            platformBindings->push_back(new PlatformBinding(
                                            GraphComponent(NULL, 0, "initial"),
                                            platformGraph, applicationGraph));
        }

        // No storage distributions selected
        selectedStorageDistribution.clear();
    }

    /**
     * getPlatformBinding()
     * The function returns a pointer to a platform binding object with the
     * specified name. An exception is thrown when no such object exists.
     */
    PlatformBinding *MemoryDimAlgoBase::getPlatformBinding(const CString &name) const
    {
        for (PlatformBindings::iterator i = platformBindings->begin();
             i != platformBindings->end(); i++)
        {
            PlatformBinding *pb = *i;

            if (pb->getName() == name)
                return *i;
        }

        throw CException("No platform binding exists with name '" + name + "'");
    }

    /**
     * getInitialPlatformBinding()
     * The function returns a pointer to the initial platform binding object. An
     * exception is thrown when no such object exists.
     */
    PlatformBinding *MemoryDimAlgoBase::getInitialPlatformBinding() const
    {
        for (PlatformBindings::iterator i = platformBindings->begin();
             i != platformBindings->end(); i++)
        {
            PlatformBinding *pb = *i;

            if (pb->isInitialBinding())
                return *i;
        }

        throw CException("No initial platform binding exists");
    }

    /**
     * computeStorageDist ()
     * Compute trade-off space between storage space allocated to channels and
     * maximal throughput of the application graph.
     */
    bool MemoryDimAlgoBase::computeStorageDist()
    {
        SDFstateSpaceBufferAnalysis bufferAnalysisAlogo;

        // Application graph has no isolated scenarios?
        if (!applicationGraph->hasIsolatedScenarios())
            throw CException("Graph has no isolated scenarios.");

        // Iterate over the scenarios
        for (Scenarios::iterator i = applicationGraph->getScenarios().begin();
             i != applicationGraph->getScenarios().end(); i++)
        {
            Scenario *s = *i;
            ScenarioGraph *sg = s->getScenarioGraph();

            // Create an SDF graph for this scenario
            TimedSDFgraph *g = sg->extractSDFgraph(s);

            // Assume absense of auto-concurrency (i.e. add self-edge to actors)
            for (SDFactorsIter j = g->actorsBegin(); j != g->actorsEnd(); j++)
                g->createChannel(*j, 1, *j, 1, 1);

            // Compute throughput/storage-space trade-offs
            StorageDistributionSet *dset = bufferAnalysisAlogo.analyze(g);

            // Convert storage distributions to FSM-SADF type
            StorageDistributions ds;
            while (dset != NULL)
            {
                struct _StorageDistribution *distributions = dset->distributions;

                while (distributions != NULL)
                {
                    StorageDistribution d;

                    // Set throughput of the distribution
                    d.setThroughput(distributions->thr);

                    // Set buffer size of the channels
                    for (Channels::iterator j = sg->getChannels().begin();
                         j != sg->getChannels().end(); j++)
                    {
                        Channel *c = *j;

                        d[c] = distributions->sp[c->getId()];
                    }

                    // Add distribution to the set of distributions
                    ds.insert(d);

                    // Next distribution
                    distributions = distributions->next;
                }

                // Next set
                dset = dset->next;
            }

            // Storage distribution with throughput 0 not present?
            if (ds.size() == 0 || ds.begin()->getThroughput() != 0)
            {
                StorageDistribution dz;

                // Set throughput of the distribution
                dz.setThroughput(0);

                // Set buffer size of the channels
                for (Channels::iterator j = sg->getChannels().begin();
                     j != sg->getChannels().end(); j++)
                {
                    Channel *c = *j;

                    dz[c] = 0;
                }

                // Add distribution to the set of distributions
                ds.insert(dz);

            }

            // Assign storage distributions to the scenario graph
            sg->setStorageDistributionsOfScenario(s, ds);

            // Initialize the selected storage distribution of this scenario to the
            // first storage distribution
            selectedStorageDistribution[s] =
                sg->getStorageDistributionsOfScenario(s).begin();

            // Cleanup
            delete g;
        }

        return true;
    }

    /**
     * selectStorageDist ()
     * Select storage distribution from the trade-off space.
     */
    bool MemoryDimAlgoBase::selectStorageDist()
    {
        bool selectedNewDistribution = false;

        // Iterate over the scenarios
        for (map<Scenario *, set<StorageDistribution>::iterator>::iterator
             i = selectedStorageDistribution.begin();
             i !=  selectedStorageDistribution.end(); i++)
        {
            Scenario *s = i->first;
            ScenarioGraph *sg = s->getScenarioGraph();
            set<StorageDistribution>::iterator di = i->second;

            // Select next storage distribution
            di++;

            // Larger storage distribution available?
            if (di != sg->getStorageDistributionsOfScenario(s).end())
            {
                i->second++;

                // Selected a new storage distribution for at least one graph
                selectedNewDistribution = true;
            }

            logMsg(CString("Selected distribution of size ")
                   + CString(i->second->getSize()) + " with throughput "
                   + CString(i->second->getThroughput())
                   + " for scenario graph '" + sg->getName() + "'");
        }

        // Selected no new storage distribution for any scenario graph?
        if (!selectedNewDistribution)
        {
            logMsg("No storage distribution left.");
            return false;
        }

        return true;
    }

    /**
     * estimateStorageDist ()
     * Set the storage space constraints of the channels based on the
     * selected storage distribution.
     */
    bool MemoryDimAlgoBase::estimateStorageDist()
    {
        StorageDistribution maxSz, minSz;
        map<CString, Rate> maxSrcRate, maxDstRate;
        map<CString, uint> maxInitialToken;

        // Remove the existing platform bindings (except the initial binding)
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
             i !=  selectedStorageDistribution.end(); i++)
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

                // Select minimal deadlock free buffersize for channels mapped
                // to memory (connected actors are fired sequentially anyhow)
                sz[BufferLocation::Mem] = minSz[c];

                // Compute source and destination buffer sizes
                sz[BufferLocation::Src] = maxSz[c] / 2 + (maxSz[c] % 2 > 0 ? 1 : 0);
                sz[BufferLocation::Dst] = maxSz[c] / 2 + (maxSz[c] % 2 > 0 ? 1 : 0);

                // Correct buffer sizes for constraints
                if (sz[BufferLocation::Src] < maxInitialToken[c->getName()])
                    sz[BufferLocation::Src] = maxInitialToken[c->getName()];
                if (sz[BufferLocation::Src] < maxSrcRate[c->getName()])
                    sz[BufferLocation::Src] = maxSrcRate[c->getName()];
                if (sz[BufferLocation::Dst] < maxDstRate[c->getName()])
                    sz[BufferLocation::Dst] = maxDstRate[c->getName()];

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

} // End namespace FSMSADF

