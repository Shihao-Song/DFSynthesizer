/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   mpexplore.cc
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

#include <assert.h>
#include "mpexplore.h"

namespace FSMSADF
{

    namespace MPExplore
    {
        /**
         * exploreDelayForSchedule()
         * Compute the minimal delay of a delay-period model for give period, starting
         * schedule and steady-state schedule according to the algorithm presented in
         * the paper "Synchronous Dataflow Scenarios".
         */
        MPTime Exploration::exploreDelayForSchedule(MaxPlus::Vector *gamma0,
                MaxPlus::Vector *gamma_s, MPTime period)
        {
            State *gamma = State::fromVector(G, gamma0);
            State *schedule = State::fromVector(G, gamma_s);
            MPTime tau = MP_MINUSINFINITY;
            State *m;

            // delta accumulates the total elapsed time minus the number of iterations
            // times period such that the actual schedule equals the normalized schedule
            // + k.period + delta
            MPTime delta = gamma->normalize() - period;

            // do a regular exploration and compute delay tau along the way.
            State *y;
            StoredStates *S = new StoredStates();
            S->store(gamma->copy());

            // make one iteration to determine the schedule;
            ActorCount *E = G->repetitionVector();
            unsigned int sch_length = E->sum();
            staticSchedule = new Actor*[sch_length];
            unsigned int sch_idx = 0;

            Actors::const_iterator i = G->scenarioGraph->getActors().begin();
            while (! E->isEmpty())
            {
                uint iterCnt = 0;
                Actor *a;
                // find an arbitrary enabled actor
                while (true)
                {
                    a = (Actor *)(*i);
                    if (E->getValue(a) > 0 && gamma->actorEnabled(G->scenario, a))
                        break;
                    if (++i == G->scenarioGraph->getActors().end())
                    {
                        i = G->scenarioGraph->getActors().begin();
                        iterCnt++;
                        if (iterCnt > 1)
                            throw CException("Graph is deadlocked.");
                    }
                }
                E->decreaseValue(a);
                staticSchedule[sch_idx++] = a;
                G->fire(gamma, a);
            }

            // determine the difference between the reference schedule and the actual
            // schedule
            m = gamma->minus(schedule);
            tau = MP_MAX(tau, m->norm() + delta);
            delete m;

            // update delta by adding the normalization difference and subtracting
            // period
            delta += gamma->normalize();
            delta -= period;

            // after the first iteration, continue with fixed schedule
            // until a recurrent state is found
            while (!S->includes(gamma, &y))
            {
                S->store(gamma->copy());
                for (uint k = 0; k < sch_length; k++)
                {
                    G->fire(gamma, staticSchedule[k]);
                }

                // update tau and delta
                State *m = gamma->minus(schedule);
                tau = MP_MAX(tau, m->norm() + delta);
                delete m;
                delta += gamma->normalize();
                delta -= period;
            }

            // cleanup
            delete [] staticSchedule;
            delete E;
            delete S;

            // tau equals the delay we seek.
            return tau;
        }

        /**
         * exploreEigen()
         * Compute the eigenvector and eigenvalue of the MaxPlus matrix associated with
         * the scenario graph.
         */
        void Exploration::exploreEigen(MaxPlus::Vector **eigenvector,
                                       MPTime *eigenvalue)
        {
            MPDelay lambda ;
            State *gamma = State::zeroState(G);
            State *gamma_prev;

            // make one iteration to determine the schedule;
            ActorCount *E = G->repetitionVector();
            unsigned int sch_length = E->sum();
            staticSchedule = new Actor*[sch_length];
            unsigned int sch_idx = 0;

            Actors::const_iterator i = G->scenarioGraph->getActors().begin();
            while (! E->isEmpty())
            {
                uint iterCnt = 0;
                Actor *a;
                while (true) // find an enabled actor
                {
                    a = (Actor *)(*i);
                    if (E->getValue(a) > 0 && gamma->actorEnabled(G->scenario, a))
                        break;
                    if (++i == G->scenarioGraph->getActors().end())
                    {
                        i = G->scenarioGraph->getActors().begin();
                        iterCnt++;
                        if (iterCnt > 1)
                            throw CException("Graph is deadlocked.");
                    }
                }
                E->decreaseValue(a);
                staticSchedule[sch_idx++] = a;
                G->fire(gamma, a);
            }
            lambda = gamma->normalize();
            gamma_prev = gamma->copy();

            // continue with fixed schedule
            bool equal;
            do
            {
                for (uint k = 0; k < sch_length; k++)
                {
                    G->fire(gamma, staticSchedule[k]);
                }
                lambda = gamma->normalize();
                gamma->smooth(gamma_prev);
                equal = gamma->equals(gamma_prev);
                delete gamma_prev;
                gamma_prev = gamma->copy();
            }
            while (!equal);
            delete gamma_prev;
            delete [] staticSchedule;
            delete E;
            *eigenvalue = lambda;
            *eigenvector = gamma->asVector();
        }

