/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   constraint.h
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
 * $Id: constraint.h,v 1.1 2009-12-23 13:37:22 sander Exp $
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

#ifndef FSMSADF_RESOURCE_ALLOCATION_PLATFORM_BINDING_CONSTRAINT_H_INCLUDED
#define FSMSADF_RESOURCE_ALLOCATION_PLATFORM_BINDING_CONSTRAINT_H_INCLUDED

#include "../../base/graph.h"

namespace FSMSADF
{

    /**
     * ChannelBindingConstraints
     * Container for channel binding constraints.
     */
    class ChannelBindingConstraints
    {
        public:
            // Constructor
            ChannelBindingConstraints(const Channel *c);

            // Destructor
            ~ChannelBindingConstraints();

            // Construct
            ChannelBindingConstraints *clone(Channel *c);

            // Construct
            void constructFromXML(const CNodePtr channelConstraintsNode);

            // Convert
            void convertToXML(const CNodePtr channelConstraintsNode);

            // Channel
            const Channel *getChannel() const
            {
                return channel;
            };

            // Buffer size
            BufferSize getBufferSize() const;
            void setBufferSize(const BufferSize sz);
            bool hasBufferSizeConstraint() const;

            // Delay
            Time getDelay() const;
            void setDelay(const Time t);
            bool hasDelayConstraint() const;

            // Bandwidth
            Bandwidth getBandwidth() const;
            void setBandwidth(const Bandwidth b);
            bool hasBandwidthConstraint() const;

        private:
            // Channel
            const Channel *channel;

            // Buffer size
            BufferSize bufferSize;
            bool bufferSizeConstraint;

            // Bandwidth
            Bandwidth bandwidth;
            bool bandwidthConstraint;

            // Delay
            Time delay;
            bool delayConstraint;
    };

    /**
     * ScenarioBindingConstraints
     * Container for scenario binding constraints.
     */
    class ScenarioBindingConstraints
    {
        public:
            // Constructor
            ScenarioBindingConstraints(const Scenario *s);

            // Destructor
            ~ScenarioBindingConstraints();

            // Construct
            ScenarioBindingConstraints *clone(Scenario *s);

            // Construct
            void constructFromXML(const CNodePtr constraintsNode);

            // Convert
            void convertToXML(const CNodePtr constraintsNode);

            // Scenario
            const Scenario *getScenario() const
            {
                return scenario;
            };

            // Channel binding constraints
            ChannelBindingConstraints *getConstraintsOfChannel(const Channel *c);
            void setConstraintsOfChannel(const Channel *c,
                                         ChannelBindingConstraints *bc);
            bool hasChannelBindingConstraints(const Channel *c);

        private:
            // Scenario
            const Scenario *scenario;

            // Channel binding constraints
            map<const Channel *, ChannelBindingConstraints *> channelBindingConstraints;
    };

    /**
     * GraphBindingConstraints
     * Container for graph binding constraints.
     */
    class GraphBindingConstraints
    {
        public:
            // Constructor
            GraphBindingConstraints();

            // Destructor
            ~GraphBindingConstraints();

            // Construct
            GraphBindingConstraints *clone(Graph *ag);

            // Construct
            void constructFromXML(Scenario *s, const CNodePtr constraintsNode);

            // Convert
            void convertToXML(Scenario *s, const CNodePtr constraintsNode);

            // Scenario binding constraints
            ScenarioBindingConstraints *getConstraintsOfScenario(
                const Scenario *s);
            void setConstraintsOfScenario(const Scenario *s,
                                          ScenarioBindingConstraints *bc);
            bool hasScenarioBindingConstraints(const Scenario *s);

        private:
            // Scenario binding constraints
            map<const Scenario *, ScenarioBindingConstraints *> scenarioBindingConstraints;
    };

} // End namespace FSMSADF

#endif

