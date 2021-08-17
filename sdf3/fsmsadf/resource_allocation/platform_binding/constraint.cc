/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   constraint.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 12, 2009
 *
 *  Function        :   Graph binding constraints.
 *
 *  History         :
 *      12-05-09    :   Initial version.
 *
 * $Id: constraint.cc,v 1.1 2009-12-23 13:37:22 sander Exp $
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

#include "constraint.h"

namespace FSMSADF
{

    /**
     * ChannelBindingConstraints()
     * Constructor.
     */
    ChannelBindingConstraints::ChannelBindingConstraints(const Channel *c)
        : channel(c),
          bufferSizeConstraint(false),
          bandwidthConstraint(false),
          delayConstraint(false)
    {
    }

    /**
     * ~ChannelBindingConstraints()
     * Destructor.
     */
    ChannelBindingConstraints::~ChannelBindingConstraints()
    {
    }

    /**
     * clone()
     * Create a cloned copy of this channel binding constraints object. The channel
     * relation and all associated scenarios are automatically updated to the
     * supplied channel.
     */
    ChannelBindingConstraints *ChannelBindingConstraints::clone(Channel *c)
    {
        ChannelBindingConstraints *cb = new ChannelBindingConstraints(c);

        // Buffer size
        if (hasBufferSizeConstraint())
            cb->setBufferSize(getBufferSize());

        // Delay
        if (hasDelayConstraint())
            cb->setDelay(getDelay());

        // Bandwidth
        if (hasBandwidthConstraint())
            cb->setBandwidth(getBandwidth());

        return cb;
    }

    /**
     * constructFromXML()
     * Construct channel binding constraints for scenario s from XML data.
     */
    void ChannelBindingConstraints::constructFromXML(
        const CNodePtr channelConstraintsNode)
    {
        // Buffer size
        if (CHasChildNode(channelConstraintsNode, "bufferSize"))
        {
            CNode *bufferSizeNode = CGetChildNode(channelConstraintsNode,
                                                  "bufferSize");
            BufferSize buf;

            buf[BufferLocation::Mem] = SIZE_MAX;
            buf[BufferLocation::Src] = SIZE_MAX;
            buf[BufferLocation::Dst] = SIZE_MAX;

            if (CHasAttribute(bufferSizeNode, BufferLocation::Src))
                buf[BufferLocation::Src] = CGetAttribute(bufferSizeNode, BufferLocation::Src);

            if (CHasAttribute(bufferSizeNode, BufferLocation::Dst))
                buf[BufferLocation::Dst] = CGetAttribute(bufferSizeNode, BufferLocation::Dst);

            if (CHasAttribute(bufferSizeNode, BufferLocation::Mem))
                buf[BufferLocation::Mem] = CGetAttribute(bufferSizeNode, BufferLocation::Mem);

            setBufferSize(buf);
        }

        // Bandwidth
        if (CHasChildNode(channelConstraintsNode, "bandwidth"))
        {
            CNode *bwNode = CGetChildNode(channelConstraintsNode, "bandwidth");

            if (CHasAttribute(bwNode, "min"))
            {
                setBandwidth(CGetAttribute(bwNode, "min"));
            }
        }

        // Delay
        if (CHasChildNode(channelConstraintsNode, "delay"))
        {
            CNode *delayNode = CGetChildNode(channelConstraintsNode, "delay");

            if (CHasAttribute(delayNode, "min"))
            {
                setDelay(CGetAttribute(delayNode, "min"));
            }
        }
    }

    /**
     * convertToXML()
     * Convert channel bindings constraints of scenario s to XML data.
     */
    void ChannelBindingConstraints::convertToXML(
        const CNodePtr channelConstraintsNode)
    {
        // Channel
        CAddAttribute(channelConstraintsNode, "channel", getChannel()->getName());

        // Buffer size
        if (hasBufferSizeConstraint())
        {
            BufferSize buf = getBufferSize();
            CNode *bufferSizeNode = CAddNode(channelConstraintsNode, "bufferSize");

            /// @todo ugly looking, would be nice todo this nicer.
            CAddAttribute(bufferSizeNode,
                          BufferLocation::toString(BufferLocation::Mem),
                          buf[BufferLocation::Mem]);
            CAddAttribute(bufferSizeNode,
                          BufferLocation::toString(BufferLocation::Src),
                          buf[BufferLocation::Src]);
            CAddAttribute(bufferSizeNode,
                          BufferLocation::toString(BufferLocation::Dst),
                          buf[BufferLocation::Dst]);
        }

        // Bandwidth
        if (hasBandwidthConstraint())
        {
            CNode *bwNode = CAddNode(channelConstraintsNode, "bandwidth");
            CAddAttribute(bwNode, "min", getBandwidth());
        }

        // Delay
        if (hasDelayConstraint())
        {
            CNode *delayNode = CAddNode(channelConstraintsNode, "delay");
            CAddAttribute(delayNode, "min", getDelay());
        }
    }