        /**
         * convertEigenvector()
         * The function converts the time stamps in the eigenvector v1 of scenario
         * graph sg1 to an eigenvector for this scenario graph. The eigen vector
         * contains an entry for each initial token in the scenario graph. The
         * entries are ordered based on the channel ids (low to high).
         */
        MaxPlus::Vector *Exploration::convertEigenvector(ScenarioGraph *sg1,
                MaxPlus::Vector *v1)
        {
            map<CString, vector<uint> > mapTokensSG1, mapTokensSG2;
            ScenarioGraph *sg2 = G->scenarioGraph;
            MaxPlus::Vector *v2;
            uint idx;

            // Iterate over the channels in the scenario graph sg1 to build index
            idx = 0;
            for (Channels::iterator i = sg1->getChannels().begin();
                 i != sg1->getChannels().end(); i++)
            {
                Channel *c = *i;

                // Add index entry for each initial token in the channel c
                for (uint j = 0; j < c->getInitialTokens(); j++)
                {
                    mapTokensSG1[c->getName()].push_back(idx++);
                }
            }

            // Iterate over the channels in the scenario graph sg1 to build index
            idx = 0;
            for (Channels::iterator i = sg2->getChannels().begin();
                 i != sg2->getChannels().end(); i++)
            {
                Channel *c = *i;

                // Add index entry for each initial token in the channel c
                for (uint j = 0; j < c->getInitialTokens(); j++)
                {
                    mapTokensSG2[c->getName()].push_back(idx++);
                }
            }

            // Allocate memory for the eigen vector of scenario graph sg2
            v2 = new MaxPlus::Vector(idx);

            // Initialize all entries to minus infinity (tokens initially available)
            for (uint i = 0; i < idx; i++)
                v2->put(i, MP_MINUSINFINITY);

            // Create eigen vector for scenario graph sg2
            for (map<CString, vector<uint> >::iterator i = mapTokensSG2.begin();
                 i != mapTokensSG2.end(); i++)
            {
                CString cName = i->first;

                // Corresponding channel exists in sg1?
                map<CString, vector<uint> >::iterator j = mapTokensSG1.find(cName);

                // Channel exists in sg1?
                if (j != mapTokensSG1.end())
                {
                    // Check that both scenario graphs have same number of entries
                    // for the channel (i.e. same number of initial tokens)
                    if (i->second.size() != j->second.size())
                    {
                        throw CException("Channel '" + cName + "' should have same "
                                         "number of initial tokens in all scenario "
                                         "graphs.");
                    }

                    // Copy time stamps from vector v1 to vector v2
                    for (uint k = 0; k < i->second.size(); k++)
                    {
                        v2->put(i->second.at(k), v1->get(j->second.at(k)));
                    }
                }
            }

            return v2;
        }

        /**
         * ActorCount()
         * Constructor.
         */
        ActorCount::ActorCount(RepetitionVector &v) : RepetitionVector(v)
        {
        }

        /**
         * isEmpty()
         * The function returns true when all actor counts are zero. Otherwise it
         * returns false.
         */
        bool ActorCount::isEmpty()
        {
            for (uint i = 0; i < size(); i++)
            {
                if (at(i) > 0)
                    return false;
            }
            return true;
        }

        /**
         * decreaseValue()
         * Decrement the actor count of actor a.
         */
        void ActorCount::decreaseValue(Actor *a)
        {
            at(a->getId())--;
        }

        /**
         * getValue()
         * The function returns the actor count of actor a.
         */
        uint ActorCount::getValue(Actor *a)
        {
            return at(a->getId());
        }

