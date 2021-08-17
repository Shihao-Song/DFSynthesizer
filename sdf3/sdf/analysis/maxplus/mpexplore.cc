/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   mpexplore.cc
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

#include <assert.h>
#include <sstream>
#include "mpexplore.h"
#include "mpstorage.h"
#include "sdf/base/algo/repetition_vector.h"

namespace SDF
{

    namespace MaxPlusAnalysis
    {

        using namespace GraphDecoration;

        ostream *out = NULL;

        /**
         * explore()
         * perform state space exploration till recurrent state to determine throughput
         */
        MPThroughput Exploration::explore(void)
        {
            // set x to initial state (zero vector)
            State *x = State::zeroState(G);
            State *y;

            // store all visited states (schedules) in S
            StoredStates *S = new StoredStates();

            // first make one iteration to determine the static schedule;

            // every actor needs to fire a number of times according to the repetition
            // vector
            ActorCount *E = G->repetitionVector();

            // create table to hold the schedule
            // size is equal to sum of the repetition vector
            int sch_length = E->sum();
            this->staticSchedule = new Actor*[sch_length];
            int sch_idx = 0;

            ActorList::const_iterator i = G->actors->begin();

            //until a complete iteration has been scheduled...
            while (! E->isEmpty())
            {
                Actor *a;
                while (true) // find an enabled actor
                {
                    a = *i;
                    // break if actor is enabled
                    if (E->getValue(a) > 0 && x->actorEnabled(a)) break;
                    // try next actor, wrap around if at end of actor list
                    if (++i == G->actors->end()) i = G->actors->begin();
                }
                // account for firing of a
                E->decreaseValue(a);
                // fill in the schedule table
                this->staticSchedule[sch_idx++] = a;
                // let G fire a on state x
                G->fire(x, a);
            }
            // an iteration is complete, normalize the schedule
            this->normalize(x);

            // from now on continue with fixed schedule in staticSchedule[]

            // until a normalised schedule is reached that we saw before
            while (!S->includes(x, &y))
            {
                // store the state x in the set S of visited schedules
                S->store(x->copy());
                // count the number of iterations explored
                (x->count)++;
                // fire a complete iteration according to the static schedule
                for (int k = 0; k < sch_length; k++)
                {
                    G->fire(x, this->staticSchedule[k]);
                }
                // normalizer the result
                this->normalize(x);
            }

            // we have reached a recurrent state and we are done.

            // clean up
            delete this->staticSchedule;
            delete E;
            // return the throughput: number of iterations between x and y divided by
            // their time difference
            return (x->count - y->count) / (x->timestamp - y->timestamp);
        }

        /**
         * explore_backward()
         * Perform state space exploration backward in time till recurrent state to
         * determine throughput
         */
        MPThroughput Exploration::explore_backward(void)
        {
            // set x to initial state (zero vector)
            State *x = State::zeroState(G);
            State *y;

            // store all visited states (schedules) in S
            StoredStates *S = new StoredStates();

            // first make one iteration to determine the static schedule

            // every actor needs to fire a number of times according to the repetition
            // vector
            ActorCount *E = G->repetitionVector();

            // create table to hold the schedule
            // size is equal to sum of the repetition vector
            int sch_length = E->sum();
            staticSchedule = new Actor*[sch_length];
            int sch_idx = 0;

            ActorList::const_iterator i = G->actors->begin();

            //until a complete iteration has been scheduled...
            while (! E->isEmpty())
            {
                Actor *a;
                while (true) // find an enabled actor
                {
                    a = *i;
                    // break if actor is 'reverse-enabled'
                    if (E->getValue(a) > 0 && x->actorReverseEnabled(a)) break;
                    // try next actor, warp around if at end of actor list
                    if (++i == G->actors->end()) i = G->actors->begin();
                }
                // account for firing of a
                E->decreaseValue(a);
                //fill in the scheduling table
                staticSchedule[sch_idx++] = a;
                // let G fire backward on state x
                G->fire_reverse(x, a);
            }

            // an iteration is complete, normalize the schedule
            this->normalize(x);

            // from now on continue with fixed schedule in staticSchedule[]

            // until a normalised schedule is reached that we saw before
            while (!S->includes(x, &y))
            {
                // store the state x in the set S of visited schedules
                S->store(x->copy());
                // count the number of iterations explored
                (x->count)++;
                // fire a complete iteration according to the static schedule
                for (int k = 0; k < sch_length; k++)
                {
                    G->fire_reverse(x, staticSchedule[k]);
                }

                // normalize the result
                this->normalize(x);
            }

            // we have reached a recurrent state and we are done.

            // clean up
            delete staticSchedule;
            delete E;

            // return the throughput: number of iterations between x and y divided by
            // their time difference
            return (x->count - y->count) / (y->timestamp - x->timestamp);
        }

