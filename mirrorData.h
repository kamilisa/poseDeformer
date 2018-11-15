// ---------------------------------------------------------------------------
// mirrorData.h - C++ File
// Copyright ©2004 Michael B. Comet
// ---------------------------------------------------------------------------
//
// DESCRIPTION:
//	The poseDeformer deformer plugin definition header file
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
#include <maya/MPxDeformerNode.h> 
#include <maya/MGlobal.h> 
#include <maya/MTypeId.h> 
#include <maya/MStatus.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>

#include <maya/MFnNumericAttribute.h>
#include <maya/MFnMatrixAttribute.h>
#include <maya/MFnMatrixData.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnGenericAttribute.h>
#include <maya/MFnTypedAttribute.h>

#include <maya/MFnStringData.h>
#include <maya/MString.h> 

#include <maya/MMatrix.h>
#include <maya/MVector.h>
#include <maya/MPoint.h>
#include <maya/MPointArray.h>
#include <maya/MItGeometry.h>
#include <maya/MFnDependencyNode.h>


// ---------------------------------------------------------------------------


/*
 * mirrorData - Class Definition
 */
class mirrorData : public MPxDeformerNode
{
public:
	mirrorData();
	virtual	~mirrorData();

    static  void*	creator();
	static  MStatus	initialize();

	// deformation function
	//
	virtual MStatus	deform(MDataBlock& block,
			   MItGeometry& 	iter,
			   const MMatrix& 	mat,
			   unsigned int		multiIndex);

public:
	static  MTypeId		id;

public:
	// Local functions

public:
	// local node attributes
	static MObject	aMirrorIndex ;			// Index of matching vert.
	static MObject	aThreshold ;			// How much to allow off..
	static MObject	aMirrorAxis ;			// What axis to read mirroring for?
	static MObject	aMirrorSpace ;				// What coordinate space to calc in

private:

} ;


// ---------------------------------------------------------------------------
