/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   scenario.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 22, 1009
 *
 *  Function        :   FSM-based SADF scenario
 *
 *  History         :
 *      22-04-09    :   Initial version.
 *
 * $Id: scenario.cc,v 1.1.2.2 2010-04-22 07:07:10 mgeilen Exp $
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

#include "scenario.h"
#include "graph.h"

namespace FSMSADF
{

    /**
     * Scenario ()
     * Constructor.
     */
    Scenario::Scenario(GraphComponent c)
        :
        GraphComponent(c),
        scenarioGraph(NULL)
    {
    }

    /**
     * ~Scenario ()
     * Destructor.
     */
    Scenario::~Scenario()
    {
    }

    /**
     * create ()
     * The function returns a pointer to a newly allocated scenario object.
     */
    Scenario *Scenario::create(GraphComponent c) const
    {
        return new Scenario(c);
    }

    /**
     * createCopy ()
     * The function returns a pointer to a newly allocated scenario object.
     * The name of the new scenario is equal to the this scenario. However, the
     * scenario graph is not copied. To also initialize the scenario graph, you
     * should use the clone function.
     */
    Scenario *Scenario::createCopy(GraphComponent c) const
    {
        Scenario *s = new Scenario(c);

        // Properties
        s->setName(this->getName());
        s->setReward(this->getReward());

        return s;
    }

    /**
     * clone ()
     * The function returns a pointer to a newly allocated scenario object.
     * All properties of the scenario are cloned.
     */
    Scenario *Scenario::clone(GraphComponent c) const
    {
        Scenario *s = createCopy(c);

        // Scenario graph
        s->setScenarioGraph(s->getGraph()->getScenarioGraph(
                                getScenarioGraph()->getName()));
        s->setReward(this->getReward());

        return s;
    }

    /**
     * constructFromXML ()
     * The function initializes all actor properties based on the XML data.
     */
    void Scenario::constructFromXML(const CNodePtr scenarioNode)
    {
        Scenario *s = this;
        ScenarioGraph *sg;

        // Name
        if (!CHasAttribute(scenarioNode, "name"))
            throw CException("Invalid scenario, missing actor name.");
        s->setName(CGetAttribute(scenarioNode, "name"));

        // Graph
        if (!CHasAttribute(scenarioNode, "graph"))
            throw CException("Invalid scenario, missing graph name.");
        sg = getGraph()->getScenarioGraph(CGetAttribute(scenarioNode, "graph"));
        s->setScenarioGraph(sg);
        sg->constructPropertiesFromXML(s, scenarioNode);
    }

    /**
     * convertToXML ()
     * The function converts all port properties to XML data.
     */
    void Scenario::convertToXML(const CNodePtr scenarioNode)
    {
        ScenarioGraph *sg;

        // Name
        CAddAttribute(scenarioNode, "name", getName());

        // Graph
        sg = getScenarioGraph();
        CAddAttribute(scenarioNode, "graph", sg->getName());
        sg->convertPropertiesToXML(this, scenarioNode);
    }

    /**
     * print ()
     * Print the scenario to the supplied output stream.
     */
    ostream &Scenario::print(ostream &out)
    {
        out << "Scenario (" << getName() << ")" << endl;
        out << "id:        " << getId() << endl;
        out << "graph:     " << getScenarioGraph()->getName() << endl;
        out << endl;

        return out;
    }

} // End namespace FSMSADF

