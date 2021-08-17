/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   type.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 22, 1009
 *
 *  Function        :   FSM-based SADF type definitions
 *
 *  History         :
 *      22-04-09    :   Initial version.
 *
 * $Id: type.cc,v 1.1.2.3 2010-04-25 01:21:17 mgeilen Exp $
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

#include "type.h"

namespace FSMSADF
{

    /**
     * constructFromXML()
     * Create a storage distribution from the XML data.
     */
    void StorageDistribution::constructFromXML(
        const CNodePtr storageDistributionNode)
    {
        // Throughput
        if (!CHasAttribute(storageDistributionNode, "thr"))
            throw CException("Storage distribution has no thr");
        setThroughput((double)(CGetAttribute(storageDistributionNode, "thr")));

        // Channels
        for (CNode *n = CGetChildNode(storageDistributionNode, "channel");
             n != NULL; n = CNextNode(n, "channel"))
        {
            CString name;
            CSize sz;

            // Name
            if (!CHasAttribute(n, "name"))
                throw CException("Channel in storage distribution has no name.");
            name = CGetAttribute(n, "name");

            // Size
            if (!CHasAttribute(n, "sz"))
                throw CException("Channel in storage distribution has no sz.");
            sz = CGetAttribute(n, "sz");

            // Insert storage space of channel to map
            insert(std::pair<CString, Size>(name, (Size) sz));
        }
    }

    /**
     * convertToXML()
     * Convert the storage distribution to XML data.
     */
    void StorageDistribution::convertToXML(const CNodePtr storageDistributionNode)
    {
        // Size
        CAddAttribute(storageDistributionNode, "sz", getSize());

        // Throughput
        CAddAttribute(storageDistributionNode, "thr", throughput);

        // Channels
        for (StorageDistribution::iterator i = begin(); i != end(); i++)
        {
            // Channel
            CNode *channelNode = CAddNode(storageDistributionNode, "channel");

            // Name
            CAddAttribute(channelNode, "name", i->first);

            // Size
            CAddAttribute(channelNode, "sz", i->second);
        }
    }

    /**
     * constructFromXML()
     * Create a storage distribution set from the XML data.
     */
    void StorageDistributions::constructFromXML(
        const CNodePtr storageDistributionsNode)
    {
        for (CNode *n = CGetChildNode(storageDistributionsNode,
                                      "storageDistribution");
             n != NULL; n = CNextNode(n, "storageDistribution"))
        {
            StorageDistribution s;

            // Create a storage distribution
            s.constructFromXML(n);

            // Insert storage distribution in the set
            insert(s);
        }
    }

    /**
     * convertToXML()
     * Convert the storage distribution set to XML data.
     */
    void StorageDistributions::convertToXML(
        const CNodePtr storageDistributionsNode)
    {
        // Storage distributions
        for (set<StorageDistribution>::iterator i = begin(); i != end(); i++)
        {
            StorageDistribution s = *i;
            s.convertToXML(CAddNode(storageDistributionsNode,
                                    "storageDistribution"));
        }
    }

} // End namespace FSMSADF

