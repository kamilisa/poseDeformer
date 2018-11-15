// ---------------------------------------------------------------------------
// poseDeformer.cpp - C++ File
// Copyright ©2004 Michael B. Comet
// ---------------------------------------------------------------------------
//
// DESCRIPTION:
//	The poseDeformer deformer plugin
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



// ---------------------------------------------------------------------------

/*
 * Includes
 */
#include <string.h>
#include <fstream>
#include <iostream>
#include <math.h>

#include "poseDeformer.h" 
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
MTypeId poseDeformer::id( ID_POSEDEFORMER );

/*
 * static attributes
 */
MObject	poseDeformer::aInputSettings ;		// Cmpd input settings
MObject	poseDeformer::aBlendMode ;			// Do normal additive, or try to normalize weights if >1.0?
MObject	poseDeformer::aIsolate ;			// Turn on a specific pose weight 100% for editing/viewing.
MObject	poseDeformer::aRBFWidth ;			// Width of blending for Radial Basis Function/LU Factorization
MObject	poseDeformer::aDeformSpace ;		// Deform relative to joint space or poseReader space?
MObject	poseDeformer::aUserScale ;			// Cmpd scale of how much to alter output by
MObject	poseDeformer::aUserScaleX ;			// X
MObject	poseDeformer::aUserScaleY ;			// Y
MObject	poseDeformer::aUserScaleZ ;			// Z


MObject	poseDeformer::aInputData ;			// Cmpd input data
MObject	poseDeformer::aWorldMatrix ;		// Array of world matrix's for each live skin infl object.  Matches skinCluser "matrix" attr.

MObject	poseDeformer::aPose ;				// Cmpd array of poses
MObject poseDeformer::aPoseName ;			// String for user defined name for pose
MObject	poseDeformer::aPoseWeight ;			// Weight of the pose
MObject	poseDeformer::aPoseActive;			// True if this pose is not currently deleted.
MObject	poseDeformer::aPoseXForm ;			// Cmpd Array of Data of point for each xform that created this pose.
MObject	poseDeformer::aPoseXFormStr ;		// Relative str of the given transform for this pose.  Usually 1/#posexforms when the pose was made
MObject	poseDeformer::aPoseXFormIdx ;		// Index of which infl the data is stored for.
MObject	poseDeformer::aPoseXFormWorldMatrix ; // World Matrix for this poseReader
MObject poseDeformer::aPoseXFormReadAxis ;	// What axis this poseReader is using...
MObject	poseDeformer::aPoseXFormNumPts ;		// Num of Delta Pts
MObject	poseDeformer::aPoseDelta ;			// Cmpd array delta points for the pose one for each pt
MObject	poseDeformer::aPoseDeltaX ;			// What was change in X relative to this matrix?
MObject	poseDeformer::aPoseDeltaY ;			// What was change in Y relative to this matrix?
MObject	poseDeformer::aPoseDeltaZ ;			// What was change in Z relative to this matrix?


// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//	Main Maya Plugin Functions
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------

/*
 * poseDeformer::creator() - Proc to allocated a new node
 */
void* poseDeformer::creator()
{
    return new poseDeformer();
}

// ---------------------------------------------------------------------------


/*
 * poseDeformer::poseDeformer() - constructor
 */
poseDeformer::poseDeformer() 
{

}

// ---------------------------------------------------------------------------

/*
 * poseDeformer::~poseDeformer() - Destructor
 */
poseDeformer::~poseDeformer()
{
}

// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//	Attr, Compute and Deform Functions
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------



