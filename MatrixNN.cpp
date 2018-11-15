// ---------------------------------------------------------------------------
// MatrixNN.cpp - C++ File
// Copyright ©2004 Michael B. Comet
// ---------------------------------------------------------------------------
//
// DESCRIPTION:
//	A plugin class to do stuff with square NxN matrices.
//
// AUTHOR:
//	Michel B. Comet - comet@comet-cartoons.com
//	Based on some LU pseudocode from http://www.utsc.utoronto.ca/~nick/cscC50/02may/notes03
//
// VERSIONS:
//	1.05 - 10/12/04 - mcomet - First Version of this file.
//	1.06 - 10/16/04 - mcomet - Added mirrorData node to help with mirroring.
//	1.07 - 11/03/04 - mcomet - Now only uses membership points when creating
//				targets, also put in missing scripts to ZIP file.
//  1.09c - 12/01/04 - mcomet - Now has option for joint-space vs. pose-space
//				deformation.  So that the sculpt is either tied to the joint
//				or the reader as it is brought back up.
//	1.10 - 01/21/05 - mcomet - Fix for wig out mesh when pose deleted.
//	
// ---------------------------------------------------------------------------
//
//  poseDeformer - Pose Space Deformer Maya Plugin by Michael B. Comet
//  Copyright ©2004 Michael B. Comet
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, write to the Free Software
//   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//   For information on poseDeformer contact:
//			Michael B. Comet - comet@comet-cartoons.com
//			or visit http://www.comet-cartooons.com/toons/
//
// --------------------------------------------------------------------------

/*
 * Includes
 */
#include "MatrixNN.h"


// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//	Main Node Functions
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------


/*
 * MatrixNN::MatrixNN() - constructor
 */
MatrixNN::MatrixNN() 
{
	ptrDbl = NULL ;
	uDimension = 0 ;
}

// ---------------------------------------------------------------------------

/*
 * MatrixNN::~MatrixNN() - Destructor
 */
MatrixNN::~MatrixNN()
{
	free() ;			// free any memory alloced before we are totally destroyed!
}

// ---------------------------------------------------------------------------

/*
 * MatrixNN::free() - Frees any alloced memory
 */
void MatrixNN::free(void)
{
	if (ptrDbl != NULL)
		delete [] ptrDbl ;
	ptrDbl = NULL ;
	uDimension = 0 ;

}

// ---------------------------------------------------------------------------
/*
 * MatrixNN::setDimension() - Aloc the array.  Returns alloced dimension or
 *								0 on error.
 */
unsigned MatrixNN::setDimension(unsigned uDim)
{
	free() ;		// First free anything
	if (uDim == 0)	// Can't alloc 0 bytes
		return 0 ;

	unsigned uTotal = uDim * uDim ;	// How many entries in a Nrow x NCol matrix?

	ptrDbl = new double [uTotal] ;		// Alloc
	if (ptrDbl == NULL)
		return 0 ;			// out of memory

	uDimension = uDim ;

	identity() ;			// By default set to this.

	return uDimension ;	
}

// ---------------------------------------------------------------------------

/*
 * MatrixNN::getDimension() - Gets the current dimension of the matrix
 */
unsigned MatrixNN::getDimension(void)
{
	return uDimension ;
}

// ---------------------------------------------------------------------------

/*
 * MatrixNN::fill() - Fills matrix to all the specified value
 */
void MatrixNN::fill(double dVal)
{
	if (uDimension == 0 || ptrDbl == NULL)
		return ;

	unsigned i,j;
	for (i=0; i < uDimension; ++i)
		{
		for (j=0; j < uDimension; ++j)
			{
			unsigned idx = getIndex(i, j);
			ptrDbl[idx] = dVal ;
			} // end of j
		} // end of i
}

// ---------------------------------------------------------------------------

/*
 * MatrixNN::getIndex() - Given a 2d index i,j pair, return the flat
 *			index into the 1D array allocated
 */