    /**
     * getBufferSize ()
     * The function returns the buffer size of this channel. When no buffer size is
     * specified, the an exception is thrown.
     */
    BufferSize ChannelBindingConstraints::getBufferSize() const
    {
        if (hasBufferSizeConstraint())
            return bufferSize;

        throw CException("Channel '" + getChannel()->getName() +
                         "' has no buffer size.");
    }

    /**
     * setBufferSize ()
     * Set the buffer size of this channel.
     */
    void ChannelBindingConstraints::setBufferSize(const BufferSize sz)
    {
        bufferSize = sz;
        bufferSizeConstraint = true;
    }

    /**
     * hasBufferSizeConstraint ()
     * The function returns true when a buffer size has been specified. Otherwise
     * the function returns false.
     */
    bool ChannelBindingConstraints::hasBufferSizeConstraint() const
    {
        return bufferSizeConstraint;
    }

    /**
     * getDelay ()
     * The function returns the delay of this channel. When no delay has been
     * specified, an exception is thrown.
     */
    Time ChannelBindingConstraints::getDelay() const
    {
        if (hasDelayConstraint())
            return delay;

        throw CException("Channel '" + getChannel()->getName() + "' has no delay.");
    }

    /**
     * setDelay ()
     * Set the delay of this channel.
     */
    void ChannelBindingConstraints::setDelay(const Time t)
    {
        delay = t;
        delayConstraint = true;
    }

    /**
     * hasDelayConstraint ()
     * The function returns true when a delay has been specified. Otherwise the
     * function returns false.
     */
    bool ChannelBindingConstraints::hasDelayConstraint() const
    {
        return delayConstraint;
    }

    /**
     * getBandwidth ()
     * The function returns the bandwidth of this channel. When no bandwidth has
     * been specified, an exception is thrown.
     */
    Bandwidth ChannelBindingConstraints::getBandwidth() const
    {
        if (hasBandwidthConstraint())
            return bandwidth;

        throw CException("Channel '" + getChannel()->getName() +
                         "' has no bandwidth.");
    }

    /**
     * setBandwidth ()
     * Set the bandwidth of this channel.
     */
    void ChannelBindingConstraints::setBandwidth(const Bandwidth b)
    {
        bandwidth = b;
        bandwidthConstraint = true;
    }

    /**
     * hasBandwidthConstraint ()
     * The function returns true when a bandwidth has been specified. Otherwise
     * the function returns false.
     */
    bool ChannelBindingConstraints::hasBandwidthConstraint() const
    {
        return bandwidthConstraint;
    }

    /**
     * ScenarioBindingConstraints()
     * Constructor.
     */
    ScenarioBindingConstraints::ScenarioBindingConstraints(const Scenario *s)
        : scenario(s)
    {
    }

    /**
     * ~GraphBindingConstraints()
     * Destructor.
     */
    ScenarioBindingConstraints::~ScenarioBindingConstraints()
    {
        for (map<const Channel *, ChannelBindingConstraints *>::iterator
             i = channelBindingConstraints.begin();
             i != channelBindingConstraints.end(); i++)
        {
            delete(i->second);
        }
    }

    /**
     * clone()
     * Create a cloned copy of this scenario binding object. The channel relations
     * are updated to the corresponding channels in the scenario graph of
     * scenario s.
     */
    ScenarioBindingConstraints *ScenarioBindingConstraints::clone(Scenario *s)
    {
        ScenarioBindingConstraints *bc = new ScenarioBindingConstraints(s);

        // Channel binding constraints
        for (map<const Channel *, ChannelBindingConstraints *>::iterator
             i = channelBindingConstraints.begin();
             i != channelBindingConstraints.end(); i++)
        {
            // Find corresponding channel in the scenario graph of scenario s
            ScenarioGraph *sg = s->getScenarioGraph();
            Channel *c = sg->getChannel(i->first->getName());

            // Set the channel binding constraints of channel c
            bc->setConstraintsOfChannel(c, i->second->clone(c));
        }

        return bc;
    }

    /**
     * constructFromXML()
     * Construct scenario binding constraints from XML data.
     */
    void ScenarioBindingConstraints::constructFromXML(
        const CNodePtr constraintsNode)
    {
        ScenarioGraph *sg = getScenario()->getScenarioGraph();

        // Iterate over all child nodes in the constraint node
        for (CNode *n = CGetChildNode(constraintsNode); n != NULL; n = CNextNode(n))
        {
            // Channel?
            if (CIsNode(n, "channelConstraints"))
            {
                // Find channel in the application graph
                if (!CHasAttribute(n, "name"))
                    throw CException("channelConstraints has no name identifier.");
                Channel *c = sg->getChannel(CGetAttribute(n, "name"));

                // Construct binding constraints
                getConstraintsOfChannel(c)->constructFromXML(n);
            }
        }
    }

