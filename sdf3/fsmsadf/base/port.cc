/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   port.cc
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
 * $Id: port.cc,v 1.1.2.5 2010-04-22 07:07:10 mgeilen Exp $
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

#include "port.h"
#include "graph.h"

namespace FSMSADF
{

    /**
     * Port ()
     * Constructor.
     */
    Port::Port(GraphComponent c)
        :
        GraphComponent(c),
        type(Undef),
        channel(NULL)
    {
        // Set the default rate
        setRateOfScenario(getDefaultScenario(), 1);
    }

    /**
     * ~Port ()
     * Destructor.
     */
    Port::~Port()
    {
    }

    /**
     * create ()
     * The function returns a pointer to a newly allocated port object.
     */
    Port *Port::create(GraphComponent c) const
    {
        return new Port(c);
    }

    /**
     * createCopy ()
     * The function returns a pointer to a newly allocated SDF port object.
     * The properties of the port are also copied.
     */
    Port *Port::createCopy(GraphComponent c) const
    {
        Port *p = create(c);

        p->setName(getName());
        p->setType(getType());
        p->setRate(getRate());

        return p;
    }

    /**
     * clone ()
     * The function returns a pointer to a newly allocated port object.
     * The properties of the port are also cloned. Note: the connection to the
     * channel is lost. Cloning the channel restores this connection.
     */
    Port *Port::clone(GraphComponent c) const
    {
        Port *p = createCopy(c);

        return p;
    }

    /**
     * constructFromXML ()
     * The function initializes all port properties based on the XML data.
     */
    void Port::constructFromXML(const CNodePtr portNode)
    {
        // Name
        if (!CHasAttribute(portNode, "name"))
            throw CException("Invalid graph, missing port name.");
        setName(CGetAttribute(portNode, "name"));

        // Type
        if (!CHasAttribute(portNode, "type"))
            throw CException("Invalid graph, missing port type.");
        setType(CGetAttribute(portNode, "type"));

        // Rate
        if (!CHasAttribute(portNode, "rate"))
            throw CException("Invalid graph, missing port rate.");
        setRateOfScenario(getDefaultScenario(), CGetAttribute(portNode, "rate"));
    }

    /**
     * convertToXML ()
     * The function converts all port properties to XML data.
     */
    void Port::convertToXML(const CNodePtr portNode)
    {
        // Name
        CAddAttribute(portNode, "name", getName());

        // Type
        CAddAttribute(portNode, "type", getTypeAsString());

        // Rate
        CAddAttribute(portNode, "rate", getRateOfScenario(getDefaultScenario()));
    }

    /**
     * isolateScenario()
     * The function removes all scenario except scenario s from the actor. The rate
     * of this scenario is set as the rate of the default scenario on this graph.
     */
    void Port::isolateScenario(Scenario *s)
    {
        // Rate
        Rate r = getRateOfScenario(s);
        rate.clear();
        setRateOfScenario(getDefaultScenario(), r);
    }

    /**
     * getTypeAsString()
     * The function returns a string which specifies the port type.
     */
    CString Port::getTypeAsString() const
    {
        if (type == In)
            return "in";
        else if (type == Out)
            return "out";
        else
            return "undef";
    }

    /**
     * setType()
     * The function sets the port type based on the string value.
     */
    void Port::setType(const CString &t)
    {
        if (t == CString("in"))
            type = In;
        else if (t == CString("out"))
            type = Out;
        else
            type = Undef;
    }

    /**
     * setRate ()
     * The function sets the port rates of all scenarios.
     */
    void Port::setRate(const map<Scenario *, Rate> &r)
    {
        // Clear existing map
        rate.clear();

        // Iterate over the new map
        for (map<Scenario *, Rate>::const_iterator i = r.begin(); i != r.end(); i++)
        {
            // Find corresponding scenario in this graph
            Scenario *s = getActor()->getScenarioGraph()->getGraph()->getScenario(
                              i->first->getName());
            setRateOfScenario(s, i->second);
        }
    }

    /**
     * getRateOfScenario()
     * The function returns the rate of the port in the given scenario. When no
     * specific rate is specified for the scenario, the default scenario is used.
     */
    Rate Port::getRateOfScenario(Scenario *s) const
    {
        if (rate.find(s) != rate.end())
            return rate.find(s)->second;

        if (rate.find(getDefaultScenario()) == rate.end())
            throw CException("Port '" + getName() + "' has no rate.");

        return rate.find(getDefaultScenario())->second;
    }

    /**
     * setRateOfScenario()
     * The function sets the rate of the port in the given scenario.
     */
    void Port::setRateOfScenario(Scenario *s, const Rate r)
    {
        rate[s] = r;
    }

    /**
     * connectToChannel ()
     * The function connects the port to a channel. A connection can only be made
     * if the port is not yet connected.
     */
    void Port::connectToChannel(Channel *c)
    {
        if (isConnected() && c != NULL)
            throw CException("Port '" + getParent()->getName()
                             + "." + getName() + "' already connected.");

        channel = c;
    }

    /**
     * isConnected ()
     * The function return true if the port is connected to a channel, else it
     * returns false.
     */
    bool Port::isConnected() const
    {
        if (getChannel() == NULL)
            return false;

        return true;
    }

    /**
     * getDefaultScenario ()
     * The function returns a pointer to the default scenario.
     */
    Scenario *Port::getDefaultScenario() const
    {
        return getActor()->getScenarioGraph()->getGraph()->getDefaultScenario();
    }

    /**
     * print ()
     * Print the port to the supplied output stream.
     */
    ostream &Port::print(ostream &out)
    {
        out << "Port (" << getName() << ")" << endl;
        out << "id:        " << getId() << endl;
        out << "type:      " << getTypeAsString() << endl;
        out << "connected: " << (isConnected() ? "true" : "false") << endl;

        for (map<Scenario *, Rate>::iterator i = rate.begin(); i != rate.end(); i++)
        {
            out << "rate: " << i->second << " (scenario: " << i->first->getName();
            out << ")" << endl;
        }

        out << endl;

        return out;
    }

} // End namespace FSMSADF

