/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   binding.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 14, 2009
 *
 *  Function        :   Tile binding and scheduling
 *
 *  History         :
 *      29-05-09    :   Initial version.
 *      19-04-11    :   Add accessors (M. Koedam)
 *                      Virtual functions, rename to SDF3FlowBase
 *
 * $Id: binding.cc,v 1.1 2009-12-23 13:37:22 sander Exp $
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

#include "binding.h"
#include "../../../../analysis/base/repetition_vector.h"
#include "../../../scheduling/earliest_deadline_first/edf.h"

namespace FSMSADF
{

    /**
     * TileBindingAlgo()
     * Constructor.
     */
    TileBindingAlgoBase::TileBindingAlgoBase()
        :
        applicationGraph(NULL),
        platformGraph(NULL),
        platformBindings(NULL)
    {

    }

    /**
     * ~TileBindingAlgo()
     * Destructor.
     */
    TileBindingAlgoBase::~TileBindingAlgoBase()
    {
    }

    /**
     * initialize()
     * The function initializes the tile binding and scheduling algorithm.
     */
    void TileBindingAlgoBase::init()
    {
        // Has application graph?
        if (applicationGraph == NULL)
            throw CException("No application graph given.");

        // Has platform graph?
        if (platformGraph == NULL)
            throw CException("No platform graph given.");

        // Has list of platform bindings?
        if (platformBindings == NULL)
            throw CException("No list of platform bindings given.");

        // Application graph has no isolated scenarios?
        if (!applicationGraph->hasIsolatedScenarios())
            throw CException("Graph has no isolated scenarios.");

        // No platform bindings given?
        if (platformBindings->size() == 0)
        {
            // Create an empty initial platform binding
            platformBindings->push_back(new PlatformBinding(
                                            GraphComponent(NULL, 0, "initial"),
                                            platformGraph, applicationGraph));
        }

        // Initialize the relation between the application and platform graph
        initGraphRelations();
    }

    /**
     * getPlatformBinding()
     * The function returns a pointer to a platform binding object with the
     * specified name. An exception is thrown when no such object exists.
     */
    PlatformBinding *TileBindingAlgoBase::getPlatformBinding(const CString &name) const
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
    PlatformBinding *TileBindingAlgoBase::getInitialPlatformBinding() const
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
     * analyzeThroughput ()
     * The function returns the throughput of the application graph
     * mapped onto the platform graph.
     */
    Throughput TileBindingAlgoBase::analyzeThroughput(PlatformBinding *pb)
    {
        EXCEPTION("%s: This function should be implemented by the platform "
                  "specific implementation", __FUNCTION__);
        return 0;
    }

    /**
     * isThroughputConstraintSatisfied ()
     * Check wether or not the throughput constraint is satisfied.
     */
    bool TileBindingAlgoBase::isThroughputConstraintSatisfied(PlatformBinding *pb)
    {
        Throughput thrGraph, thrConstraint;

        // Compute throughput of mapped graph
        thrGraph = analyzeThroughput(pb);
        thrConstraint = applicationGraph->getThroughputConstraint();

        // Output throughput
        logMsg("Throughput of binding: " + CString(thrGraph));

        // Is throughput constraint met?
        if (thrConstraint > thrGraph)
        {
            logInfo("Binding does not meet throughput constraint.");
            return false;
        }

        logInfo("Binding satisfies throughput constraint.");

        return true;
    }

    /**
     * estimateBandwidthConstraint()
     * Estimate the required bandwidth for each channel in the application graph.
     */
    bool TileBindingAlgoBase::estimateBandwidthConstraint()
    {
        // Iterate over all platform bindings (except the initial binding)
        for (PlatformBindings::iterator i = platformBindings->begin();
             i != platformBindings->end(); i++)
        {
            PlatformBinding *pb = *i;
            GraphBindingConstraints *gb = pb->getGraphBindingConstraints();

            if (pb->isInitialBinding())
                continue;

            // Estimate bandwidth for binding b
            logMsg("Estimate bandwidth for binding '" + pb->getName() + "'");

            // Iterate over the scenario graphs
            for (Scenarios::iterator j = applicationGraph->getScenarios().begin();
                 j != applicationGraph->getScenarios().end(); j++)
            {
                Scenario *s = *j;
                ScenarioGraph *sg = s->getScenarioGraph();

                // Compute repetition vector of the scenario graph sg in scenario s
                RepetitionVector repVec = computeRepetitionVector(sg, s);

                // Iterate over the channels
                for (Channels::iterator k = sg->getChannels().begin();
                     k != sg->getChannels().end(); k++)
                {
                    Channel *c = *k;
                    Bandwidth b;
                    /*ceil*/
                    b = (c->getSrcPort()->getRateOfScenario(s)
                         * repVec[c->getSrcActor()->getId()]
                         * c->getTokenSizeOfScenario(s)
                         * applicationGraph->getThroughputConstraint());

                    gb->getConstraintsOfScenario(s)->getConstraintsOfChannel(c)
                    ->setBandwidth(b);

                    logMsg("Bandwidth channel '" + c->getName()
                           + "' in scenario graph '" + sg->getName() + "': "
                           + CString(b) + " bytes/time-units");
                }
            }
        }

        return true;
    }