        /**
         * sum()
         * The function returns the sum of the actor counts.
         */
        uint ActorCount::sum()
        {
            uint s = 0;

            for (uint i = 0; i < size(); i++)
            {
                s += at(i);
            }

            return s;
        }

        /**
         * SGraph()
         * Constructor.
         */
        SGraph::SGraph(ScenarioGraph *g, Scenario *s)
            : scenarioGraph(g), scenario(s)
        {
            // Compute repetition vector
            RepetitionVector E = computeRepetitionVector(scenarioGraph, scenario);
            repVec = new ActorCount(E);

            // copy channels
            Channels::iterator j;
            int stateIndex = 0;
            int historyIndex = 0;
            int k;
            for (j = scenarioGraph->getChannels().begin(), k = 0;
                 j != scenarioGraph->getChannels().end(); j++, k++)
            {
                Channel *c = *j;
                Actor *a = c->getSrcActor();
                Actors::const_iterator i;
                unsigned int cRepetitionVectorEntry = 0;
                int m = 0;
                for (i = scenarioGraph->getActors().begin();
                     i != scenarioGraph->getActors().end(); i++, m++)
                {
                    Actor *b = *i;
                    if (a == b)
                    {
                        synchStorage[c] = c->getInitialTokens() + E.at(m)
                                          * c->getSrcPort()->getRateOfScenario(scenario);
                        cRepetitionVectorEntry = E.at(m)
                                                 * c->getSrcPort()->getRateOfScenario(scenario);
                        break;
                    }
                    assert(i != scenarioGraph->getActors().end());
                }
                stateIndex += c->getInitialTokens();
                if (c->getInitialTokens() > cRepetitionVectorEntry)
                    historyIndex += c->getInitialTokens() - cRepetitionVectorEntry;
            }

            this->stateSize = stateIndex;
            this->historySize = historyIndex;
        }

        /**
         * fire()
         */
        void SGraph::fire(State *x, Actor *a)
        {
            MPTime time, ftime;
            ftime = MP_MINUSINFINITY;
            Ports::iterator i;

            for (i = a->getPorts().begin(); i != a->getPorts().end(); i++)
            {
                Port *p = *i;
                if (p->getType() == Port::In)
                {
                    time = x->consume(scenario, p);
                    ftime = MP_MAX(time, ftime);
                }
            }
            time = ftime + MPTime(a->getExecutionTimeOfScenario(scenario,
                                  a->getDefaultProcessorType()));
            for (i = a->getPorts().begin(); i != a->getPorts().end(); i++)
            {
                Port *p = *i;
                if (p->getType() == Port::Out)
                {
                    x->produce(scenario, p, time);
                }
            }
        }

        /**
         * repetitionVector()
         */
        ActorCount *SGraph::repetitionVector()
        {
            return new ActorCount(*repVec);
        }

        /**
         * State()
         * Constructor.
         */
        State::State(SGraph *G)
        {
            init(G->scenarioGraph->getChannels().size());
        }

        /**
         * State()
         * Constructor.
         */
        State::State(size_t sz)
        {
            init(sz);
        }

        /**
         * init()
         * Initialize the state.
         */
        void State::init(size_t sz)
        {
            table.resize(sz);
            for (uint i = 0; i < sz; i++)
                table[i] = new TokenFIFO(sz);
        }

        /**
         * ~State()
         * Destructor.
         */
        State::~State()
        {
            for (uint i = 0; i < table.size(); i++)
            {
                delete table[i];
            }
        }

        /**
         * actorEnabled()
         * The function returns true when the actor is enabled (i.e. ready to fire).
         * Otherwise the function returns false.
         */
        bool State::actorEnabled(Scenario *s, Actor *a)
        {
            // Check all ports
            for (Ports::iterator i = a->getPorts().begin();
                 i != a->getPorts().end(); i++)
            {
                Port *p = *i;

                // Is this port an input port?
                if (p->getType() == Port::In)
                {
                    // Not enough tokens available?
                    if (table[p->getChannel()->getId()]->size()
                        < p->getRateOfScenario(s))
                    {
                        return false;
                    }
                }
            }

            return true;
        }

