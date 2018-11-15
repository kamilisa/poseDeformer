// ---------------------------------------------------------------------------
// mirrorData.cpp - C++ File
// Copyright ©2004 Michael B. Comet
// ---------------------------------------------------------------------------
//
// DESCRIPTION:
//	The mirrorData deformer plugin.  This is a little node that should be 
//		added below any deformer geo to read the base mesh of an object.
//		It then stores matching index info for each vertex.  That way
//		say mirrorIdx[3] = 12  if the .vtx[3] is mirrored from .vtx[12] and so on...
//		Doesn't actually do any deformation.
//
// AUTHOR:
//	Michel B. Comet - comet@comet-cartoons.com
//
// VERSIONS:
//	09/22/04 - comet - Initial Rev
//	1.03 - 10/05/04 - mcomet - Fixed so does entire loop of all points even
//				if not in membership, so that targets work and ed-mem works.
//				Also removed MProgressWindow since seemed buggy.
//	1.04 - 10/07/04 - mcomet - Made it so now much faster at generating poses.
//	1.05 - 10/15/04 - mcomet - Now has new RBF interpolation mode.
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
//  mirrorData - Pose Space Deformer Maya Plugin by Michael B. Comet
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
//   For information on mirrorData contact:
//			Michael B. Comet - comet@comet-cartoons.com
//			or visit http://www.comet-cartooons.com/toons/
//
// --------------------------------------------------------------------------



// ---------------------------------------------------------------------------

/*
 * Includes
 */
#include <string.h>
#include <fstream>
#include <iostream>
#include <math.h>

#include "mirrorData.h" 
#include "plugin.h" 
#include "MatrixNN.h"

// ---------------------------------------------------------------------------

/*
 * For local testing of nodes you can use any identifier between 
 * 0x00000000 and 0x0007ffff, but for any node that you plan to use 
 * for more permanent purposes, you should get a universally unique id 
 * from Alias|Wavefront Assist. You will be assigned a unique range 
 * that you can manage on your own.
 */
MTypeId mirrorData::id( ID_MIRRORDATA );

/*
 * static attributes
 */
MObject	mirrorData::aMirrorIndex ;			// Index of matching vert on other side
MObject	mirrorData::aThreshold ;			// How much to allow off..
MObject	mirrorData::aMirrorAxis ;			// What axis to mirror for
MObject	mirrorData::aMirrorSpace ;			// What coordinate space to calc in


// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//	Main Maya Plugin Functions
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------

/*
 * mirrorData::creator() - Proc to allocated a new node
 */
void* mirrorData::creator()
{
    return new mirrorData();
}

// ---------------------------------------------------------------------------


/*
 * mirrorData::mirrorData() - constructor
 */
mirrorData::mirrorData() 
{

}

// ---------------------------------------------------------------------------

/*
 * mirrorData::~mirrorData() - Destructor
 */
mirrorData::~mirrorData()
{
}

// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//	Attr, Compute and Deform Functions
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------



/*
 * mirrorData::initialize() - Set up our attrs
 *
 * By default, attributes are: 
 * ---------------------------
 * readable 
 * writable 
 * connectable 
 * storable 
 * cached 
 * not arrays 
 * have indices that matter 
 * do not use an array builder 
 * not keyable 
 * not hidden 
 * not used as colors 
 * not indeterminant 
 * set to disconnect behavior kNothing 
 *
 */
MStatus mirrorData::initialize()
{ 
    MStatus stat;
    
    // local attribute initialization
    MFnNumericAttribute nAttr ;
    MFnMatrixAttribute mAttr ;
    MFnCompoundAttribute cAttr ;
    MFnEnumAttribute eAttr ;
    MFnGenericAttribute gAttr ;
	MFnTypedAttribute tAttr ;

	// Make Attributes
	//
	aMirrorIndex = nAttr.create("mirrorIndex", "midx", MFnNumericData::kInt, 0) ;
	nAttr.setArray(true) ;
	nAttr.setUsesArrayDataBuilder(true) ;
	nAttr.setStorable(true) ;

	aThreshold = nAttr.create("threshold", "thr", MFnNumericData::kDouble, 0.0001) ;
	nAttr.setMin(0.0) ;
	nAttr.setKeyable(true) ;
	
	aMirrorAxis = eAttr.create("mirrorAxis", "axi", 0) ;
	eAttr.addField("X-Axis", 0) ;
	eAttr.addField("Y-Axis", 1) ;
	eAttr.addField("Z-Axis", 2) ;
	eAttr.setKeyable(true) ;

	aMirrorSpace = eAttr.create("mirrorSpace", "spc", 0) ;
	eAttr.addField("Object", 0) ;
	eAttr.addField("World", 1) ;
	eAttr.setKeyable(true) ;

	// Add Attrs
	stat = addAttribute( aMirrorIndex );
		MERR(stat, "Cannot add attribute aMirrorIndex.") ;
	stat = addAttribute( aThreshold );
		MERR(stat, "Cannot add attribute aThreshold.") ;
	stat = addAttribute( aMirrorAxis );
		MERR(stat, "Cannot add attribute aMirrorAxis.") ;
	stat = addAttribute( aMirrorSpace );
		MERR(stat, "Cannot add attribute aMirrorSpace.") ;

	// Affect them 
    attributeAffects( aThreshold, outputGeom );
    attributeAffects( aMirrorAxis, outputGeom );
    attributeAffects( aMirrorSpace, outputGeom );
	
	return MS::kSuccess;
}