    /**
     * bindGraphtoTiles()
     * Bind the application graph to the platform tiles. The function produces
     * Pareto trade-off space with different platform bindings. The number of
     * bindings are outputted by the function does not exceed the maxNrAppBindings.
     */
    bool TileBindingAlgoBase::bindGraphtoTiles(const uint maxNrAppBindings)
    {
        GraphBindings applicationBindings;
        list<CString> sortedActorNames;

        // Iterate over all platform bindings (except the initial binding)
        for (PlatformBindings::iterator i = platformBindings->begin();
             i != platformBindings->end(); i++)
        {
            PlatformBinding *pb = *i;

            if (pb->isInitialBinding())
                continue;

            // Create tile binding for binding b
            logMsg("Creating tile binding for binding '" + pb->getName() + "'");

            // Create an application binding given the platform binding pb
            applicationBindings.push_back(createGraphBinding(pb));

            // Sort actor names on criticality
            sortedActorNames = sortActorNamesOnCriticality();

            // Iterate over the actor names in order of criticality
            for (list<CString>::iterator i = sortedActorNames.begin();
                 i != sortedActorNames.end(); i++)
            {
                CString an = *i;

                // Is there at least one application binding?
                if (applicationBindings.empty())
                    return false;

                // Create all bindings for the actor
                createAllBindingsOfActor(an, applicationBindings);

                // Iterate over all partial application bindings
                for (GraphBindings::iterator j = applicationBindings.begin();
                     j != applicationBindings.end();)
                {
                    GraphBindingBase *b = *j;
                    bool validBinding = true;

                    // Iterate over the channel names in arbitrary order
                    for (map<CString, Channels>::iterator
                         k = channelNamesToChannels.begin();
                         k != channelNamesToChannels.end(); k++)
                    {
                        CString cn = k->first;

                        // Try to bind channel with name cn to platform
                        if (!createBindingOfChannel(cn, b))
                        {
                            // All possible bindings of channel to platfrom have a
                            // resource conflict. So, binding b cannot be realized
                            validBinding = false;
                            break;
                        }
                    }

                    // Channel to platform binding is not possible?
                    if (!validBinding)
                    {
                        // Erase binding and continue with next binding
                        applicationBindings.erase(j++);
                    }
                    else
                    {
                        // Compute cost of this binding
                        b->computeParetoQuantities(repetitionVectors, false);

                        // Next binding
                        j++;
                    }
                }

                // Remove all non-Pareto optimal bindings
                paretoMinimizationOfApplicationBindings(applicationBindings);

                // Reduce the number of application bindings
                reduceApplicationBindings(applicationBindings, maxNrAppBindings);
            }
        }

        // Insert new application bindings to the list of platform bindings
        replacePlatformBindings(applicationBindings);

        return true;
    }

    /**
     * estimateConnectionDelay()
     * Estimate the channel delay of channels which have been mapped to a
     * connection.
     */
    bool TileBindingAlgoBase::estimateConnectionDelay()
    {
        // Iterate over all platform bindings (except the initial binding)
        for (PlatformBindings::iterator i = platformBindings->begin();
             i != platformBindings->end(); i++)
        {
            PlatformBinding *pb = *i;
            GraphBindingConstraints *gb = pb->getGraphBindingConstraints();

            if (pb->isInitialBinding())
                continue;

            // Estimate connection delay for binding b
            logInfo("Estimate connection delay for binding '" + pb->getName() + "'");

            // Iterate over all scenarios
            for (Scenarios::iterator j = applicationGraph->getScenarios().begin();
                 j != applicationGraph->getScenarios().end(); j++)
            {
                Scenario *s = *j;
                ScenarioGraph *sg = s->getScenarioGraph();

                // Iterate over all channels
                for (Channels::iterator k = sg->getChannels().begin();
                     k != sg->getChannels().end(); k++)
                {
                    Channel *c = *k;

                    gb->getConstraintsOfScenario(s)->getConstraintsOfChannel(c)
                    ->setDelay(0);

                    logMsg("Delay of channel '" + c->getName() + "' in scenario" +
                           + " graph '" + sg->getName() + "': 0"
                           + " time-units");
                }
            }
        }

        return true;
    }

    /**
     * constructTileSchedules()
     * Construct static-order and TDMA schedules for the application on all platform
     * tiles.
     */
    bool TileBindingAlgoBase::constructTileSchedules(const uint maxNrAppBindings)
    {
        EXCEPTION("%s: This should be implemented in the platform specific "
                  "implementation", __FUNCTION__);
        return false;
    }