        /**
         * convertToMaxPlusMatrix()
         */
        Matrix *Exploration::convertToMaxPlusMatrix(void)
        {

            // do a symbolic exploration of a single iteration

            // make symbolic zero-vector
            SymbolicState *x = SymbolicState::zeroState(G);

            // every actor needs to fire a number of times according to the repetition
            // vector
            ActorCount *E = G->repetitionVector();

            ActorList::const_iterator i = G->actors->begin();
            while (! E->isEmpty())
            {
                Actor *a;
                while (true) // find an enabled actor
                {
                    a = *i;
                    if (E->getValue(a) > 0 && x->actorEnabled(a)) break;
                    if (++i == G->actors->end()) i = G->actors->begin();
                }
                E->decreaseValue(a);
                G->fireSymbolic(x, a);
            }

            delete E;


            // Construct a Maxplus matrix

            unsigned int N = x->numberOfTokens();
            Matrix *MG = new Matrix(N);

            // Add fill matrix rows symbolic tokens
            SymbolicToken *t = x->firstToken();
            for (unsigned int i = 0; i < N; i++)
            {
                for (unsigned int j = 0; j < N; j++)
                {
                    //take the j'th entry out of symbolic token t
                    MG->put(i, j, ((*t)[j])->productionTime);
                }
                t = x->nextToken();
            }
            return MG;
        }

        /**
         * convertToMaxPlusMatrix()
         */
        Matrix *Exploration::convertToMaxPlusMatrixForWeakFSMSADF(void)
        {

            // do a symbolic exploration of a single iteration

            // make symbolic zero-vector
            SymbolicState *x = SymbolicState::zeroState(G);
            unsigned int NC = x->numberOfTokens();

            // every actor needs to fire a number of times according to the repetition
            // vector
            ActorCount *E = G->repetitionVector();

            ActorList::const_iterator i = G->actors->begin();
            while (! E->isEmpty())
            {
                Actor *a;
                while (true) // find an enabled actor
                {
                    a = *i;
                    if (E->getValue(a) > 0 && x->actorEnabled(a)) break;
                    if (++i == G->actors->end()) i = G->actors->begin();
                }
                E->decreaseValue(a);
                G->fireSymbolic(x, a);
            }

            delete E;


            // Construct a Maxplus matrix

            unsigned int NR = x->numberOfTokens();
            Matrix *MG = new Matrix(NR, NC);

            // Add fill matrix rows symbolic tokens
            SymbolicToken *t = x->firstToken();
            for (unsigned int i = 0; i < NR; i++)
            {
                for (unsigned int j = 0; j < NC; j++)
                {
                    //take the j'th entry out of symbolic token t
                    MG->put(i, j, ((*t)[j])->productionTime);
                }
                t = x->nextToken();
            }
            return MG;
        }


