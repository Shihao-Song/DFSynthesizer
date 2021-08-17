/*
*  TU Eindhoven
*  Eindhoven, The Netherlands
*
*  Name            :   mpexplore.h
*
*  Author          :   Marc Geilen (m.c.w.geilen@tue.nl)
*
*  Date            :   May 13, 2008
*
*  Function        :   Max-Plus based state-space exploration algorithms.
*
*  History         :
*      13-05-08    :   Initial version.
*
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

#ifndef MPEXPLORE_H_INCLUDED
#define MPEXPLORE_H_INCLUDED

#include <vector>
#include <string>
#include <list>

#include "../../base/timed/graph.h"
#include "../../../base/maxplus/maxplus.h"
#include "sdf/base/algo/repetition_vector.h"

using namespace std;
using namespace MaxPlus;

namespace SDF
{

    namespace MaxPlusAnalysis
    {

        // forward declarations
        class State;
        class SymbolicState;

        /*
         * Name space GraphDecoration is used to store information about an SDF graph
         * related to MaxPlus based analysis.
         */
        namespace GraphDecoration
        {

            // Forward class definition
            class Graph;

            /**
             * class Channel
             * Decorates an SDF channel with information required for MaxPLus analysis
             */
            class Channel
            {
                public:
                    unsigned int index;
                    unsigned int nrInitialTokens;
                    unsigned int synchStorage;
                    unsigned int repetitionVectorEntry;

                    // where to start storing tokens slots in token table / state
                    unsigned int stateStartIndex;

                    // where to start storing history variables in token table
                    unsigned int historyStartIndex;

                    // is this a channel that represents a storage constraint
                    bool isStorageChannel;
            };

            /**
             * class ChannelList
             * A list of GraphDecoration::SDFChannels
             */
            class ChannelList : public std::vector<Channel *>
            {
            };

            /**
             * class Port
             * Decorates an SDF port with information required for MaxPLus analysis.
             */
            class Port
            {
                public:
                    Port(unsigned int _rate, Channel *_channel);
                    unsigned int rate;
                    Channel *channel;
            };

            /**
             * class PortList
             * A list of ports
             */
            class PortList : public std::vector<Port *>
            {
            };

            /**
             * class Actor
             * Decorates an SDF actor with information for MaxPlus analysis
             */
            class Actor
            {
                public:
                    unsigned int index;
                    MPDelay executionTime;
                    PortList *inputPorts;
                    PortList *outputPorts;
                    Actor();
                    ~Actor();
            };

            /**
             * class ActorList
             * A list of actors
             */
            class ActorList : public std::vector<Actor *>
            {
            };

            /**
             * class ActorCount
             * Associates with every actor a number, representing a number of firings
             * (e.g., a repetition vector)
             */
            class ActorCount
            {
                public:
                    ActorCount(Graph *G);
                    ~ActorCount();
                    bool isEmpty();
                    unsigned int getValue(Actor *a);
                    void setValue(Actor *a, unsigned int v);
                    void setValue(unsigned int i, unsigned int v);
                    void decreaseValue(Actor *a);
                    ActorCount *copy();
                    unsigned int sum();

                private:
                    ActorCount(size_t sz);
                    size_t size;
                    unsigned int *table;
            };

            /**
             * class Graph
             * Decorates an SDF graph with information for MaxPlus analysis.
             */
            class Graph
            {
                public:
                    Graph();
                    virtual ~Graph();
                    Graph(TimedSDFgraph *g, bool weakConsistent = false, RepetitionVector *q = NULL);
                    ActorList *actors;
                    ChannelList *channels;
                    ActorCount *repetitionVector();
                    unsigned int stateSize;
                    unsigned int historySize;
                    unsigned int newTokenIndex;
                    static inline void fire(State *x, Actor *a);
                    void fire(State *x, Actor *a, double precision);
                    void fireSymbolic(SymbolicState *x, Actor *a);
                    void fire_reverse(State *x, Actor *a);
                    //    CString fireSchedule(State* x, Actor* a, ActorCount *remainingVector,
                    //            ostream& sout);

                private:
                    ActorCount *repVec;
                    void init();
            };

        }

        /**
         * class Exploration
         * A MaxPlusExploration object encapsulates MaxPlus analysis algorithms.
         */
        class Exploration
        {
            public:
                GraphDecoration::Graph *G;
                TimedSDFgraph *g;
                MPThroughput explore(void);
                MPThroughput explore_backward(void);
                Matrix *convertToMaxPlusMatrix(void);
                Matrix *convertToMaxPlusMatrixForWeakFSMSADF(void);
                TimedSDFgraph *convertToHSDFByMP(void);
                MPTime exploreDelayForSchedule(MaxPlus::Vector *gamma0,
                                               MaxPlus::Vector *gamma_s, MPTime period);
                void exploreEigen(MaxPlus::Vector **eigenvector, MPTime *eigenvalue);
                GraphDecoration::Actor **staticSchedule;
                MPDelay timeProgress(State *x);
                MPDelay normalize(State *x);

            private:
                CString &replaceString(CString &s, const CString &fnd, const CString &rep);
        };

        /**
         * mpStateSpaceThroughputAnalysis()
         * Compute throughput by MaxPlus exploration.
         */
        MPTime mpStateSpaceThroughputAnalysis(TimedSDFgraph *g, ostream &sout);

        /**
         * mpStateSpaceThroughputAnalysis_backward()
         * Compute throughput by backward MaxPlus exploration
         */
        MPTime mpStateSpaceThroughputAnalysis_backward(TimedSDFgraph *g, ostream &sout);

        /**
         * mpStateSpaceDelayAnalysis()
         * Compute the initial delay to obtain a delay-period linear model for a given
         * period and a given initialial schedule and steady-state schedule
         */
        MPTime mpStateSpaceDelayAnalysis(TimedSDFgraph *g, MaxPlus::Vector *gamma0,
                                         MaxPlus::Vector *gamma_s, MPTime period, ostream &sout);

        /**
         * mpStateSpaceEigenAnalysis()
         * Compute the eigenvector and eigenvalue of an SDF graph.
         */
        void mpStateSpaceEigenAnalysis(TimedSDFgraph *g, MaxPlus::Vector **eigenvector,
                                       MPTime *eigenvalue, ostream &sout);

    }

}

#endif