        /**
         * zeroState()
         * Initialize this state as an initial state (i.e. channels contains only
         * initial tokens and no time has passed so far).
         */
        State *State::zeroState(SGraph *G)
        {
            State *s = new State(G);

            // Iterate over all channels
            for (Channels::iterator i = G->scenarioGraph->getChannels().begin();
                 i != G->scenarioGraph->getChannels().end(); i++)
            {
                Channel *c = *i;
                s->table[c->getId()] = new TokenFIFO(G->synchStorage[c]);

                // Add a token to channel c with production time 0 for each
                // initial tokens which this channel contains
                for (uint j = 0; j < c->getInitialTokens(); j++)
                {
                    s->table[c->getId()]->append(0.0);
                }
            }

            // This is the initial state
            s->timestamp = 0.0;

            return s;
        }

        /**
         * fromVector()
         * Initialize this state as an initial state (i.e. channels contains only
         * initial tokens and no time has passed so far). The production times of the
         * initial tokens are taken from the vector v.
         */
        State *State::fromVector(SGraph *G, MaxPlus::Vector *v)
        {
            State *s = new State(G);
            uint m = 0;

            for (Channels::iterator i = G->scenarioGraph->getChannels().begin();
                 i != G->scenarioGraph->getChannels().end(); i++)
            {
                Channel *c = *i;
                s->table[c->getId()] = new TokenFIFO(G->synchStorage[c]);

                // Add a token to channel c with the production time given in the vector
                // v for each initial tokens which this channel contains
                for (uint j = 0; j < c->getInitialTokens(); j++)
                {
                    s->table[c->getId()]->append(v->get(m++));
                }
            }

            // This is the initial state
            s->timestamp = 0.0;

            return s;
        }

        /**
         * zeroScheduleState()
         * Initialize this state (i.e. channels contains only initial tokens). The
         * production times of the initial tokens are equal to a counter which is
         * raised on each initial tokens that is placed in a FIFO.
         */
        State *State::zeroScheduleState(SGraph *G)
        {
            State *s = new State(G);
            int posCount = 0;

            for (Channels::iterator i = G->scenarioGraph->getChannels().begin();
                 i != G->scenarioGraph->getChannels().end(); i++)
            {
                Channel *c = *i;
                s->table[c->getId()] = new TokenFIFO(G->synchStorage[c]);

                // Add a token to channel c with the production time equal to a counter
                // which is raised on each initial tokens that is placed in a FIFO.
                for (uint j = 0; j < c->getInitialTokens(); j++)
                {
                    s->table[c->getId()]->append(posCount++);
                }
            }

            return s;
        }

        /**
         * copy()
         * The function returns a copy of this state.
         */
        State *State::copy()
        {
            State *s = new State(table.size());

            for (uint i = 0; i < table.size(); i++)
            {
                s->table[i] = table[i]->copy();
            }

            s->timestamp = timestamp;

            return s;
        }

        /**
         * smooth()
         * The function performs a smooth operation on all FIFOs.
         */
        void State::smooth(State *y)
        {
            for (uint i = 0; i < table.size(); i++)
            {
                table[i]->smooth(y->table[i]);
            }
        }

        /**
         * hashValue()
         * The function returns a hash value for this state.
         */
        size_t State::hashValue() const
        {
            size_t h = 0;

            for (uint i = 0; i < table.size(); i++)
            {
                h = 31 * h + table[i]->hashValue();
            }

            return h;
        }

        /**
         * equals()
         * The function performs an equals operation on all FIFOs.
         */
        bool State::equals(const State *s) const
        {
            if (table.size() != s->table.size())
            {
                return false;
            }

            for (uint i = 0; i < table.size(); i++)
            {
                if (!table[i]->equals(s->table[i]))
                    return false;
            }

            return true;
        }

        /**
         * compare()
         * The function implements a comparison to generate a total order for MS hash_set
         * return -1 if receiver smaller than s,
         * 0 if equal, otherwise 1
         * performs a lexicographical order on all FIFOs.
         */
        int State::compare(const State *s) const
        {
            if (this->table.size() != s->table.size())
            {
                if (this->table.size() < s->table.size()) return -1;
                return 1;
            }

            for (uint i = 0; i < table.size(); i++)
            {
                int fc;
                fc = table[i]->compare(s->table[i]);
                if (fc < 0) return -1;
                if (fc > 0) return 1;
            }

            return 0;
        }

