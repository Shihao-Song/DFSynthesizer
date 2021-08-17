/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   clookup.h
 *
 *  Author          :   Peter Poplavko (poplavko@ics.ele.tue.nl)
 *
 *  Date            :   September 26, 2003
 *
 *  Function        :   CLookupXXX = handy wrappers to access hash maps
 *                         e.g. from pointer to pointer
 *
 *  History         :
 *      15-08-09    :   Initial version.
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

#ifndef BASE_STRING_CLOOKUP_H
#define BASE_STRING_CLOOKUP_H

/*****************************************************/
/* CLookupXXX classes                                */
/*   Handy wrappers to access hash maps              */
/*****************************************************/

#ifdef _MSC_VER
#include <hash_map>
typedef stdext::hash_map<void *, void *> CLOOKUP_MAP_VOID;
typedef stdext::hash_map<void *, int> CLOOKUP_MAP_INT;
#else
#undef __DEPRECATED
#include <ext/hash_map>
using namespace __gnu_cxx;

struct hash_void
{
    size_t operator()(const void *__x) const
    {
        return reinterpret_cast<size_t>(__x);
    }
};
typedef hash_map<void *, void *, hash_void> CLOOKUP_MAP_VOID;
typedef hash_map<void *, int, hash_void>   CLOOKUP_MAP_INT;
#endif

/**
 * CLookupPtr<T>
 * Lookup from void pointer to pointer 'T*'
 */

template< class T>
class CLookupPtr
{

    public:
        friend class CIterator;

        /**
        * Iterator over CLookupPtr: visit all <key, value> pairs
        *
        */
        class CIterator
        {
            public:
                CIterator(const CLookupPtr<T>* lookup) : m_lookup(lookup), m_iterator(lookup->m_impl.begin())
                {
                }
                bool next(void*& key, T*& value)
                {
                    if (m_iterator != m_lookup->m_impl.end())
                    {
                        key = (*m_iterator).first;
                        value = static_cast<T *>((*m_iterator).second);
                        m_iterator++;
                        return true;
                    }
                    else
                    {
                        key = NULL;
                        value = NULL;
                        return false;
                    }
                }
                void reset()
                {
                    m_iterator = m_lookup->m_impl.begin();
                }

            private:
                const CLookupPtr<T>* m_lookup;
                CLOOKUP_MAP_VOID::const_iterator m_iterator;
        };

    public:
        //------------------------------------------
        //----- CLookupPtr -------------------------
        //------------------------------------------

        // TODO: void erase(key)

        CLookupPtr(bool freePointers) : m_freePointers(freePointers) {}
        ~CLookupPtr()
        {
            if (m_freePointers)
            {
                CIterator iterator(this);
                void *key;
                T *value;
                while (iterator.next(key, value))
                {
                    delete value;
                }
            }
        }

        T *get(void *key) const
        {
            CLOOKUP_MAP_VOID::const_iterator foundPointer = m_impl.find(key);
            if (foundPointer == m_impl.end())
            {
                // not found
                return NULL;
            }
            return static_cast<T *>((*foundPointer).second);
        }

        T *getOrCreate(void *key)
        {
            CLOOKUP_MAP_VOID::const_iterator foundPointer = m_impl.find(key);
            T *value;
            if (foundPointer == m_impl.end())
            {
                // not found
                value = new T;
                m_impl[key] = value;
            }
            else
            {
                value = static_cast<T *>((*foundPointer).second);
            }
            return value;
        }

        void put(void *key, T *value)
        {
            CLOOKUP_MAP_VOID::iterator foundPointer = m_impl.find(key);
            if (foundPointer == m_impl.end())
            {
                // not found
                m_impl[key] = value;
                return;
            }
            // found
            if (m_freePointers)
            {
                delete static_cast<T *>((*foundPointer).second);
            }
            (*foundPointer).second = value;
        }

