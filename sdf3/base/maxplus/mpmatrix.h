/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   mpmatrix.h
 *
 *  Author          :   Marc Geilen (m.c.w.geilen@tue.nl)
 *
 *  Date            :   March 23, 2009
 *
 *  Function        :   MaxPlus matrices
 *
 *  History         :
 *      23-03-09    :   Initial version.
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

#ifndef MPMATRIX_H_INCLUDED
#define MPMATRIX_H_INCLUDED

#include "mptype.h"
#include <vector>

class CString;

namespace MaxPlus
{

    /**
     * Vector, represents a MaxPlus column vector
     */
    class Vector
    {
        public:
            Vector(unsigned int size = 0, MPTime value = MP_MINUSINFINITY);
            Vector(std::vector<MPTime> *v);
            ~Vector();
            inline unsigned int getSize(void) const
            {
                return (unsigned int) this->table.size();
            }
            inline MPTime get(unsigned int row) const
            {
                return this->table[row];
            }
            void put(unsigned int row, MPTime value);
            void toString(CString &outString, double scale = 1.0) const;

            Vector(const Vector &);
            Vector &operator=(const Vector &);

            // unary operations:
            MPTime norm();
            void negate(); // for ALAP
            MPTime normalize();

            // arithmetic operations:
            Vector *add(MPTime increase) const;
            void add(MPTime increase, Vector *result) const;
            void maximum(const Vector *matB, Vector *result) const;

            Vector *add(const Vector *vecB) const;
            void add(const Vector *vecB, Vector *res) const ;

            Vector &operator+=(MPTime increase)
            {
                this->add(increase, this);
                return *this;
            }

            Vector &operator-=(MPTime decrease)
            {
                assert(!MP_ISMINUSINFINITY(decrease));
                this->add(-decrease, this);
                return *this;
            }

            bool compare(const Vector &v) ;

            Vector &incrementalMaximum(const Vector *vec)
            {
                this->maximum(vec, this);
                return *this;
            }

            // misc:

            /**
             * Get minimal finite element
             * returns the smallest amoung the finite elements in the vector or
             * MP_MINUSINFINITY if no finite elements exist
             * itsPosition returns the index of the (a) smallest finite element is set
             * to a pointer to unsigned int, otherwise set or defaults to NULL
             */
            MPTime minimalFiniteElement(unsigned int *itsPosition_Ptr = NULL) const;

        private:
            vector<MPTime> table;
    };


    /****************************************************
    * represents a possibly rectangular MaxPlus matrix
    ****************************************************/
    class Matrix
    {
        public:
            Matrix(unsigned int nrows, unsigned int ncols);
            Matrix(unsigned int N);
            ~Matrix();
            inline unsigned int getRows(void) const
            {
                return this->szRows;
            }
            inline unsigned int getCols(void) const
            {
                return this->szCols;
            }
            unsigned int getSize(void) const;

            MPTime get(unsigned int row, unsigned int column) const;
            void put(unsigned int row, unsigned int column, MPTime value);

            void paste(unsigned int top_row, unsigned int left_column, const Matrix *pastedMatrix);
            Matrix *createCopy() const;
            Matrix *getTransposedCopy() const;
            Matrix *getSubMatrix(const list<unsigned int> &rowIndices, const list<unsigned int> &colIndices) const;
            Matrix *getSubMatrix(const list<unsigned int> &indices) const;

            void toString(CString &outString, double scale = 1.0) const;


            // algebraic operations:
            Matrix *add(MPTime increase) const;
            void add(MPTime increase, Matrix *result) const ;
            void maximum(const Matrix *matB, Matrix *result);

            Vector *mpmultiply(const Vector &v) const;

            Matrix &operator+=(MPTime increase)
            {
                this->add(increase, this);
                return *this;
            }

            Matrix &operator-=(MPTime decrease)
            {
                assert(!MP_ISMINUSINFINITY(decrease));
                this->add(-decrease, this);
                return *this;
            }

            Matrix &incrementalMaximum(const Matrix *matrix)
            {
                this->maximum(matrix, this);
                return *this;
            }

            // misc:
            MPTime largestFiniteElement() const; // element having the largest abs()
            MPTime minimalFiniteElement() const;

            // more complex operations:
            Matrix *plusClosureMatrix(MPTime posCycleThre) const;
            Matrix *starClosureMatrix(MPTime posCycleThre) const;
            Matrix *allPairLongestPathMatrix(MPTime posCycleThre, bool implyZeroSelfEdges) const;

        private:
            // Implicit copying is not allowed
            //  => Intentionally private and not implemented
            Matrix(const Matrix &);
            Matrix &operator=(const Matrix &);

            void init();

        private:
            Matrix();

        private:
            vector<MPTime> table;
            unsigned int szRows;
            unsigned int szCols;
    };


    /****************************************************
    * VectorList: usually represents a set of eigenvectors
    * More efficient than vector<MaxPlus::Vector>
    ****************************************************/

    class VectorList : private std::vector<Vector * >
    {
        public:
            VectorList(unsigned int oneVectorSizeInit);
            ~VectorList();

            const Vector &vectorRefAt(int n) const; // vector at index 'n'
            Vector &vectorRefAt(int n);

            const Vector &lastVectorRef() const; // last vector
            Vector &lastVectorRef();

            unsigned int getSize() const; // vector count
            unsigned int getOneVectorSize() const
            {
                return this->oneVectorSize;
            }

            void grow(); // append one vector place

            void toString(CString &outString, double scale = 1.0) const;

            bool findSimilar(const Vector &vec, double threshold) const;
            // similar - differs by a constant within a threshold

        private:
            // Implicit copying is not allowed
            //  => Intentionally private and not implemented
            VectorList(const VectorList &);
            VectorList &operator=(const VectorList &);

        private:
            const unsigned int oneVectorSize;
    };

    inline VectorList::VectorList(unsigned int oneVectorSizeInit)
        : oneVectorSize(oneVectorSizeInit)
    {
        assert(oneVectorSize > 0);
    }

    inline VectorList::~VectorList()
    {
        for (unsigned int pos = 0; pos < size(); pos++)
        {
            delete this->at(pos);
        }
    }

    inline const Vector &VectorList::vectorRefAt(int n) const
    {
        return *this->at(n);
    }

    inline Vector &VectorList::vectorRefAt(int n)
    {
        return *this->at(n);
    }

    inline const Vector &VectorList::lastVectorRef() const
    {
        return *this->at(this->size() - 1);
    }

    inline Vector &VectorList::lastVectorRef()
    {
        return *this->at(this->size() - 1);
    }

    inline unsigned int VectorList::getSize() const
    {
        return (unsigned int) vector<Vector * >::size();
    }

    inline void VectorList::grow()
    {
        unsigned int last = (unsigned int) this->size();
        this->resize(last + 1);
        this->at(last) = new Vector(oneVectorSize, MP_MINUSINFINITY);
    }
}

#endif