    /**
     * allocateActorBinding()
     * The function creates an actor bindings for the actor with the supplied name.
     */
    bool TileBindingAlgoBase::allocateActorBinding(GraphBindingBase *applicationBinding,
            CString &name, ActorBinding &actorBinding)
    {
        // Iterate over the actor with the specified name
        for (Actors::iterator j = actorNamesToActors[name].begin();
             j != actorNamesToActors[name].end(); j++)
        {
            Actor *a = *j;

            // Iterate over all scenarios in which the actor appears
            for (Scenarios::iterator k = actorToScenarios[a].begin();
                 k != actorToScenarios[a].end(); k++)
            {
                Scenario *s = *k;
                if (!applicationBinding->bindActorToTile(s, a, actorBinding))
                {
                    return false;
                }
            }
        }

        return true;
    }

    /**
     * releaseActorBinding()
     * The function removes the specified actor binding.
     */
    void TileBindingAlgoBase::releaseActorBinding(GraphBindingBase *applicationBinding,
            CString &name)
    {
        // Iterate over the actor with the specified name
        for (Actors::iterator j = actorNamesToActors[name].begin();
             j != actorNamesToActors[name].end(); j++)
        {
            Actor *a = *j;

            // Iterate over all scenarios in which the actor appears
            for (Scenarios::iterator k = actorToScenarios[a].begin();
                 k != actorToScenarios[a].end(); k++)
            {
                Scenario *s = *k;

                applicationBinding->removeActorToTileBinding(s, a);
            }
        }
    }

    /**
     * allocateChannelBinding()
     * The function creates a channel binding for all channels which match the
     * supplied name.
     */
    bool TileBindingAlgoBase::allocateChannelBinding(GraphBindingBase *applicationBinding,
            CString &name, ChannelBinding &channelBinding)
    {
        // Iterate over the channel with the specified name
        for (Channels::iterator j = channelNamesToChannels[name].begin();
             j != channelNamesToChannels[name].end(); j++)
        {
            Channel *c = *j;

            // Iterate over all scenarios in which the channel appears
            for (Scenarios::iterator k = channelToScenarios[c].begin();
                 k != channelToScenarios[c].end(); k++)
            {
                Scenario *s = *k;

                // Bind channel to connection?
                if (channelBinding.connection != NULL)
                {
                    if (!applicationBinding->bindChannelToConnection(s, c,
                            channelBinding))
                    {
                        return false;
                    }
                }
                else
                {
                    if (!applicationBinding->bindChannelToTile(s, c, channelBinding))
                        return false;
                }
            }
        }

        return true;
    }

    /**
     * releaseChannelBinding()
     * The function removes the specified channel binding.
     */
    void TileBindingAlgoBase::releaseChannelBinding(GraphBindingBase *applicationBinding,
            CString &name)
    {
        // Iterate over the channel with the specified name
        for (Channels::iterator j = channelNamesToChannels[name].begin();
             j != channelNamesToChannels[name].end(); j++)
        {
            Channel *c = *j;

            // Iterate over all scenarios in which the channel appears
            for (Scenarios::iterator k = channelToScenarios[c].begin();
                 k != channelToScenarios[c].end(); k++)
            {
                Scenario *s = *k;

                applicationBinding->removeChannelBinding(s, c);
            }
        }
    }

    /**
     * findActorBindingCandidates()
     * The function returns a list with all possible actor bindings. Note that
     * it is not guaranteed that sufficient resources are available to actually
     * create a binding.
     */
    ActorBindings TileBindingAlgoBase::findActorBindingCandidates(
        GraphBindingBase *applicationBinding, Actor *a)
    {
        ActorBindings bindings;

        // Has an actor with the same name already been mapped?
        if (hasActorBindingBeenFixed(applicationBinding, a->getName()))
        {
            bindings.push_back(getActorBinding(applicationBinding, a->getName()));
            return bindings;
        }

        // Iterate over all possible processors
        for (Processors::iterator
             i = actorNamesToProcessorCandidates[a->getName()].begin();
             i != actorNamesToProcessorCandidates[a->getName()].end(); i++)
        {
            Processor *p = *i;
            Tile *t = p->getTile();

            // Iterate over all memories within the tile
            for (Memories::iterator j = t->getMemories().begin();
                 j != t->getMemories().end(); j++)
            {
                Memory *m = *j;

                bindings.push_back(ActorBinding(p, m));
            }
        }

        return bindings;
    }

