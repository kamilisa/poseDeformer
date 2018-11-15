// ---------------------------------------------------------------------------
// poseDeformerEdit.cpp - C++ File
// Copyright ©2004 Michael B. Comet
// ---------------------------------------------------------------------------
//
// DESCRIPTION:
//	The command that edit poseDeformer node data for targets etc..
//
// AUTHOR:
//	Michel B. Comet - comet@comet-cartoons.com
//
// VERSIONS:
//	09/25/04 - comet - Initial Rev
//	1.03 - 10/05/04 - mcomet - Fixed so does entire loop of all points even
//				if not in membership, so that targets work and ed-mem works.
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



// ---------------------------------------------------------------------------

/*
 * Includes
 */

#include <string.h>
#include <iostream>
#include <math.h>


#include "poseDeformerEdit.h" 
#include "poseDeformer.h"
#include "plugin.h"

// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//	Main Maya Plugin Functions
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------

/*
 * poseDeformerEdit::creator() - Allocate new item
 */
void *poseDeformerEdit::creator()
{
	return new poseDeformerEdit ;
}

// ---------------------------------------------------------------------------

/* 
 * poseDeformerEdit::poseDeformerEdit() - Constructor
 */
poseDeformerEdit::poseDeformerEdit()
{
	ptrXFormUndoData = NULL ;
}

// ---------------------------------------------------------------------------

/* 
 * poseDeformerEdit::~poseDeformerEdit() - Destructor
 */
poseDeformerEdit::~poseDeformerEdit()
{
	if (ptrXFormUndoData != NULL)
		{
		delete [] ptrXFormUndoData ;		// Free the array!
		ptrXFormUndoData = NULL ;
		}
}

// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//	Main Command Functions
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------



/*
 * poseDeformerEdit::newSyntax() - Make up rules
 */
MSyntax poseDeformerEdit::newSyntax()
{
	MStatus stat ;
	MSyntax syntax ;

	syntax.addFlag(kHelpFlag, kHelpFlagLong ) ;
	
	stat = syntax.addFlag(kXFormFlag, kXFormFlagLong, MSyntax::kSelectionItem ) ;
		MERRSYN(stat, syntax, "Can't add kXFormFlag flag.");
	stat = syntax.makeFlagMultiUse( kXFormFlag ) ;
		MERRSYN(stat, syntax, "Can't make kXFormFlag multiuse.");
	stat = syntax.makeFlagMultiUse( kXFormFlagLong ) ;
		MERRSYN(stat, syntax, "Can't make kXFormFlagLong multiuse.");

	stat = syntax.addFlag(kGeoFlag, kGeoFlagLong, MSyntax::kSelectionItem ) ;
		MERRSYN(stat, syntax, "Can't add kGeoFlag flag.");

	stat = syntax.addFlag(kPoseIdxFlag, kPoseIdxFlagLong, MSyntax::kLong ) ;
		MERRSYN(stat, syntax, "Can't add kPoseIdxFlag flag.");

	stat = syntax.addFlag(kMergeFlag, kMergeFlagLong, MSyntax::kBoolean) ;
		MERRSYN(stat, syntax, "Can't add kMergeFlag flag.");

	syntax.enableQuery(false);
	syntax.useSelectionAsDefault(true);						// allow selection to be used
	syntax.setObjectType(MSyntax::kSelectionList, 1, 1);	// 1-1 Must have poseDeformer node.

	return syntax;
}

// ---------------------------------------------------------------------------

/*
 * poseDeformerEdit::parseArgs() - Parse arguments
 */