    /**
     * convertToXML()
     * Convert the scenario binding constraints to XML data.
     */
    void ScenarioBindingConstraints::convertToXML(const CNodePtr constraintsNode)
    {
        // Iterate over all channel binding constraints
        for (map<const Channel *, ChannelBindingConstraints *>::iterator
             i = channelBindingConstraints.begin();
             i != channelBindingConstraints.end(); i++)
        {
            i->second->convertToXML(CAddNode(constraintsNode, "channelConstraints"));
        }
    }

    /**
     * getConstraintsOfChannel()
     * The function returns the channel binding constraints of channel c.
     */
    ChannelBindingConstraints *ScenarioBindingConstraints::
    getConstraintsOfChannel(const Channel *c)
    {
        // Create empty set of binding constraints when no constraints exist
        if (!hasChannelBindingConstraints(c))
            setConstraintsOfChannel(c, new ChannelBindingConstraints(c));

        return channelBindingConstraints.find(c)->second;
    }

    /**
     * setConstraintsOfChannel()
     * The function sets the channel binding constraints of channel c.
     */
    void ScenarioBindingConstraints::setConstraintsOfChannel(const Channel *c,
            ChannelBindingConstraints *bc)
    {
        channelBindingConstraints[c] = bc;
    }

    /**
     * hasChannelBindingConstraints()
     * The function returns true when channel c has channel binding constraints.
     * Otherwise the function returns false.
     */
    bool ScenarioBindingConstraints::hasChannelBindingConstraints(const Channel *c)
    {
        if (channelBindingConstraints.find(c) != channelBindingConstraints.end())
            return true;

        return false;
    }

    /**
     * GraphBindingConstraints()
     * Constructor.
     */
    GraphBindingConstraints::GraphBindingConstraints()
    {
    }

    /**
     * ~GraphBindingConstraints()
     * Destructor.
     */
    GraphBindingConstraints::~GraphBindingConstraints()
    {
        for (map<const Scenario *, ScenarioBindingConstraints *>::iterator
             i = scenarioBindingConstraints.begin();
             i != scenarioBindingConstraints.end(); i++)
        {
            delete(i->second);
        }
    }

    /**
     * clone()
     * Create a cloned copy of this graph binding object. The scenario relations
     * are updated to the corresponding scenario in the graph ag.
     */
    GraphBindingConstraints *GraphBindingConstraints::clone(Graph *ag)
    {
        GraphBindingConstraints *g = new GraphBindingConstraints;

        // Scenario binding constraints
        for (map<const Scenario *, ScenarioBindingConstraints *>::iterator
             i = scenarioBindingConstraints.begin();
             i != scenarioBindingConstraints.end(); i++)
        {
            // Find corresponding scenario in application graph ag
            Scenario *s = ag->getScenario(i->first->getName());

            // Set the scenario binding constraints of scenario s
            g->setConstraintsOfScenario(s, i->second->clone(s));
        }

        return g;
    }

    /**
     * constructFromXML()
     * Construct graph binding constraints for scenario s from XML data.
     */
    void GraphBindingConstraints::constructFromXML(Scenario *s,
            const CNodePtr constraintsNode)
    {
        getConstraintsOfScenario(s)->constructFromXML(constraintsNode);
    }

    /**
     * convertToXML()
     * Convert the graph binding constraints of scenario s to XML data.
     */
    void GraphBindingConstraints::convertToXML(Scenario *s,
            const CNodePtr constraintsNode)
    {
        getConstraintsOfScenario(s)->convertToXML(constraintsNode);
    }

    /**
     * getConstraintsOfScenario()
     * The function returns the scenario binding constraints of scenario s.
     */
    ScenarioBindingConstraints *GraphBindingConstraints::
    getConstraintsOfScenario(const Scenario *s)
    {
        // Create empty set of binding constraints when no constraints exist
        if (!hasScenarioBindingConstraints(s))
            setConstraintsOfScenario(s, new ScenarioBindingConstraints(s));

        return scenarioBindingConstraints.find(s)->second;
    }

    /**
     * setConstraintsOfScenario()
     * The function sets the scenario binding constraints of scenario s.
     */
    void GraphBindingConstraints::setConstraintsOfScenario(const Scenario *s,
            ScenarioBindingConstraints *bc)
    {
        scenarioBindingConstraints[s] = bc;
    }

    /**
     * hasScenarioBindingConstraints()
     * The function returns true when scenario s has scenario binding constraints.
     * Otherwise the function returns false.
     */
    bool GraphBindingConstraints::hasScenarioBindingConstraints(const Scenario *s)
    {
        if (scenarioBindingConstraints.find(s) != scenarioBindingConstraints.end())
            return true;

        return false;
    }

} // End namespace FSMSADF

