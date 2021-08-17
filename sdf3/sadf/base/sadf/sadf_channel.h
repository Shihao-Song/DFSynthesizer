/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_channel.h
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   29 August 2006
 *
 *  Function        :   SADF Channel
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

#ifndef SADF_CHANNEL_H_INCLUDED
#define SADF_CHANNEL_H_INCLUDED

// Include type definitions

#include "sadf_component.h"

// Forward declarations

class SADF_Process;

// SADF_Channel Definition

class SADF_Channel : public SADF_Component
{

    public:
        // Constructor

        SADF_Channel(const CString &N, const CId ID, const CId T);

        // Destructor

        ~SADF_Channel() { };

        // Access to Processes

        void setSource(SADF_Process *S)
        {
            Source = S;
        };
        void setDestination(SADF_Process *D)
        {
            Destination = D;
        };

        SADF_Process *getSource() const
        {
            return Source;
        };
        SADF_Process *getDestination() const
        {
            return Destination;
        };

        // Access to Productions and Consumptions

        void addConsumption();
        void addProduction();

        void setConsumptionRate(const CId ScenarioID, const CId Rate)
        {
            ConsumptionRates[ScenarioID] = Rate;
        };
        void setProductionRate(const CId ScenarioID, const CId Rate)
        {
            ProductionRates[ScenarioID] = Rate;
        };

        void setProductionScenarioID(const CId SubScenarioID, const CId ScenarioID)
        {
            ProductionScenarioIDs[SubScenarioID] = ScenarioID;
        };
        void setProductionScenarioName(const CId SubScenarioID, const CString &ScenarioName)
        {
            ProductionScenarioNames[SubScenarioID] = ScenarioName;
        };

        void setConsumptionScenarioID(const CId ScenarioID, const CId ControlID)
        {
            ConsumptionScenarioIDs[ScenarioID] = ControlID;
        };
        void setConsumptionScenarioName(const CId ScenarioID, const CString &ControlValue)
        {
            ConsumptionScenarioNames[ScenarioID] = ControlValue;
        };

        CId getConsumptionRate(const CId ScenarioID) const
        {
            return ConsumptionRates[ScenarioID];
        };
        CId getProductionRate(const CId ScenarioID) const
        {
            return ProductionRates[ScenarioID];
        };

        CId getProductionScenarioID(const CId SubScenarioID) const
        {
            return ProductionScenarioIDs[SubScenarioID];
        };
        CString &getProductionScenarioName(const CId SubScenarioID)
        {
            return ProductionScenarioNames[SubScenarioID];
        };

        CId getConsumptionScenarioID(const CId ScenarioID) const
        {
            return ConsumptionScenarioIDs[ScenarioID];
        };
        CString &getConsumptionScenarioName(const CId ScenarioID)
        {
            return ConsumptionScenarioNames[ScenarioID];
        };

        // Access to Properties

        void setBufferSize(const CId S)
        {
            BufferSize = S;
        };
        void setNumberOfInitialTokens(const CId N)
        {
            NumberOfInitialTokens = N;
        };
        void setTokenSize(const CId S)
        {
            TokenSize = S;
        };

        void addInitialTokens(const CId Number, const CId ScenarioID);

        CSize getBufferSize() const
        {
            return BufferSize;
        };
        CId getNumberOfInitialTokens() const
        {
            return NumberOfInitialTokens;
        };
        CId getTokenSize() const
        {
            return TokenSize;
        };

        CQueue &getContentQueue()
        {
            return ContentQueue;
        };
        CQueue &getNumbersQueue()
        {
            return NumbersQueue;
        };

    private:
        // Instance variables

        SADF_Process *Source;
        SADF_Process *Destination;

        CSize BufferSize;
        CId NumberOfInitialTokens;
        CId TokenSize;

        vector<CId> ConsumptionRates;               // Index is (Sub)ScenarioID of Destination
        vector<CId> ProductionRates;                // Index is (Sub)ScenarioID of Source

        vector<CId> ProductionScenarioIDs;          // Index is SubScenarioID of Source
        vector<CString> ProductionScenarioNames;    // Index is SubScenarioID of Source

        vector<CId> ConsumptionScenarioIDs;         // Index is ScenarioID of Destination
        vector<CString> ConsumptionScenarioNames;   // Index is ScenarioID of Destination

        CQueue ContentQueue;
        CQueue NumbersQueue;
};

#endif