MStatus poseDeformerEdit::parseArgs(const MArgList &argList)
{
    MStatus stat ;

	MArgDatabase argData(syntax(), argList, &stat);
	
	// Set defaults for not parsed...
	//
	bUsage = false ;
	nPoseIdx = -1 ;
	uXForms = 0 ;
	bMerge = false ;

	
	// -help
    if (argData.isFlagSet(kHelpFlag))
		{	
		bUsage = true ;
		showUsage() ;
		return MS::kSuccess ;
		}

	// -geo <targetGeo>
    if (argData.isFlagSet(kGeoFlag))
		{	
		argData.getFlagArgument(kGeoFlag, 0, sListGeo);
		}
	else
		{
		showUsage() ;
		MGlobal::displayError(MString("poseDeformerEdit: You must provide the -geo flag.")) ;
		return MS::kFailure ;
		}

	uXForms = argData.numberOfFlagUses( kXFormFlag ) ;
	if (uXForms == 0)
		{
		showUsage() ;
		MGlobal::displayError(MString("poseDeformerEdit: You must provide the -xform flag at least once and provide a valid transform for it.")) ;
		return MS::kFailure ;
		}
	unsigned u ;
	for (u=0; u < uXForms; ++u)
		{
		MArgList argListSub ;	// Multi sub arg list for the u-th usage of the flag
		stat = argData.getFlagArgumentList( kXFormFlag, u, argListSub) ;	// Get u-th usage argList.
			MERR(stat, "poseDeformerEdit: Can't getFlagArgumentList for multi flag.") ;

			// Now then the 0th item in this sub arg list will be the string/object the user put down...
		sListXForm.add( argListSub.asString(0) ) ;	// Add to our master xform list...
		}



	// -pindex #
    if (argData.isFlagSet(kPoseIdxFlag))
		{
		argData.getFlagArgument(kPoseIdxFlag, 0, nPoseIdx);
		}
	else
		{
		showUsage() ;
		MGlobal::displayError(MString("poseDeformerEdit: You must provide the -pindex flag.")) ;
		return MS::kFailure ;
		}


	// -merge <true|false>
    if (argData.isFlagSet(kMergeFlag))
		{	
		argData.getFlagArgument(kMergeFlag, 0, bMerge);
		}

	argData.getObjects( sListDef );		// Finally also get deformer node as usual object tacked to end.

	return MS::kSuccess ;
}

// ---------------------------------------------------------------------------

/*
 * showUsage() - Shows basic help
 */
void poseDeformerEdit::showUsage()
{
    MString str;

	str += ("// ---------------------------------------\n") ;
	str += ("// poseDeformerEdit USAGE: - Sets up pose targets for a poseDeformer. \n") ;
	str += ("// ---------------------------------------\n") ;
	str += ("//     poseDeformerEdit -geom TargetGeom -xform joint1 -xform joint2 -pindex 0 poseDeformer1 ;  \n");
	str += ("//     poseDeformerEdit -geom TargetGeom -xform joint1 -xform joint2 -pindex 0 -merge true poseDeformer1 ;  \n");
	str += ("// \n") ;
	str += ("//   FLAGS:\n") ;
	str += ("//       -h     | -help           :  Show Help. \n") ;
	str += ("//       -geo   | -geom           :  Object that is the geometry of the target shape.\n") ;
	str += ("//       -x     | -xform          :  What transform(s) were used to generate this target. (Multi-Use)\n") ;
	str += ("//       -pi    | -pindex         :  The pose index in the main pose array that is being created/edited. \n") ;
	str += ("//       -mrg   | -merge          :  Whether or not to merge current values with the existing pose index values. true of false.  Defaults to false. \n") ;

	MGlobal::displayInfo( str ) ;
}

// ---------------------------------------------------------------------------

/*
 * poseDeformerEdit::doIt() - Main action of command
 */
MStatus poseDeformerEdit::doIt(const MArgList &argList)
{
    MStatus stat ;

	stat = parseArgs(argList) ;	// Parse the args!
	if (stat != MS::kSuccess)
		return stat ;

	stat = redoIt() ;	// do real work!

	return stat ;
}

// ---------------------------------------------------------------------------

/*
 * poseDeformerEdit::redoIt() - Main action of command
 */