    /**
     * findChannelBindingCandidates()
     * The function returns a list with all possible channel bindings. Note that
     * it is not guaranteed that sufficient resources are available to actually
     * create a binding. Channel binding candidates can only be created when the
     * source and destination actor of the channel are already mapped to a
     * processor.
     */
    ChannelBindings TileBindingAlgoBase::findChannelBindingCandidates(
        GraphBindingBase *applicationBinding, Channel *c)
    {
        Tile *srcTile, *dstTile;
        ChannelBindings bindings;

        // Has a channel with the same name already been mapped?
        if (hasChannelBindingBeenFixed(applicationBinding, c->getName()))
        {
            bindings.push_back(getChannelBinding(applicationBinding, c->getName()));
            return bindings;
        }

        // Are the source actor and destination actor bound to a tile?
        if (hasActorBindingBeenFixed(applicationBinding, c->getSrcActor()->getName())
            && hasActorBindingBeenFixed(applicationBinding,
                                        c->getDstActor()->getName()))
        {
            srcTile = getActorBinding(applicationBinding, c->getSrcActor()
                                      ->getName()).processor->getTile();
            dstTile = getActorBinding(applicationBinding, c->getDstActor()
                                      ->getName()).processor->getTile();
        }
        else
        {
            return bindings;
        }

        // Are the source and destination tile equal?
        if (srcTile == dstTile)
        {
            // Iterate over all memories within the tile
            for (Memories::iterator i = srcTile->getMemories().begin();
                 i != srcTile->getMemories().end(); i++)
            {
                Memory *m = *i;

                bindings.push_back(ChannelBinding(m));
            }
        }
        else
        {
            // Create all possible NI, connection and memory combinations
            for (NetworkInterfaces::iterator
                 i = srcTile->getNetworkInterfaces().begin();
                 i != srcTile->getNetworkInterfaces().end(); i++)
            {
                NetworkInterface *nSrc = *i;

                // Iterate over the outgoing connections
                for (Connections::iterator j = nSrc->getOutConnections().begin();
                     j != nSrc->getOutConnections().end(); j++)
                {
                    Connection *co = *j;
                    NetworkInterface *nDst = co->getDstNetworkInterface();

                    // Connection to the correct destination tile?
                    if (nDst->getTile() == dstTile)
                    {
                        // Iterate over all memories in the source tile
                        for (Memories::iterator k = srcTile->getMemories().begin();
                             k != srcTile->getMemories().end(); k++)
                        {
                            Memory *mSrc = *k;

                            // Iterate over all memories in the destination tile
                            for (Memories::iterator
                                 k = dstTile->getMemories().begin();
                                 k != dstTile->getMemories().end(); k++)
                            {
                                Memory *mDst = *k;

                                // Add channel binding
                                bindings.push_back(ChannelBinding(mSrc, nSrc,
                                                                  mDst, nDst, co));
                            }
                        }
                    }
                }
            }
        }

        return bindings;
    }

    /**
     * hasActorBindingBeenFixed()
     * The function returns true when an actor with the supplied name has been
     * bound to the platform in at least one scenario. Otherwise the function
     * return false.
     */
    bool TileBindingAlgoBase::hasActorBindingBeenFixed(
        GraphBindingBase *applicationBinding, const CString &name) const
    {
        // Iterate over the actor bindings in the application binding
        for (map<Scenario *, map<Actor *, ActorBinding> >::const_iterator
             i = applicationBinding->getActorBindings().begin();
             i != applicationBinding->getActorBindings().end(); i++)
        {
            for (map<Actor *, ActorBinding>::const_iterator
                 j = i->second.begin(); j != i->second.end(); j++)
            {
                if (j->first->getName() == name)
                    return true;
            }
        }

        return false;
    }

    /**
     * getActorBinding()
     * The function returns an actor binding of an actor with the supplied name.
     * When no binding of an actor with this name exists, an exception is thrown.
     */
    ActorBinding TileBindingAlgoBase::getActorBinding(
        GraphBindingBase *applicationBinding, const CString &name) const
    {
        for (map<Scenario *, map<Actor *, ActorBinding> >::const_iterator
             i = applicationBinding->getActorBindings().begin();
             i != applicationBinding->getActorBindings().end(); i++)
        {
            for (map<Actor *, ActorBinding>::const_iterator
                 j = i->second.begin(); j != i->second.end(); j++)
            {
                if (j->first->getName() == name)
                    return j->second;
            }
        }

        throw CException("Actor with name '" + name + "' has not been bound.");
    }

    /**
     * hasChannelBindingBeenFixed()
     * The function returns true when a channel with the supplied name has been
     * bound to the platform in at least one scenario. Otherwise the function
     * return false.
     */
    bool TileBindingAlgoBase::hasChannelBindingBeenFixed(
        GraphBindingBase *applicationBinding, const CString &name) const
    {
        // Iterate over the channel bindings in the application binding
        for (map<Scenario *, map<Channel *, ChannelBinding> >::const_iterator
             i = applicationBinding->getChannelBindings().begin();
             i != applicationBinding->getChannelBindings().end(); i++)
        {
            for (map<Channel *, ChannelBinding>::const_iterator
                 j = i->second.begin(); j != i->second.end(); j++)
            {
                if (j->first->getName() == name)
                    return true;
            }
        }

        return false;
    }