/*
 * poseDeformer::initialize() - Set up our attrs
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
MStatus poseDeformer::initialize()
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
	aRBFWidth = nAttr.create("avgPoseSepRBF", "rbfw", MFnNumericData::kDouble, 45) ;
	nAttr.setMin(0.001) ;
	nAttr.setMax(180.0) ;
	nAttr.setKeyable(true) ;

	aBlendMode = eAttr.create( "blendMode", "bmod", 2 );
	eAttr.setKeyable(true);
	eAttr.addField("Additive", eBlendAdditive) ;
	eAttr.addField("Normalize", eBlendNormalize) ;
	eAttr.addField("RBF-LUFactorize", eBlendRBF) ;

	aDeformSpace = eAttr.create( "deformSpace", "dspc", 0 );
	eAttr.setKeyable(true);
	eAttr.addField("joint-space", eSpaceJoint ) ;
	eAttr.addField("pose-space", eSpacePose ) ;

	aIsolate = nAttr.create("isolate", "iso", MFnNumericData::kInt, 0) ;
	nAttr.setMin(0) ;
	nAttr.setKeyable(true) ;

	aUserScaleX = nAttr.create("userScaleX", "usx", MFnNumericData::kDouble, 1.0 ) ;
	nAttr.setKeyable(true) ;
	aUserScaleY = nAttr.create("userScaleY", "usy", MFnNumericData::kDouble, 1.0 ) ;
	nAttr.setKeyable(true) ;
	aUserScaleZ = nAttr.create("userScaleZ", "usz", MFnNumericData::kDouble, 1.0 ) ;
	nAttr.setKeyable(true) ;

	aUserScale = cAttr.create( "userScale", "uscl") ;
    cAttr.addChild( aUserScaleX ) ;
    cAttr.addChild( aUserScaleY ) ;
    cAttr.addChild( aUserScaleZ ) ;


	aInputSettings = cAttr.create( "inputSettings", "inset") ;
    cAttr.addChild( aRBFWidth ) ;
    cAttr.addChild( aBlendMode ) ;
    cAttr.addChild( aDeformSpace ) ;
    cAttr.addChild( aIsolate ) ;
    cAttr.addChild( aUserScale ) ;


/*
    aMeshIn = fnGenericAttr.create( "meshIn", "mesh", &stat );
    fnGenericAttr.addAccept(MFnData::kNurbsSurface);
    fnGenericAttr.addAccept(MFnData::kMesh);
//    fnGenericAttr.addAccept(MFnData::kSubdSurface);
*/

	aWorldMatrix = mAttr.create("worldMatrix", "wm", MFnMatrixAttribute::kDouble) ;
	mAttr.setArray(true);
	mAttr.setUsesArrayDataBuilder(true);

	aInputData = cAttr.create( "inputData", "indat") ;
    cAttr.addChild( aWorldMatrix ) ;

	MFnStringData fnStr ;
	MObject oStr ;
	oStr = fnStr.create("");  // Set default value.
	aPoseName = tAttr.create("poseName", "pnm", MFnData::kString, oStr) ;

	aPoseWeight = nAttr.create("poseWeight", "pwt", MFnNumericData::kDouble, 0.0) ;
	aPoseActive = nAttr.create("poseActive", "pact", MFnNumericData::kBoolean, 1) ;

	aPoseXFormStr = nAttr.create("poseXFormStr", "pxstr", MFnNumericData::kDouble, 0.0) ;
	aPoseXFormIdx = nAttr.create("poseXFormIdx", "pxidx", MFnNumericData::kInt, -1) ;
	aPoseXFormWorldMatrix = mAttr.create("poseXFormWorldMatrix", "pxmat", MFnMatrixAttribute::kDouble) ;
	aPoseXFormReadAxis = eAttr.create( "poseXFormReadAxis", "pxaxi", 1) ;
	eAttr.addField("X-Axis", 0) ;
	eAttr.addField("Y-Axis", 1) ;
	eAttr.addField("Z-Axis", 2) ;
	aPoseXFormNumPts = nAttr.create("poseXFormNumPts", "pxnpt", MFnNumericData::kInt, 0) ;


	aPoseDeltaX = nAttr.create("poseDeltaX", "pdx", MFnNumericData::kDouble, 0.0) ;
	aPoseDeltaY = nAttr.create("poseDeltaY", "pdy", MFnNumericData::kDouble, 0.0) ;
	aPoseDeltaZ = nAttr.create("poseDeltaZ", "pdz", MFnNumericData::kDouble, 0.0) ;

	aPoseDelta = cAttr.create( "poseDelta", "pdlt") ;
	cAttr.setArray(true) ;
	cAttr.setUsesArrayDataBuilder(true) ;
    cAttr.addChild( aPoseDeltaX ) ;
    cAttr.addChild( aPoseDeltaY ) ;
    cAttr.addChild( aPoseDeltaZ ) ;

	aPoseXForm = cAttr.create( "poseXForm", "pxfm") ;
	cAttr.setArray(true) ;
	cAttr.setUsesArrayDataBuilder(true) ;
	cAttr.addChild( aPoseXFormStr ) ;
	cAttr.addChild( aPoseXFormIdx ) ;
	cAttr.addChild( aPoseXFormWorldMatrix ) ;
	cAttr.addChild( aPoseXFormReadAxis ) ;
	cAttr.addChild( aPoseXFormNumPts ) ;
	cAttr.addChild( aPoseDelta ) ;


	aPose = cAttr.create( "pose", "pos") ;
	cAttr.setArray(true) ;
	cAttr.setUsesArrayDataBuilder(true) ;
	cAttr.addChild( aPoseName ) ;
	cAttr.addChild( aPoseWeight ) ;
	cAttr.addChild( aPoseXForm ) ;
	cAttr.addChild( aPoseActive ) ;


	// Add Attrs
	stat = addAttribute( aInputSettings );
		MERR(stat, "Cannot add attribute aInputSettings.") ;
	stat = addAttribute( aInputData );
		MERR(stat, "Cannot add attribute aInputData.") ;
	stat = addAttribute( aPose );
		MERR(stat, "Cannot add attribute aPose.") ;

	// Affect them 
    attributeAffects( aInputSettings, outputGeom );
    attributeAffects( aInputData, outputGeom );
    attributeAffects( aPose, outputGeom );
	
	return MS::kSuccess;
}


