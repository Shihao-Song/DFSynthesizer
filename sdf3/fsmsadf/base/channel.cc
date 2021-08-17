/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   channel.cc
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
 * $Id: channel.cc,v 1.1.2.2 2010-04-22 07:07:10 mgeilen Exp $
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

#include "channel.h"
#include "graph.h"

namespace FSMSADF
{

    /**
     * Channel ()
     * Constructor.
     */
    Channel::Channel(GraphComponent c)
        :
        GraphComponent(c),
        src(NULL),
        dst(NULL),
        initialTokens(0)
    {
    }

    /**
     * ~Channel ()
     * Destructor.
     */
    Channel::~Channel()
    {
    }

    /**
     * create ()
     * The function returns a pointer to a newly allocated channel object.
     */
    Channel *Channel::create(GraphComponent c) const
    {
        return new Channel(c);
    }

    /**
     * createCopy ()
     * The function returns a pointer to a newly allocated channel object.
     * All properties of the channel are copied.
     */
    Channel *Channel::createCopy(GraphComponent c) const
    {
        Channel *ch = create(c);

        // Properties
        ch->setName(getName());
        ch->setTokenSize(getTokenSize());

        // Initial tokens
        ch->setInitialTokens(getInitialTokens());

        // Final tokens
        ch->setFinalTokens(getFinalTokens());

        // Token names
        ch->scPersistentTokenNames = this->scPersistentTokenNames;
        ch->scPersistentFinalTokenNames = this->scPersistentFinalTokenNames;

        return ch;
    }

    /**
     * clone ()
     * The function returns a pointer to a newly allocated channel object.
     * All properties and the connection of the channel are cloned.
     */
    Channel *Channel::clone(GraphComponent c) const
    {
        Channel *ch = createCopy(c);

        // Connection
        if (getSrcActor() != NULL)
        {
            Port *p = ch->getScenarioGraph()->getActor(getSrcActor()->getName())
                      ->getPort(getSrcPort()->getName());
            ch->connectSrc(p);
        }
        if (getDstActor() != NULL)
        {
            Port *p = ch->getScenarioGraph()->getActor(getDstActor()->getName())
                      ->getPort(getDstPort()->getName());
            ch->connectDst(p);
        }

        return ch;
    }

    /**
     * constructFromXML ()
     * The function initializes all basic channel properties based on the XML data.
     */
    void Channel::constructFromXML(const CNodePtr channelNode)
    {
        CString strSrcActor, strSrcPort, strDstActor, strDstPort, strPersist;
        Port *srcPort, *dstPort;
        ScenarioGraph *g;
        Channel *c = this;

        // Graph
        g = c->getScenarioGraph();

        // Name
        if (!CHasAttribute(channelNode, "name"))
            throw CException("Invalid graph, missing channel name.");
        c->setName(CGetAttribute(channelNode, "name"));

        // Initial tokens
        if (CHasAttribute(channelNode, "initialTokens"))
            c->setInitialTokens(CGetAttribute(channelNode, "initialTokens"));

        // Final tokens
        if (CHasAttribute(channelNode, "finalTokens"))
            c->setFinalTokens(CGetAttribute(channelNode, "finalTokens"));
        else
            c->setFinalTokens(c->getInitialTokens());

        // Source and destination actor ports
        if (!CHasAttribute(channelNode, "srcActor"))
            throw CException("Invalid graph, missing channel srcActor.");
        strSrcActor = CGetAttribute(channelNode, "srcActor");

        if (!CHasAttribute(channelNode, "srcPort"))
            throw CException("Invalid graph, missing channel srcPort.");
        strSrcPort = CGetAttribute(channelNode, "srcPort");

        if (!CHasAttribute(channelNode, "dstActor"))
            throw CException("Invalid graph, missing channel dstActor.");
        strDstActor = CGetAttribute(channelNode, "dstActor");

        if (!CHasAttribute(channelNode, "dstPort"))
            throw CException("Invalid graph, missing channel dstPort.");
        strDstPort = CGetAttribute(channelNode, "dstPort");

        // Find actors and port
        srcPort = g->getActor(strSrcActor)->getPort(strSrcPort);
        dstPort = g->getActor(strDstActor)->getPort(strDstPort);

        // Check scenario persistent names
        if (CHasAttribute(channelNode, "persistentTokenNames"))
        {
            strPersist = CGetAttribute(channelNode, "persistentTokenNames");
            setPersistentTokenNames(strPersist);
        }

        // Check scenario persistent final token names
        if (CHasAttribute(channelNode, "persistentFinalTokenNames"))
        {
            strPersist = CGetAttribute(channelNode, "persistentFinalTokenNames");
            setPersistentFinalTokenNames(strPersist);
        }
        else
        {
            if (this->finalTokens > 0)
                this->setPersistentFinalTokenNames(this->getPersistentTokenNames());
        }

        // Connect channel to source and destination ports
        c->connectSrc(srcPort);
        c->connectDst(dstPort);
    }