        /**
         * minus()
         * The function performs a minus operation on all FIFOs.
         */
        State *State::minus(State *v) const
        {
            State *s = new State(table.size());

            for (uint i = 0; i < table.size(); i++)
            {
                TokenFIFO *l = table[i];
                s->table[i] = l->minus(v->table[i]);
            }

            return s;
        }

        /**
         * add()
         * The function performs a add operation on all FIFOs.
         */
        void State::add(double factor, State *v)
        {
            for (uint i = 0; i < table.size(); i++)
            {
                TokenFIFO *l = table[i];
                l->add(factor, v->table[i]);
            }
        }

        /**
         * firstToken()
         * The function returns a pointer to the first token in any of the FIFOs. When
         * all FIFOs are empty, the function returns NULL.
         */
        Token *State::firstToken()
        {
            for (uint i = 0; i < table.size(); i++)
            {
                TokenFIFO *l = table[i];
                if (l->size() > 0)
                    return l->first();
            }

            return NULL;
        }

        /**
         * normalize()
         * The function normalizes the production time fo the tokens in all FIFOs.
         */
        MPDelay State::normalize()
        {
            MPDelay amount = norm();

            for (uint i = 0; i < table.size(); i++)
            {
                table[i]->decreaseAll(amount);
            }

            timestamp += amount;

            return amount;
        }

        /**
         * norm()
         * The function returns the norm of the token production times.
         */
        MPTime State::norm()
        {
            MPTime max = 0;

            for (uint i = 0; i < table.size(); i++)
            {
                if (i == 0)
                    max = table[i]->getMax();
                else
                    max = MP_MAX(max, table[i]->getMax());
            }

            return max;
        }

        /**
         * asVector()
         * The function returns the token production times as a MaxPlus vector.
         */
        MaxPlus::Vector *State::asVector(void)
        {
            MaxPlus::Vector *v = new MaxPlus::Vector();
            uint k = 0;

            for (uint i = 0; i < table.size(); i++)
            {
                k = table[i]->addToVector(v, k);
            }

            return v;
        }

        /**
         * TokenFIFO()
         * Constructor.
         */
        TokenFIFO::TokenFIFO(uint sz)
        {
            // Fifo should be able to hold sz tokens
            tokens.resize(sz + 1);
            begin = 0;
            end = 0;
        }

        /**
         * copy()
         * The function returns a copy of the FIFO.
         */
        TokenFIFO *TokenFIFO::copy()
        {
            return new TokenFIFO(*this);
        }

        /**
         * hashValue()
         * The function returns a hash value for the FIFO.
         */
        size_t TokenFIFO::hashValue() const
        {
            size_t h = 0;

            uint i = begin;
            while (i != end)
            {
                h = 23 * h + (size_t)tokens[i].productionTime;
                i = (i + 1) % tokens.size();
            }

            return h;
        }

        /**
         * equals()
         * The function returns true when two FIFO are identical (i.e. same size and
         * same content).
         */
        bool TokenFIFO::equals(const TokenFIFO *l) const
        {
            if (tokens.size() != l->tokens.size())
                return false;

            uint i = begin;
            uint j = l->begin;

            while (i != end)
            {
                // Check equality of production times. Consider production times
                // equal when difference is less then some MP_EPSILON
                // handle -infinity separately
                MPTime ti = tokens[i].productionTime;
                MPTime tj = l->tokens[j].productionTime;
                if (MP_ISMINUSINFINITY(tj))
                {
                    if (!MP_ISMINUSINFINITY(ti)) return false;
                }
                else
                {
                    if (MP_ISMINUSINFINITY(ti)) return false;

                    if (fabs(ti - tj) > MP_EPSILON)
                    {
                        return false;
                    }
                }
                ++i %= tokens.size();
                ++j %= tokens.size();
            }
            return true;
        }

