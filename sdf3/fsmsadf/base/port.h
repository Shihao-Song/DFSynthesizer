/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   port.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 22, 1009
 *
 *  Function        :   FSM-based SADF port
 *
 *  History         :
 *      22-04-09    :   Initial version.
 *
 * $Id: port.h,v 1.1.2.3 2010-04-22 07:07:10 mgeilen Exp $
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

#ifndef FSMSADF_BASE_PORT_H_INCLUDED
#define FSMSADF_BASE_PORT_H_INCLUDED

#include "type.h"
#include "scenario.h"

namespace FSMSADF
{

    // Rate on ports of graphs
    typedef uint Rate;

    // Forward class definition
    class Port;
    class Actor;
    class Channel;
    class ScenarioGraph;

    // List of port pointers
    typedef list<Port *> Ports;

    /**
     * Port
     * Port on an actor.
     */
    class Port : public GraphComponent
    {
        public:
            enum PortType { In, Out, Undef };

            // Constructor
            Port(GraphComponent c);

            // Destructor
            ~Port();

            // Construct
            Port *create(GraphComponent c) const;
            Port *createCopy(GraphComponent c) const;
            Port *clone(GraphComponent c) const;

            // Construct
            void constructFromXML(const CNodePtr portNode);

            // Convert
            void convertToXML(const CNodePtr portNode);

            // Isolate scenario
            void isolateScenario(Scenario *s);

            // Type
            PortType getType() const
            {
                return type;
            };
            CString getTypeAsString() const;
            void setType(const PortType t)
            {
                type = t;
            };
            void setType(const CString &t);

            // Rate
            const map<Scenario *, Rate> &getRate() const
            {
                return rate;
            };
            void setRate(const map<Scenario *, Rate> &r);
            Rate getRateOfScenario(Scenario *s) const;
            void setRateOfScenario(Scenario *s, const Rate r);

            // Channel
            Channel *getChannel() const
            {
                return channel;
            };
            void connectToChannel(Channel *c);

            // Actor
            Actor *getActor() const
            {
                return (Actor *)getParent();
            };

            // Properties
            bool isConnected() const;

            // Print
            ostream &print(ostream &out);
            friend ostream &operator<<(ostream &out, Port &p)
            {
                return p.print(out);
            };
        private:
            // Default scenario
            Scenario *getDefaultScenario() const;

        private:
            // Port type
            PortType type;

            // Channel
            Channel *channel;

            // Rate
            map<Scenario *, Rate> rate;
    };

} // End namespace FSMSADF

#endif
