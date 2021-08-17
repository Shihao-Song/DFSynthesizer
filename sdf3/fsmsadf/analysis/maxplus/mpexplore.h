/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   mpexplore.h
 *
 *  Author          :   Sander Stuijk (s.stuijk@tue.nl)
 *
 *  Date            :   May 5, 2009
 *
 *  Function        :   Max-Plus based state-space exploration algorithms.
 *
 *  History         :
 *      05-05-09    :   Initial version.
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

#ifndef FSMSADF_ANALYSIS_MAXPLUS_MPEXPLORE_H_INCLUDED
#define FSMSADF_ANALYSIS_MAXPLUS_MPEXPLORE_H_INCLUDED

#include <vector>
#include <string>
#include <list>
#include <map>

// todo: change to unordered_set
#ifdef _MSC_VER
#include <hash_set>
#else
#undef __DEPRECATED
#include <ext/hash_set>
using namespace __gnu_cxx;
#endif

#include "../base/repetition_vector.h"
#include "../../base/graph.h"

#include "../../../base/maxplus/maxplus.h"
using namespace MaxPlus;

namespace FSMSADF
{

    namespace MPExplore
    {
        // forward declarations
        class State;

        /**
         * class ActorCount
         * Associates with every actor a number, representing a number of
         * firings (e.g., a repetition vector).
         */
        class ActorCount : public RepetitionVector
        {
            public:
                // Constructor
                ActorCount(RepetitionVector &v);

                // All counts are zero
                bool isEmpty();

                // Get actor count of actor a
                uint getValue(Actor *a);

                // Decrement actor count of actor a
                void decreaseValue(Actor *a);

                // Get sum of the actor counts
                uint sum();
        };

        /**
         * class SGraph
         * Decorates a scenario graph with information for MaxPlus analysis.
         */
        class SGraph
        {
            public:
                // Constructor
                SGraph(ScenarioGraph *g, Scenario *s);

                // Get a copy of the repetition vector
                ActorCount *repetitionVector();

                // Fire actor a while in state x
                inline void fire(State *x, Actor *a);

                // Some graph annotations...
                unsigned int stateSize;
                unsigned int historySize;
                unsigned int newTokenIndex;
                map<Channel *, unsigned int> synchStorage;

            public:
                // Scenario graph
                ScenarioGraph *scenarioGraph;

                // Scenario
                Scenario *scenario;

            private:
                // Repetition vector of the graph
                ActorCount *repVec;
        };

        /**
         * class Token
         * Token represents a MaxPlus timestamped token in a graph.
         */
        class Token
        {
            public:
                // Constructor
                Token()
                {
                    productionTime = 0.0;
                };
                Token(MPTime t)
                {
                    productionTime = t;
                };

            public:
                MPTime productionTime;
        };

        /**
         * class TokenFIFO
         * A token fifo
         */
        class TokenFIFO
        {
            public:
                // Constructor
                TokenFIFO(uint sz);

                /*
                 * Read N timestamped tokens from the FIFO, return the max of all read
                 * timestamps. To save time it is not checked whether the token contains
                 * N tokens, only use this method if you are sure there are.
                 */
                inline MPTime removeFirstN(uint n)
                {
                    MPTime time;
                    n--;
                    Token *r = &(tokens[begin]);
                    ++begin %= tokens.size();
                    time = r->productionTime;
                    while (n-- > 0)
                    {
                        Token *r = &(tokens[begin]);
                        ++begin %= tokens.size();
                        time = MP_MAX(r->productionTime, time);
                    }
                    return time;
                }

                /* Append n tokens to the FIFO, all timestamped with t. To save time it
                 * is not checked whether there is space for the tokens, only use this
                 * method if you are sure there is.
                 */
                inline void append(MPTime t, uint n = 1)
                {
                    while (n-- > 0)
                    {
                        tokens[end].productionTime = t;
                        ++end %= tokens.size();
                    }
                }

                // Create a copy of this FIFO
                TokenFIFO *copy();

                // Compute hash value for this FIFO
                size_t hashValue() const;

                // Compare FIFOs for equality
                bool equals(const TokenFIFO *l) const;

                // Compare FIFOs
                int compare(const TokenFIFO *l) const;

                // Get a pointer to the first token in the FIFO
                Token *first();

                // Get the largest production time of the tokens in the FIFO
                MPTime getMax();

                // Decrease the production time of all tokens in the FIFO
                void decreaseAll(MPDelay amount);

                // Average the production times of the tokens with the times in FIFO l
                void smooth(TokenFIFO *l);

                // Get the number of tokens stored in the FIFO
                uint size() const;

                // Get a new FIFO where all current token production times are lowered
                // by the amount given in FIFO l
                TokenFIFO *minus(TokenFIFO *l) const;

                // Add the tokens in FIFO l to this FIFO, but scale production times
                // with the supplied factor
                void add(double factor, TokenFIFO *l);