    /**
     * getChannelBinding()
     * The function returns a chanenl binding of a channel with the supplied name.
     * When no binding of a channel with this name exists, an exception is thrown.
     */
    ChannelBinding TileBindingAlgoBase::getChannelBinding(
        GraphBindingBase *applicationBinding, const CString &name) const
    {
        for (map<Scenario *, map<Channel *, ChannelBinding> >::const_iterator
             i = applicationBinding->getChannelBindings().begin();
             i != applicationBinding->getChannelBindings().end(); i++)
        {
            for (map<Channel *, ChannelBinding>::const_iterator
                 j = i->second.begin(); j != i->second.end(); j++)
            {
                if (j->first->getName() == name)
                    return j->second;
            }
        }

        throw CException("Channel with name '" + name + "' has not been bound.");
    }

    /**
     * initGraphRelations()
     * Initialize the relations between the application and platform graph.
     */
    void TileBindingAlgoBase::initGraphRelations()
    {
        // Clear existing repetition vectors
        repetitionVectors.clear();

        // Clear existing mapping of actors names to actors
        actorNamesToActors.clear();

        // Clear existing mapping of actors to scenarios
        actorToScenarios.clear();

        // Clear exisiting mapping of actor names to processor candidates
        actorNamesToProcessorCandidates.clear();

        // Clear exisiting mapping of channel names to channels
        channelNamesToChannels.clear();

        // Clear existing mapping of channels to scenarios
        channelToScenarios.clear();

        // Repetition vectors
        for (Scenarios::iterator i = applicationGraph->getScenarios().begin();
             i != applicationGraph->getScenarios().end(); i++)
        {
            Scenario *s = *i;
            ScenarioGraph *sg = s->getScenarioGraph();

            repetitionVectors[s] = computeRepetitionVector(sg, s);
        }

        // Mapping of actor names to actors
        for (Scenarios::iterator i = applicationGraph->getScenarios().begin();
             i != applicationGraph->getScenarios().end(); i++)
        {
            Scenario *s = *i;
            ScenarioGraph *sg = s->getScenarioGraph();

            for (Actors::iterator j = sg->getActors().begin();
                 j != sg->getActors().end(); j++)
            {
                Actor *a = *j;
                actorNamesToActors[a->getName()].push_back(a);
                actorToScenarios[a].insert(s);
            }
        }

        // Mapping of actor names to processor binding candidates
        for (map<CString, Actors >::iterator i = actorNamesToActors.begin();
             i != actorNamesToActors.end(); i++)
        {
            set<CString> procTypes;

            // Compute set of processor types to which actor with given name can
            // always be mapped.
            for (Actors::iterator j = i->second.begin(); j != i->second.end(); j++)
            {
                set<CString> types;
                Actor *a = *j;

                // Create set of all processor types supported by this actor
                for (Actor::ProcessorTypes::const_iterator
                     k = a->getProcessorTypes().begin();
                     k != a->getProcessorTypes().end(); k++)
                {
                    types.insert(k->type);
                }

                // Take union of processor types supported by this actor and by
                // actors with the same name
                if (j == i->second.begin())
                {
                    procTypes = types;
                }
                else
                {
                    set<CString>::iterator s1 = types.begin();
                    set<CString>::iterator s2 = procTypes.begin();
                    set<CString> tmp;

                    while (s1 != types.end() && s2 != procTypes.end())
                    {
                        if (*s1 < * s2)
                        {
                            ++s1;
                        }
                        else if (*s2 < *s1)
                        {
                            ++s2;
                        }
                        else
                        {
                            tmp.insert(*s1++);
                            s2++;
                        }
                    }

                    procTypes = set<CString>(tmp.begin(), tmp.end());
                }
            }

            // Find all processors which are of a type inside the set procTypes
            for (Tiles::iterator
                 j = platformGraph->getTiles().begin();
                 j != platformGraph->getTiles().end(); j++)
            {
                Tile *t = *j;

                for (Processors::iterator k =
                         t->getProcessors().begin();
                     k != t->getProcessors().end(); k++)
                {
                    Processor *p = *k;

                    // Is the processor of a type supported by the actor?
                    if (procTypes.find(p->getType()) != procTypes.end())
                    {
                        actorNamesToProcessorCandidates[i->first].push_back(p);
                    }
                }
            }
        }

        // Mapping of channel names to channels
        for (Scenarios::iterator i = applicationGraph->getScenarios().begin();
             i != applicationGraph->getScenarios().end(); i++)
        {
            Scenario *s = *i;
            ScenarioGraph *sg = s->getScenarioGraph();

            for (Channels::iterator j = sg->getChannels().begin();
                 j != sg->getChannels().end(); j++)
            {
                Channel *c = *j;
                channelNamesToChannels[c->getName()].push_back(c);
                channelToScenarios[c].insert(s);
            }
        }
    }

