// ---------------------------------------------------------------------------
// poseDeformerEdit.h - C++ File
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
#include <math.h>

#include <maya/MPxCommand.h>

#include <maya/MGlobal.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MArgList.h>

#include <maya/MDagPath.h>
#include <maya/MFnDagNode.h>
#include <maya/MSelectionList.h>
#include <maya/MObjectArray.h>


#include <maya/MPoint.h>
#include <maya/MPointArray.h>
#include <maya/MItGeometry.h>
#include <maya/MFnGeometryFilter.h>
#include <maya/MFnSet.h>

#if USEPROGRESSWIN > 0
#include <maya/MProgressWindow.h>
#endif

// ---------------------------------------------------------------------------

#define MERRSYN(STATUS, SYNTAX, MSG)	if (STATUS != MS::kSuccess) { MGlobal::displayError(MSG); return SYNTAX ;}

// ---------------------------------------------------------------------------

// Some classes to store data for undo...
//

/*
 * poseDeformerXFormData - Class to store undo data for a transform pose 
 */
class poseDeformerXFormData 
{
public:
	poseDeformerXFormData() { dStr=0.0; nIdx=-1; nNumPts=0; } ;

	double dStr ;		// Relative xform Strength
	int nIdx ;			// Index into main deformer array
	int nNumPts ;		// Num of pts

	MPointArray ptArrDelta ;	// Array of delta values for each point

} ;

// ---------------------------------------------------------------------------



/*
 * poseDeformerEdit - Command class definition
 */
class poseDeformerEdit : public MPxCommand
{
public:
	poseDeformerEdit();
	virtual ~poseDeformerEdit();
	virtual MStatus doIt(const MArgList &args);
	virtual MStatus redoIt() ;
	virtual MStatus undoIt() ;
	virtual bool isUndoable() const ;
	static void *creator();
	static MSyntax newSyntax();


private:
	MStatus parseArgs(const MArgList &);

	bool bUsage ;				 // Are we just showing usage?
	int nPoseIdx ;				 // What pose index are we creating/modifying?
	unsigned uXForms ;			 // How many XForms created this target?
	MIntArray nArrXFormIdx ;	 // What index into deformer does each transform have?
	bool bMerge	;				 // Are we going to combine the data from now with the data from an existing pose?
	MSelectionList sListXForm ;	 // xform sel list (multi)
	MSelectionList sListGeo ;	 // geo target sel list
	MSelectionList sListDef ;	 // deformer sel list


private:
	void showUsage(void) ;
	MStatus getDeformerPathComp(MObject &oNode, unsigned idx, MDagPath &dpath, MObject &oComp) ;

	poseDeformerXFormData *ptrXFormUndoData ;	// Ptr for undo data to be stored.

} ;


// ---------------------------------------------------------------------------

/*
 * Syntax options
 */
#define kHelpFlag									"-h"
#define kHelpFlagLong								"-help"

#define kXFormFlag									"-x"
#define kXFormFlagLong								"-xform"

#define kGeoFlag									"-geo"
#define kGeoFlagLong								"-geom"

#define kPoseIdxFlag								"-pi"
#define kPoseIdxFlagLong							"-pindex"

#define kMergeFlag									"-mrg"
#define kMergeFlagLong								"-merge"


// ---------------------------------------------------------------------------

