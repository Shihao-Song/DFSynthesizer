/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   mpmatrix.cc
 *
 *  Author          :   Marc Geilen (m.c.w.geilen@tue.nl)
 *
 *  Date            :   March 23, 2009
 *
 *  Function        :   MaxPlus vectors and matrices
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


#include "mpmatrix.h"
#include "../exception/exception.h"
#include <stdlib.h>
#include <math.h>

namespace MaxPlus
{

    /**
     * Construct a Maxplus vector of size
     */
    Vector::Vector(unsigned int size, MPTime value)
    {
        this->table.resize(size);
        for (unsigned int k = 0; k < size; k++)
        {
            this->table[k] = value;
        }
    }

    /**
     * Construct a Maxplus vector from an std:vector
     */
    Vector::Vector(std::vector<MPTime> *v)
    {
        this->table.resize(v->size());
        for (unsigned int i = 0; i < v->size(); i++)
        {
            this->table[i] = v->at(i);
        }
    }

    /**
     * copy constructor
     */
    Vector::Vector(const Vector &other)
    {
        this->table.resize(other.getSize());
        for (unsigned int row = 0; row < this->getSize(); row++)
        {
            this->table[row] = other.table[row];
        }
    }

    /**
     * vector assignmnent
     */
    Vector &Vector::operator =(const Vector &other)
    {
        if (this->getSize() != other.getSize())
        {
            throw CException("Vectors of different size in"
                             "Vector::operator=");
        }
        for (unsigned int row = 0; row < this->getSize(); row++)
        {
            this->table[row] = other.table[row];
        }
        return *this;
    }

    /**
     * vector negate
     */
    void Vector::negate()
    {
        for (unsigned int row = 0; row < this->getSize(); row++)
        {
            if (this->get(row) == MP_MINUSINFINITY)
            {
                throw CException("Cannot negate vectors with MP_MINUSINFINITY elements in"
                                 "Vector::negate");
            }
            else
            {
                this->put(row, -this->get(row));
            }
        }
    }


    /**
     * calculate vector norm
     */
    MPTime Vector::norm()
    {
        MPTime maxEl = MP_MINUSINFINITY;
        for (unsigned int row = 0; row < this->getSize(); row++)
        {
            maxEl = MP_MAX(maxEl, this->get(row));
        }
        return maxEl;
    }

    /**
     * normalize vector
     */
    MPTime Vector::normalize()
    {
        double maxEl = this->norm();

        if (maxEl == MP_MINUSINFINITY)
        {
            throw CException("Cannot normalize vector with norm MP_MINUSINFINITY"
                             "Vector::normalize");
        }
        else
        {
            for (unsigned int row = 0; row < this->getSize(); row++)
            {
                MPTime x_i =  this->get(row); // MPTime handles -INF correctly
                x_i = x_i - maxEl;          // overloaded using MP_PLUS
                this->put(row, x_i);
            }
        }
        return maxEl;
    }

    /**
     * add scalar to vector
     */
    Vector *Vector::add(MPTime increase) const
    {
        unsigned int M = this->getSize();
        Vector *result = new Vector(M);
        this->add(increase, result);
        return result;
    }

    /**
     * add scalar to vector
     */
    void Vector::add(MPTime increase, Vector *result) const
    {
        unsigned int M = this->getSize();
        assert(result->getSize() == M);

        for (unsigned int pos = 0; pos < M; pos++)
        {
            result->put(pos, this->get(pos) + increase); // uses MP_PLUS()
        }
    }

    /**
     * max of vectors
     */
    void Vector::maximum(const Vector *vecB, Vector *result) const
    {
        unsigned int M = this->getSize();
        assert(vecB->getSize() == M);
        assert(result->getSize() == M);

        for (unsigned int pos = 0; pos < M; pos++)
        {
            result->put(pos, MP_MAX(this->get(pos), vecB->get(pos)));
        }
    }


    /**
     * Destructor of MaxPlus vector
     */
    Vector::~Vector()
    {
    }

    /**
     * Put an entry into the vector. Grows vector if necessary
     */
    void Vector::put(unsigned int row, MPTime value)
    {
        if (this->table.size() <= row)
            this->table.resize(row + 1, 0);
        this->table[row] = value;
    }