        bool exists(void *key) const
        {
            CLOOKUP_MAP_VOID::const_iterator foundPointer = m_impl.find(key);
            if (foundPointer == m_impl.end())
            {
                return false;
            }
            else
            {
                return true;
            }
        }
    private:
        CLOOKUP_MAP_VOID m_impl;
        bool m_freePointers;
};

/**
*  CLookupInt: Lookup from pointer to integer
*
*/

class CLookupInt
{
    public:
        friend class CIterator;

        /**
        * Iterator over CLookupInt: visit all <key_pointer, integer_value> pairs
        *
        */
        class CIterator
        {
            public:
                CIterator(const CLookupInt *lookup) : m_lookup(lookup), m_iterator(lookup->m_impl.begin())
                {
                }
                bool next(void*& key, int &value)
                {
                    if (m_iterator != m_lookup->m_impl.end())
                    {
                        key   = (*m_iterator).first;
                        value = (*m_iterator).second;
                        m_iterator++;
                        return true;
                    }
                    else
                    {
                        key = NULL;
                        value = 0;
                        return false;
                    }
                }
                void reset()
                {
                    m_iterator = m_lookup->m_impl.begin();
                }

            private:
                const CLookupInt *m_lookup;
                CLOOKUP_MAP_INT::const_iterator m_iterator;
        };

    public:
        //------------------------------------------
        //----- CLookupInt -------------------------
        //------------------------------------------

        // TODO: void erase(key)

        int get(void *key) const
        {
            CLOOKUP_MAP_INT::const_iterator foundPointer = m_impl.find(key);
            if (foundPointer == m_impl.end())
            {
                // not found
                return 0;
            }
            return (*foundPointer).second;
        }

        void put(void *key, int value)
        {
            m_impl[key] = value;
        }

        bool exists(void *key) const
        {
            CLOOKUP_MAP_INT::const_iterator foundPointer = m_impl.find(key);
            if (foundPointer == m_impl.end())
            {
                return false;
            }
            else
            {
                return true;
            }
        }

    private:
        CLOOKUP_MAP_INT m_impl;
};

/**
 * CLookupAny<T>
 *   Lookup from void pointer to 'T'
 *   T must be a compact data structure that allows copying by the '=' operator
 *
 */

template< class T>
class CLookupAny
{
    public:
        friend class CIterator;

        /**
        * Iterator over CLookupAny: visit all <key_pointer, T_value> pairs
        *
        */
        class CIterator : private CLookupPtr<T >::CIterator
        {
            public:
                CIterator(const CLookupAny *lookup)
                    : CLookupPtr<T >::CIterator(&lookup->m_lookup_ptr), m_default_value(lookup->m_default_value)
                {
                }
                bool next(void*& key, T &value)
                {
                    T *value_ptr;
                    bool have_next = CLookupPtr<T >::CIterator::next(key, value_ptr);

                    if (value_ptr)
                    {
                        value = *value_ptr;
                    }
                    else
                    {
                        value = m_default_value;
                    }

                    return have_next;
                }

                void reset()
                {
                    CLookupPtr<T >::CIterator::reset();
                }

            private:
                T m_default_value;
        };

    public:
        //------------------------------------------
        //------------ CLookupAny ------------------
        //------------------------------------------

        // TODO: void erase(key)

        CLookupAny(T default_value) : m_lookup_ptr(true/*free pointers*/), m_default_value(default_value)
        {
        }

        const T get(void *key) const
        {
            T *ptr = m_lookup_ptr.get(key);
            if (!ptr)
            {
                ptr = new T;
                *ptr = m_default_value;
                CLookupPtr<T>* lookup_impl = const_cast<CLookupPtr<T>* >(&m_lookup_ptr);
                lookup_impl->put(key, ptr);
            }
            return *ptr;
        }

        void put(void *key, T value)
        {
            T *ptr = new T;
            *ptr = value;
            m_lookup_ptr.put(key, ptr);
        }

        bool exists(void *key) const
        {
            return m_lookup_ptr.exists(key);
        }

    private:
        CLookupPtr<T> m_lookup_ptr;
        T m_default_value;
};
#endif