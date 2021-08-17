/*
*  TU Eindhoven
*  Eindhoven, The Netherlands
*
*  Name            :   mpstorage.h
*
*  Author          :   Marc Geilen (m.c.w.geilen@tue.nl)
*
*  Date            :   May 13, 2008
*
*  Function        :   Max-Plus based state representations.
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

#ifndef MPSTORAGE_H_INCLUDED
#define MPSTORAGE_H_INCLUDED

#include "mpexplore.h"

// todo: change to unordered_set?
#ifdef _MSC_VER
#include <hash_set>
#else
#undef __DEPRECATED
#include <ext/hash_set>
using namespace __gnu_cxx;
#endif

namespace SDF
{

    namespace MaxPlusAnalysis
    {

        using namespace GraphDecoration;

        /**
         * class Token
         * Token represents a MaxPlus timestamped token in an SDF graph.
         */

        class Token
        {
            public:
                // Constructor
                Token()
                {
                    productionTime = 0.0;
                }
                Token(MPTime t)
                {
                    productionTime = t;
                }

                // Copy
                Token *copy();

            public:
                MPTime productionTime;
        };

        // Switch between dynamic and static allocation of token FIFOs,
        // but static performs best
#define TokenFIFO TokenFIFO_Static
        //#define TokenFIFO TokenFIFO_Dynamic


        /**
         * class TokenFIFO_Dynamic
         * A dynamically managed token fifo based on std:list
         */
        class TokenFIFO_Dynamic: public std::list<Token *>
        {
            public:
                MPTime removeFirst();
                void append(MPTime t);
                TokenFIFO_Dynamic *copy();
                size_t hashValue() const;
                bool compare(TokenFIFO_Dynamic *l) const;
                Token *first();
                void decreaseAll(MPDelay amount);
                void streamOn(std::ostream &s);
        };

        /**
         * class TokenFIFO_Static
         * A statically managed token fifo
         */
        class TokenFIFO_Static
        {
            public:
                TokenFIFO_Static(int sz);
                MPTime removeFirst();

                /*
                 * Read N timestamped tokens from the FIFO, return the max of all read
                 * timestamps. To save time it is not checked whether the token contains
                 * N tokens, only use this method if you are sure there are.
                 */
                inline MPTime removeFirstN(unsigned int n)
                {
                    MPTime time;
                    n--;
                    Token *r = &(tokens[begin]);
                    ++begin %= asize;
                    time = r->productionTime;
                    while (n-- > 0)
                    {
                        Token *r = &(tokens[begin]);
                        ++begin %= asize;
                        time = max(r->productionTime, time);
                    }
                    return time;
                }

                MPTime removeLast();
                void append(MPTime t);

                /* Append n tokens to the FIFO, all timestamped with t. To save time it is
                 * not checked whether there is space for the tokens, only use this method
                 * if you are sure there is.
                 */
                inline void append(MPTime t, unsigned int n)
                {
                    while (n-- > 0)
                    {
                        tokens[end].productionTime = t;
                        ++end %= asize;
                    }
                }

                void append_front(MPTime t);
                TokenFIFO_Static *copy();
                size_t hashValue() const;
                bool compare(TokenFIFO_Static *l) const;
                Token *first();
                MPTime getMax();
                void decreaseAll(MPDelay amount);
                void smooth(TokenFIFO_Static *l);
                void streamOn(std::ostream &s) const;
                unsigned int size() const;
                bool equals(const TokenFIFO_Static *l) const;
                TokenFIFO_Static *minus(TokenFIFO_Static *l) const;
                void add(double factor, TokenFIFO_Static *l);
                void maxWith(double offset, TokenFIFO_Static *l);
                void addToStdVector(vector<double>* v) const;
                unsigned int addToVector(MaxPlus::Vector *v, unsigned int k) const;

            private:
                Token *tokens;
                unsigned int asize; // size of the array
                unsigned int begin;
                unsigned int end;
        };


        /**
         * class State
         * State represents a distribution of timestamped tokens over the channels of
         * the SDFG
         */
        class State
        {
            public:
                State(Graph *G);
                State(size_t sz);
                ~State();
                State *copy();
                static State *zeroState(Graph *G);
                static State *fromVector(Graph *G, MaxPlus::Vector *v);
                static State *zeroScheduleState(Graph *G);
                inline  MPTime consume(Port *p)
                {
                    Channel *c = p->channel;
                    TokenFIFO *l = table[c->index];
                    return l->removeFirstN(p->rate);
                }
                MPTime consume(Port *p, int n);
                MPTime consume_back(Port *p);
                inline
                void produce(Port *p, MPTime t)
                {
                    Channel *c = p->channel;
                    TokenFIFO *l = table[c->index];
                    l->append(t, p->rate);
                }
                void produce_front(Port *p, MPTime t);
                bool actorEnabled(Actor *a);
                bool actorReverseEnabled(Actor *a);
                size_t hashValue() const;
                bool compare(const State *s) const;
                bool equals(const State *s) const;
                State *minus(State *v) const;
                void add(double factor, State *v);
                void maxWith(double offset, State *v);
                Token *firstToken();
                MPDelay normalize();
                MPTime norm();
                void smooth(State *y);
                void streamOn(std::ostream &s);
                vector<double>* asStdVector(void);
                MaxPlus::Vector *asVector(void);
                MPTime timestamp;
                unsigned int count;

            private:
                size_t size;
                TokenFIFO **table;
                void initSize(size_t sz);
        };

        /**
         * class TokenList
         */
        class TokenList : public vector<Token *>
        {
            public:
                // Destructor
                ~TokenList()
                {
                    for (vector<Token *>::iterator i = this->begin();
                         i != this->end(); i++)
                    {
                        delete *i;
                    }
                }
        };

        /**
         * class SymbolicToken
         */
        class SymbolicToken : public TokenList
        {
            public:
                static SymbolicToken *zeroToken(unsigned int sz);
                static SymbolicToken *minusInfinityToken(unsigned int sz);
                static SymbolicToken *initialToken(unsigned int sz, unsigned int n);
                SymbolicToken *copy(void);
                SymbolicToken *maxWith(SymbolicToken *t);
                SymbolicToken *add(MPTime a);
                void streamOn(std::ostream &s);
        };

        /**
         * class SymbolicTokenFIFO
         */
        class SymbolicTokenFIFO
        {
            public:
                SymbolicTokenFIFO(int fsz, int tsz);
                inline void append(SymbolicToken *t, int n)
                {
                    // assume there is space, don't check
                    while (n-- > 0)
                    {
                        tokens[end] = t;
                        ++end %= asize;
                    }
                }
                inline SymbolicToken *removeFirstN(int n)
                {
                    // check if n is 0
                    if (n == 0)
                    {
                        return SymbolicToken::minusInfinityToken(this->tokenSize);
                    }

                    // assume there is a token to be read, don't check
                    SymbolicToken *time, *t, *oldtime;

                    n--;
                    time = tokens[begin]->copy();
                    t = tokens[begin];
                    ++begin %= asize;

                    while (n-- > 0)
                    {
                        t = tokens[begin];
                        ++begin %= asize;
                        oldtime = time;
                        time = t->maxWith(time);
                        delete oldtime;
                    }
                    return time;
                }
                inline
                SymbolicToken *peek(unsigned int n)
                {
                    return tokens[(begin + n) % asize];
                }
                int size();
                void streamOn(std::ostream &s);
            private:
                SymbolicToken **tokens;
                int asize; // size of the array
                int begin;
                int end;
                int tokenSize; // size of a single symbolic token in the fifio
                //positions starting from begin up to but not including end are filled
        };

        /**
         * class SymbolicState
         */
        class SymbolicState
        {
            public:
                SymbolicState(Graph *G);
                SymbolicState(size_t sz, size_t stsz);
                ~SymbolicState();
                static SymbolicState *zeroState(Graph *G);
                inline
                SymbolicToken *consume(Port *p)
                {
                    Channel *c = p->channel;
                    SymbolicTokenFIFO *f = table[c->index];
                    return f->removeFirstN(p->rate);
                }
                inline
                void produce(Port *p, SymbolicToken *t)
                {
                    Channel *c = p->channel;
                    SymbolicTokenFIFO *f = table[c->index];
                    f->append(t, p->rate);
                }
                bool actorEnabled(Actor *a);
                void streamOn(std::ostream &s);
                inline
                size_t getSize()
                {
                    return size;
                }
                inline
                size_t getSymbolicTokenSize()
                {
                    return symbolicTokenSize;
                }
                inline
                SymbolicTokenFIFO *get(unsigned int i)
                {
                    return table[i];
                }
                // to support iterating over all (initial tokens)
                unsigned int numberOfTokens();
                SymbolicToken *firstToken(void);
                SymbolicToken *nextToken(void);
            private:
                size_t size;
                size_t symbolicTokenSize;
                SymbolicTokenFIFO **table;
                void initSize(size_t sz);

                // state to support iterating over all tokens
                // make nicer implementation later...
                unsigned int currentChannelIndex;
                unsigned int currentTokenIndex;
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
                    return s1->compare(s2);
                }
        };

#else


        /**
         * class StateHasher
         */
        class StateHasher
        {
            public:
                size_t operator()(const State *s) const
                {
                    size_t h = s->hashValue();
                    return h;
                }
        };

        /**
         * class StateComparator
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
         */
#ifdef _MSC_VER
        class StoredStates : hash_set<State *, StateHasherAndComparator>
#else
        class StoredStates : hash_set<State *, StateHasher, StateComparator>
#endif
        {
            public:
                void store(State *x)
                {
                    this->insert(x);
                }
                bool includes(State *x, State **y)
                {
                    StoredStates::const_iterator i;
                    i = this->find(x);
                    if (i != this->end())
                    {
                        (*y) = (*i);
                        return true;
                    }
                    return false;
                }
                bool includes(State *x)
                {
                    return this->find(x) != this->end();
                }
        };

    } // end namespace


}//namespace SDF
#endif
