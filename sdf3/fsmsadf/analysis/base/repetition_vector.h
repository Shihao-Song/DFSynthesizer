/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   repetition_vector.h
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
 * $Id: repetition_vector.h,v 1.1.2.2 2010-04-22 07:07:08 mgeilen Exp $
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

#ifndef FSMSADF_ANALYSIS_BASE_REPETITION_VECTOR_H_INCLUDED
#define FSMSADF_ANALYSIS_BASE_REPETITION_VECTOR_H_INCLUDED

#include "../../../base/base.h"
#include "../../base/graph.h"

namespace FSMSADF
{

    /**
     * Repetition vector
     */
    typedef std::vector<int> RepetitionVector;

    /**
     * computeRepetitionVector ()
     * The function computes the repetition vector of a scenario graph for a given
     * scenario.
     */
    RepetitionVector computeRepetitionVector(ScenarioGraph *g, Scenario *s);

    /**
     * isScenarioGraphConsistent ()
     * The function checks the consistency of the scenario graph and returns true
     * when the graph is consistent, else it returns false.
     */
    bool isScenarioGraphConsistent(ScenarioGraph *g, Scenario *s);

} // End namespace FSMSADF

#endif
