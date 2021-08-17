/*
*  TU Eindhoven
*  Eindhoven, The Netherlands
*
*  Name            :   mpstorage.cc
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

#include "mpstorage.h"
namespace SDF
{


    namespace MaxPlusAnalysis
    {

        using namespace GraphDecoration;

        State::State(Graph *G)
        {
            this->initSize(G->channels->size());
        }

        State::State(size_t sz)
        {
            this->initSize(sz);
        }

        void State::initSize(size_t sz)
        {
            size = sz;
            table = new TokenFIFO*[size];
            for (uint i = 0; i < sz; i++)
                table[i] = new TokenFIFO(sz);
        }

        State::~State()
        {
            for (unsigned int i = 0; i < this->size; i++)
            {
                delete table[i];
            }
            delete [] table;
        }

        MPTime State::consume(Port *p, int n)
        {
            MPTime time = 0;
            Channel *c = p->channel;
            TokenFIFO *l = table[c->index];
            for (int i = 0; i < n; i++)
            {
                MPTime t = l->removeFirst();
                if (i == 0) time = t;
                else time = MAX(t, time);
            }
            return time;
        }

        MPTime State::consume_back(Port *p)
        {
            MPTime time = 0;
            Channel *c = p->channel;
            TokenFIFO *l = table[c->index];
            for (uint i = 0; i < p->rate; i++)
            {
                MPTime t;
                t = l->removeLast();
                if (i == 0) time = t;
                else time = MIN(t, time);
            }
            return time;
        }

        void State::produce_front(Port *p, MPTime t)
        {
            Channel *c = p->channel;
            TokenFIFO *l = table[c->index];
            for (uint i = 0; i < p->rate; i++)
            {
                l->append_front(t);
            }
        }

        bool State::actorEnabled(Actor *a)
        {
            PortList::iterator i;
            for (i = a->inputPorts->begin(); i != a->inputPorts->end(); i++)
            {
                Port *p = *i;
                if (this->table[p->channel->index]->size() < p->rate) return false;
            }
            return true;
        }

        bool State::actorReverseEnabled(Actor *a)
        {
            PortList::iterator i;
            for (i = a->outputPorts->begin(); i != a->outputPorts->end(); i++)
            {
                Port *p = *i;
                if (this->table[p->channel->index]->size() < p->rate) return false;
            }
            return true;
        }

        State *State::zeroState(GraphDecoration::Graph *G)
        {
            State *s = new State(G);
            ChannelList::iterator i;

            int k;
            for (k = 0, i = G->channels->begin(); i != G->channels->end(); i++, k++)
            {
                Channel *c = *i;
                s->table[k] = new TokenFIFO(c->synchStorage);
                for (uint j = 0; j < c->nrInitialTokens; j++)
                {
                    s->table[k]->append(0.0);
                }
            }
            s->count = 0;
            s->timestamp = 0.0;

            return s;
        }

        State *State::fromVector(GraphDecoration::Graph *G, MaxPlus::Vector *v)
        {
            State *s = new State(G);
            ChannelList::iterator i;

            unsigned int k;
            unsigned int m = 0;
            for (k = 0, i = G->channels->begin(); i != G->channels->end(); i++, k++)
            {
                Channel *c = *i;
                s->table[k] = new TokenFIFO(c->synchStorage);
                for (uint j = 0; j < c->nrInitialTokens; j++)
                {
                    s->table[k]->append(v->get(m++));
                }
            }
            s->count = 0;
            s->timestamp = 0.0;

            return s;
        }

        State *State::zeroScheduleState(GraphDecoration::Graph *G)
        {
            State *s = new State(G);
            int posCount = 0;
            ChannelList::iterator i;
            ActorCount *E = G->repetitionVector();
            int k;
            for (k = 0, i = G->channels->begin(); i != G->channels->end(); i++, k++)
            {
                Channel *c = *i;
                s->table[k] = new TokenFIFO(c->synchStorage);
                for (uint j = 0; j < c->nrInitialTokens; j++)
                {
                    s->table[k]->append(posCount++);
                }
            }
            delete E;
            return s;
        }

        State *State::copy()
        {
            State *s = new State(this->size);
            unsigned int i;
            for (i = 0; i < this->size; i++)
            {
                TokenFIFO *l = table[i];
                s->table[i] = l->copy();
            }
            s->timestamp = this->timestamp;
            s->count = this->count;
            return s;
        }

        void State::smooth(State *y)
        {
            unsigned int i;
            for (i = 0; i < this->size; i++)
            {
                table[i]->smooth(y->table[i]);
            }
        }

        size_t State::hashValue() const
        {
            size_t h = 0;
            for (uint i = 0; i < this->size; i++)
            {
                h = 31 * h + this->table[i]->hashValue();
            }
            return h;
        }

        bool State::compare(const State *s) const
        {
            //compare lexicographically
            if (this->size != s->size)
            {
                throw new CException("storing states of different sizes");
            }
            for (uint i = 0; i < this->size; i++)
            {
                if (this->table[i]->compare(s->table[i])) return true;
                if (s->table[i]->compare(this->table[i])) return false;
            }
            return false;
        }



        bool State::equals(const State *s) const
        {
            if (this->size != s->size)
            {
                return false;
            }
            for (uint i = 0; i < this->size; i++)
            {
                if (!this->table[i]->equals(s->table[i])) return false;
            }
            return true;
        }

        State *State::minus(State *v) const
        {
            State *s = new State(this->size);
            unsigned int i;
            for (i = 0; i < this->size; i++)
            {
                TokenFIFO *l = table[i];
                s->table[i] = l->minus(v->table[i]);
            }
            return s;
        }

        void State::add(double factor, State *v)
        {
            unsigned int i;
            for (i = 0; i < this->size; i++)
            {
                TokenFIFO *l = table[i];
                l->add(factor, v->table[i]);
            }
        }

        void State::maxWith(double offset, State *v)
        {
            unsigned int i;
            for (i = 0; i < this->size; i++)
            {
                TokenFIFO *l = table[i];
                l->maxWith(offset, v->table[i]);
            }
        }

        SymbolicState::SymbolicState(GraphDecoration::Graph *G)
        {
            this->initSize(G->channels->size());
            this->symbolicTokenSize = G->stateSize;
        }

        SymbolicState::SymbolicState(size_t sz, size_t stsz)
        {
            this->initSize(sz);
            this->symbolicTokenSize = stsz;
        }

        void SymbolicState::initSize(size_t sz)
        {
            size = sz;
            table = new SymbolicTokenFIFO*[size];
        }

        SymbolicState::~SymbolicState()
        {
            for (unsigned int i = 0; i < this->size; i++)
            {
                delete table[i];
            }
            delete table;
        }

        SymbolicState *SymbolicState::zeroState(GraphDecoration::Graph *G)
        {
            SymbolicState *s = new SymbolicState(G);
            ChannelList::iterator i;
            unsigned int k, n = 0;

            for (k = 0, i = G->channels->begin(); i != G->channels->end(); i++, k++)
            {
                Channel *c = *i;
                s->table[k] = new SymbolicTokenFIFO(c->synchStorage, G->stateSize);
                for (uint j = 0; j < c->nrInitialTokens; j++)
                {
                    s->table[k]->append(SymbolicToken::initialToken(G->stateSize, n++), 1);
                }
            }
            return s;
        }

        bool SymbolicState::actorEnabled(Actor *a)
        {
            PortList::iterator i;
            for (i = a->inputPorts->begin(); i != a->inputPorts->end(); i++)
            {
                Port *p = *i;
                if (this->table[p->channel->index]->size() < (int) p->rate) return false;
            }
            return true;
        }

        void SymbolicState::streamOn(std::ostream &s)
        {
            unsigned int i;
            s << "SymbolicState<";
            for (i = 0; i < this->size; i++)
            {
                this->table[i]->streamOn(s);
                s << "; ";
            }
            s << ">";
        }

        SymbolicToken *SymbolicState::firstToken(void)
        {
            this->currentChannelIndex = 0;
            this->currentTokenIndex = 0;
            return this->nextToken();
        }

        SymbolicToken *SymbolicState::nextToken(void)
        {
            SymbolicToken *result = NULL;
            SymbolicTokenFIFO *f;
            bool found = false;
            do
            {
                f = this->table[this->currentChannelIndex];
                if (this->currentTokenIndex < (uint)f->size())
                {
                    result = f->peek(this->currentTokenIndex);
                    this->currentTokenIndex++;
                    found = true;
                }
                else
                {
                    this->currentChannelIndex++;
                    this->currentTokenIndex = 0;
                }
            }
            while (!found && this->currentChannelIndex < this->size);
            return result;
        }

        unsigned int SymbolicState::numberOfTokens()
        {
            unsigned int total = 0;
            for (unsigned int i = 0; i < this->size; i++)
            {
                total += this->table[i]->size();
            }
            return total;
        }

        Token *State::firstToken()
        {
            for (uint i = 0; i < this->size; i++)
            {
                TokenFIFO *l = this->table[i];
                if (l->size() > 0) return l->first();
            }
            return NULL;
        }

        MPDelay State::normalize()
        {
            MPDelay amount = this->norm();
            for (uint i = 0; i < this->size; i++)
            {
                this->table[i]->decreaseAll(amount);
            }
            this->timestamp += amount;
            return amount;
        }

        MPTime State::norm()
        {
            MPTime max = 0;
            for (uint i = 0; i < this->size; i++)
            {
                if (i == 0)
                    max = this->table[i]->getMax();
                else
                    max = MAX(max, this->table[i]->getMax());
            }
            return max;
        }

        void State::streamOn(std::ostream &s)
        {
            for (uint i = 0; i < this->size; i++)
            {
                //s << i << ": ";
                TokenFIFO *l = this->table[i];
                l->streamOn(s);
            }
        }

        vector<double>* State::asStdVector(void)
        {
            vector<double> *v = new vector<double>;
            for (uint i = 0; i < this->size; i++)
            {
                TokenFIFO *l = this->table[i];
                l->addToStdVector(v);
            }
            return v;
        }

        MaxPlus::Vector *State::asVector(void)
        {
            unsigned int k = 0;
            MaxPlus::Vector *v = new MaxPlus::Vector();
            for (uint i = 0; i < this->size; i++)
            {
                TokenFIFO *l = this->table[i];
                k = l->addToVector(v, k);
            }
            return v;
        }

        void TokenFIFO_Dynamic::streamOn(std::ostream &s)
        {
            TokenFIFO_Dynamic::const_iterator j;
            for (j = this->begin(); j != this->end(); j++)
            {
                Token *t = *j;
                s << t->productionTime << " ";
            }
        }

        void TokenFIFO_Dynamic::decreaseAll(MPDelay amount)
        {
            TokenFIFO_Dynamic::const_iterator j;
            for (j = this->begin(); j != this->end(); j++)
            {
                Token *t = *j;
                t->productionTime = t->productionTime - amount;
            }
        }

        Token *TokenFIFO_Dynamic::first()
        {
            return *(this->begin());
        }

        MPTime TokenFIFO_Dynamic::removeFirst()
        {
            Token *t = this->front();
            this->pop_front();
            MPTime tm = t->productionTime;
            delete t;
            return tm;
        }

        void TokenFIFO_Dynamic::append(MPTime t)
        {
            this->push_back(new Token(t));
        }

        TokenFIFO_Dynamic *TokenFIFO_Dynamic::copy()
        {
            TokenFIFO_Dynamic *l = new TokenFIFO_Dynamic();
            TokenFIFO_Dynamic::iterator i;
            for (i = this->begin(); i != this->end(); i++)
            {
                Token *t = *i;
                l->append(t->productionTime);
            }
            return l;
        }

        size_t TokenFIFO_Dynamic::hashValue() const
        {
            size_t h = 0;
            TokenFIFO_Dynamic::const_iterator i;
            for (i = this->begin(); i != this->end(); i++)
            {
                Token *t = *i;
                h = 23 * h + (size_t)(t->productionTime);
            }
            return h;
        }

        bool TokenFIFO_Dynamic::compare(TokenFIFO_Dynamic *l) const
        {
            if (this->size() < l->size()) return true;
            if (l->size() < this->size()) return false;
            TokenFIFO_Dynamic::const_iterator i, j;
            for (i = this->begin(), j = l->begin(); i != this->end(); i++, j++)
            {
                Token *ti = *i;
                Token *tj = *j;
                if (ti->productionTime < tj->productionTime) return true;
                if (tj->productionTime < ti->productionTime) return false;
            }
            return false;
        }

        TokenFIFO_Static::TokenFIFO_Static(int sz)
        {
            asize = sz + 1; // needs to be able to hold sz tokens
            tokens = new Token[asize];
            begin = 0;
            end = 0;
        }

        MPTime TokenFIFO_Static::removeFirst()
        {
            // assume there is a token to be read, don't check
            Token *r = &(tokens[begin]);
            begin = (begin + 1) % asize;
            return r->productionTime;
        }

        MPTime TokenFIFO_Static::removeLast()
        {
            // assume there is a token to be read, don't check
            end = (end == 0) ? asize - 1 : end - 1;
            Token *r = &(tokens[end]);
            return r->productionTime;
        }

        void TokenFIFO_Static::append(MPTime t)
        {
            // assume there is space, don't check
            tokens[end].productionTime = t;
            ++end %= asize;
        }

        void TokenFIFO_Static::append_front(MPTime t)
        {
            // assume there is space, don't check
            begin = (begin == 0) ? asize - 1 : begin - 1;
            tokens[begin].productionTime = t;
        }

        TokenFIFO_Static *TokenFIFO_Static::copy()
        {
            TokenFIFO_Static *l = new TokenFIFO_Static(asize - 1);
            l->asize = this->asize;
            l->begin = this->begin;
            l->end = this->end;
            l->tokens = (Token *)malloc(asize * sizeof(Token));
            memcpy(l->tokens, tokens, asize * sizeof(Token));
            return l;
        }

        size_t TokenFIFO_Static::hashValue() const
        {
            size_t h = 0;
            uint i = this->begin;
            while (i != this->end)
            {
                h = 23 * h + size_t(this->tokens[i].productionTime);
                i = (i + 1) % this->asize;
            }
            return h;
        }

        bool TokenFIFO_Static::compare(TokenFIFO_Static *l) const
        {
            if (this->asize < l->asize) return true;
            if (l->asize < this->asize) return false;
            uint i = this->begin, j = l->begin;
            while (i != this->end)
            {
                if (this->tokens[i].productionTime < l->tokens[j].productionTime)
                    return true;
                if (l->tokens[j].productionTime < this->tokens[i].productionTime)
                    return false;
                i = (i + 1) % this->asize;
                j = (j + 1) % this->asize;
            }
            return false;
        }

        bool TokenFIFO_Static::equals(const TokenFIFO_Static *l) const
        {
            if (this->asize != l->asize) return false;
            uint i = this->begin, j = l->begin;
            while (i != this->end)
            {
                if (this->tokens[i].productionTime != l->tokens[j].productionTime)
                    return false;
                ++i %= this->asize;
                ++j %= this->asize;
            }
            return true;
        }

        TokenFIFO_Static *TokenFIFO_Static::minus(TokenFIFO_Static *m) const
        {
            TokenFIFO_Static *l = new TokenFIFO_Static(asize - 1);
            l->asize = this->asize;
            l->begin = this->begin;
            l->end = this->end;
            l->tokens = (Token *)malloc(asize * sizeof(Token));
            memcpy(l->tokens, tokens, asize * sizeof(Token));
            uint i = this->begin, j = m->begin;
            while (i != this->end)
            {
                l->tokens[i].productionTime =
                    this->tokens[i].productionTime - m->tokens[j].productionTime;
                i = (i + 1) % this->asize;
                j = (j + 1) % this->asize;
            }
            return l;
        }

        void TokenFIFO_Static::add(double factor, TokenFIFO_Static *m)
        {
            uint i = this->begin, j = m->begin;
            while (i != this->end)
            {
                this->tokens[i].productionTime +=
                    factor * m->tokens[j].productionTime;
                i = (i + 1) % this->asize;
                j = (j + 1) % this->asize;
            }
        }

        void TokenFIFO_Static::maxWith(double offset, TokenFIFO_Static *l)
        {
            uint i = this->begin, j = l->begin;
            while (i != this->end)
            {
                this->tokens[i].productionTime =
                    MAX(this->tokens[i].productionTime,
                        offset + l->tokens[j].productionTime);
                i = (i + 1) % this->asize;
                j = (j + 1) % this->asize;
            }
        }

        void TokenFIFO::addToStdVector(vector<double>* v) const
        {
            uint i = this->begin;
            while (i != this->end)
            {
                v->push_back(this->tokens[i].productionTime);
                i = (i + 1) % this->asize;
            }
        }

        unsigned int TokenFIFO::addToVector(MaxPlus::Vector *v, unsigned int k) const
        {
            uint i = this->begin;
            while (i != this->end)
            {
                v->put(k++, this->tokens[i].productionTime);
                i = (i + 1) % this->asize;
            }
            return k;
        }

        Token *TokenFIFO_Static::first()
        {
            return &(this->tokens[begin]);
        }

        void TokenFIFO_Static::decreaseAll(MPDelay amount)
        {
            uint i = this->begin;
            while (i != this->end)
            {
                this->tokens[i].productionTime -= amount;
                i = (i + 1) % asize;
            }
        }

        MPTime TokenFIFO_Static::getMax()
        {
            MPTime max = 0.0;
            uint i = this->begin;
            while (i != this->end)
            {
                if (i == this->begin)
                    max = this->tokens[i].productionTime;
                else
                    max = MAX(max, this->tokens[i].productionTime);
                i = (i + 1) % asize;
            }
            return max;
        }

        void TokenFIFO_Static::streamOn(std::ostream &s) const
        {
            uint i = this->begin;
            while (i != this->end)
            {
                s << this->tokens[i].productionTime << " ";
                i = (i + 1) % asize;
            }
        }

        unsigned int TokenFIFO_Static::size() const
        {
            int r = this->end - this->begin;
            if (r < 0) r += asize;
            return (unsigned int) r;
        }

        void TokenFIFO_Static::smooth(TokenFIFO_Static *l)
        {
            uint i = this->begin, j = l->begin;
            while (i != this->end)
            {
                this->tokens[i].productionTime = 0.5 * this->tokens[i].productionTime
                                                 + 0.5 * l->tokens[j].productionTime;
                i = (i + 1) % this->asize;
                j = (j + 1) % this->asize;
            }

        }

        SymbolicTokenFIFO::SymbolicTokenFIFO(int fsz, int tsz)
        {
            this->asize = fsz + 1; // needs to be able to hold sz tokens
            this->tokens = new SymbolicToken*[asize];
            this->begin = 0;
            this->end = 0;
            this->tokenSize = tsz;
        }

        int SymbolicTokenFIFO::size()
        {
            int r = this->end - this->begin;
            if (r < 0) r += asize;
            return r;
        }

        void SymbolicTokenFIFO::streamOn(std::ostream &s)
        {
            int i = this->begin;
            while (i != this->end)
            {
                tokens[i]->streamOn(s);
                i = (i + 1) % asize;
            }
        }

        Token *Token::copy()
        {
            return new Token(productionTime);
        }

        SymbolicToken *SymbolicToken::zeroToken(unsigned int sz)
        {
            SymbolicToken *t = new SymbolicToken();
            for (unsigned int i = 0; i < sz; i++)
            {
                t->push_back(new Token(0.0));
            }
            return t;
        }

        SymbolicToken *SymbolicToken::minusInfinityToken(unsigned int sz)
        {
            SymbolicToken *t = new SymbolicToken();
            for (unsigned int i = 0; i < sz; i++)
            {
                t->push_back(new Token(MP_MINUSINFINITY));
            }
            return t;
        }

        SymbolicToken *SymbolicToken::initialToken(unsigned int sz, unsigned int n)
        {
            SymbolicToken *t = new SymbolicToken();
            for (unsigned int i = 0; i < sz; i++)
            {
                if (i == n) t->push_back(new Token(0.0));
                else t->push_back(new Token(MP_MINUSINFINITY));
            }
            return t;
        }

        SymbolicToken *SymbolicToken::add(MPTime a)
        {
            SymbolicToken *t = new SymbolicToken();
            TokenList::iterator i;
            for (i = this->begin(); i != this->end(); i++)
            {
                t->push_back(new Token((*i)->productionTime + a));
            }
            return t;
        }

        void SymbolicToken::streamOn(std::ostream &s)
        {
            TokenList::iterator i;
            s << "SymbolicToken<";
            for (i = this->begin(); i != this->end(); i++)
            {
                if ((*i)->productionTime >= 0.0) s << (*i)->productionTime;
                else s << "-infty";
                s << ";";
            }
            s << ">";
        }

        SymbolicToken *SymbolicToken::copy()
        {
            SymbolicToken *t = new SymbolicToken();
            TokenList::iterator i;
            for (i = this->begin(); i != this->end(); i++)
            {
                t->push_back(new Token((*i)->productionTime));
            }
            return t;
        }

        SymbolicToken *SymbolicToken::maxWith(SymbolicToken *t)
        {
            SymbolicToken *nt = new SymbolicToken();
            TokenList::iterator i, j;
            for (i = this->begin(), j = t->begin(); i != this->end(); i++, j++)
            {
                nt->push_back(new Token(MAX((*i)->productionTime ,
                                            (*j)->productionTime)));
            }
            return nt;
        }

    } // end namespace


}//namespace SDF