    /**
     * sortActorNamesOnCriticality()
     * The function returns a list of actor names in which those names have been
     * ordered (high-to-low) based on the criticality of the actors.
     */
    list<CString> TileBindingAlgoBase::sortActorNamesOnCriticality()
    {
        map<Scenario *, uint> scenarioCnt;
        map<Scenario *, map<Actor *, Time> > actorExecTime;
        uint maxScenarioCnt, maxActorFiringCnt;
        map<CString, double> actorCost;
        Time maxActorExecTime;
        list<CString> actorNames;

        // Compute scenario occurance count
        scenarioCnt = computeScenariosOccuranceCount(applicationGraph);

        // Find maximum scenario occurance count over all scenarios
        maxScenarioCnt = 0;
        for (map<Scenario *, uint>::iterator i = scenarioCnt.begin();
             i != scenarioCnt.end(); i++)
        {
            maxScenarioCnt = max(maxScenarioCnt, i->second);
        }
        ASSERT(maxScenarioCnt != 0, "max cannot be zero.");

        // Find maximum repetition vector entry
        maxActorFiringCnt = 0;
        for (map<Scenario *, RepetitionVector>::iterator
             i = repetitionVectors.begin();
             i != repetitionVectors.end(); i++)
        {
            for (RepetitionVector::iterator j = i->second.begin();
                 j != i->second.end(); j++)
            {
                maxActorFiringCnt = max(maxActorFiringCnt, (uint)(*j));
            }
        }
        ASSERT(maxActorFiringCnt != 0, "max cannot be zero.");

        // Compute maximum actor execution time
        maxActorExecTime = 0;
        for (Scenarios::iterator i = applicationGraph->getScenarios().begin();
             i != applicationGraph->getScenarios().end(); i++)
        {
            Scenario *s = *i;
            ScenarioGraph *sg = s->getScenarioGraph();

            for (Actors::iterator j = sg->getActors().begin();
                 j != sg->getActors().end(); j++)
            {
                Actor *a = *j;

                // Iterate over the processor types
                actorExecTime[s][a] = 0;
                for (Actor::ProcessorTypes::const_iterator
                     k = a->getProcessorTypes().begin();
                     k != a->getProcessorTypes().end(); k++)
                {
                    const Actor::ProcessorType &p = *k;

                    // Maximum execution time of actor a in scenario s
                    actorExecTime[s][a] = max(actorExecTime[s][a],
                                              a->getExecutionTimeOfScenario(s, p.type));

                    // Maximum actor execution time over all actors and scenarios
                    maxActorExecTime = max(maxActorExecTime, actorExecTime[s][a]);
                }
            }
        }
        ASSERT(maxActorExecTime != 0, "max cannot be zero.");

        // Compute actor cost
        for (map<CString, Actors>::iterator i = actorNamesToActors.begin();
             i != actorNamesToActors.end(); i++)
        {
            CString an = i->first;

            // Initialize cost to zero
            actorCost[an] = 0;

            // Iterate over all actors with this name
            for (Actors::iterator j = i->second.begin();
                 j != i->second.end(); j++)
            {
                Actor *a = *j;

                // Iterate over all scenarios in which this actor appears
                for (Scenarios::iterator k = actorToScenarios[a].begin();
                     k != actorToScenarios[a].end(); k++)
                {
                    Scenario *s = *k;

                    actorCost[an] = actorCost[an]
                                    + ((scenarioCnt[s]
                                        / (double) maxScenarioCnt)
                                       * (repetitionVectors[s][a->getId()]
                                          / (double) maxActorFiringCnt)
                                       * (actorExecTime[s][a]
                                          / (double) maxActorExecTime));
                }
            }
        }

        // Sort actors on cost
        for (map<CString, double>::iterator i = actorCost.begin();
             i != actorCost.end(); i++)
        {
            list<CString>::iterator j = actorNames.end();

            // Find first actor j in list with lower occurance count then i
            while (j != actorNames.end() && actorCost[*j] > i->second)
            {
                j++;
            }

            // Add actor i to the list before actor j
            actorNames.insert(j, i->first);
        }

        return actorNames;
    }

    /**
     * computeScenariosOccuranceCount()
     * The function returns the occurance frequency of the scenarios in the FSM.
     */
    map<Scenario *, uint> TileBindingAlgoBase::computeScenariosOccuranceCount(
        Graph *g) const
    {
        map<Scenario *, uint> scenarioCnt;

        // No FSM specified?
        if (g->getFSM() == NULL)
        {
            scenarioCnt[g->getDefaultScenario()] = 1;
            return scenarioCnt;
        }

        // Iterate over the states in the FSM
        for (FSMstates::iterator i = g->getFSM()->getStates().begin();
             i != g->getFSM()->getStates().end(); i++)
        {
            scenarioCnt[(*i)->getScenario()]++;
        }

        return scenarioCnt;
    }