        /**
         * convertToHSDFByMP()
         * Convert SDF graph to canonical HSDFG according to DAC09 submission procedure.
         */
        TimedSDFgraph *Exploration::convertToHSDFByMP(void)
        {
            // calculate the MaxPlus
            Matrix *matrix = this->convertToMaxPlusMatrix();

            // construct the equivalent graph
            TimedSDFgraph *h;

            // Construct a new (H)SDF graph
            SDFcomponent component = SDFcomponent(NULL, 0);
            h = g->createCopy(component);

            unsigned int N = matrix->getSize();
            TimedSDFactor **newActors = (TimedSDFactor **) malloc((N + 2) * N * sizeof(TimedSDFactor *));

            // make cross actors
            for (unsigned int i = 0; i < N; i++)
            {
                for (unsigned int j = 0; j < N; j++)
                {
                    // MaxPlus calculations involving -infty need to be handled properly
                    if (matrix->get(i, j) >= 0.0)
                    {
                        component = SDFcomponent(h, h->nrActors());
                        TimedSDFactor *a = new TimedSDFactor(*h);
                        a->setName(CString("CrossActor_") + CString(i)
                                   + CString("_") + CString(j));
                        TimedSDFactor::Processor *p = new TimedSDFactor::Processor();
                        // note: the execution time is being converted from float to
                        // integer here! perhaps introduce granularity / time unit later
                        // shouldn't be a problem if the original SDF only has integer
                        // execution times which should be the case for an SDF
                        // specification in xml
                        p->execTime = (uint) matrix->get(i, j);
                        p->type = "default";
                        p->stateSize = 1;
                        a->addProcessor(p);
                        a->setDefaultProcessor("default");
                        h->addActor(a);
                        newActors[i * N + j] = a;
                    }
                    else
                    {
                        newActors[i * N + j] = NULL;
                    }
                }
            }
            // make sync actors
            for (unsigned int i = 0; i < N; i++)
            {
                TimedSDFactor *a = NULL;
                TimedSDFactor::Processor *p;

                // if it has more than 1 outgoing edge, i.e. if more than one of
                // its (k,i)>-inf
                unsigned int edges = 0;
                for (unsigned int k = 0; k < N; k++)
                {
                    if (matrix->get(k, i) >= 0.0)
                    {
                        edges++;
                        a = newActors[k * N + i];
                    }
                }
                if (edges > 1)
                {
                    component = SDFcomponent(h, h->nrActors());
                    a = new TimedSDFactor(*h);
                    a->setName(CString("SyncActorL_") + CString(i));
                    p = new TimedSDFactor::Processor();
                    p->execTime = 0;
                    p->type = "default";
                    p->stateSize = 1;
                    a->addProcessor(p);
                    a->setDefaultProcessor("default");
                    h->addActor(a);
                }
                newActors[N * N + i] = a;

                // if it has more than 1 incoming edge, i.e. if more than one of
                // its (i,k)>-inf
                edges = 0;
                for (unsigned int k = 0; k < N; k++)
                {
                    if (matrix->get(i, k) >= 0.0)
                    {
                        edges++;
                        a = newActors[i * N + k];
                    }
                }
                if (edges > 1)
                {
                    component = SDFcomponent(h, h->nrActors());
                    a = new TimedSDFactor(*h);
                    a->setName(CString("SyncActorR_") + CString(i));
                    p = new TimedSDFactor::Processor();
                    p->execTime = 0;
                    p->type = "default";
                    p->stateSize = 1;
                    a->addProcessor(p);
                    a->setDefaultProcessor("default");
                    h->addActor(a);
                }
                newActors[(N + 1)*N + i] = a;
            }

            // add cross edges
            for (unsigned int i = 0; i < N; i++)
            {
                for (unsigned int j = 0; j < N; j++)
                {
                    if (newActors[i * N + j] != NULL)
                    {
                        // from j to i
                        SDFactor *srcActor = newActors[N * N + j];
                        SDFactor *midActor = newActors[i * N + j];
                        SDFactor *dstActor = newActors[(N + 1) * N + i];

                        // if left sync exists
                        if (newActors[N * N + j] != newActors[i * N + j])
                        {
                            // Create port on source node
                            component = SDFcomponent(srcActor, srcActor->nrPorts());
                            SDFport *srcP = new SDFport(component);
                            srcP->setName(CString("toCross_") + CString(i));
                            srcP->setRate(1);
                            srcP->setType(SDFport::Out);
                            srcActor->addPort(srcP);

                            // Create left port on mid node
                            component = SDFcomponent(midActor, midActor->nrPorts());
                            SDFport *midlP = new SDFport(component);
                            midlP->setName(CString("fromSync_") + CString(j));
                            midlP->setRate(1);
                            midlP->setType(SDFport::In);
                            midActor->addPort(midlP);

                            // Create channel between left sync and cross
                            component = SDFcomponent(h, h->nrChannels());
                            SDFchannel *c = new TimedSDFchannel(component);
                            c->setName(CString("Sync_") + CString(j)
                                       + CString("_toCross_") + CString(i));
                            c->connectSrc(srcP);
                            c->connectDst(midlP);
                            c->setInitialTokens(0);
                            h->addChannel(c);
                        }

                        // if left sync exists
                        if (newActors[(N + 1)*N + i] != newActors[i * N + j])
                        {
                            // Create port on dest node
                            component = SDFcomponent(dstActor, dstActor->nrPorts());
                            SDFport *dstP = new SDFport(component);
                            dstP->setName(CString("fromCross_") + CString(j));
                            dstP->setRate(1);
                            dstP->setType(SDFport::In);
                            dstActor->addPort(dstP);

                            // Create right port on mid node
                            component = SDFcomponent(midActor, midActor->nrPorts());
                            SDFport *midrP = new SDFport(component);
                            midrP->setName(CString("toSync_") + CString(i));
                            midrP->setRate(1);
                            midrP->setType(SDFport::Out);
                            midActor->addPort(midrP);

                            // Create channel between cross and right sync
                            component = SDFcomponent(h, h->nrChannels());
                            SDFchannel *c = new TimedSDFchannel(component);
                            c->setName(CString("Cross_") + CString(j)
                                       + CString("_toSync_") + CString(i));
                            c->connectSrc(midrP);
                            c->connectDst(dstP);
                            c->setInitialTokens(0);
                            h->addChannel(c);
                        }
                    }
                }
            }

            // make channels with initial tokens
            for (unsigned int i = 0; i < N; i++)
            {
                // from right sync i to left sync i
                SDFactor *srcActor = newActors[(N + 1) * N + i];
                SDFactor *dstActor = newActors[N * N + i];

                // Create port on source node
                component = SDFcomponent(srcActor, srcActor->nrPorts());
                SDFport *srcP = new SDFport(component);
                srcP->setName(CString("toInitialTokenEdge_") + CString(i));
                srcP->setRate(1);
                srcP->setType(SDFport::Out);
                srcActor->addPort(srcP);

                // Create port on dest node
                component = SDFcomponent(dstActor, dstActor->nrPorts());
                SDFport *dstP = new SDFport(component);
                dstP->setName(CString("fromInitialEdge_") + CString(i));
                dstP->setRate(1);
                dstP->setType(SDFport::In);
                dstActor->addPort(dstP);

                // Create channel between left sync and cross
                component = SDFcomponent(h, h->nrChannels());
                SDFchannel *c = new TimedSDFchannel(component);
                c->setName(CString("InitialEdge_") + CString(i));
                c->connectSrc(srcP);
                c->connectDst(dstP);
                c->setInitialTokens(1);
                h->addChannel(c);
            }

            // clean up
            free(newActors);
            delete matrix;

            return h;
        }