// ---------------------------------------------------------------------------

/*
 * poseDeformer::deform() - Main deform stuff
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
MStatus poseDeformer::deform( MDataBlock& data, MItGeometry& iter, const MMatrix& matWorld,	
						unsigned int multiIndex)
{
    MStatus stat;


	MMatrix invmatWorld = matWorld.inverse() ;

    MObject thisNode = thisMObject();
    MString name = MFnDependencyNode(thisNode).name() ;

	MDataHandle hNodeState = data.inputValue( state, &stat) ;
	int nNodeState = hNodeState.asShort() ;
	if (nNodeState == 1)
		return MS::kSuccess ;

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
    MDataHandle hBlendMode = data.inputValue( aBlendMode, &stat );
    int nBlendMode = hBlendMode.asShort();	

    MDataHandle hDeformSpace = data.inputValue( aDeformSpace, &stat );
    int nDeformSpace = hDeformSpace.asShort();	
	
    MDataHandle hIsolate = data.inputValue( aIsolate, &stat );
    int nIsolate = hIsolate.asInt();	

    MDataHandle hRBFWidth = data.inputValue( aRBFWidth, &stat );
    double dRBFWidth = hRBFWidth.asDouble() / 180.0 ;		// We want 0-1, but are giving user 0-180 to be nice.

    MDataHandle hUserScaleX = data.inputValue( aUserScaleX, &stat );
    double dUserScaleX = hUserScaleX.asDouble() ;
    MDataHandle hUserScaleY = data.inputValue( aUserScaleY, &stat );
    double dUserScaleY = hUserScaleY.asDouble() ;
    MDataHandle hUserScaleZ = data.inputValue( aUserScaleZ, &stat );
    double dUserScaleZ = hUserScaleZ.asDouble() ;


	MMatrix *matArr=NULL ;		// Array of matrices for each input influence transform.
	unsigned uMat = 0 ;			// How big is array?
    stat = readMatrixArray(data, name, &matArr, uMat) ;

	// Make sure we had an array of matrices to read...
	if (stat != MS::kSuccess || uMat == 0 || matArr == NULL)
		return MS::kSuccess ;

	unsigned uPoses ;
	MDoubleArray dArrWts ;
	MVectorArray vArrRead ;
	MVector vCur ;
	stat = readPoseWeights(data, name, uPoses, dArrWts, nIsolate, vArrRead, vCur, matArr ) ;



	// Make sure we even had poses made...
	if (stat != MS::kSuccess || uPoses == 0)
		{
		if (matArr != NULL)
			{
			delete [] matArr ;
			matArr = NULL ;
			}
		return MS::kSuccess ;
		}

	if (nBlendMode == eBlendNormalize && nIsolate == 0 )
		{
		normalizeWeights(dArrWts) ;
		}
	else if (nBlendMode == eBlendRBF && nIsolate == 0 )
		{
		interpWeights(dArrWts, vArrRead, vCur, dRBFWidth) ;
		}

// cout << name << ": dArrWts="<< dArrWts <<endl ;


    // iterate through each point in the geometry
    //
	unsigned uPtIdx = 0;
	for ( iter.reset(); !iter.isDone() ; iter.next() ) 
        {
		uPtIdx = iter.index() ;	// Do THIS since we actually store pose data this way...

#if DEBUG > 0
		cout << "DEBUG: pt [" << uPtIdx << "/" << iter.count() << "]" << endl ;
#endif
		
		MPoint pt = iter.position();
		MPoint ptWorld = pt * matWorld ;
		MPoint ptDef = ptWorld ;

		// What is user assigned weighting for this point?
		float fWt = weightValue(data, multiIndex, iter.index());
		if ( fWt <= 0.0 )	// if this pt isn't affected at all...just ignore it!
			continue ;


		// poseDeformer algorithm ******************************************
	    //
		
		// Get handle to main pose cmpd array...
		MArrayDataHandle hArrCmpdPose = data.inputArrayValue( aPose, &stat) ;
		if (stat != MS::kSuccess)
			continue ;
		
		// For this point...go thru each pose
		do
			{
			MDataHandle hCmpdPose = hArrCmpdPose.inputValue(&stat) ;		// Get compound element item.
			if (stat != MS::kSuccess)
				continue ;

			unsigned uPoseIdx = hArrCmpdPose.elementIndex(&stat) ;		// What index'd pose are we looking at?

			double dPoseWt = dArrWts[uPoseIdx] ;		// Get stored weight for that pose

			if (dPoseWt == 0.0)		// If weight is zero, nothing to do anyhow...
				continue ;
			

			// And in each pose, go thru each transform that generated it...
			MArrayDataHandle hArrCmpdPoseXForm = hCmpdPose.child( aPoseXForm ) ;	// Get cmpd array of xforms in pose
			do
				{


				MDataHandle hCmpdPoseXForm  = hArrCmpdPoseXForm.inputValue(&stat) ;	// Get xform element
				if (stat != MS::kSuccess)
					continue ;

				MDataHandle hPoseXFormStr = hCmpdPoseXForm.child( aPoseXFormStr ) ;	// Get relative str of this xform in the pose
				double dPoseXFormStr = hPoseXFormStr.asDouble() ;
				if (dPoseXFormStr == 0.0)		// If weight is zero, nothing to do anyhow...
					continue ;

				MDataHandle hPoseXFormIdx = hCmpdPoseXForm.child( aPoseXFormIdx ) ;	// Get index of what matrix to use
				int nMatIdx = hPoseXFormIdx.asInt() ;
				if (nMatIdx < 0)		// Make sure a valid index.
					continue ;

				MDataHandle hPoseXFormWorldMatrix = hCmpdPoseXForm.child( aPoseXFormWorldMatrix ) ;	// Get poseReader matrix for this pose/xform combo
				MMatrix matReader = hPoseXFormWorldMatrix.asMatrix();


				// And within each Transform, read the point data it stored....
				MArrayDataHandle hArrCmpdPoseDelta = hCmpdPoseXForm.child( aPoseDelta ) ;	// Get delta cmpd array in pose
				stat = hArrCmpdPoseDelta.jumpToElement(uPtIdx) ;	// Go to proper index for this point.
				if (stat != MS::kSuccess)
					continue ;

				MDataHandle hCmpdPoseDelta = hArrCmpdPoseDelta.inputValue(&stat) ;	// Get cmpd element of delta data
				if (stat != MS::kSuccess)
					continue ;

				MDataHandle hPoseDeltaX = hCmpdPoseDelta.child( aPoseDeltaX ) ;	
				MDataHandle hPoseDeltaY = hCmpdPoseDelta.child( aPoseDeltaY ) ;	
				MDataHandle hPoseDeltaZ = hCmpdPoseDelta.child( aPoseDeltaZ ) ;	
				double dDeltaX = hPoseDeltaX.asDouble() ;
				double dDeltaY = hPoseDeltaY.asDouble() ;
				double dDeltaZ = hPoseDeltaZ.asDouble() ;

				// At this point we have the point we are deforming in the main loop here...
				// We've got the pose weight for the pose we are calculating....
				// We know which matrix index the offset is for and the relative weight of the transform
				//		for how much it in particular influences the overal pose against other transforms in the same pose.
				// So...basic calc is make a ptVector which is in the given matrix space,
				//		and mult it by the world matrix, this will put the vector into
				//		world space.  Then tack on this world offset to the point based on weight.
				//
				MVector vDelta( dDeltaX, dDeltaY, dDeltaZ) ;
				// FINDME!
				if (nDeformSpace == (int)eSpaceJoint)		// Joint space
					vDelta = vDelta * matArr[nMatIdx] ;			// Mult by proper matrix
				else if (nDeformSpace == (int)eSpacePose)	// Pose space
					vDelta = vDelta * matReader ;				// Mult by proper matrix

					// Take into acct user scale...this effectively increases or decreases
					// the stored "offset" so that if the rig is scaling, we can dampend down
					// or raise up as needed.  In World Space!
				vDelta.x *= dUserScaleX ;
				vDelta.y *= dUserScaleY ;
				vDelta.z *= dUserScaleZ ;

					// Now offset point by amount of this pose.  We base change both on actual live weight
					//	as well as stored xform strength.  This is typically is 1/#infl that made the pose,
					//	so that if 3 influences created this, each one would do 1/3 of the work to make the 
					//	final pose.
					//
				ptDef = ptDef + ( (dPoseWt * dPoseXFormStr) * vDelta ) ;

//				cout << name << ": nMatIdx=" << nMatIdx << " dPoseXFormStr=" << dPoseXFormStr << " dPoseWt=" << dPoseWt << endl ;
				
				} while (hArrCmpdPoseXForm.next()) ;	// Go thru each XForm in pose


			} while (hArrCmpdPose.next()) ;		// End of each pose

	

	    //
	    // end of poseDeformer algorithm ************************************


		// Do final blending on point between what is deformed, and original based on Maya deformer weight and envelope.
		double dPct = fWt * fEnv ;
		ptDef = ((1.0-dPct) * ptWorld) + (dPct * ptDef);

		ptDef = ptDef * invmatWorld ;		// Back to local space
		iter.setPosition( ptDef ) ;

	    } // end of iter


	// Free any alloced sutff
	//
	if (matArr != NULL)
		{
		delete [] matArr ;
		matArr = NULL ;
		}

	return MS::kSuccess ;   // if we got this far, return success.
}

// ---------------------------------------------------------------------------

/*
 * poseDeformer::readMatrixArray() - Reads in the matrix array of data of all infl matrices
 *		Sets a ptr to the allocated matrices, and sets uMat to number alloced.
 */