    /**
     * String representation of vector
     */
    void Vector::toString(CString &outString, double scale) const
    {
        outString = "";
        for (unsigned int row = 0; row < this->getSize(); row++)
        {
            outString += timeToString(scale * this->get(row)) + " ";
        }
    }

    /**
     * Vector::add
     * add vectors
     */
    void Vector::add(const Vector *vecB, Vector *res) const
    {
        assert(this->getSize() == vecB->getSize());
        assert(this->getSize() == res->getSize());
        for (unsigned int row = 0; row < this->getSize(); row++)
        {
            res->put(row, this->get(row) + vecB->get(row));
        }
    }

    /**
     * add vectors
     */
    Vector *Vector::add(const Vector *vecB) const
    {
        Vector *res = new Vector(this->getSize());
        this->add(vecB, res);
        return res;
    }

    /**
     * Get minimal finite element
     * returns the smallest amoung the finite elements in the vector or
     * MP_MINUSINFINITY if no finite elements exist
     * itsPosition returns the index of the (a) smallest finite element is set
     * to a pointer to unsigned int, otherwise set or defaults to NULL
     */
    MPTime Vector::minimalFiniteElement(unsigned int *itsPosition_Ptr) const
    {
        unsigned int itsPosition_tmp;
        unsigned int *itsPosition;
        if (itsPosition_Ptr != NULL)
            itsPosition = itsPosition_Ptr;
        else
            itsPosition = &itsPosition_tmp;
        *itsPosition = this->getSize() + 1; // arbitrary value, invalid

        MPTime minEl = MP_MINUSINFINITY;
        bool minEl_initialized = false;
        for (unsigned int row = 0; row < this->getSize(); row++)
        {
            MPTime val = this->get(row);
            if (MaxPlus::MP_ISMINUSINFINITY(val)) continue;
            if (!minEl_initialized || val < minEl)
            {
                minEl = val;
                minEl_initialized = true;
                *itsPosition = row;
            }
        }
        return minEl;
    }


    /**
     * Compare vectors up to MP_EPSILON
     */
    bool Vector::compare(const Vector &v)
    {
        if (this->getSize() != v.getSize())
            return false;
        for (unsigned int k = 0; k < this->getSize(); k++)
        {
            if (fabs(this->get(k) - v.get(k)) > MP_EPSILON)
                return false;
        }
        return true;
    }


    /**
     * initialize matrix
     */
    void Matrix::init()
    {
        unsigned int nels = this->getRows() * this->getCols();
        this->table.resize(nels);
        for (unsigned int pos = 0; pos < nels; pos++)
        {
            this->table[pos] = MP_MINUSINFINITY;
        }
    }

    /**
     * Construct a square Maxplus matrix of N by N
     */
    Matrix::Matrix(unsigned int N)
    {
        this->szRows = N;
        this->szCols = N;
        this->init();
    }

    /**
     * Construct a Maxplus matrix of nrows by ncols
     */
    Matrix::Matrix(unsigned int nrows, unsigned int ncols)
    {
        this->szRows = nrows;
        this->szCols = ncols;
        this->init();
    }

    /**
     * Destructor of MaxPlus matrix
     */
    Matrix::~Matrix()
    {
    }

    /**
     * Get size of a square matrix
     */
    unsigned int Matrix::getSize(void) const
    {
        assert(this->getRows() == this->getCols());
        return this->getRows();
    }


    /**
     * Get an entry from the matrix. Row and column index must be between 0 and size-1
     */
    MPTime Matrix::get(unsigned int row, unsigned int column) const
    {
        if ((row >= this->getRows()) || (column >= this->getCols()))
        {
            throw CException("Index out of bounds in"
                             "Matrix::get");
        }
        return this->table[row * this->getCols() + column];
    }

    /**
     * Put a value in the matrix. Row and column index must be between 0 and size-1
     */
    void Matrix::put(unsigned int row, unsigned int column, MPTime value)
    {
        if ((row >= this->getRows()) || (column >= this->getCols()))
        {
            throw CException("Index out of bounds in"
                             "Matrix::put");
        }
        this->table[row * this->getCols() + column] = value;
    }

