/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   repetition_vector.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 11, 2009
 *
 *  Function        :   Repetition vector
 *
 *  History         :
 *      11-05-09    :   Initial version.
 *
 * $Id: repetition_vector.cc,v 1.1.2.3 2010-04-25 01:21:16 mgeilen Exp $
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

#include "repetition_vector.h"

namespace FSMSADF
{

    /**
     * calcFractionsConnectedActors ()
     * The function calculates and firing ration (as fractions) of
     * all actors connected to actor 'a' based on its firing rate. In case
     * of an inconsistent graph, all fractions are set to 0.
     */
    static
    void calcFractionsConnectedActors(CFractions &fractions, Actor *a, Scenario *s)
    {
        CFraction fractionA = fractions[a->getId()];

        // Inconsistent graph?
        if (fractionA == CFraction(0, 0))
            return;

        // Calculate the rate for each actor 'b' connected to actor 'a'
        for (Ports::iterator i = a->getPorts().begin();
             i != a->getPorts().end(); i++)
        {
            Port *pA = *i;

            // Get actor 'b' on other side of channel and its port 'pB'
            Channel *c = pA->getChannel();
            ASSERT(c != NULL, "Port '" + pA->getName() + "' on actor: '" + a->getName() +
                   "' in scenario: '" + s->getName() + "' has no channel.");
            Port *pB = c->getSrcPort();
            Actor *b = pB->getActor();
            if (a->getId() == b->getId())
            {
                pB = c->getDstPort();
                b = pB->getActor();
            }

            // Calculate firing rate 'b'
            CFraction ratioAB = CFraction(pA->getRateOfScenario(s),
                                          pB->getRateOfScenario(s));
            CFraction fractionB = fractionA * ratioAB;

            // Known firing rate for 'b'
            CFraction knownFractionB = fractions[b->getId()];

            // Compare known and calculated firing rate of 'b'
            if (knownFractionB != CFraction(0, 1)
                && fractionB != knownFractionB)
            {
                // Inconsistent graph, set all fractions to 0
                for (uint i = 0; i < fractions.size(); i++)
                    fractions[i] = CFraction(0, 0);

                return;
            }
            else if (knownFractionB == CFraction(0, 1))
            {
                // Set the firing rate of actor 'b'
                fractions[b->getId()] = fractionB;

                // Calculate firing rate for all actors connnected to 'b'
                calcFractionsConnectedActors(fractions, b, s);

                // Is graph inconsistent?
                if (fractions[b->getId()] == CFraction(0, 0))
                    return;
            }
        }
    }

    /**
     * calcRepetitionVector ()
     * Convert the fractions to the smallest integers.
     */
    static
    RepetitionVector calcRepetitionVector(CFractions &fractions)
    {
        RepetitionVector repetitionVector(fractions.size(), 0);
        long long int l = 1;

        // Find lowest common multiple (lcm) of all denominators
        for (CFractionsIter iter = fractions.begin();
             iter != fractions.end(); iter++)
        {
            CFraction &f = *iter;

            l = lcm(l, f.denominator());
        }

        // Zero vector?
        if (l == 0)
            return repetitionVector;

        // Calculate non-zero repetition vector
        for (uint i = 0; i < fractions.size(); i++)
        {
            repetitionVector[i] =
                (int)((fractions[i].numerator() * l) / fractions[i].denominator());
        }

        // Find greatest common divisor (gcd)
        int g = repetitionVector[0];
        for (uint i = 1; i < repetitionVector.size(); i++)
        {
            g = gcd(g, repetitionVector[i]);
        }

        // Minimize the repetition vector using the gcd
        for (uint i = 0; i < repetitionVector.size(); i++)
        {
            repetitionVector[i] = repetitionVector[i] / g;
        }

        return repetitionVector;
    }

    /**
     * computeRepetitionVector ()
     * The function computes the repetition vector of a scenario graph for a given
     * scenario.
     */
    RepetitionVector computeRepetitionVector(ScenarioGraph *g, Scenario *s)
    {
        CFractions fractions(g->getActors().size(), CFraction(0, 1));

        // Calculate firing ratio (as fraction) for each actor
        for (Actors::iterator i = g->getActors().begin();
             i != g->getActors().end(); i++)
        {
            Actor *a = *i;
            CFraction &f = fractions[a->getId()];

            if (f == CFraction(0, 1))
            {
                f = CFraction(1, 1);
                calcFractionsConnectedActors(fractions, a, s);
            }
        }

        // Calculate repetition vector based on firing ratios
        return calcRepetitionVector(fractions);
    }

    /**
     * isScenarioGraphConsistent ()
     * The function checks the consistency of the scenario graph and returns true
     * when the graph is consistent, else it returns false.
     */
    bool isScenarioGraphConsistent(ScenarioGraph *g, Scenario *s)
    {
        RepetitionVector repVec = computeRepetitionVector(g, s);

        if (repVec[0] == 0)
            return false;

        return true;
    }

} // End of namespace