// ---------------------------------------------------------------------------

/*
 * mirrorData::deform() - Main deform stuff
 *
 * Method: deform
 *
 * Description:   Deform the point with a squash algorithm
 *
 * Arguments:
 *   data		: the datablock of the node
 *   iter		: an iterator for the geometry to be deformed
 *   m    		: matrix to transform the point into world space
 *   multiIndex	        : the index of the geometry that we are deforming
 *
 */
MStatus mirrorData::deform( MDataBlock& data, MItGeometry& iter, const MMatrix& matWorld,	
						unsigned int multiIndex)
{
    MStatus stat;

	MMatrix invmatWorld = matWorld.inverse() ;

    MObject thisNode = thisMObject();
    MString name = MFnDependencyNode(thisNode).name() ;

    // Envelope data from the base class.
    // The envelope is simply a scale factor.
    //
    MDataHandle hEnvelope = data.inputValue(envelope, &stat);
    if (MS::kSuccess != stat)  return stat;
    float fEnv = hEnvelope.asFloat();	
	if (fEnv <= 0.0)			// if off...done!
		return MS::kSuccess; 

	// Get other data...
	//
    MDataHandle hThreshold = data.inputValue( aThreshold, &stat );
    double dThreshold = hThreshold.asDouble();	

    MDataHandle hMirrorAxis = data.inputValue( aMirrorAxis, &stat );
    int nMirrorAxis = hMirrorAxis.asShort();	

	MDataHandle hMirrorSpace = data.inputValue( aMirrorSpace, &stat );
    int nMirrorSpace = hMirrorSpace.asShort();	
 
    
	unsigned uPts = iter.count() ;		// How many?
	MPointArray ptArr ;
	ptArr.setLength( uPts );			// Set array to hold pos's

	// PASS 1, get pos's
	unsigned uPtIdx = 0;
	for ( iter.reset(); !iter.isDone() ; iter.next() ) 
        {
		uPtIdx = iter.index() ;	// Do THIS since we actually store pose data this way...
	
		MPoint pt = iter.position();
		if (nMirrorSpace == 1)
			pt *= matWorld ;

			// For nurbs, we may have 5 pts, but index might be up to 8, so grow if needed.
		if (uPtIdx >= uPts)
			{
			uPts = uPtIdx + 1 ;
			ptArr.setLength( uPts ) ;
			}

		ptArr[uPtIdx] = pt ;		// Store pos
		
		} // end of iter


	// Get plug to array of output
	MPlug plugArrIdx(thisNode, aMirrorIndex) ;

	// PASS 2 - Go and find matching verts and store.
	unsigned u;
	for (u=0; u < uPts; ++u)
		{
		double dClosest = 100000000;	// start with something big
		unsigned uClosest = u ;
		
		MPoint ptUMirror = ptArr[u] ;
		if (nMirrorAxis == 0)
			ptUMirror.x *= -1 ;		// pretend it is on other side.
		else if (nMirrorAxis == 1)
			ptUMirror.y *= -1 ;		// pretend it is on other side.
		else
			ptUMirror.z *= -1 ;		// pretend it is on other side.

		// Find closest vert
		unsigned v;
		for (v=0; v < uPts; ++v)
			{
			MVector vDelta = ptUMirror - ptArr[v] ;
			double dDist = vDelta.length() ;
			if (dDist < dClosest)
				{
				dClosest = dDist ;
				uClosest = v ;
				}

			} // end of v loop
		
		// Now if the closest point is still too far off, we will just keep data as
		// the original point...so that it won't change.
		if (dClosest > dThreshold)
			uClosest = u ;

		// So get plug to set
		MPlug plugIdx = plugArrIdx.elementByLogicalIndex( u ) ;
		plugIdx.setValue( (int)uClosest ) ;		// and set it

		} //end of u loop


	return MS::kSuccess ;   // if we got this far, return success.
}

// ---------------------------------------------------------------------------