        /**
         * replaceString()
         * Utility function for substring replacement.
         */
        CString &Exploration::replaceString(CString &s, const CString &fnd,
                                            const CString &rep)
        {
            CString::size_type p;
            while ((p = s.find(fnd, 0)) != CString::npos)
            {
                s = s.replace(p, fnd.size(), rep);
            }
            return s;
        }

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

            ActorList::const_iterator i = G->actors->begin();
            while (! E->isEmpty())
            {
                Actor *a;
                // find an arbitrary enabled actor
                while (true)
                {
                    a = *i;
                    if (E->getValue(a) > 0 && gamma->actorEnabled(a)) break;
                    if (++i == G->actors->end()) i = G->actors->begin();
                }
                E->decreaseValue(a);
                staticSchedule[sch_idx++] = a;
                G->fire(gamma, a);
            }

            // determine the difference between the reference schedule and the actual
            // schedule
            m = gamma->minus(schedule);
            tau = MAX(tau, m->norm() + delta);
            delete m;

            // update delta by adding the normalization difference and subtracting
            // period
            delta += this->normalize(gamma);
            delta -= period;

            // after the first iteration, continue with fixed schedule
            // until a recurrent state is found
            while (!S->includes(gamma, &y))
            {
                S->store(gamma->copy());
                (gamma->count)++;
                for (uint k = 0; k < sch_length; k++)
                {
                    G->fire(gamma, staticSchedule[k]);
                }

                // update tau and delta
                State *m = gamma->minus(schedule);
                tau = MAX(tau, m->norm() + delta);
                delete m;
                delta += this->normalize(gamma);
                delta -= period;
            }

            // cleanup
            delete [] staticSchedule;
            delete E;

            // tau equals the delay we seek.
            return tau;
        }

        /**
         * exploreEigen()
         * Compute the eigenvector and eigenvalue of the MaxPlus matrix associated with
         * the SDFG.
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

            ActorList::const_iterator i = G->actors->begin();
            while (! E->isEmpty())
            {
                Actor *a;
                while (true) // find an enabled actor
                {
                    a = *i;
                    if (E->getValue(a) > 0 && gamma->actorEnabled(a)) break;
                    if (++i == G->actors->end()) i = G->actors->begin();
                }
                E->decreaseValue(a);
                staticSchedule[sch_idx++] = a;
                G->fire(gamma, a);
            }
            lambda = this->normalize(gamma);
            gamma_prev = gamma->copy();

            // continue with fixed schedule
            bool equal;
            do
            {
                for (uint k = 0; k < sch_length; k++)
                {
                    G->fire(gamma, staticSchedule[k]);
                }
                lambda = this->normalize(gamma);
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
         * normalize()
         */
        MPDelay Exploration::normalize(State *x)
        {
            return x->normalize();
        }