    /**
     * sortScenariosOnOccuranceCount()
     * The function returns a list where the scenarios in the graph g are sorted
     * on their occurance frequency in the FSM. The ordering is from high to low.
     */
    list<Scenario *> TileBindingAlgoBase::sortScenariosOnOccuranceCount(Graph *g) const
    {
        map<Scenario *, uint> scenarioCnt;
        list<Scenario *> scenarios;

        // No FSM specified?
        if (g->getFSM() == NULL)
        {
            scenarios.push_back(g->getDefaultScenario());
            return scenarios;
        }

        // Iterate over the states in the FSM
        for (FSMstates::iterator i = g->getFSM()->getStates().begin();
             i != g->getFSM()->getStates().end(); i++)
        {
            scenarioCnt[(*i)->getScenario()]++;
        }

        // Sort scenarios on occurance frequency
        for (map<Scenario *, uint>::iterator i = scenarioCnt.begin();
             i != scenarioCnt.end(); i++)
        {
            list<Scenario *>::iterator j = scenarios.end();

            // Find first scenario j in list with lower occurance count then i
            while (j != scenarios.end() && scenarioCnt[*j] > i->second)
            {
                j++;
            }

            // Add scenario i to the list before scenario j
            scenarios.insert(j, i->first);
        }

        return scenarios;
    }

    /**
     * createAllBindingsOfActor()
     * The function returns all possible bindings of the actors with the supplied
     * name on the platform using the (partial) bindings given in the list
     * bindings.
     */
    void TileBindingAlgoBase::createAllBindingsOfActor(CString &name,
            GraphBindings &bindings)
    {
        GraphBindings newBindings;

        // Iterate over the existing partial bindings
        for (GraphBindings::iterator i = bindings.begin(); i != bindings.end(); i++)
        {
            ActorBindings actorBindingCandidates;
            GraphBindingBase *b = *i;

            // Find all actor bindings for the actor name. Note that the unified
            // mapping implies that all actors with the same name have the same
            // actor binding candidates.
            Actor *a = actorNamesToActors[name].front();
            actorBindingCandidates = findActorBindingCandidates(b, a);

            // Binding options available for this actor?
            if (!actorBindingCandidates.empty())
            {
                GraphBindingBase *bNew = b->clone();
                bNew->getPlatformBinding()->setName(newBindings.size());

                // Try all binding candidates
                for (ActorBindings::iterator j = actorBindingCandidates.begin();
                     j != actorBindingCandidates.end(); j++)
                {
                    ActorBinding ab = *j;

                    // Try resource allocation for this actor binding
                    if (allocateActorBinding(bNew, name, ab))
                    {
                        // Add binding to the set of solutions
                        newBindings.push_back(bNew);

                        // Create a new binding based on binding b
                        bNew = b->clone();
                        bNew->getPlatformBinding()->setName(newBindings.size());
                    }
                }

                // Cleanup
                delete bNew;
            }
        }

        // Cleanup existing bindings
        for (GraphBindings::iterator i = bindings.begin(); i != bindings.end(); i++)
            delete(*i);

        // New bindings become bindings
        bindings = newBindings;
    }

    /**
     * createBindingOfChannel()
     * The function tries to bind the channels with the supplied name to the
     * platform using the binding b. A binding can only be made when the source
     * and destination actor of the channel are already bound to a tile. The
     * function will try all possible bindings till a valid binding is found. When
     * no valid binding exists (due to resource constraints), the function returns
     * false. Otherwise the function returns true. So, the function also returns
     * true when no binding can be made because the source or destination actor
     * is not bound.
     */
    bool TileBindingAlgoBase::createBindingOfChannel(CString &name, GraphBindingBase *b)
    {
        ChannelBindings channelBindingCandidates;

        // Find all channel bindings for the channel name. Note that the unified
        // mapping implies that all channels with the same name have the same
        // channel binding candidates.
        Channel *c = channelNamesToChannels[name].front();
        channelBindingCandidates = findChannelBindingCandidates(b, c);

        // No channel binding candidates exist?
        if (channelBindingCandidates.empty())
            return true;

        // Iterate over the channel binding candidates
        for (ChannelBindings::iterator i = channelBindingCandidates.begin();
             i != channelBindingCandidates.end(); i++)
        {
            ChannelBinding &cb =  *i;

            // Does this channel binding produce a valid binding?
            if (allocateChannelBinding(b, name, cb))
                return true;
        }

        return false;
    }