MStatus poseDeformer::readMatrixArray(MDataBlock& data, MString name, MMatrix **matArr, unsigned &uMat)
{
	MStatus stat ;
	
	MMatrix *matPtr = NULL ;

	*matArr = NULL ;
	uMat = 0 ;

	MArrayDataHandle hArrWorldMatrix = data.inputArrayValue( aWorldMatrix, &stat) ;
		MERR(stat, name+MString(": Cannot obtain worldMatrix array data handle.")) ;
	
	// First pass, go thru all of the entries and see what highed logical index is..
	do
		{
		unsigned uIdx = hArrWorldMatrix.elementIndex(&stat) ;
		if (stat != MS::kSuccess)
			continue ;
		if (uIdx >= uMat)
			uMat = uIdx + 1 ;

		} while (hArrWorldMatrix.next()) ;

	// Alloc Matrix array
	if (uMat == 0)
		return MS::kFailure ;		// nothing to alloc!


	matPtr = new MMatrix [uMat] ;
	if (matPtr == NULL)
		return MS::kFailure ;		// out of memory.
	*matArr = matPtr ;

		// RESET
	hArrWorldMatrix = data.inputArrayValue( aWorldMatrix, &stat) ;

	// Second pass, actually read in each matrix...
	unsigned uIdx ;
	for (uIdx=0; uIdx < uMat; ++uIdx)
		{
		stat = hArrWorldMatrix.jumpToElement( uIdx ) ;
		if (stat != MS::kSuccess)
			continue ;

		MDataHandle hEleWorldMatrix = hArrWorldMatrix.inputValue(&stat) ;
		if (stat != MS::kSuccess)
			continue ;

			// Store it in the array!
		matPtr[uIdx] = hEleWorldMatrix.asMatrix();	
		}

	return MS::kSuccess ;
}

