/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   memory.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 14, 2009
 *
 *  Function        :   Memory dimensioning
 *
 *  History         :
 *      29-05-09    :   Initial version.
 *      19-04-11    :   Add accessors (M. Koedam)
 *                      Virtual functions, rename to SDF3FlowBase
 * $Id: memory.h,v 1.1 2009-12-23 13:37:22 sander Exp $
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

#ifndef FSMSADF_RESOURCE_ALLOCATION_FLOW_MEMORY_MEMORY_H_INCLUDED
#define FSMSADF_RESOURCE_ALLOCATION_FLOW_MEMORY_MEMORY_H_INCLUDED

#include "../../../platform_binding/graph.h"

namespace FSMSADF
{

    /**
     * Memory dimensioning
     */
    class MemoryDimAlgoBase
    {
        protected:
            // Constructor
            MemoryDimAlgoBase()
                :
                applicationGraph(NULL)
            {

            };
        public:


            // Destructor
            virtual ~MemoryDimAlgoBase() {};

            // Initialize algorithm
            void init();

            // Compute storage distributions
            virtual bool computeStorageDist();

            // Select storage distribution
            virtual bool selectStorageDist();

            // Estimate storage distibution
            virtual bool estimateStorageDist();

            //////////////
            // Accessor //
            //////////////


            // Application graph
            Graph *getApplicationGraph() const
            {
                return applicationGraph;
            };
            void setApplicationGraph(Graph *g)
            {
                applicationGraph = g;
            };

            // Platform graph
            PlatformGraph *getPlatformGraph() const
            {
                return platformGraph;
            };
            void setPlatformGraph(PlatformGraph *g)
            {
                platformGraph = g;
            };

            // Platform bindings
            PlatformBindings *getPlatformBindings() const
            {
                return platformBindings;
            };
            void setPlatformBindings(PlatformBindings *b)
            {
                platformBindings = b;
            };
            PlatformBinding *getPlatformBinding(const CString &name) const;
            PlatformBinding *getInitialPlatformBinding() const;

        protected:

            // Application graph
            Graph *applicationGraph;

            // Platform graph
            PlatformGraph *platformGraph;

            // Platform bindings
            PlatformBindings *platformBindings;

            // Selected storage distribution for each scenario
            map<Scenario *, set<StorageDistribution>::iterator>
            selectedStorageDistribution;
    };

} // End namespace FSMSADF

#endif