        /**
         * timeProgress()
         */
        MPDelay Exploration::timeProgress(State *x)
        {
            // return first entry
            return x->firstToken()->productionTime;
        }

        /**
         * Port()
         */
        Port::Port(unsigned int _rate, Channel *_channel)
        {
            this->rate = _rate;
            this->channel = _channel;
        }

        /**
         * Actor()
         */
        Actor::Actor()
        {
            this->executionTime = 0.0;
            this->inputPorts = new PortList();
            this->outputPorts = new PortList();
        }

        /**
         * ~Actor()
         */
        Actor::~Actor()
        {
            delete this->inputPorts;
            delete this->outputPorts;
        }

        /**
         * ActorCount()
         */
        ActorCount::ActorCount(GraphDecoration::Graph *G)
        {
            size = G->actors->size();
            table = new unsigned int[size];
        }

        /**
         * ActorCount()
         */
        ActorCount::ActorCount(size_t sz)
        {
            size = sz;
            table = new unsigned int[size];
        }


        /**
         * ~ActorCount()
         */
        ActorCount::~ActorCount()
        {
            delete [] table;
        }

        /**
         * isEmpty()
         */
        bool ActorCount::isEmpty()
        {
            for (unsigned int i = 0; i < this->size; i++)
            {
                if (table[i] > 0) return false;
            }
            return true;
        }

        /**
         * decreaseValue()
         */
        void ActorCount::decreaseValue(Actor *a)
        {
            table[a->index]--;
        }

        /**
         * getValue()
         */
        unsigned int ActorCount::getValue(Actor *a)
        {
            return table[a->index];
        }

        /**
         * setValue()
         */
        void ActorCount::setValue(Actor *a, unsigned int v)
        {
            table[a->index] = v;
        }

        /**
         * setValue()
         */
        void ActorCount::setValue(unsigned int i, unsigned int v)
        {
            table[i] = v;
        }

        /**
         * copy()
         */
        ActorCount *ActorCount::copy()
        {
            ActorCount *ac = new ActorCount(this->size);
            for (uint i = 0; i < this->size; i++)
            {
                ac->setValue(i, this->table[i]);
            }
            return ac;
        }

        /**
         * sum()
         */
        unsigned int ActorCount::sum()
        {
            unsigned int s = 0;
            for (uint i = 0; i < this->size; i++)
            {
                s += this->table[i];
            }
            return s;
        }

        /**
         * init()
         */
        void GraphDecoration::Graph::init()
        {
            this->actors = new ActorList();
            this->channels = new ChannelList();
        }

        /**
         * Graph()
         */
        GraphDecoration::Graph::Graph()
        {
            init();
        }

        /**
         * ~Graph()
         */
        GraphDecoration::Graph::~Graph()
        {
            delete this->actors;
            delete this->channels;
        }