        /**
         * compare()
         * impose a total ordering on FIFOs
         */
        int TokenFIFO::compare(const TokenFIFO *l) const
        {
            if (tokens.size() < l->tokens.size())
                return -1;
            if (tokens.size() > l->tokens.size())
                return 1;

            uint i = begin;
            uint j = l->begin;

            while (i != end)
            {
                // Check equality of production times. Consider production times
                // equal when difference is less then some MP_EPSILON
                MPTime ti = tokens[i].productionTime;
                MPTime tj = l->tokens[j].productionTime;
                if (MP_ISMINUSINFINITY(tj))
                {
                    if (!MP_ISMINUSINFINITY(ti)) return 1; // ti>tj
                }
                else
                {
                    if (MP_ISMINUSINFINITY(ti)) return -1; // ti<tj

                    MPTime diff = ti - tj;
                    if (-diff > MP_EPSILON) return -1; // ti<tj
                    if (diff > MP_EPSILON) return 1; // tj<ti
                }
                // Next
                ++i %= tokens.size();
                ++j %= tokens.size();
            }
            return 0;
        }


        /**
         * minus()
         * The function returns a pointer to a new FIFO which is a copy of the tokens
         * in this FIFO, but with their production times lowered by the amount specified
         * in the FIFO m.
         */
        TokenFIFO *TokenFIFO::minus(TokenFIFO *m) const
        {
            // Create a copy of the FIFO
            TokenFIFO *l = new TokenFIFO(*this);

            uint i = begin;
            uint j = m->begin;
            while (i != end)
            {
                l->tokens[i].productionTime = tokens[i].productionTime
                                              - m->tokens[j].productionTime;
                i = (i + 1) % tokens.size();
                j = (j + 1) % tokens.size();
            }

            return l;
        }

        /**
         * add()
         * The function adds the token production times of the tokens in FIFO m to this
         * fifo. These token production times are however scaled with the supplied
         * factor.
         */
        void TokenFIFO::add(double factor, TokenFIFO *m)
        {
            uint i = begin;
            uint j = m->begin;

            while (i != end)
            {
                tokens[i].productionTime += factor * m->tokens[j].productionTime;
                i = (i + 1) % tokens.size();
                j = (j + 1) % tokens.size();
            }
        }

        /**
         * addToVector()
         * The function adds the token production times of all tokens which are stored
         * in this fifo to the vector v. The first token production time is placed at
         * position k in this vector. The function returns the position after which the
         * last addition to vector v has taken place.
         */
        uint TokenFIFO::addToVector(MaxPlus::Vector *v, uint k) const
        {
            uint i = begin;

            while (i != end)
            {
                v->put(k++, tokens[i].productionTime);
                i = (i + 1) % tokens.size();
            }

            return k;
        }

        /**
         * first()
         * The function returns a pointer to the first token in the FIFO.
         */
        Token *TokenFIFO::first()
        {
            return &(tokens[begin]);
        }

        /**
         * decreaseAll()
         * The function decreases the production time of all tokens in the FIFO by the
         * specified amount.
         */
        void TokenFIFO::decreaseAll(MPDelay amount)
        {
            uint i = begin;

            while (i != end)
            {
                tokens[i].productionTime -= amount;
                i = (i + 1) % tokens.size();
            }
        }

        /**
         * getMax()
         * The function returns the largest production time of the tokens in the FIFO.
         */
        MPTime TokenFIFO::getMax()
        {
            MPTime max = 0.0;
            uint i = begin;

            while (i != end)
            {
                if (i == begin)
                    max = tokens[i].productionTime;
                else
                    max = MP_MAX(max, tokens[i].productionTime);
                i = (i + 1) % tokens.size();
            }

            return max;
        }

        /**
         * size()
         * The function returns the number of tokens in the FIFO.
         */
        uint TokenFIFO::size() const
        {
            int r = end - begin;

            if (r < 0)
                r += tokens.size();

            return (uint)(r);
        }

        /**
         * smooth()
         * Average the production time of the tokens in this fifo with the production
         * times in the fifo l.
         */
        void TokenFIFO::smooth(TokenFIFO *l)
        {
            uint i = begin;
            uint j = l->begin;

            while (i != end)
            {
                tokens[i].productionTime = 0.5 * tokens[i].productionTime
                                           + 0.5 * l->tokens[j].productionTime;
                i = (i + 1) % tokens.size();
                j = (j + 1) % tokens.size();
            }
        }

    } // End namespace MPExplore

} // End namespace FSMSADF