    /**
     * Paste submatrix into matrix
     */
    void Matrix::paste(unsigned int top_row, unsigned int left_column, const Matrix *pastedMatrix)
    {
        const unsigned int p_rsz = pastedMatrix->getRows();
        const unsigned int p_csz = pastedMatrix->getCols();
        const unsigned int bottom_row   = top_row     +  p_rsz - 1;
        const unsigned int right_column = left_column +  p_csz - 1;
        assert(bottom_row < this->getRows());
        assert(right_column < this->getCols());
        unsigned int p_row = 0;
        unsigned int p_col = 0;

        for (unsigned int row = top_row; row <= bottom_row; row++, p_row++)
        {
            p_col = 0;
            for (unsigned int col = left_column; col <= right_column; col++, p_col++)
            {
                this->put(row, col, pastedMatrix->get(p_row, p_col));
            }
        }
    }

    /**
     * mpmultiply()
     * Matrix-vector multiplication.
     */
    Vector *Matrix::mpmultiply(const Vector &v) const
    {
        // Check size of the matrix and vector
        if (this->getCols() != v.getSize())
        {
            throw CException("Matrix and vector are of unequal size in"
                             "Matrix::mpmultiply");
        }

        // Allocate space of the resulting vector
        Vector *res = new Vector(this->getRows());

        // Perform point-wise multiplication
        for (unsigned int i = 0; i < this->getRows(); i++)
        {
            MPTime m = MP_MINUSINFINITY;
            for (unsigned int k = 0; k < this->getCols(); k++)
            {
                m = MP_MAX(m, MP_PLUS(this->get(i, k), v.get(k)));
            }
            res->put(i, m);
        }
        return res;
    }

    /**
     * Matrix copy.
     */
    Matrix *Matrix::createCopy() const
    {
        Matrix *newMatrix = new Matrix(this->getRows(), this->getCols());
        unsigned int nels = this->getRows() * this->getCols();
        for (unsigned int pos = 0; pos < nels; pos++)
        {
            newMatrix->table[pos] = this->table[pos];
        }
        return newMatrix;
    }

    /**
     * Matrix transposed copy.
     */
    Matrix *Matrix::getTransposedCopy() const
    {
        unsigned int MR = this->getCols();
        unsigned int MC = this->getRows();
        Matrix *newMatrix = new Matrix(MR, MC);
        for (unsigned int col = 0; col < MC; col++)
        {
            for (unsigned int row = 0; row < MR; row++)
            {
                newMatrix->put(row, col, this->get(col, row));
            }
        }
        return newMatrix;
    }

    /**
     * Make submatrix with indices in list.
     */
    Matrix *Matrix::getSubMatrix(const list<unsigned int> &rowIndices, const list<unsigned int> &colIndices) const
    {
        unsigned int NR = rowIndices.size();
        unsigned int NC = colIndices.size();
        Matrix *newMatrix = new Matrix(NR, NC);

        list<unsigned int>::const_iterator rit = rowIndices.begin();
        for (unsigned int r = 0; r < NR; r++, rit++)
        {
            unsigned int ri = (*rit);
            list<unsigned int>::const_iterator cit = colIndices.begin();
            for (unsigned int c = 0; c < NC; c++, cit++)
            {
                unsigned int ci = (*cit);
                newMatrix->put(r, c, this->get(ri, ci));
            }
        }
        return newMatrix;
    }

    /**
     * Make submatrix with indices in list from square matrix
     */
    Matrix *Matrix::getSubMatrix(const list<unsigned int> &indices) const
    {
        assert(this->getRows() == this->getCols());
        return this->getSubMatrix(indices, indices);
    }


    /**
     * Matrix addition of scalar.
     */
    Matrix *Matrix::add(MPTime increase) const
    {
        unsigned int MR = this->getRows();
        unsigned int MC = this->getCols();
        Matrix *result = new Matrix(MR, MC);

        this->add(increase, result);
        return result;
    }