MStatus poseDeformerEdit::redoIt()
{
	MStatus stat ;

	MObject oDef ;
	MObjectArray oArrXForms ;
	MDagPath dpathGeoTgt ;
	MObject oCompGeoTgt ;
	
	unsigned u ;

   

		// If just doing usage, return already, we are done.
	if  (bUsage)
		return MS::kSuccess ;


	// Now from the sLists, get the dependency nodes....
	sListDef.getDependNode(0, oDef ) ;
	MFnDependencyNode depDef(oDef, &stat) ;

	oArrXForms.setLength( uXForms ) ;	// Alloc object array for xforms
	nArrXFormIdx.setLength( uXForms ) ;

	for (u=0; u < uXForms; ++u)		// Convert sList to MObjects, and also to index into transform array.
		{
		stat = sListXForm.getDependNode(u, oArrXForms[u] ) ;		// Store MObject for the xform


		// Now find the index of the connected transform into the deformers worldMatrix array.
		nArrXFormIdx[u] = -1 ;		// default
		MPlug plugArrWorldMatrix = depDef.findPlug("worldMatrix", &stat) ;	// Get plug
			MERR(stat, "poseDeformerEdit: Can't find worldMatrix plug on deformer.") ;
		unsigned uMats = plugArrWorldMatrix.numElements(&stat) ;
			MERR(stat, "poseDeformerEdit: Can't determine number of elements on worldMatrix plug.") ;
		unsigned m ;
		for (m=0; m < uMats; ++m)
			{
				// Get m-th plug element for what exists.
            MPlug plugWorldMatrix = plugArrWorldMatrix.elementByPhysicalIndex(m, &stat) ;

			MPlugArray plugArrMatIn ;
			plugWorldMatrix.connectedTo( plugArrMatIn, true, false, &stat)	; // See what's connected into it...
				MERR(stat, "poseDeformerEdit: Can't get connectedTo for worldMatrix element.") ;
			// Now check each connection and see if it is the transform node we are using 
			// Since it's an input connection, just need to check 0th item.
			if (plugArrMatIn.length() == 0)
				continue ;		// try next plug if no connection

			if (plugArrMatIn[0].node() == oArrXForms[u])	// That's this transform...
				{
				nArrXFormIdx[u] = (int)plugWorldMatrix.logicalIndex(&stat) ;	// So what idx is it?
				break ;
				}
			} // end of each matrix plug element loop

		// Now did we find a match?
		if (nArrXFormIdx[u] == -1)	
			{
			MFnDependencyNode depXForm(oArrXForms[u], &stat) ;
			MGlobal::displayError(MString("poseDeformerEdit: The provided transform \"")+depXForm.name()+MString("\" does not appear to be connected to the poseDeformer node."));
			return MStatus::kFailure;
			}

		} // end of each xform storing index and object

	sListGeo.getDagPath(0, dpathGeoTgt, oCompGeoTgt ) ;
	MObject oGeoTgt = dpathGeoTgt.transform() ;		// Get transform too for nice printing
	MFnDependencyNode depGeoTgt(oGeoTgt, &stat) ;



	// Check nodeType of what we have for the deformer...
	if (depDef.typeId(&stat) != ID_POSEDEFORMER )
		{
		showUsage() ; 
		MGlobal::displayError("poseDeformerEdit: You must specify a poseDeformer deformer node.");
		return MStatus::kFailure;
		}


	// Get iter for Target Geo
//	MItGeometry iterTgt(dpathGeoTgt, oCompGeoTgt, &stat) ;
	MItGeometry iterTgt(dpathGeoTgt, &stat) ;
	if (stat != MS::kSuccess) 
		{
		MGlobal::displayError("poseDeformerEdit: Can't create geometry iter from target.");
		return MStatus::kFailure;
		}
	int nPtsTgt = iterTgt.count(&stat);


	// Get iter for REAL geo that deformer is deforming...
	MDagPath dpathGeo ;
	MObject oCompGeo ;
	stat = getDeformerPathComp(oDef, 0, dpathGeo, oCompGeo) ;
	if (stat != MS::kSuccess)
		return stat ;
//	MItGeometry iter(dpathGeo, oCompGeo, &stat) ;
	MItGeometry iter(dpathGeo, &stat) ;
	if (stat != MS::kSuccess) 
		{
		MGlobal::displayError("poseDeformerEdit: Can't create geometry iter from deforming geometry.");
		return MStatus::kFailure;
		}
	int nPts = iter.count(&stat);
	MObject oGeo = dpathGeo.transform() ;		// Get transform too for nice printing and for getting geo matrix
	MFnDependencyNode depGeo(oGeo, &stat) ;

	MItGeometry iterMem(dpathGeo, oCompGeo, &stat) ;
//	MItGeometry iterTgt(dpathGeoTgt, oCompGeo, &stat) ;
	int nPtsMem = iterMem.count(&stat);	// How many point actually in membership....

	
	MPlug plugArrMatGeo = depGeo.findPlug( "worldMatrix", &stat) ;
		MERR(stat, "poseDeformerEdit: Can't find worldMatrix on geometry transform.") ;
	MPlug plugMatGeo = plugArrMatGeo.elementByLogicalIndex(0, &stat) ;
	MObject oMatGeo ;
	plugMatGeo.getValue(oMatGeo) ;
	MFnMatrixData fnMatGeo( oMatGeo ) ;
	MMatrix matGeoWorld = fnMatGeo.matrix() ;



/*
cout << "-------------------------------------------" << endl ;
cout << "poseDeformerEdit:" << endl ;
cout << "-------------------------------------------" << endl ;
cout << "Deformer=" << depDef.name() << endl ;
cout << "nPoseIdx=" << nPoseIdx << endl ;
cout << "Geo=" << depGeo.name() << endl ;
cout << "GeoPointCount=" << nPts << endl ;
cout << "TgtGeo=" << depGeoTgt.name() << endl ;
cout << "TgtGeoPointCount=" << nPtsTgt << endl ;
cout << "uXForms=" << uXForms << endl ;
cout << "sListXForm.length()=" << sListXForm.length() << endl ;
for (u=0; u < uXForms; ++u)
	{
	MObject oXForm ;
	MFnDependencyNode depXForm(oArrXForms[u], &stat) ;
	cout << "  XFORM " << u << ") = " << depXForm.name() << " IDX=" << nArrXFormIdx[u] << endl ;
	}
cout << endl ;
*/

	MString str ;
	str += "poseDeformerEdit: Generating pose at index " ;
	str += nPoseIdx ;
	str += " with " ;
	str += (int)uXForms;
	str += " transforms from " ;
	str += depGeoTgt.name() ;
	str += " onto " ;
	str += depGeo.name();
	str += " with " ;
	str += nPts ;
	str += " points." ;
	MGlobal::displayInfo(str) ;

	// Make sure point count matches...warning if not, but still allow!
	if (nPts != nPtsTgt)
		{
		MGlobal::displayWarning("poseDeformerEdit: !!! Target and Deforming point counts do not match! !!!");
		}


	// We are now ready to do the real work.  We need to go thru and get to the right pose attr element
	// on the deformer.  
	// Then in that, for each transform, set the transform index attribute as well as the 
	// strength value (1/#XForms).
	// Then inside of each transform pose date, go through and for each point, compare the points 
	// from the original to the target, and then store that in the space of the transform.
	//
	// We need to store all the original values also for undo.
	//

	// Alloc undo data, one for each transform...
	ptrXFormUndoData = new poseDeformerXFormData [uXForms] ;

	MPlug plugArrPose = depDef.findPlug("pose", &stat) ;	// Get plug
		MERR(stat, "poseDeformerEdit: Can't find pose plug on deformer.") ;
	MPlug plugPose = plugArrPose.elementByLogicalIndex( nPoseIdx, &stat) ;	// Go to right index
		MERR(stat, "poseDeformerEdit: Can't get to pose index on deformer.") ;

	MPlug plugArrWorldMatrix = depDef.findPlug("worldMatrix", &stat) ;	// Get plug
		MERR(stat, "poseDeformerEdit: Can't find worldMatrix plug on deformer.") ;

	double dRelStr = 1.0 / uXForms ;		// Relative strength for each transform, defaults to even distribution.

	// Set up progress window stuff..
	double dPctCnt = 0 ;
	double dPctTotal = uXForms * nPtsMem ;
	int nPct = (int)(100.0 * dPctCnt / dPctTotal) ;
	
#if USEPROGRESSWIN > 0
	if (!MProgressWindow::reserve())
		{
		MGlobal::displayError("MProgressWindow already in use.");
		return MS::kFailure ;
		}

	MProgressWindow::setProgressRange(0, 100) ;
	MProgressWindow::setTitle("poseDeformer: Generating Pose");
	MProgressWindow::setInterruptable(true);
	MProgressWindow::setProgress(nPct) ;

	MProgressWindow::startProgress();		// Start it!
#endif

	for (u=0; u < uXForms; ++u)
		{
#if USEPROGRESSWIN > 0
		if (MProgressWindow::isCancelled()) 
			{
			MGlobal::displayInfo("poseDeformerEdit: Interrrupted at users request!");
			break ;
			}
#endif


		// Read current xform value on deformer for this xform...
		MPlug plugWorldMatrix = plugArrWorldMatrix.elementByLogicalIndex( nArrXFormIdx[u], &stat) ;	// Go to right index
			MERR(stat, "poseDeformerEdit: Can't get to worldMatrix index on deformer.") ;
		MObject oMat;
		plugWorldMatrix.getValue( oMat );
		MFnMatrixData fnMat( oMat );
		MMatrix matXForm = fnMat.matrix() ; 
		MMatrix invmatXForm = matXForm.inverse() ;
		

		MPlug plugArrXForm = plugPose.child( poseDeformer::aPoseXForm, &stat) ;		// Get XForm sub array
			MERR(stat, "poseDeformerEdit: Can't get poseXForm array in pose.") ;
		MPlug plugXForm = plugArrXForm.elementByLogicalIndex(u, &stat) ;			// Go to this element
			MERR(stat, "poseDeformerEdit: Can't get to XForm index in poseXForm.");
		
		MPlug plugXFormStr = plugXForm.child( poseDeformer::aPoseXFormStr, &stat) ;	// Get child str
			MERR(stat, "poseDeformerEdit: Can't get XForm Str in XForm.");
		plugXFormStr.getValue( ptrXFormUndoData[u].dStr ) ;		// Store undo str
		plugXFormStr.setValue( dRelStr ) ;						//	Set new value

		MPlug plugXFormIdx = plugXForm.child( poseDeformer::aPoseXFormIdx, &stat) ;	// Get child idx
			MERR(stat, "poseDeformerEdit: Can't get XForm Idx in XForm.");
		plugXFormIdx.getValue( ptrXFormUndoData[u].nIdx ) ;		// Store undo idx
		plugXFormIdx.setValue( nArrXFormIdx[u] ) ;				//	Set new value
		if (bMerge)
			{
			// Sanity check...user SHOULD have chosen exact same xforms in exact same order to
			// modify this pose..if not, at least print out a warning.
			if (ptrXFormUndoData[u].nIdx != nArrXFormIdx[u])
				{
				MString str;
				str += ("The transform index ");
				str += (int)u ;
				str += (" does not match from the original pose to this edit.  Make sure you pick the exact same transforms in the exact same order before merging/editing.");
				MGlobal::displayWarning(str) ;
				}
			}

		MPlug plugXFormNumPts = plugXForm.child( poseDeformer::aPoseXFormNumPts, &stat) ;	// Get child npts
			MERR(stat, "poseDeformerEdit: Can't get XForm NumPts in XForm.");
		plugXFormNumPts.getValue( ptrXFormUndoData[u].nNumPts ) ;		// Store undo str
		plugXFormNumPts.setValue( nPts ) ;						//	Set new value

		MPlug plugArrDelta = plugXForm.child( poseDeformer::aPoseDelta, &stat) ;	// Get child delta array
			MERR(stat, "poseDeformerEdit: Can't get poseDelta array in XForm.");


			// Now build out the array with array data builder, this makes setting the data MUCH faster
			// since it grows the array ahead of time instead of using MPlugs and is what Maya prefers.
		MObject aPlugArrDelta = plugArrDelta.attribute(&stat) ;
			if (stat != MS::kSuccess) cout << "Can't get attribute aPlugArrDelta for grow." << endl ;
		MArrayDataBuilder bldDelta( aPlugArrDelta, nPts, &stat) ;
			if (stat != MS::kSuccess) cout << "Can't get attribute bldDelta ArrayBuilder for grow." << endl ;
		int p;
		for (p=0; p < nPts; ++p)
			{
			bldDelta.addElement(p, &stat) ;
			}
			

			// Now alloc up point storage undo for this transform also
		stat = ptrXFormUndoData[u].ptArrDelta.setLength( nPts ) ;
			MERR(stat, "poseDeformerEdit: Can't set points for undo.");
		

			// Due to what appears to be a Maya bug, have to recreate iters each loop.
//		MItGeometry iter(dpathGeo, oCompGeo, &stat) ;
//		MItGeometry iterTgt(dpathGeoTgt, oCompGeoTgt, &stat) ;
			// Do this so goes over ALL points, not just ones in membership...
//		MItGeometry iter(dpathGeo, MObject::kNullObj, &stat) ;
//		MItGeometry iterTgt(dpathGeoTgt, MObject::kNullObj, &stat) ;
			// Ok this will now ONLY go over points in membership...Basically this way it's faster...
			// ie: only calcs points in membership at time of pose creation...
			// And also, will still match so long as tgt and orig have same # of points.
		MItGeometry iter(dpathGeo, oCompGeo, &stat) ;
		MItGeometry iterTgt(dpathGeoTgt, oCompGeo, &stat) ;

		// Note: this won't work too well if point counts are different...if they are whatever the 
		// "ordering" is between the two will be how things are mapped.  ie: if your base has 
		// point 1,2,3  but the target has chosen 1,12,18  then 1-1 2-12 adn 3-18 will be the mapping.
		// Like any blendshape...definitely a good idea to make point counts match.
		//
		unsigned uPtIdx=0;
		for (iter.reset(), iterTgt.reset(); !iter.isDone() && !iterTgt.isDone(); iter.next(), iterTgt.next() )
			{
			uPtIdx = iter.index();

			MString statusStr ;
			statusStr += "XForm: " ;
			statusStr += (int)(u+1) ;
			statusStr += "/" ;
			statusStr += (int)(uXForms);
			statusStr += " Point: " ;
			statusStr += (int)uPtIdx ;
			statusStr += "/" ;
			statusStr += nPts ;

			nPct = (int)(100.0 * dPctCnt / dPctTotal) ;
			++dPctCnt ;		// keep count for progress.

#if USEPROGRESSWIN > 0
			if (MProgressWindow::isCancelled())
				break ;

			MProgressWindow::setProgress(nPct) ;
			MProgressWindow::setProgressStatus(statusStr) ;
#else
			if ((int)(dPctCnt-1) % 25 == 0)		// Every so many loops print to scriptEditor window
				{
				MString statusPctStr ;
				statusPctStr += nPct ;
				statusPctStr += "% " ;
				MGlobal::displayInfo(statusPctStr + statusStr) ;
				}
#endif


				// Get to right spot in delta array
			MPlug plugDelta = plugArrDelta.elementByLogicalIndex( uPtIdx, &stat) ;
				MERR(stat, "poseDeformerEdit: Can't get to delta index in Delta") ;
			MPlug plugDeltaX = plugDelta.child( poseDeformer::aPoseDeltaX, &stat) ;	// Get actual XYZ plugs
			MPlug plugDeltaY = plugDelta.child( poseDeformer::aPoseDeltaY, &stat) ;
			MPlug plugDeltaZ = plugDelta.child( poseDeformer::aPoseDeltaZ, &stat) ;
			plugDeltaX.getValue( ptrXFormUndoData[u].ptArrDelta[uPtIdx].x ) ;		// Store for undo xyz
			plugDeltaY.getValue( ptrXFormUndoData[u].ptArrDelta[uPtIdx].y ) ;	
			plugDeltaZ.getValue( ptrXFormUndoData[u].ptArrDelta[uPtIdx].z ) ;	
			plugDeltaX.setValue( 0.0 ) ;	 // in case don't make it farther
			plugDeltaY.setValue( 0.0 ) ;	
			plugDeltaZ.setValue( 0.0 ) ;	

				// Get the positions of the points in OBJECT Space
			MPoint ptGeo = iter.position(MSpace::kObject, &stat) ;
			MPoint ptTgt = iterTgt.position(MSpace::kObject, &stat) ;
			
			MVector vDelta = ptTgt - ptGeo ;	// Get delta vector 
			if (vDelta == MVector(0,0,0))		// No change in world space!  Skip!
				;
			else
				{
				vDelta = vDelta * matGeoWorld ;		// Put from object space into World Space...
				vDelta = vDelta * invmatXForm ;		// Then put into space of xform 

					// If we are merging, also add in whatever values were already there...
				if (bMerge)
					{
					vDelta.x += ptrXFormUndoData[u].ptArrDelta[uPtIdx].x ;
					vDelta.y += ptrXFormUndoData[u].ptArrDelta[uPtIdx].y ;
					vDelta.z += ptrXFormUndoData[u].ptArrDelta[uPtIdx].z ;
					}
				}

			plugDeltaX.setValue( vDelta.x ) ;	// set new value
			plugDeltaY.setValue( vDelta.y ) ;	
			plugDeltaZ.setValue( vDelta.z ) ;	

			}	// end of set for each pt
	

		} // end of set for each xform

#if USEPROGRESSWIN > 0
	MProgressWindow::endProgress() ;
#endif

	clearResult() ;


	return MS::kSuccess ;
}