    /**
     * constructPropertiesFromXML ()
     * The function initializes all channel properties based on the XML data.
     */
    void Channel::constructPropertiesFromXML(Scenario *s,
            const CNodePtr channelPropertiesNode)
    {
        // Token size
        if (CHasChildNode(channelPropertiesNode, "tokenSize"))
        {
            CNode *tokenSizeNode = CGetChildNode(channelPropertiesNode, "tokenSize");

            if (CHasAttribute(tokenSizeNode, "sz"))
            {
                setTokenSizeOfScenario(s, CGetAttribute(tokenSizeNode, "sz"));
            }
        }
    }

    /**
     * convertToXML ()
     * The function converts all basic channel properties to XML data.
     */
    void Channel::convertToXML(const CNodePtr channelNode)
    {
        // Name
        CAddAttribute(channelNode, "name", getName());

        // Source and destination actor ports
        CAddAttribute(channelNode, "srcActor", getSrcActor()->getName());
        CAddAttribute(channelNode, "srcPort", getSrcPort()->getName());
        CAddAttribute(channelNode, "dstActor", getDstActor()->getName());
        CAddAttribute(channelNode, "dstPort", getDstPort()->getName());

        // Initial tokens
        CAddAttribute(channelNode, "initialTokens", getInitialTokens());

        // Final tokens
        CAddAttribute(channelNode, "finalTokens", getFinalTokens());

        if (this->scPersistentTokenNames.size() > 0)
            CAddAttribute(channelNode, "persistentTokenNames", CString::join(this->scPersistentTokenNames, ','));

        if (this->scPersistentFinalTokenNames.size() > 0)
            CAddAttribute(channelNode, "persistentFinalTokenNames", CString::join(this->scPersistentFinalTokenNames, ','));

    }

    /**
     * convertPropertiesToXML ()
     * The function converts all channel properties to XML data.
     */
    void Channel::convertPropertiesToXML(Scenario *s,
                                         const CNodePtr channelPropertiesNode)
    {
        // Name
        CAddAttribute(channelPropertiesNode, "channel", getName());

        // Token size
        if (tokenSize.find(s) != tokenSize.end())
        {
            CNode *tokenSizeNode = CAddNode(channelPropertiesNode, "tokenSize");
            CAddAttribute(tokenSizeNode, "sz", getTokenSizeOfScenario(s));
        }
    }

    /**
     * isolateScenario()
     * The function removes all scenario except scenario s from the channel.
     */
    void Channel::isolateScenario(Scenario *s)
    {
        // Token size
        if (hasTokenSizeInScenario(s))
        {
            Size sz = getTokenSizeOfScenario(s);
            tokenSize.clear();
            setTokenSizeOfScenario(s, sz);
        }
    }

    /**
     * connectSrc ()
     * The function connects the channel to a source port and port to channel.
     */
    void Channel::connectSrc(Port *p)
    {
        if (p == NULL)
        {
            src->connectToChannel(NULL);
            src = NULL;
            return;
        }

        if (getSrcPort() != NULL)
            throw CException("Channel '" + getName() + "' already connected.");

        if (p->getType() != Port::Out)
        {
            cerr << "type: " << p->getTypeAsString() << endl;
            throw CException("Cannot connect port '" + p->getName() + "' to "
                             "channel '" + getName() + "'; conflicting port type.");
        }

        src = p;
        src->connectToChannel(this);
    }

    /**
     * connectDst ()
     * The function connects the channel to a destination port and port to channel.
     */
    void Channel::connectDst(Port *p)
    {
        if (p == NULL)
        {
            dst->connectToChannel(NULL);
            dst = NULL;
            return;
        }

        if (getDstPort() != NULL)
            throw CException("Channel '" + getName() + "' already connected.");

        if (p->getType() != Port::In)
        {
            cerr << "type: " << p->getTypeAsString() << endl;
            throw CException("Cannot connect port '" + p->getName() + "' to "
                             "channel '" + getName() + "'; conflicting port type.");
        }

        dst = p;
        dst->connectToChannel(this);
    }

    /**
     * Set the names of all persistent tokens stored on this channel.
     * @param list of persistent token names
     */
    void Channel::setPersistentTokenNames(const CStrings &n)
    {
        this->scPersistentTokenNames = n;
    }

    /**
     * Add a name to the persistent tokens stored on this channel.
     * @param persistent token names
     */
    void Channel::addPersistentTokenName(const CString &n)
    {
        this->scPersistentTokenNames.push_back(n);
    }