    /**
     * Matrix addition of scalar with existing result matrix.
     */
    void Matrix::add(MPTime increase, Matrix *result) const
    {
        unsigned int MR = this->getRows();
        unsigned int MC = this->getCols();
        if ((MR != result->getRows()) || (MC != result->getCols()))
        {
            throw CException("Matrices are of different size in"
                             "Matrix::add(Matrix*, MPTime, Matrix*");
        }
        for (unsigned int r = 0; r < MR; r++)
        {
            for (unsigned int c = 0; c < MC; c++)
            {
                result->put(r, c, this->get(r, c) + increase); // uses MP_PLUS()
            }
        }
    }


    /**
     * Matrix maximum with existing result matrix.
     */
    void Matrix::maximum(const Matrix *matB, Matrix *result)
    {
        unsigned int MR = this->getRows();
        unsigned int MC = this->getCols();
        if ((matB->getRows() != MR) ||
            (matB->getCols() != MC) ||
            (result->getRows() != MR) ||
            (result->getCols() != MC))
        {
            throw CException("Matrices are of different size in"
                             "Matrix::maximum(Matrix*, Matrix*, Matrix*");
        }

        for (unsigned int r = 0; r < MR; r++)
        {
            for (unsigned int c = 0; c < MC; c++)
            {
                result->put(r, c, MP_MAX(this->get(r, c), matB->get(r, c)));
            }
        }
    }

    /**
     * Matrix to string.
     */
    void Matrix::toString(CString &outString, double scale) const
    {
        outString = "";
        unsigned int MR = this->getRows();
        unsigned int MC = this->getCols();
        for (unsigned int i = 0; i < MR; i++)
        {
            for (unsigned int j = 0; j < MC; j++)
            {
                outString += timeToString(this->get(i, j) * scale) + " ";
            }
            outString += "\n";
        }
    }



    /**
     * Matrix return largest element.
     */
    MPTime Matrix::largestFiniteElement() const
    {
        // finite element with the largest absolute value
        // all -INF => 0
        //
        MPTime largestEl = 0, largestMag = 0;
        unsigned int MR = this->getRows();
        unsigned int MC = this->getCols();

        for (unsigned int r = 0; r < MR; r++)
        {
            for (unsigned int c = 0; c < MC; c++)
            {
                if (this->get(r, c) == MP_MINUSINFINITY) continue;

                MPTime mag = fabs(this->get(r, c));
                if (mag > largestMag)
                {
                    largestEl  = this->get(r, c);
                    largestMag = mag;
                }
            }
        }

        return largestEl;
    }

    /**
     * Matrix get minimal element
     */
    MPTime Matrix::minimalFiniteElement() const
    {
        // smallest finite element
        // all -INF => -INF
        unsigned int MR = this->getRows();
        unsigned int MC = this->getCols();

        if (MR == 0 || MC == 0) return MP_MINUSINFINITY;

        MPTime minimalEl = MP_MINUSINFINITY;

        for (unsigned int r = 0; r < MR; r++)
        {
            for (unsigned int c = 0; c < MC; c++)
            {
                if (this->get(r, c) == MP_MINUSINFINITY) continue;

                if (minimalEl == MP_MINUSINFINITY)
                {
                    minimalEl = this->get(r, c);
                }
                else
                {
                    minimalEl = MP_MIN(minimalEl, this->get(r, c));
                }
            }
        }

        return minimalEl;
    }

    /**
     * Matrix plus closure.
     */
    Matrix *Matrix::plusClosureMatrix(MPTime posCycleThre) const
    {
        // notation: A^(+) = max(A, A^2, ...)
        return Matrix::allPairLongestPathMatrix(posCycleThre, false /*implyZeroSelfEdges*/);
    }

    /**
     * Matrix star closure.
     */
    Matrix *Matrix::starClosureMatrix(MPTime posCycleThre) const
    {
        // notation: A^(*) = max(E,A,A^2,...)
        // E - diagonal matrix with 'e'==0 on the diagonal

        return Matrix::allPairLongestPathMatrix(posCycleThre, true /*implyZeroSelfEdges*/);
    }