// ---------------------------------------------------------------------------

/*
 * poseDeformerEdit::undoIt() - Main action of command
 */
MStatus poseDeformerEdit::undoIt() 
{
	MStatus stat ;
	unsigned u ;
	
	MObject oDef ;

	// If we were just in query...nothing to undo...just return
	if (bUsage)
		return MS::kSuccess ;


	// Now from the sLists, get the dependency nodes....
	sListDef.getDependNode(0, oDef ) ;
	MFnDependencyNode depDef(oDef, &stat) ;

	// Get iter for REAL geo that deformer is deforming...
	MDagPath dpathGeo ;
	MObject oCompGeo ;
	stat = getDeformerPathComp(oDef, 0, dpathGeo, oCompGeo) ;
	if (stat != MS::kSuccess)
		return stat ;
	MItGeometry iter(dpathGeo, oCompGeo, &stat) ;
	if (stat != MS::kSuccess) 
		{
		MGlobal::displayError("poseDeformerEdit: Can't create geometry iter from deforming geometry.");
		return MStatus::kFailure;
		}
	int nPts = iter.count(&stat);
	MObject oGeo = dpathGeo.transform() ;		// Get transform too for nice printing
	MFnDependencyNode depGeo(oGeo, &stat) ;



	// For undo, we just loop thru the data we had and reset values....
	//
	MPlug plugArrPose = depDef.findPlug("pose", &stat) ;	// Get plug
		MERR(stat, "poseDeformerEdit: Can't find pose plug on deformer.") ;
	MPlug plugPose = plugArrPose.elementByLogicalIndex( nPoseIdx, &stat) ;	// Go to right index
		MERR(stat, "poseDeformerEdit: Can't get to pose index on deformer.") ;

	for (u=0; u < uXForms; ++u)
		{
		MPlug plugArrXForm = plugPose.child( poseDeformer::aPoseXForm, &stat) ;		// Get XForm sub array
			MERR(stat, "poseDeformerEdit: Can't get poseXForm array in pose.") ;
		MPlug plugXForm = plugArrXForm.elementByLogicalIndex(u, &stat) ;			// Go to this element
			MERR(stat, "poseDeformerEdit: Can't get to XForm index in poseXForm.");
		
		MPlug plugXFormStr = plugXForm.child( poseDeformer::aPoseXFormStr, &stat) ;	// Get child str
			MERR(stat, "poseDeformerEdit: Can't get XForm Str in XForm.");
		plugXFormStr.setValue( ptrXFormUndoData[u].dStr ) ;		// UNDO

		MPlug plugXFormIdx = plugXForm.child( poseDeformer::aPoseXFormIdx, &stat) ;	// Get child idx
			MERR(stat, "poseDeformerEdit: Can't get XForm Idx in XForm.");
		plugXFormIdx.setValue( ptrXFormUndoData[u].nIdx ) ;		// UNDO

		MPlug plugXFormNumPts = plugXForm.child( poseDeformer::aPoseXFormNumPts, &stat) ;	// Get child idx
			MERR(stat, "poseDeformerEdit: Can't get XForm NumPts in XForm.");
		plugXFormNumPts.setValue( ptrXFormUndoData[u].nNumPts ) ;		// UNDO

		MPlug plugArrDelta = plugXForm.child( poseDeformer::aPoseDelta, &stat) ;	// Get child delta array
			MERR(stat, "poseDeformerEdit: Can't get poseDelta array in XForm.");

			// Due to a Maya bug, have to recreate iters each loop.
		MItGeometry iter(dpathGeo, oCompGeo, &stat) ;

		unsigned uPtIdx=0;
		for (iter.reset(); !iter.isDone(); iter.next())
			{
			uPtIdx = iter.index() ;

				// Get to right spot in delta array
			MPlug plugDelta = plugArrDelta.elementByLogicalIndex( uPtIdx, &stat) ;
				MERR(stat, "poseDeformerEdit: Can't get to delta index in Delta") ;
			MPlug plugDeltaX = plugDelta.child( poseDeformer::aPoseDeltaX, &stat) ;	// Get actual XYZ plugs
			MPlug plugDeltaY = plugDelta.child( poseDeformer::aPoseDeltaY, &stat) ;
			MPlug plugDeltaZ = plugDelta.child( poseDeformer::aPoseDeltaZ, &stat) ;
			plugDeltaX.setValue( ptrXFormUndoData[u].ptArrDelta[uPtIdx].x ) ;		// UNDO
			plugDeltaY.setValue( ptrXFormUndoData[u].ptArrDelta[uPtIdx].y ) ;	
			plugDeltaZ.setValue( ptrXFormUndoData[u].ptArrDelta[uPtIdx].z ) ;	

			}	// end of set for each pt

		} // end of set for each xform



	return MS::kSuccess ;
}