        /**
         * Graph()
         */
        GraphDecoration::Graph::Graph(TimedSDFgraph *g, bool weakConsistent, RepetitionVector *q)
        {
            init();
            Channel **channelList = new Channel*[g->nrChannels()];

            RepetitionVector E;
            if (!weakConsistent)
            {
                E = computeRepetitionVector(g);
            }
            else
            {
                E = *q;
            }

            // copy channels
            SDFchannelsIter j;
            int stateIndex = 0;
            int historyIndex = 0;
            int k;
            for (j = g->channelsBegin(), k = 0; j != g->channelsEnd(); j++, k++)
            {
                TimedSDFchannel *t = dynamic_cast<TimedSDFchannel *>(*j);
                Channel *c = new Channel();
                c->nrInitialTokens = t->getInitialTokens();
                c->index = k;
                SDFactor *a = t->getSrcActor();
                SDFactorsCIter i;
                int m = 0;
                for (i = g->actorsBegin(); i != g->actorsEnd(); i++, m++)
                {
                    SDFactor *b = *i;
                    if (a == b)
                    {
                        c->synchStorage = t->getInitialTokens() + E.at(m)
                                          * t->getSrcPort()->getRate();
                        c->repetitionVectorEntry = E.at(m) * t->getSrcPort()->getRate();
                        break;
                    }
                    assert(i != g->actorsEnd());
                }
                c->stateStartIndex = stateIndex;
                stateIndex += c->nrInitialTokens;
                c->historyStartIndex = historyIndex;
                if (c->nrInitialTokens > c->repetitionVectorEntry)
                    historyIndex += c->nrInitialTokens - c->repetitionVectorEntry;

                this->channels->push_back(c);
                channelList[k] = c;
            }

            this->stateSize = stateIndex;
            this->historySize = historyIndex;

            //copy actors
            SDFactorsIter i;
            for (i = g->actorsBegin(), k = 0; i != g->actorsEnd(); i++, k++)
            {
                TimedSDFactor *t = dynamic_cast<TimedSDFactor *>(*i);
                Actor *a = new Actor();
                a->executionTime = t->getExecutionTime();

                SDFportsIter pi;
                for (pi = t->portsBegin(); pi != t->portsEnd(); pi++)
                {
                    SDFport *p = *pi;

                    int m = 0;
                    SDFchannelsIter ci;
                    for (ci = g->channelsBegin(); ci != g->channelsEnd(); ci++, m++)
                    {
                        SDFchannel *c = *ci;
                        if (c == p->getChannel()) break;
                    }
                    assert(ci != g->channelsEnd());
                    Port *np = new Port(p->getRate(), channelList[m]);

                    if (p->getType() == SDFport::In)
                    {
                        a->inputPorts->push_back(np);
                    }
                    else
                    {
                        a->outputPorts->push_back(np);
                    }
                }
                a->index = k;
                this->actors->push_back(a);
            }
            delete [] channelList;

            RepetitionVector rv;
            if (!weakConsistent)
            {
                rv = computeRepetitionVector(g);
            }
            else
            {
                rv = *q;
            }
            this->repVec = new ActorCount(this);

            RepetitionVector::iterator v;
            int m = 0;
            for (m = 0, v = rv.begin(); v != rv.end(); v++, m++)
            {
                int c = *v;
                this->repVec->setValue(m, c);
            }
        }

        /**
         * fire()
         */
        inline
        void GraphDecoration::Graph::fire(State *x, Actor *a)
        {
            MPTime time, ftime;
            PortList::iterator i;
            PortList *pl;
            for (i = (pl = a->inputPorts)->begin(); i != pl->end(); i++)
            {
                Port *p = *i;
                time = x->consume(p);
                if (i != pl->begin()) ftime = MAX(time, ftime);
                else ftime = time;
            }
            time = ftime + a->executionTime;
            for (i = a->outputPorts->begin(); i != a->outputPorts->end(); i++)
            {
                Port *p = *i;
                x->produce(p, time);
            }
        }

        // MG removed, obviously not correct
        ///**
        // * fireSchedule()
        // */
        //CString GraphDecoration::Graph::fireSchedule(State* x, Actor* a,
        //        ActorCount *remainingVector, ostream& sout)
        //{
        //    int idx;
        //    PortList::iterator i;
        //
        //    for (i=a->inputPorts->begin(); i!=a->inputPorts->end(); i++)
        //    {
        //        Port *p = *i;
        //        for (uint k = 0; k<p->rate; k++)
        //        {
        //            idx = x->consume(p,1);
        //            sout << "CONSUME";
        //            if ((i==a->inputPorts->begin()) && k==0) sout << "FIRST";
        //            sout << "(" << idx << ");" << endl;
        //        }
        //    }
        //    sout << "FIRE(" << a->executionTime << ");" << endl;
        //    for (i=a->outputPorts->begin(); i!=a->outputPorts->end(); i++)
        //    {
        //        Port *p = *i;
        //
        //        //tokens remaining to be produced: remainingVector*rate
        //        uint remProd = remainingVector->getValue(a) * p->rate;
        //
        //        for (uint k = 0; k<p->rate; k++)
        //        {
        //            if (remProd<=p->channel->nrInitialTokens)
        //            {
        //                idx = p->channel->stateStartIndex + p->channel->nrInitialTokens
        //                      - remProd;
        //            }
        //            else
        //            {
        //                idx = this->newTokenIndex ++;
        //            }
        //            sout << "PRODUCE(" << idx << ");" << endl;
        //            remProd--;
        //        }
        //        x->produce(p, idx);
        //    }
        //    return "";
        //}

