// ---------------------------------------------------------------------------
// poseDeformer.h - C++ File
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
//	1.11 - 08/05/05 - mcomet - Now has user Scale settings.
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

#include <maya/MPoint.h>
#include <maya/MPointArray.h>
#include <maya/MVector.h>
#include <maya/MVectorArray.h>
#include <maya/MMatrix.h>
#include <maya/MTransformationMatrix.h>
#include <maya/MItGeometry.h>

#include <maya/MDagPath.h>
#include <maya/MFnDagNode.h>
#include <maya/MFnDependencyNode.h>

#include <maya/MFnMesh.h>
#include <maya/MFnNurbsSurface.h>


// ---------------------------------------------------------------------------

typedef enum { eBlendAdditive, eBlendNormalize, eBlendRBF } EBLENDMODE ;
typedef enum { eSpaceJoint, eSpacePose} eSPACEMODE ;

// ---------------------------------------------------------------------------


/*
 * poseDeformer - Class Definition
 */
class poseDeformer : public MPxDeformerNode
{
public:
	poseDeformer();
	virtual	~poseDeformer();

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
	static MObject		aInputSettings ;		// Cmpd input settings
	static MObject		aBlendMode ;			// Do normal additive, or try to normalize weights if >1.0?
	static MObject		aIsolate ;				// Turn on a specific pose weight 100% for editing/viewing.
	static MObject		aRBFWidth ;				// Width of blending for Radial Basis Function/LU Factorization
	static MObject		aDeformSpace ;			// Deform relative to joint space or poseReader space?
	static MObject		aUserScale ;			// Cmpd scale of how much to alter output by
	static MObject		aUserScaleX ;			// X
	static MObject		aUserScaleY ;			// Y
	static MObject		aUserScaleZ ;			// Z

	static MObject		aInputData ;			// Cmpd input data
	static MObject		aWorldMatrix ;			// Array of world matrix's for each live skin infl object.  Matches skinCluser "matrix" attr.
	
	static MObject		aPose ;					// Cmpd array of poses
	static MObject		aPoseName ;				// String for user defined name for pose
	static MObject		aPoseWeight ;			// Weight of the pose currently
	static MObject		aPoseActive ;			// True if this pose is not currently deleted.

	static MObject		aPoseXForm ;			// Cmpd Array of Data of point for each xform that created this pose.
	static MObject		aPoseXFormStr ;			// Relative str of the given transform for this pose.  Usually 1/#posexforms when the pose was made
	static MObject		aPoseXFormIdx ;			// Index of which infl the data is stored for.
	static MObject		aPoseXFormWorldMatrix ; // World Matrix for this poseReader
	static MObject		aPoseXFormReadAxis ;	// What axis this poseReader is using...
	static MObject		aPoseXFormNumPts ;		// Num of Delta Pts

	static MObject		aPoseDelta ;			// Cmpd array delta data for the pose one for each pt
	static MObject		aPoseDeltaX ;			// What was change in X relative to this matrix?
	static MObject		aPoseDeltaY ;			// What was change in Y relative to this matrix?
	static MObject		aPoseDeltaZ ;			// What was change in Z relative to this matrix?

private:
	MStatus readMatrixArray(MDataBlock& data, MString name, MMatrix **matPtr, unsigned &uMat) ;
	MStatus readPoseWeights(MDataBlock& data, MString name, unsigned &uPoses, MDoubleArray &dArrWts, 
					int nIsolate, MVectorArray &vArrRead, MVector &vCur,
					const MMatrix *matArr ) ;
	MStatus normalizeWeights(MDoubleArray &dArrWts) ;
	double smoothStep(const double &dVal);
	double smoothGaussian(const double &dVal) ;

	MStatus interpWeights(MDoubleArray &dArrWts, 
		const MVectorArray &vArrRead,
		const MVector &vCur, const double &dRBFWidth) ;

} ;


// ---------------------------------------------------------------------------

void cartesianToSpherical(const double &x, const double &y, const double &z, 
						double &r, double &phi, double &theta) ;
void sphericalToCartesian(const double &r, const double &phi, const double &theta, 
						double &x, double &y, double &z) ;

void cartesianToCylindrical(const double &x, const double &y, const double &z, 
						double &r, double &theta, double &zc) ;
void cylindricalToCartesian(const double &r, const double &theta, const double &zc, 
						double &x, double &y, double &z) ;

void cartesianToPolar(const double &x, const double &y,
						double &r, double &theta) ;
void polarToCartesian(const double &r, const double &theta,
						double &x, double &y) ;

double arclength(const double &r, const double &theta) ;

void cartesianToSphericalUV(const double &x, const double &y, const double &z,
							double &u, double &v) ;

inline double dist2D(const double &x, const double &y) ;

#define PI			3.14159
#define HALFPI		1.570795