// ---------------------------------------------------------------------------

/*
 * poseDeformerEdit::isUndoable() - Returns true if we are in an undoable mode
 *			false otherwise.  If the command was a query, then we return false,
 *			otherwise we'll return true.
 */
bool poseDeformerEdit::isUndoable() const
{
	return true ;
}

// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Real worker procs
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------

/*
 * getDeformerPathComp()- Given a node, that can be converted to both a basic
 *		dependency node and a deformer, (ie: it's a deformer), and given the 
 *		index for what geo that is connected is desired, this then sets
 *		the DagPath for that geometry and the component that is used for it.
 *
 *		This is used so that given a deformer node, one can easily create an
 *		MItGeometry for it based on the input geo and groupParts that is coming
 *		in.   
 *
 */
MStatus poseDeformerEdit::getDeformerPathComp(MObject &oNode, unsigned uIdx, MDagPath &dpath, MObject &oComp)
{
    MStatus stat ;

	MFnDependencyNode depNode( oNode, &stat) ;
	if (stat != MS::kSuccess)
		{
		MGlobal::displayError("getDeformerPathComp, Can't get depNode node from oNode.") ;
		return stat ;
		}

		// First get a geoFilter so that we can get the DagPath for
		// For the node.  This also assures that the node is a 
		// deformer node.
		//
    MFnGeometryFilter geoFilter(oNode, &stat) ;
	if (stat != MS::kSuccess)
		{
		MGlobal::displayError("getDeformerPathComp, Can't get geoFilter from oNode.") ;
		return stat ;
		}

	MObject oDeformerSet = geoFilter.deformerSet(&stat) ;
	if (stat != MS::kSuccess)
		{
		MGlobal::displayError("getDeformerPathComp, Can't get oDeformerSet from geoFilter.") ;
		return stat ;
		}
			
	MFnSet fnSet(oDeformerSet, &stat);	// Now get as FN
	if (stat != MS::kSuccess)
		{
		MGlobal::displayError("getDeformerPathComp, Can't create MFnSet fnSet from oDeformerSet.") ;
		return stat ;
		}


		// Get all the members of the set 
	MSelectionList selList;
	stat = fnSet.getMembers(selList, true);
	if (stat != MS::kSuccess)
		{
		MGlobal::displayError("getDeformerPathComp, Can't get selList members from fnSet.") ;
		return stat ;
		}

		// Finally for the idx'th items in the set, get the dagPath and Components of the items in the set.
	selList.getDagPath(uIdx, dpath, oComp) ;

	return MS::kSuccess ;
}

//----------------------------------------------------------------------------

// ---------------------------------------------------------------------------