    /**
     * Matrix all pair longest path.
     */
    Matrix *Matrix::allPairLongestPathMatrix(MPTime posCycleThre, bool implyZeroSelfEdges) const
    {
        // Floyd-Warshall algorithm
        if (this->getRows() != this->getCols())
            throw CException("Matrix must be square in Matrix::allPaiLongestPathMatrix.");
        unsigned int N = this->getRows();


        Matrix *distMat = this->createCopy();

        //// DEBUG-MAT
        //MaxPlus::Matrix lengthMat(N);
        //for (unsigned int kk=0; kk<N*N; kk++) lengthMat.table[kk]=1;

        // k - intermediate node
        //
        for (unsigned int k = 0; k < N; k++)
        {

            //// DEBUG-MAT
            //debug_mat(distMat, k);

            for (unsigned int u = 0; u < N; u++)
            {
                for (unsigned int v = 0; v < N; v++)
                {
                    MPTime extra = (implyZeroSelfEdges && u == v) ? (MPTime)0 : MP_MINUSINFINITY;
                    MPTime path_u2v = MP_MAX(distMat->get(v, u), extra);
                    MPTime path_u2k = distMat->get(k, u);
                    MPTime path_k2v = distMat->get(v, k);

                    MPTime path_u2v_candidate = (path_u2k + path_k2v);  // uses MP_PLUS()
                    if (path_u2v_candidate > path_u2v)
                    {
                        path_u2v = path_u2v_candidate;

                        ////DEBUG-MAT
                        //lengthMat.put(v,u, lengthMat.get(k,u) + lengthMat.get(v,k));

                    }
                    distMat->put(v, u, path_u2v);
                }
            }
        }

        //// DEBUG-MAT
        //debug_mat(distMat, N);
        //cerr << "length(0,0) = " << lengthMat.get(0,0) << endl;

        for (unsigned int k = 0; k < N; k++)
        {
            if (distMat->get(k, k) > posCycleThre)
            {
                CString tmp;
                distMat->toString(tmp, 1.0e-06);
                cout << tmp << endl;
                throw CException("Positive cycle!");
            }
        }

        return distMat;
    }


    /**
     * class VectorList
     */

    /**
     * VectorList::toString()
     */
    void VectorList::toString(CString &outString, double scale) const
    {
        outString = "";
        for (unsigned int i = 0; i < this->getSize(); i++)
        {
            CString vec_str;
            const Vector *v = &vectorRefAt(i);
            assert(v->getSize() == this->oneVectorSize);
            v->toString(vec_str, scale);
            outString += vec_str;
            outString += "\n";
        }
    }

    /**
     * VectorList::findSimilar
     * test if list contains a vector which differs to vecX by less than threshold
     */
    bool VectorList::findSimilar(const Vector &vecX, double threshold) const
    {

        // similar - differs by a constant within a threshold
        bool found = false;
        assert(vecX.getSize() == this->oneVectorSize);
        // test all vectors in list
        for (unsigned int i = 0; i < this->size(); i++)
        {
            const Vector &vecY = this->vectorRefAt(i);
            assert(vecY.getSize() == this->oneVectorSize);

            // determine min and max difference
            double minDiff = 0;
            double maxDiff = 0;
            bool min_def = false;
            bool max_def = false;
            for (unsigned int j = 0; j < this->oneVectorSize; j++)
            {
                bool vecX_inf = MP_ISMINUSINFINITY(vecX.get(j));
                bool vecY_inf = MP_ISMINUSINFINITY(vecY.get(j));
                if (vecX_inf || vecY_inf)
                {
                    if (vecX_inf && vecY_inf)
                    {
                        continue;
                    }
                    else
                    {
                        // Difference is infinity
                        minDiff = MP_MINUSINFINITY;
                        maxDiff = 0;
                        break;
                    }
                }

                double diff = vecX.get(j) - vecY.get(j);
                if (!max_def || diff > maxDiff)
                {
                    maxDiff = diff;
                    max_def = true;
                }
                if (!min_def || diff < minDiff)
                {
                    minDiff = diff;
                    min_def =  true;
                }
            }

            if ((maxDiff - minDiff) <= threshold)
            {
                found = true;
                break;
            }
        }

        return found;
    }


}