        /**
         * fire()
         */
        void GraphDecoration::Graph::fire(State *x, Actor *a, double precision)
        {
            MPTime time, ftime = 0;
            PortList::iterator i;

            for (i = a->inputPorts->begin(); i != a->inputPorts->end(); i++)
            {
                Port *p = *i;
                time = x->consume(p);
                if (i != a->inputPorts->begin()) ftime = MAX(time, ftime);
                else ftime = time;
            }
            time = ftime + a->executionTime;
            time = precision * (double)((long)(time / precision));
            for (i = a->outputPorts->begin(); i != a->outputPorts->end(); i++)
            {
                Port *p = *i;
                x->produce(p, time);
            }
        }

        /**
         * fire_reverse()
         */
        void GraphDecoration::Graph::fire_reverse(State *x, Actor *a)
        {
            MPTime time, ftime = 0;
            PortList::iterator i;

            for (i = a->outputPorts->begin(); i != a->outputPorts->end(); i++)
            {
                Port *p = *i;
                time = x->consume_back(p);
                if (i != a->outputPorts->begin()) ftime = MIN(time, ftime);
                else ftime = time;
            }
            time = ftime - a->executionTime;
            for (i = a->inputPorts->begin(); i != a->inputPorts->end(); i++)
            {
                Port *p = *i;
                x->produce_front(p, time);
            }
        }

        /**
         * fireSymbolic()
         */
        void GraphDecoration::Graph::fireSymbolic(SymbolicState *x, Actor *a)
        {
            SymbolicToken *time, *ftime = NULL, *nftime;
            PortList::iterator i;

            for (i = a->inputPorts->begin(); i != a->inputPorts->end(); i++)
            {
                Port *p = *i;
                time = x->consume(p);

                if (i != a->inputPorts->begin())
                {
                    nftime = time->maxWith(ftime);
                }
                else
                {
                    nftime = time->copy();
                }
                if (ftime) delete ftime;
                delete time;
                ftime = nftime;
            }
            time = ftime->add(a->executionTime);
            delete ftime;
            for (i = a->outputPorts->begin(); i != a->outputPorts->end(); i++)
            {
                Port *p = *i;
                x->produce(p, time);
            }
        }

        /**
         * repetitionVector()
         */
        ActorCount *GraphDecoration::Graph::repetitionVector()
        {
            return this->repVec->copy();
        }

        /**
         * mpStateSpaceThroughputAnalysis()
         */
        MPTime mpStateSpaceThroughputAnalysis(TimedSDFgraph *g, ostream &sout)
        {
            out = &sout;
            GraphDecoration::Graph *G = new GraphDecoration::Graph(g);
            Exploration mpe;
            mpe.G = G;
            MPThroughput th = mpe.explore();
            delete G;
            return th;
        }

        /**
         * mpStateSpaceThroughputAnalysis_backward()
         */
        MPTime mpStateSpaceThroughputAnalysis_backward(TimedSDFgraph *g, ostream &sout)
        {
            out = &sout;
            GraphDecoration::Graph *G = new GraphDecoration::Graph(g);
            Exploration mpe;
            mpe.G = G;
            MPThroughput th = mpe.explore_backward();
            delete G;
            return th;
        }

        /**
         * mpStateSpaceDelayAnalysis()
         */
        MPTime mpStateSpaceDelayAnalysis(TimedSDFgraph *g, MaxPlus::Vector *gamma0,
                                         MaxPlus::Vector *gamma_s, MPTime period, ostream &sout)
        {
            out = &sout;
            GraphDecoration::Graph *G = new GraphDecoration::Graph(g);
            Exploration mpe;
            mpe.G = G;
            MPTime d = mpe.exploreDelayForSchedule(gamma0, gamma_s, period);
            delete G;
            return d;
        }

        /**
         * mpStateSpaceEigenAnalysis()
         */
        void mpStateSpaceEigenAnalysis(TimedSDFgraph *g, MaxPlus::Vector **eigenvector,
                                       MPTime *eigenvalue, ostream &sout)
        {
            out = &sout;
            GraphDecoration::Graph *G = new GraphDecoration::Graph(g);
            Exploration mpe;
            mpe.G = G;
            mpe.exploreEigen(eigenvector, eigenvalue);
            delete G;
        }

    } // end namespace

} // end namespace

