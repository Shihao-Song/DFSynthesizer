/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   matrix.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   29 September 2006
 *
 *  Function        :   Sparse Matrices
 *
 *  History         :
 *      29-09-06    :   Initial version.
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

#include "matrix.h"


/**
 * SparseMAtrix constructor
 */
SparseMatrix::SparseMatrix(const CId Size)
{

    // set size
    Matrix.resize(Size);

    // create row vectors
    for (CId i = 0; i != Size; i++)
        Matrix[i] = (SparseVector *) new SparseVector();
}

/**
 * SparseVector destructor
 */
SparseVector::~SparseVector()
{
    while (Root->Next != NULL)
        removeFirstElement();
    delete Root;
}

/**
 * SparseMatrix destructor
 */
SparseMatrix::~SparseMatrix()
{
    // destruct row vectors
    for (CId i = 0; i != Matrix.size() - 1; i++)
        delete Matrix[i];
}


/**
 * SparseVector::addElement
 */
void SparseVector::addElement(const CId Index, const CDouble Value)
{
    SparseVectorElement *Element = new SparseVectorElement(Index, Value);
    bool ElementAdded = false;

    // find place to insert
    SparseVectorElement *i = Root;
    for (i = Root; !ElementAdded && i->Next != NULL; i = i->Next)
    {
        if (i->Next->Index > Index)
        {
            Element->Next = i->Next;
            i->Next = Element;
            ElementAdded = true;
        }
    }

    // if not added, it goes at the end
    if (!ElementAdded)
        i->Next = Element;
}

/**
 * SparseVector::getElement
 */
SparseVectorElement *SparseVector::getElement(const CId Index)
{

    for (SparseVectorElement *i = Root->Next; i != NULL; i = i->Next)
    {
        if (i->Index == Index)
            return i;
    }
    return NULL;
}

/**
 * SparseVector::removeFirstElement
 */
void SparseVector::removeFirstElement()
{
    SparseVectorElement *Element = Root->Next;
    Root->Next = Element->Next;
    delete Element;
}

/**
 * SparseMatrix::computeEigenVector
 * Function to compute eigen vector
 */
vector<CDouble> SparseMatrix::computeEigenVector()
{

    // trivial case
    if (Matrix.size() == 1)
    {
        vector<CDouble> EigenVector(1, 1);
        return EigenVector;
    }

    // Gaussian elimination

    for (CId i = 0; i != Matrix.size() - 1; i++)
    {

        // Find row j with maximum element starting from row i
        // postpone using last row as much as possible (MG: why?)

        CDouble Maximum = Matrix[i]->getFirstElement()->Value;
        CId Index = i;

        // find pivot element
        bool NonZeroElementFound = false;
        for (CId j = i + 1; j != Matrix.size() - 1; j++)
        {
            SparseVectorElement *Element = Matrix[j]->getFirstElement();
            if (Element->Index == i)
            {
                NonZeroElementFound = true;
                if (fabs(Element->Value) > fabs(Maximum))
                {
                    Maximum = Element->Value;
                    Index = j;
                }
            }
        }

        // nothing found, use the last row
        if (!NonZeroElementFound)
        {
            Maximum = Matrix[Matrix.size() - 1]->getFirstElement()->Value;
            Index = Matrix.size() - 1;
        }

        // Swap current row i with row that has maximum element (if necessary) and division by that element
        if (Index != i)
        {
            SparseVector *Temp = Matrix[i];
            Matrix[i] = Matrix[Index];
            Matrix[Index] = Temp;
        }

        // Subtraction of rows
        SparseVectorElement *FirstElementOfRow = Matrix[i]->getFirstElement();
        // iterate throught rows i+1 to size-1
        for (CId j = i + 1; j != Matrix.size(); j++)
        {
            // Element iterates through row j
            SparseVectorElement *Element = Matrix[j]->getFirstElement();
            // only action required if row j has a non-zero value at column i...
            if (Element->Index == i)
            {
                CDouble Factor = -Element->Value / Maximum;
                SparseVectorElement *PreviouslyAccessedElement = Element;
                // iterate through the non-zero elements of row i
                for (SparseVectorElement *ElementOfRow = FirstElementOfRow->Next; ElementOfRow != NULL;)
                {
                    SparseVectorElement *ElementToChange = PreviouslyAccessedElement->Next;
                    // if row j still has non-zero elements
                    if (ElementToChange != NULL)
                    {
                        // test if the next element is at the index we are trying to modify
                        if (ElementToChange->Index == ElementOfRow->Index)
                        {
                            // update its value accordingly
                            ElementToChange->Value = ElementToChange->Value + Factor * ElementOfRow->Value;
                            // if the result becomes zero remove it
                            if (ElementToChange->Value == 0.0)   // FIXME: testing double for equality is risky?
                            {
                                PreviouslyAccessedElement->Next = ElementToChange->Next;
                                delete ElementToChange;
                            }
                            else
                            {
                                // else move on
                                PreviouslyAccessedElement = ElementToChange;
                            }
                            ElementOfRow = ElementOfRow->Next;
                        }
                        else if (ElementToChange->Index > ElementOfRow->Index)
                        {
                            // next element in row j comes after next element in i, create new element
                            SparseVectorElement *NewElement =
                                new SparseVectorElement(ElementOfRow->Index, Factor * ElementOfRow->Value);
                            NewElement->Next = ElementToChange;
                            PreviouslyAccessedElement->Next = NewElement;
                            PreviouslyAccessedElement = NewElement;
                            // move forward
                            ElementOfRow = ElementOfRow->Next;
                        }
                        else
                        {
                            // next element in row j comes before element to change, just move on
                            PreviouslyAccessedElement = ElementToChange;
                        }
                    }
                    else
                    {
                        // the row j has no more non zero elements, create a new element
                        SparseVectorElement *NewElement = new SparseVectorElement(ElementOfRow->Index, Factor * ElementOfRow->Value);
                        // update all references
                        NewElement->Next = ElementToChange;
                        PreviouslyAccessedElement->Next = NewElement;
                        PreviouslyAccessedElement = NewElement;
                        // move forward
                        ElementOfRow = ElementOfRow->Next;
                    }
                }
                // the first element of the row becomes always zero and is therefore removed
                Matrix[j]->removeFirstElement();
            }
        }
    }

    // Backtrack solutions

    // create result matrix
    vector<CDouble> EigenVector(Matrix.size(), 0);

    //
    for (CId i = Matrix.size() - 1; i + 1 != 0; i--)
    {

        CDouble Sum = 0;

        if (i < Matrix.size() - 1)
            for (SparseVectorElement *ElementOfRow = Matrix[i]->getFirstElement()->Next; ElementOfRow != NULL; ElementOfRow = ElementOfRow->Next)
                if (ElementOfRow->Index < Matrix.size())
                    Sum += ElementOfRow->Value * EigenVector[ElementOfRow->Index];

        CDouble Result;
        SparseVectorElement *Element = Matrix[i]->getElement(Matrix.size());

        if (Element != NULL)
            Result = (Element->Value - Sum) / Matrix[i]->getFirstElement()->Value;
        else
            Result = -Sum / Matrix[i]->getFirstElement()->Value;

        EigenVector[i] = Result;
    }

    return EigenVector;
}
