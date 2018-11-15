// --------------------------------------------------------------------------
// plugin.h - C++ Header File
// --------------------------------------------------------------------------
// Copyright ©2004 Michael B. Comet
// --------------------------------------------------------------------------
//
// DESCRIPTION:
//	Main heade  file for poseDeformer nodes.
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
 * Defines
 */
#define VERSION		"1.10"

#define MERR(STATUS, MSG)	if (STATUS != MS::kSuccess) { MGlobal::displayError(MSG); return STATUS ;}

#define DEBUG	0					// Set to 1 for some debug to stdout
#define	USEPROGRESSWIN	0			// Set to 1 to use progress window, which seems buggy so defaults to off.



// 0x0010A580 - 0x0010A5BF  COMET BLOCK Assigned by ALIAS! 64 entries!
//
#define ID_BLOCKSTART			0x0010A580

// Previous entries used by other plugins I wrote...DO NOT CHANGE THIS OR USE OTHERS it has been 
//		provided to me by Alias and I will be using other following values. -comet
#define ID_POSEDEFORMER			ID_BLOCKSTART+6		// 0x0010A586
#define ID_MIRRORDATA			ID_BLOCKSTART+7		// 0x0010A587


// --------------------------------------------------------------------------