// ---------------------------------------------------------------------------

/*
 * readPoseWeights()  - Reads weights for all the poses in
 */
MStatus poseDeformer::readPoseWeights(MDataBlock& data, MString name, 
			unsigned &uPoses, MDoubleArray &dArrWts, int nIsolate,
			MVectorArray &vArrRead, MVector &vCur, const MMatrix *matArr )
{
	MStatus stat ;
    uPoses = 0 ;

	// Get handle to main pose cmpd array...
	MArrayDataHandle hArrCmpdPose = data.inputArrayValue( aPose, &stat) ;
	if (stat != MS::kSuccess)
		return stat ;

	// Pass 1 see how many
	do
		{
		unsigned uIdx = hArrCmpdPose.elementIndex() ;
		if (stat != MS::kSuccess)
			continue ;
		if (uIdx >= uPoses)
			uPoses = uIdx + 1 ;

		} while (hArrCmpdPose.next()) ;		// End of each pose

	if (uPoses == 0)
		return MS::kFailure ;		// nothing to alloc!

	// Alloc
	stat = dArrWts.setLength(uPoses) ;
	stat = vArrRead.setLength(uPoses) ;

	// RESET
	hArrCmpdPose = data.inputArrayValue( aPose, &stat) ;

	// Second pass, actually read in each weight...
	unsigned uIdx ;
	for (uIdx=0; uIdx < uPoses; ++uIdx)
		{
		stat = hArrCmpdPose.jumpToElement( uIdx ) ;
		if (stat != MS::kSuccess)
			continue ;
		
		MDataHandle hCmpdPose = hArrCmpdPose.inputValue(&stat) ;		// Get compound element item.
		if (stat != MS::kSuccess)
			continue ;

		MDataHandle hPoseActive = hCmpdPose.child( aPoseActive ) ;	// See if pose is even active
		bool bPoseActive = hPoseActive.asBool() ;

		if (!bPoseActive)
			{
			dArrWts[uIdx] = 0.0 ;
			// Had to remove continue here so we still get vector and stuff right for RBF so mesh doesn't wig out.
			// I believe this is because later non-initialized vectors would have a -1.QNAN value possibly.
			}
		else
			{
			MDataHandle hPoseWt = hCmpdPose.child( aPoseWeight ) ;	// Get child weight attr.
			dArrWts[uIdx] = hPoseWt.asDouble() ;
			}



		if (nIsolate > 0)
			{
			// Now if we are isolating, set any weight not at the isolate index to 0,
			// and the weight at this index to 1.  Have to use -1 since 0 means off..so 1=first.
			//
			unsigned uIso = nIsolate - 1 ;
			if (uIdx == uIso)
				dArrWts[uIdx] = 1.0 ;
			else
				dArrWts[uIdx] = 0.0 ;
			}
			

		// Read in stuff for RBF mode...kinda a hack for now.  We just use the first pose
		// xform in each pose...ie: if 2 joints are creating a pose, we're really only using 
		// whichever one is connected first.

		// Now get proper cmp pose element...
		MArrayDataHandle hArrCmpdPoseXForm = hCmpdPose.child( aPoseXForm ) ;	// Get cmpd array of xforms in pose
		MDataHandle hCmpdPoseXForm  = hArrCmpdPoseXForm.inputValue(&stat) ;	// Get 0th xform element
		// and index
		MDataHandle hPoseXFormIdx = hCmpdPoseXForm.child( aPoseXFormIdx ) ;
		int nMatIdx = hPoseXFormIdx.asInt() ;
		// And in each pose, go thru each transform that generated it...
		MDataHandle hPoseXFormWorldMatrix = hCmpdPoseXForm.child( aPoseXFormWorldMatrix ) ;	// Get poseReader matrix for this pose/xform combo
		MMatrix matReader = hPoseXFormWorldMatrix.asMatrix();
		// And also see which axis is used.
		MDataHandle hPoseXFormReadAxis = hCmpdPoseXForm.child( aPoseXFormReadAxis ) ;	// Get poseReader matrix for this pose/xform combo
		int nReadAxis = hPoseXFormReadAxis.asShort() ;
		

		// Now store vector for current and where each pose is so RBF mode can interpolate.
		//
		MVector vec ;
		if (nReadAxis == 0)
			vec = MVector(1.0, 0.0, 0.0) ;
		else if (nReadAxis == 1)
			vec = MVector(0.0, 1.0, 0.0) ;
		else
			vec = MVector(0.0, 0.0, 1.0) ;
		vArrRead[uIdx] = vec * matReader ;		// Now get the real vector in world space based on the pose reader matrix.
		// Kinda a hack here too, we'll keep updaing vCur.  The last pose will be the one
		// that determines which axis and joint is being used for "current".  Under normal circumstances
		// this is fine, since if one is using the RBF mode they will have to have separate poseDeformers
		// for each section of the body or joint anyhow...
		vCur = vec * matArr[ nMatIdx ] ;		
		


		} while (hArrCmpdPose.next()) ;		// End of each pose

	return MS::kSuccess ;
}