inline unsigned MatrixNN::getIndex(unsigned i, unsigned j)
{
	unsigned idx = (i*uDimension) + j ;
	return idx ;
}

// ---------------------------------------------------------------------------

/*
 * MatrixNN::identity() - Sets this matrix to identity
 */
void MatrixNN::identity(void)
{
	if (uDimension == 0 || ptrDbl == NULL)
		return ;

	unsigned i,j;
	for (i=0; i < uDimension; ++i)
		{
		for (j=0; j < uDimension; ++j)
			{
			unsigned idx = getIndex(i, j);
			if (i==j)
				ptrDbl[idx] = 1.0 ;
			else
				ptrDbl[idx] = 0.0 ;
			} // end of j
		} // end of i

}

// ---------------------------------------------------------------------------

/*
 * MatrixNN::operator[] - We need to do this so that the [][] works...
 */
double* MatrixNN::operator[](unsigned i)
{
	double *ptr = ptrDbl + (i*uDimension) ;
	return ptr ;
}

// ---------------------------------------------------------------------------

/*
 * MatrixNN::operator[] - We need to do this so that the [][] works...
 */
const double* MatrixNN::operator[](unsigned i) const
{
	double *ptr = ptrDbl + (i*uDimension) ;
	return ptr ;
}

// ---------------------------------------------------------------------------

/*
 * MatrixNN::operator() - Double index into matrix.
 */
double& MatrixNN::operator()(unsigned i, unsigned j)
{
	unsigned idx = getIndex(i, j) ;
	return (ptrDbl[idx]) ;
}

// ---------------------------------------------------------------------------

/*
 * MatrixNN::operator() const - Double index into matrix.
 */
double MatrixNN::operator()(unsigned i, unsigned j) const
{
	unsigned idx = (i*uDimension) + j ;
	return (ptrDbl[idx]) ;
}

// ---------------------------------------------------------------------------

/*
 * operator<<() - Output
 */
ostream& operator<<(ostream &os, const MatrixNN &matNN) 
{
	os << "MatrixNN " << matNN.uDimension << "x" << matNN.uDimension << endl ;
	if (matNN.uDimension > 0 && matNN.ptrDbl != NULL)
		{
		unsigned i,j;
		for (i=0; i < matNN.uDimension; ++i)
			{
			os << "ROW_" << i << "= " ;
			for (j=0; j < matNN.uDimension; ++j)
				{
				os << matNN[i][j] << " " ;
				}
			os << endl ;
			}
		}
	return os ;
}

// ---------------------------------------------------------------------------

/*
 * MatrixNN::operator=() - Assignemnt operator
 */
MatrixNN& MatrixNN::operator=(const MatrixNN &matNN) 
{
	setDimension( matNN.uDimension );

	unsigned u;
	unsigned uTotal = uDimension * uDimension ;
	for (u=0; u < uTotal; ++u)
		ptrDbl[u] = matNN.ptrDbl[u] ;

	return (*this) ;
}

// ---------------------------------------------------------------------------

/*
 * MatrixNN::factorizeLU() - For this current matrix, factorize it into 
 *	two matrices L and U, where L is a lower diagonal matrix, and U is an 
 *	upper diagonal matrix, such that this matrix A = L*U ;
 *
 *	To save space, this takes the current matrix and actually modifies it so that it
 *	end up holding both the L and U diagonal values within it when done.
 *
 *	This does so with partial (aka row) pivoting.
 *	uPivots is an array of values that will hold what row index each row was swaped to.
 *	ie: if row 3 was swapped to row 6, then uPivots[3] = 6  (assuming 0...n)
 *
 *
 *	Returns 1 on success, 0 on error
 */
