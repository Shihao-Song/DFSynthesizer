/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   scenario.h
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
 * $Id: scenario.h,v 1.1.2.2 2010-04-22 07:07:10 mgeilen Exp $
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

#ifndef FSMSADF_BASE_SCENARIO_H_INCLUDED
#define FSMSADF_BASE_SCENARIO_H_INCLUDED

#include "component.h"

namespace FSMSADF
{

    // Forward class definition
    class Scenario;
    class ScenarioGraph;
    class Graph;

    // Set of scenarios
    typedef set<Scenario *> Scenarios;
    // List of scenarios
    typedef list<Scenario *> ScenarioList;

    /**
     * Scenario
     * Scenario in FSM-based SADF graph
     */
    class Scenario : public GraphComponent
    {
        public:

            // Constructor
            Scenario(GraphComponent c);

            // Desctructor
            ~Scenario();

            // Construct
            Scenario *create(GraphComponent c) const;
            Scenario *createCopy(GraphComponent c) const;
            Scenario *clone(GraphComponent c) const;

            // Construct
            void constructFromXML(const CNodePtr scenarioNode);

            // Convert
            void convertToXML(const CNodePtr scenarioNode);

            // Scenario graph
            ScenarioGraph *getScenarioGraph() const
            {
                return scenarioGraph;
            };
            void setScenarioGraph(ScenarioGraph *g)
            {
                scenarioGraph = g;
            };

            // Graph
            Graph *getGraph() const
            {
                return (Graph *)getParent();
            };

            // Print
            ostream &print(ostream &out);
            friend ostream &operator<<(ostream &out, Scenario &s)
            {
                return s.print(out);
            };

            // reward
            CDouble getReward() const
            {
                return reward;
            };
            void setReward(CDouble r)
            {
                reward = r;
            };

        private:
            // Scenario graph
            ScenarioGraph *scenarioGraph;


            // reward (ammount of progress represented by this scenario
            CDouble reward;
    };

} // End namespace FSMSADF

#endif