    /**
     * paretoMinimizationOfApplicationBindings()
     * The function removes all non-Pareto application bindings from the list of
     * application bindings. The function implements the Simple Cull algorithm.
     */
    void TileBindingAlgoBase::paretoMinimizationOfApplicationBindings(
        GraphBindings &applicationBindings)
    {
        GraphBindings newBindings;

        // Iterate over all application bindings
        while (!applicationBindings.empty())
        {
            GraphBindings::iterator i = applicationBindings.begin();
            GraphBindingBase *c = *i;
            bool dominated = false;

            for (GraphBindings::iterator j = newBindings.begin();
                 j != newBindings.end();)
            {
                GraphBindingBase *d = *j;

                // Check for Pareto dominance
                if (c->paretoDominates(d))
                {
                    // Binding d is dominated by c, so d can be removed
                    delete d;

                    // Remove binding from the list
                    newBindings.erase(j++);
                }
                else if (d->paretoDominates(c))
                {
                    // Binding c is dominated by d
                    dominated = true;
                    break;
                }
                else
                {
                    // Bindings c and d do not dominate each other
                    j++;
                }
            }

            // Binding c is Pareto optimal?
            if (!dominated)
            {
                // Add binding to the new list
                newBindings.push_back(c);
            }
            else
            {
                // Binding c is not Pareto optimal; destroy this binding
                delete c;
            }

            // Remove binding c from the list
            applicationBindings.erase(i++);
        }

        // The new bindings become the application bindings
        applicationBindings = newBindings;
    }

    /**
     * reduceApplicationBindings()
     * The function reduces the number of application bindings in the list to the
     * specified maximum.
     */
    void TileBindingAlgoBase::reduceApplicationBindings(
        GraphBindings &applicationBindings, const uint maxNrAppBindings)
    {
        map<CString, GraphBindingBase *> mappingToGraphBinding;
        map<CString, double> distance;
        GraphBindings bindings;

        // No reduction is needed?
        if (applicationBindings.size() <= maxNrAppBindings)
            return;

        // Initialize distances to infinity
        for (GraphBindings::iterator i = applicationBindings.begin();
             i != applicationBindings.end(); i++)
        {
            GraphBindingBase *b = *i;

            distance[b->getPlatformBinding()->getName()] = INFINITY;
            mappingToGraphBinding[b->getPlatformBinding()->getName()] = b;
        }

        // Select application bindings
        while (bindings.size() < maxNrAppBindings)
        {
            map<CString, double>::iterator i = distance.begin();

            // Select binding with largest distance to bindings already in the list
            for (map<CString, double>::iterator j = i;
                 j != distance.end(); j++)
            {
                if (j->second > i->second)
                    i = j;
            }

            // Add the binding i to the new list of bindings
            bindings.push_back(mappingToGraphBinding[i->first]);

            // Update minimal distance between bindings in the list bindings and
            // bindings in the list applicationBindings
            for (GraphBindings::iterator j = applicationBindings.begin();
                 j != applicationBindings.end(); j++)
            {
                GraphBindingBase *b = *j;
                CString mappingName = b->getPlatformBinding()->getName();

                distance[mappingName] = min(b->getParetoQuantities().distance(
                                                mappingToGraphBinding[i->first]->getParetoQuantities()),
                                            distance[mappingName]);
            }

            // Remove the binding i from the old list of bindings
            applicationBindings.remove(mappingToGraphBinding[i->first]);

            // Remove the binding i from the distance
            distance.erase(i);
        }

        // Destroy the existing applicationBindings (if any)
        for (GraphBindings::iterator i = applicationBindings.begin();
             i != applicationBindings.end(); i++)
        {
            delete(*i);
        }

        // The new bindings become the application bindings
        applicationBindings = bindings;
    }

    /**
     * replacePlatformBindings()
     * The function replaces the existing platform bindings with the platform
     * bindings given by the graph bindings. The initial binding (if present) is
     * preserved.
     */
    void TileBindingAlgoBase::replacePlatformBindings(GraphBindings newBindings)
    {
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

        // Update the list of platform bindings
        for (GraphBindings::iterator i = newBindings.begin();
             i != newBindings.end(); i++)
        {
            GraphBindingBase *gb = *i;
            PlatformBinding *pb = gb->getPlatformBinding();

            // Add a clone of the platform binding to the list of platform bindings
            platformBindings->push_back(pb->clone(*pb));
        }
    }
    /**
     * This function is used by #TileBindingAlgoBase to create GraphBinding object.
     * So if you want another GraphBinding object, override this function.
     *
     * @param pb a #PlatformBinging
     * @return a new #GraphBindingBase algorithm
     */
    GraphBindingBase *TileBindingAlgoBase::createGraphBinding(PlatformBinding *pb)
    {
        return new GraphBindingBase(pb);
    }
} // End namespace FSMSADF

