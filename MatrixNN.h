// ---------------------------------------------------------------------------
// MatrixNN.h - C++ Header File
// Copyright ©2004 Michael B. Comet
// ---------------------------------------------------------------------------
//
// DESCRIPTION:
//	A plugin class to do stuff with square NxN matrices.
//
// AUTHOR:
//	Michel B. Comet - comet@comet-cartoons.com
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

#ifndef __MATRIXNN_H
#define __MATRIXNN_H

/*
 * Includes
 */ 
#include <math.h>
#include <ostream>
#include <iostream>
#include <fstream>
using namespace::std ;

// ---------------------------------------------------------------------------


/*
 * MatrixNN - Class Definition
 *
 *	i=row j=col
 *			 j=0  j=1  j=2  ... j=n
 *		i=0
 *		i=1
 *		i=2
 *		...
 *		i=n
 */
class MatrixNN
{
public:
	MatrixNN();
	virtual	~MatrixNN();

public:
	double *ptrDbl ;			// ptr to actual data of double.
	unsigned uDimension ;		// For the NxN matrix, what size is N?
	void free(void) ;			// Free any alloced memory
	unsigned setDimension(unsigned uDim) ;		// Set the array size/alloc it.
	unsigned getDimension(void) ;	// What is size?
	void fill(double dVal=0) ;		// Fill array to values Default=0
	inline unsigned getIndex(unsigned i, unsigned j) ;	// Get index into array for i,j
	void identity(void) ;			// Set to identity matrix.

	double* operator[](unsigned i) ;
	const double* operator[](unsigned i) const ;
	double& operator()(unsigned i, unsigned j) ;
	double operator()(unsigned i, unsigned j) const ;
	friend ostream& operator<<(ostream &os, const MatrixNN &matNN) ;
	MatrixNN& operator=(const MatrixNN &matNN) ;
	bool factorizeLU(unsigned uPivots[]) ;
	bool solveLU(double b[], double x[]) ;
} ;


// ---------------------------------------------------------------------------

#endif // end of __MATRIXNN_H
