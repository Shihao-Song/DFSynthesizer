/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_process.h
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   29 August 2006
 *
 *  Function        :   SADF Process
 *
 *  History         :
 *      29-08-06    :   Initial version.
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

#ifndef SADF_PROCESS_H_INCLUDED
#define SADF_PROCESS_H_INCLUDED

// Include type definitions

#include "sadf_channel.h"
#include "sadf_profile.h"
#include "sadf_markovchain.h"

// Forward declarations

class SADF_Graph;

// SADF_Process Definition

class SADF_Process : public SADF_Component
{

    public:
        // Constructor
        SADF_Process(const CString &N, const CId ID, const CId T);

        // Destructor
        virtual ~SADF_Process();

        // Access to Channels

        void addDataInputChannel(SADF_Channel *Channel)
        {
            DataInputChannels.push_back(Channel);
        };
        void addDataOutputChannel(SADF_Channel *Channel)
        {
            DataOutputChannels.push_back(Channel);
        };
        void addControlInputChannel(SADF_Channel *Channel)
        {
            ControlInputChannels.push_back(Channel);
        };
        void addControlOutputChannel(SADF_Channel *Channel)
        {
            ControlOutputChannels.push_back(Channel);
        };

        CId getDataInputChannelID(const CString &ChannelName);
        CId getDataOutputChannelID(const CString &ChannelName);
        CId getControlInputChannelID(const CString &ChannelName);
        CId getControlOutputChannelID(const CString &ChannelName);

        vector<SADF_Channel *>& getDataInputChannels()
        {
            return DataInputChannels;
        };
        vector<SADF_Channel *>& getDataOutputChannels()
        {
            return DataOutputChannels;
        };
        vector<SADF_Channel *>& getControlInputChannels()
        {
            return ControlInputChannels;
        };
        vector<SADF_Channel *>& getControlOutputChannels()
        {
            return ControlOutputChannels;
        };

        bool hasControlInputChannels() const
        {
            return !ControlInputChannels.empty();
        };
        bool hasMultipleControlInputChannels() const
        {
            return ControlInputChannels.size() > 1;
        };

        // Access to Profiles

        void deleteProfiles(const CId ScenarioID);
        void addProfile(const CId ScenarioID, SADF_Profile *P)
        {
            Profiles[ScenarioID].push_back(P);
        };
        SADF_Profile *getProfile(const CId ScenarioID, const CId ProfileNumber) const
        {
            return Profiles[ScenarioID][ProfileNumber];
        };
        CId getNumberOfProfiles(const CId ScenarioID) const
        {
            return (CId) Profiles[ScenarioID].size();
        };

        // Access to Scenarios and SubScenarios

        CId addScenario(const CString &ScenarioName);       // Returns ScenarioID
        CId addSubScenario(const CString &SubScenarioName); // Returns SubScenarioID

        CId getNumberOfScenarios() const
        {
            return (CId) Scenarios.size();
        };
        CId getNumberOfSubScenarios() const
        {
            return (CId) SubScenarios.size();
        };

        CId getScenarioID(const CString &ScenarioName);
        CId getSubScenarioID(const CString &SubScenarioName);

        CString &getScenarioName(const CId ScenarioID)
        {
            return Scenarios[ScenarioID];
        };
        CString &getSubScenarioName(const CId SubScenarioID)
        {
            return SubScenarios[SubScenarioID];
        };

        // Access to Controls

        void setControlValue(const CId ScenarioID, const CId ControlChannelNumber, CString &Value)
        {
            ControlInputChannels[ControlChannelNumber]->setConsumptionScenarioName(ScenarioID, Value);
        };
        void setControlID(const CId ScenarioID, const CId ControlChannelNumber, const CId ID)
        {
            ControlInputChannels[ControlChannelNumber]->setConsumptionScenarioID(ScenarioID, ID);
        };

        CString &getControlValue(const CId ScenarioID, const CId ControlChannelNumber)
        {
            return ControlInputChannels[ControlChannelNumber]->getConsumptionScenarioName(ScenarioID);
        };
        CId getControlID(const CId ScenarioID, const CId ControlChannelNumber) const
        {
            return ControlInputChannels[ControlChannelNumber]->getConsumptionScenarioID(ScenarioID);
        };

        // Access to Markov Chains

        void setMarkovChain(const CId ScenarioID, SADF_MarkovChain *M)
        {
            MarkovChains[ScenarioID] = M;
        };
        SADF_MarkovChain *getMarkovChain(CId ScenarioID) const
        {
            return MarkovChains[ScenarioID];
        };

        // Activeness of Scenarios

        void makeInactive(const CId ScenarioID)
        {
            ActiveScenario[ScenarioID] = false;
        };
        bool isActive(const CId ScenarioID) const
        {
            return ActiveScenario[ScenarioID];
        };
        bool checkActivity(const CId ScenarioID);

    private:
        // Instance variables

        vector<SADF_Channel *> DataInputChannels;       // Index is Data Input Sequence Number
        vector<SADF_Channel *> DataOutputChannels;      // Index is Data Output Sequence Number
        vector<SADF_Channel *> ControlInputChannels;    // Index is Control Input Sequence Number
        vector<SADF_Channel *> ControlOutputChannels;   // Index is Control Output Sequence Number

        vector< vector<SADF_Profile *> > Profiles;      // Index1 is (Sub)ScenarioID, Index2 is Profile Sequence Number
        vector<SADF_MarkovChain *> MarkovChains;        // Index is ScenarioID

        vector<CString> Scenarios;                      // Index is ScenarioID
        vector<CString> SubScenarios;                   // Index is SubScenarioID

        vector<bool> ActiveScenario;                    // Index is ScenarioID
};

#endif