                // Insert the token production times into the vector v
                uint addToVector(MaxPlus::Vector *v, uint k) const;

            private:
                // The actual FIFO
                vector<Token> tokens;

                // Index which indicates begin and end of tokens in FIFO. The FIFO is
                // empty when these indices are equal.
                uint begin;
                uint end;
        };


        /**
         * class State
         * State represents a distribution of timestamped tokens over the channels
         * of the graph.
         */
        class State
        {
            public:
                // Constructor
                State(SGraph *G);
                State(size_t sz);

                // Destrcutor
                ~State();

                // Create a copy of this state
                State *copy();

                // Initialize the state
                static State *zeroState(SGraph *G);
                static State *fromVector(SGraph *G, MaxPlus::Vector *v);
                static State *zeroScheduleState(SGraph *G);

                // Consume tokens from port p
                inline MPTime consume(Scenario *s, Port *p)
                {
                    Channel *c = p->getChannel();
                    TokenFIFO *l = table[c->getId()];
                    return l->removeFirstN(p->getRateOfScenario(s));
                }

                // Produce tokens on port p
                inline void produce(Scenario *s, Port *p, MPTime t)
                {
                    Channel *c = p->getChannel();
                    TokenFIFO *l = table[c->getId()];
                    l->append(t, p->getRateOfScenario(s));
                }

                // Check whether actor a is enabled
                bool actorEnabled(Scenario *s, Actor *a);

                // Compute hash value of the state
                size_t hashValue() const;

                // Compare the states using FIFO::equals() on each FIFO
                bool equals(const State *s) const;

                // Compare the states
                int compare(const State *s) const;

                // Perform a FIFO wise substraction
                State *minus(State *v) const;

                // Perform a FIFO wise addition
                void add(double factor, State *v);

                // Get a pointer to the first token in any FIFO
                Token *firstToken();

                // Normalize the production times of the tokens in the FIFOs
                MPDelay normalize();

                // Get the norm of the production times of the tokens in the FIFOs
                MPTime norm();

                // Smooth the production times of the tokens in the FIFOs
                void smooth(State *y);

                // Get the production times of the tokens in the FIFOs as a MP vector
                MaxPlus::Vector *asVector();

            private:
                // Initialize the FIFOs
                void init(size_t sz);

            private:
                // Time stamp of the state
                MPTime timestamp;

                // Token FIFOs
                vector<TokenFIFO *> table;
        };

#ifdef _MSC_VER

        /**
         * class StateHasherAndComparator
         * Compute hash value of state and compares.
         */
        class StateHasherAndComparator: public stdext::hash_compare<State *, less<State *>>
        {
            public:
                size_t operator()(const State *s) const
                {
                    return s->hashValue();
                }

                bool operator()(const State *s1, const State *s2) const
                {
                    return s1->compare(s2) < 0;
                }
        };

#else
        /**
         * class StateHasher
         * Compute hash value of state.
         */
        class StateHasher
        {
            public:
                size_t operator()(const State *s) const
                {
                    return s->hashValue();
                }
        };

        /**
         * class StateComparator
         * Compare two states for equality.
         */
        class StateComparator
        {
            public:
                bool operator()(const State *s1, const State *s2) const
                {
                    return s1->equals(s2);
                }
        };
#endif


        /**
         * class StoredStates
         * Hash of states.
         */
#ifdef _MSC_VER
        class StoredStates : stdext::hash_set<State *, StateHasherAndComparator>
#else
        class StoredStates : public hash_set<State *, StateHasher, StateComparator>
#endif
        {
            public:
                void store(State *x)
                {
                    insert(x);
                }
                bool includes(State *x, State **y)
                {
                    StoredStates::const_iterator i = find(x);
                    if (i != end())
                    {
                        (*y) = (*i);
                        return true;
                    }
                    return false;
                }
                bool includes(State *x)
                {
                    return find(x) != end();
                }
        };

        /**
         * class Exploration
         * A MaxPlusExploration object encapsulates MaxPlus analysis algorithms.
         */
        class Exploration
        {
            public:
                SGraph *G;
                Actor **staticSchedule;

                // Compute the minimal delay of a delay-period model for give period,
                // starting schedule and steady-state schedule according to the
                // algorithm presented in the paper "Synchronous Dataflow Scenarios".
                MPTime exploreDelayForSchedule(MaxPlus::Vector *gamma0,
                                               MaxPlus::Vector *gamma_s, MPTime period);

                // Compute the eigenvector and eigenvalue of the MaxPlus matrix
                // associated with the scenario graph.
                void exploreEigen(MaxPlus::Vector **eigenvector, MPTime *eigenvalue);

                // Convert the eigenvector v1 of scenario graph sg1 to an eigenvector
                // for this scenario graph.
                MaxPlus::Vector *convertEigenvector(ScenarioGraph *sg1,
                                                    MaxPlus::Vector *v1);
        };

    } // End namespace MPExplore

} // End namespace FSMSADF

#endif
