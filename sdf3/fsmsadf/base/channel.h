/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   channel.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 22, 1009
 *
 *  Function        :   FSM-based SADF channel
 *
 *  History         :
 *      22-04-09    :   Initial version.
 *
 * $Id: channel.h,v 1.1.2.4 2010-04-22 07:07:10 mgeilen Exp $
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

#ifndef FSMSADF_BASE_CHANNEL_H_INCLUDED
#define FSMSADF_BASE_CHANNEL_H_INCLUDED

#include "actor.h"

namespace FSMSADF
{

    // Forward class definition
    class Channel;
    class ScenarioGraph;

    // List of channel pointers
    typedef list<Channel *> Channels;

    /**
     * class Channel
     * Channel in an FSM-based SADF graph.
     */
    class Channel : public GraphComponent
    {
        public:

            // Constructor
            Channel(GraphComponent c);

            // Destructor
            ~Channel();

            // Construct
            Channel *create(GraphComponent c) const;
            Channel *createCopy(GraphComponent c) const;
            Channel *clone(GraphComponent c) const;

            // Construct
            void constructFromXML(const CNodePtr channelNode);
            void constructPropertiesFromXML(Scenario *s,
                                            const CNodePtr channelPropertiesNode);

            // Convert
            void convertToXML(const CNodePtr channelNode);
            void convertPropertiesToXML(Scenario *s,
                                        const CNodePtr channelPropertiesNode);

            // Isolate scenario
            void isolateScenario(Scenario *s);

            // Connections
            Port *getSrcPort() const
            {
                return src;
            };
            Port *getDstPort() const
            {
                return dst;
            };
            Actor *getSrcActor() const
            {
                return (src == NULL ?  NULL : getSrcPort()->getActor());
            };
            Actor *getDstActor() const
            {
                return (dst == NULL ?  NULL : getDstPort()->getActor());
            };
            void connectSrc(Port *p);
            void connectDst(Port *p);
            Port *oppositePort(Port *p) const
            {
                return (src == p ? dst : src);
            };

            // Initial tokens
            uint getInitialTokens() const
            {
                return initialTokens;
            };
            void setInitialTokens(const uint t)
            {
                initialTokens = t;
            };

            const CStrings &getPersistentTokenNames() const
            {
                return scPersistentTokenNames;
            };
            void setPersistentTokenNames(const CStrings &n);
            void setPersistentTokenNames(const CString &n);
            void addPersistentTokenName(const CString &n);

            // final tokens for weakly consistent scenarios
            uint getFinalTokens() const
            {
                return finalTokens;
            };
            void setFinalTokens(const uint t)
            {
                finalTokens = t;
            };
            const CStrings &getPersistentFinalTokenNames() const
            {
                return scPersistentFinalTokenNames;
            };
            void setPersistentFinalTokenNames(const CStrings &n);
            void setPersistentFinalTokenNames(const CString &n);

            // Properties
            bool isConnected() const;

            // Token size
            const map<Scenario *, Size> &getTokenSize() const
            {
                return tokenSize;
            };
            void setTokenSize(const map<Scenario *, Size> &s);
            Size getTokenSizeOfScenario(Scenario *s) const;
            void setTokenSizeOfScenario(Scenario *s, const Size sz);
            bool hasTokenSizeInScenario(Scenario *s) const;

            // Graph
            ScenarioGraph *getScenarioGraph() const
            {
                return (ScenarioGraph *)getParent();
            };

            // Print
            ostream &print(ostream &out);
            friend ostream &operator<<(ostream &out, Channel &c)
            {
                return c.print(out);
            };

        private:
            // Default scenario
            Scenario *getDefaultScenario() const;

        private:
            // Ports
            Port *src;
            Port *dst;

            // Initial tokens
            uint initialTokens;

            // Final tokens
            uint finalTokens;

            // names of the persistent tokens
            CStrings scPersistentTokenNames;
            CStrings scPersistentFinalTokenNames;

            // Token size
            map<Scenario *, Size> tokenSize;
    };

} // End namespace FSMSADF

#endif