bool MatrixNN::factorizeLU(unsigned uPivots[])
{
	if (uDimension == 0 || ptrDbl == NULL)
		return false ;

	unsigned i,j,k ;

	MatrixNN &A = (*this) ;	// Easier to work with...

	// Init pivot info
	for (k=0; k < uDimension; ++k)
		uPivots[k] = k ;

	for (k=0; k < uDimension-1; ++k)
		{
		// For pivoting, first find the greatest value of all the rows in the current column k.
		double dBiggest = A[k][k] ;
		unsigned uIdxPivot = k ;
		for (i=k+1; i < uDimension; ++i)
			{
			if (A[i][k] > dBiggest)
				uIdxPivot = i ;
			}
		unsigned uTemp = uPivots[k] ;
		uPivots[k] = uPivots[uIdxPivot] ;		// Store it!
		uPivots[uIdxPivot] = uTemp ;
		// And do actual swap, if needed
		if (uIdxPivot != k)
			{
			for (j=0; j < uDimension ; ++j)
				{
				double dTemp = A[k][j] ;
				A[k][j] = A[uIdxPivot][j] ;
				A[uIdxPivot][j] = dTemp ;
				}
			}


		double dPivot = A[k][k] ;
		if (dPivot == 0)
			{
			cout << "Cannot MatrixNN::factorizeLU(), A has become=" << A << endl ;
			return false ;			// Should do permutation code before this to try to avoid here...		
			}
		for (i=k+1; i < uDimension; ++i)
			{
			A[i][k] = A[i][k] / A[k][k] ;
		
			for (j=i+1; j < uDimension; ++j)
				{
				A[i][j] = A[i][j] - ( A[i][k] * A[k][j] );
				}	// end of j sub cols
			
			}	// end of i rows
		
		} // end of k cols

	return true ;
}

// ---------------------------------------------------------------------------

/*
 * solveLU() - Do an actual LU solve on this Matrix.  This is for the case
 *		where this matrix A is a set of linear equations such that Ax = b
 *		and we want to solve for x.
 *
 *		Returns true on success, false otherwise.  Length of b is assumed to
 *			be allocated to same as dimension of matrix as is length of x
 */
bool MatrixNN::solveLU(double b[], double x[])
{
	MatrixNN &A = (*this) ;	// Easier to work with...
	
	double *dB = NULL ;
	unsigned *uPivots = NULL ;
	dB = new double [uDimension] ;	// Alloc array of orig values of b
	if (dB == NULL)
		return false ;
	uPivots = new unsigned [uDimension] ;	// Alloc pivot array
	if (uPivots == NULL)
		{
		delete [] dB ;
		return false ;
		}
	// Factorize A in to an L and U matrix, that is stored back on top of what A was...
	// Return false if factorization failed...
	bool bFac = A.factorizeLU(uPivots) ;
	if (!bFac)
		{
		delete [] dB ;
		delete [] uPivots ;
		return bFac ;
		}

	unsigned i, j ;

	// First swap b array as needed.
	for (j=0; j < uDimension; ++j)
		dB[j] = b[ uPivots[j] ] ;	// get proper value into right spot based on pivot permutations done.
	// dB is now the "real" B array we use for solving...

	// First solve for y in Ly = b
	for (i=1; i < uDimension; ++i)
		{
		for (j=0; j <= i-1; ++j)
			{
			dB[i] = dB[i] - (A[i][j] * dB[j]) ; 
			}
		}

	// Then solve for x in Ux = y
	unsigned n = uDimension-1 ;
	dB[n] = dB[n] / A[n][n] ;

	for (i=n-1; i >= 0; --i)
		{
		for (j=i+1; j < uDimension; ++j)
			{
			dB[i] = dB[i] - ( A[i][j] * dB[j] / A[i][i] ) ;
			}
		if (i == 0)		// hack.  Since i is unsigned i will NEVER be < 0 above!
			break ;
		}

	// Now copy final output, ie: dPB to real X user passed in...
	for (i=0; i < uDimension; ++i)
		x[i] = dB[i] ;

	delete [] dB ;
	delete [] uPivots ;

	return true ;
}


// ---------------------------------------------------------------------------