// ---------------------------------------------------------------------------

/*
 * normalizeWeights() - Normalizes wts if total > 1.  If less than 0, leave as is..so I guess this
 *		isn't a true normalize...but it's what I want in this case.
 *
 */
MStatus poseDeformer::normalizeWeights(MDoubleArray &dArrWts) 
{
	MStatus stat ; 

	unsigned u ;

    unsigned uWts = dArrWts.length() ;
	if (uWts <= 1)
		return MS::kSuccess ;

	double dTotal = 0.0 ;
	for (u=0; u < uWts; ++u)
		dTotal += dArrWts[u] ;			// Add up totals

	
	if (dTotal > 1.00)
		{
		for (u=0; u < uWts; ++u)
			dArrWts[u] = dArrWts[u] / dTotal ;
		}

	return MS::kSuccess ;
}


// ---------------------------------------------------------------------------

/*
 * poseDeformer::smoothStep() - Take a value from 0-1 and instead of having it be linear,
 *			make it ease out and then in from the 0 and 1 locations.
 */
double poseDeformer::smoothStep(const double &dVal)
{
	// x^2*(3-2x)
	double dRet = dVal * dVal * (3.0 - (2.0 * dVal) );
	return dRet ;
}

// ---------------------------------------------------------------------------

/*
 * poseDeformer::smoothGaussian() - Take a value from 0-1 and instead of having it be linear,
 *			make it ease out and then in from the 0 and 1 locations.  This has more of a longer
 *			ease than a smoothstep and so a faster falloff in the middle.
 */
