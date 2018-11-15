// --------------------------------------------------------------------------
// plugin.cpp - C++ File
// --------------------------------------------------------------------------
// Copyright ©2004 Michael B. Comet
// --------------------------------------------------------------------------
//
// DESCRIPTION:
//	Main loading file for poseDeformer command.
//
// AUTHORS:
//		Michael B. Comet - comet@comet-cartoons.com
//
// VERSIONS:
//	1.00 - 09/21/04 - comet - Initial Rev.
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
// --------------------------------------------------------------------------
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
#include <maya/MFnPlugin.h>

#include "poseDeformer.h"
#include "poseDeformerEdit.h"
#include "mirrorData.h"

#include "plugin.h"

#include "MatrixNN.h" 

// --------------------------------------------------------------------------

/*
 * initializePlugin() - Main load of plugin
 *
 *	Description:
 *		this method is called when the plug-in is loaded into Maya.  It 
 *		registers all of the services that this plug-in provides with 
 *		Maya.
 *
 *	Arguments:
 *		obj - a handle to the plug-in object (use MFnPlugin to access it)
 *
 */
MStatus initializePlugin( MObject obj )
{ 
	MStatus   stat;
	MFnPlugin plugin( obj, "CometCartoons - Comet", "6.0", "Any");

		// poseDeformer - Main Deformation node
	stat = plugin.registerNode( "poseDeformer", poseDeformer::id, poseDeformer::creator, poseDeformer::initialize, MPxNode::kDeformerNode );
		MERR(stat, "Can't register plugin poseDeformer.");

		// poseDeformerEdit - Command to Edit targets etc on the node
	stat = plugin.registerCommand("poseDeformerEdit", poseDeformerEdit::creator, poseDeformerEdit::newSyntax) ;
		MERR(stat, "Can't register plugin poseDeformerEdit.") ;			

		// mirrorData - Extra Deformation node
	stat = plugin.registerNode( "mirrorData", mirrorData::id, mirrorData::creator, mirrorData::initialize, MPxNode::kDeformerNode );
		MERR(stat, "Can't register plugin mirrorData.");


	MGlobal::displayInfo(MString("poseDeformer ")+MString(VERSION)+MString(" - Built On: ") + MString(__DATE__) + MString(" ") + MString(__TIME__) ) ;
	MGlobal::displayInfo("poseDeformer Copyright ©2004,2005 Michael B. Comet") ;
	MGlobal::displayInfo("poseDeformer comes with ABSOLUTELY NO WARRANTY; for details read the") ;
	MGlobal::displayInfo("license.txt file.  This is free software, and you are welcome to") ;
	MGlobal::displayInfo("redistribute it under certain conditions; See license.txt for details.") ;


	return stat;
}

// --------------------------------------------------------------------------


/*
 * uninitializePlugin() - Disable/unload main plugin
 *	
 *	Description:
 *		this method is called when the plug-in is unloaded from Maya. It 
 *		deregisters all of the services that it was providing.
 *
 *	Arguments:
 *		obj - a handle to the plug-in object (use MFnPlugin to access it)
 *
 */
MStatus uninitializePlugin( MObject obj)
{
	MStatus   stat;
	MFnPlugin plugin( obj );

	stat = plugin.deregisterNode( poseDeformer::id );
		MERR(stat, "Can't un-register poseDeformer plugin.") ;

	stat = plugin.deregisterCommand( "poseDeformerEdit" );
		MERR(stat, "Can't un-register poseDeformerEdit plugin.") ;

	stat = plugin.deregisterNode( mirrorData::id );
		MERR(stat, "Can't un-register mirrorData plugin.") ;

		return stat;
}

// --------------------------------------------------------------------------