    /**
     * Set the names of all persistent tokens stored on this channel.
     * @param string with comma seperated list of persistent token names
     */
    void Channel::setPersistentTokenNames(const CString &n)
    {
        CStrings names = n.split(',');

        if (names.size() == 0)
        {
            setPersistentTokenNames(names);
            return;
        }

        if (getInitialTokens() == 0)
            throw CException("Channel without initial tokens cannot have "
                             "persistentTokenNames attribute.");

        if (names.size() > getInitialTokens())
            throw CException("Number of persistent token names should not be larger"
                             " than the number of initial tokens on a channel");

        setPersistentTokenNames(names);
    }

    /**
     * Set the names of all persistent final tokens stored on this channel.
     * @param list of persistent final token names
     */
    void Channel::setPersistentFinalTokenNames(const CStrings &n)
    {
        this->scPersistentFinalTokenNames = n;
    }

    /**
     * Set the names of all persistent final tokens stored on this channel.
     * @param string with comma seperated list of persistent final token names
     */
    void Channel::setPersistentFinalTokenNames(const CString &n)
    {
        CStrings names = n.split(',');

        if (names.size() == 0)
        {
            setPersistentFinalTokenNames(names);
            return;
        }

        if (this->getFinalTokens() == 0)
            throw CException("Channel without final tokens cannot have "
                             "persistentFinalTokenNames attribute.");

        if (names.size() > this->getFinalTokens())
            throw CException("Number of persistent final token names should not be larger"
                             " than the number of final tokens on a channel");

        setPersistentFinalTokenNames(names);
    }


    /**
     * isConnected ()
     * The function returns true if the channel is connected to a source and
     * destination port.
     */
    bool Channel::isConnected() const
    {
        if (getSrcPort() == NULL || getDstPort() == NULL)
            return false;

        return true;
    }

    /**
     * setTokenSize ()
     * The function sets the token size of all scenarios.
     */
    void Channel::setTokenSize(const map<Scenario *, Size> &sz)
    {
        // Clear existing map
        tokenSize.clear();

        // Iterate over the new map
        for (map<Scenario *, Size>::const_iterator i = sz.begin(); i != sz.end(); i++)
        {
            // Find corresponding scenario in this graph
            Scenario *s = getScenarioGraph()->getGraph()->getScenario(
                              i->first->getName());
            setTokenSizeOfScenario(s, i->second);
        }
    }

    /**
     * getTokenSizeOfScenario ()
     * The function returns the token size of a token send on this channel in
     * scenario s. When no token size has been specified for this scenario, the
     * default scenario is checked. When also no default scenario exists, an
     * exception is thrown.
     */
    Size Channel::getTokenSizeOfScenario(Scenario *s) const
    {
        if (tokenSize.find(s) != tokenSize.end())
            return tokenSize.find(s)->second;

        if (tokenSize.find(getDefaultScenario()) == tokenSize.end())
        {
            throw CException("Channel '" + getName() + "' has no token size.");
        }

        return tokenSize.find(getDefaultScenario())->second;
    }

    /**
     * setTokenSizeOfScenario ()
     * Set the token size of scenario s.
     */
    void Channel::setTokenSizeOfScenario(Scenario *s, const Size sz)
    {
        tokenSize[s] = sz;
    }

    /**
     * hasTokenSizeInScenario ()
     * The function returns true when a token size has been specified for scenario s
     * or through a default scenario. Otherwise the function returns false.
     */
    bool Channel::hasTokenSizeInScenario(Scenario *s) const
    {
        if (tokenSize.find(s) != tokenSize.end()
            || tokenSize.find(getDefaultScenario()) != tokenSize.end())
        {
            return true;
        }

        return false;
    }

    /**
     * getDefaultScenario ()
     * The function returns a pointer to the default scenario.
     */
    Scenario *Channel::getDefaultScenario() const
    {
        return getScenarioGraph()->getGraph()->getDefaultScenario();
    }

    /**
     * print ()
     * Print the channel to the supplied output stream.
     */
    ostream &Channel::print(ostream &out)
    {
        out << "Channel (" << getName() << ")" << endl;
        out << "id:             " << getId() << endl;
        out << "initial tokens: " << getInitialTokens() << endl;
        if (!getPersistentTokenNames().empty())
        {
            out << "persistent tokens: ";
            CStrings persTokens = getPersistentTokenNames();
            for (CStrings::iterator i = persTokens.begin();
                 i != persTokens.end(); i++)
            {
                CString &s = *i;
                if (i != persTokens.begin())
                    out << ",";
                out << s;
            }
            out << endl;
        }
        out << "connected:      " << (isConnected() ? "true" : "false") << endl;

        if (isConnected())
        {
            out << "source:         " << getSrcActor()->getName() << ".";
            out << getSrcPort()->getName() << endl;
            out << "destination:    " << getDstActor()->getName() << ".";
            out << getDstPort()->getName() << endl;
        }

        out << endl;

        return out;
    }

} // End namespace FSMSADF