double poseDeformer::smoothGaussian(const double &dVal)
{
	// 1.0 - exp( (-1 * x^2 ) / (2*sigma^2) )
	
	double dSigma = 1.0 ;		// Must be >0.   As drops to zero, higher value one lasts longer.

	double dRet = (1.0 - exp( -1.0 * (dVal*dVal) * 10.0  / (2.0 * dSigma*dSigma)  ) );	// RBF
	return dRet ;
}

// ---------------------------------------------------------------------------

/*
 * interpWeights() - Does a smooth weight interp based on where we are.
 *		This uses LU Factorization with my custom MatrixNN class
 */
MStatus poseDeformer::interpWeights(MDoubleArray &dArrWts, 
		const MVectorArray &vArrRead,
		const MVector &vCur, const double &dRBFWidth)
{
	MStatus stat ;

	unsigned uPoses = vArrRead.length() ;
	if (uPoses < 2)
		return MS::kSuccess ;


		// Calc inverse width
	double dInvWidth = 1.0 ;
	if (dRBFWidth != 0)
		dInvWidth = 1.0 / dRBFWidth ;

		// And then sigma squared
	double dSigma2 = - ( dInvWidth * dInvWidth );


	MatrixNN matNNA ;	// Make the A matrix for Ax = b.  
	matNNA.setDimension( uPoses ) ;		// alloc enough space for it.

	unsigned i,j ;
	for (i=0; i < uPoses; ++i)
		{
		for (j=0; j < uPoses; ++j)
			{
			// Now store how far away each pose is from each other pose.
			// ie: for pose i=#  compare to the pose j=#.  The diagonal will be
			// all zeros, which is why the LU factorization requires pivoting.
			//
			// Note: We normalize distance within 0-1 for better results.
			//	Easy since at max a pose can be 180 degrees away, or pi-radians away.
			//  So we just get the angle between and divide by pi to normalize.
			//
			double dDist = 	vArrRead[i].angle( vArrRead[j]) / 3.14159 ;
			matNNA[i][j] = exp( dSigma2 * dDist * dDist) ;	// e^(sigma*(dDist^2)) 
			} // end of j cols

		} // end of i rows


	// Now we've got our matrix built...make our "b" array/vector of doubles.
	// as well as the X output weights
	double *ptrB = NULL ;
	double *ptrX = NULL ;
	ptrB = new double [uPoses] ;
	ptrX = new double [uPoses] ;

	// now we count up orig weights and we use this when normalizing later.
	// This lets us kinda fake using cones, even tho technically the values
	// could be 1.0 for all ofB, and or the cones could all be 180.  But this
	// way we'll smoothly ramp down if cone values fall off.  Plus by using
	// the array wt value here for B we also handle twist on cone disabling...
	double dTotalOrig = 0.0 ;
	for (i=0; i < uPoses; ++i)
		{
		ptrB[i] = dArrWts[i] ;			// We could do 1.0 here, need to test.
		dTotalOrig += dArrWts[i] ;
		}
	if (dTotalOrig > 1.0)
		dTotalOrig = 1.0 ;
	
	// Now we are ready to do.  This will solve Ax=b using LU-Factorization.
	bool bSolve = matNNA.solveLU(ptrB, ptrX) ;
	if (!bSolve)
		{
		// uh-oh the matrix wasn't factorable
		MGlobal::displayWarning(name()+MString(": poseDeformer Unable to factorize matrix for LU Decomposition.")) ;
		for (i=0; i < uPoses; ++i)
			dArrWts[i] = 0.0 ;
		delete [] ptrB ;		// Dealloc before we exit!
		delete [] ptrX ;
		return MS::kFailure ;
		}

	// now that we've solved...we'll store the real weight...
	//
	double dTotal = 0.0 ;
	for (i=0; i < uPoses; ++i)
		{
		double dDist = vCur.angle( vArrRead[i]) / 3.14159 ;		// Where are we relative to this pose?
		dArrWts[i] = dArrWts[i] * exp( dSigma2 * dDist * dDist ) ;	// w_[i] * e^(sigma * |pose-pose[i]|) 
		if (dArrWts[i] < 0.0)
			dArrWts[i] = 0.0 ;
		dTotal += dArrWts[i] ;
		}

	// Now renormalize to a value of what we want...
	for (i=0; i < uPoses && dTotal != 0.0; ++i)
		dArrWts[i] = dArrWts[i] / dTotal * dTotalOrig ;
	

	delete [] ptrB ;		// Dealloc before we exit!
	delete [] ptrX ;


	return MS::kSuccess ;
}

// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//	Math Procs
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------

/*
 * cartesianToSpherical() - Convert an XYZ value into a spherical coord.
 */
void cartesianToSpherical(const double &x, const double &y, const double &z, 
						double &r, double &phi, double &theta)
{
	r = sqrt( x*x + y*y + z*z) ;
	phi = atan( y / x ) ;
	theta = acos( z / (sqrt(r)) );
}

// ---------------------------------------------------------------------------

/*
 * sphericalToCartesian() - Convert an spherical value into a cart coord.
 */
void sphericalToCartesian(const double &r, const double &phi, const double &theta, 
						double &x, double &y, double &z)
{
	x = r * sin(theta) * cos(phi) ;
	y = r * sin(theta) * sin(phi) ;
	z = r * cos(theta) ;
}

// ---------------------------------------------------------------------------


/*
 * cartesianToCylindrical() - Convert an XYZ value into a cylindrical coord.
 */
void cartesianToCylindrical(const double &x, const double &y, const double &z, 
						double &r, double &theta, double &zc)
{
	r = sqrt( x*x + y*y) ;
	theta = atan( y / x ) ;
	zc = z ;
}

// ---------------------------------------------------------------------------

/*
 * cylindricalToCartesian() - Convert an cyl value into a cart coord.
 */
void cylindricalToCartesian(const double &r, const double &theta, const double &zc, 
						double &x, double &y, double &z)
{
	x = r * cos(theta) ;
	y = r * sin(theta) ;
	z = zc ;
}

// ---------------------------------------------------------------------------


/*
 * cartesianToPolar() - Convert an XY value into a polar coord.
 */
void cartesianToPolar(const double &x, const double &y,
						double &r, double &theta)
{
	r = sqrt( x*x + y*y) ;
	theta = atan( y / x ) ;
}

// ---------------------------------------------------------------------------

/*
 * polarToCartesian() - Convert an polar value into a cart coord.
 */
void polarToCartesian(const double &r, const double &theta,
						double &x, double &y)
{
	x = r * cos(theta) ;
	y = r * sin(theta) ;
}

// ---------------------------------------------------------------------------

/*
 * arclength() - Given radius and degrees apart, get arclen.
 */
double arclength(const double &r, const double &theta)
{
	return (r * theta) ;
}

// ---------------------------------------------------------------------------

/*
 * cartesianToSphericalUV() - x,y,z should be a normalize vector.
 */
void cartesianToSphericalUV(const double &x, const double &y, const double &z,
							double &u, double &v)
{
	// Make X go from 0-1 as we go ALL the way around.  ie: instead of just pure
	// X being -1 to 1.  We want to change to so -1=1 on right side, 
	// And if z > 0  then we use x as 0-1 and if <= -1 to 0
	//
	double dx ;
	if (z >= 0)
		{
		dx = (x + 1.0) / 2.0 ;	// Convert from -1...1  to 0...1
		}
	else
		{
		dx = ((x + 1.0) / 2.0) * -1.0 ;	// Convert from 1...-1  to -1...0
		}

	u = asin( dx ) / PI + 0.5 ;
	v = asin( y ) / PI + 0.5 ;

}


// ---------------------------------------------------------------------------

/*
 * dist2D() - Computes distance/length of a 2D vector
 */
inline double dist2D(const double &x, const double &y)
{
	return ( sqrt(x*x + y*y) );
}

// ---------------------------------------------------------------------------
