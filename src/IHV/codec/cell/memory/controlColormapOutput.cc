/***********************************************************************


            EXHIBIT A - XIL 1.4.1 (OPEN SOURCE VERSION) License


The contents of this file are subject to the XIL 1.4.1 (Open Source
Version) License Agreement Version 1.0 (the "License").  You may not
use this file except in compliance with the License.  You may obtain a
copy of the License at:

    http://www.sun.com/software/imaging/XIL/xilsrc.html

Software distributed under the License is distributed on an "AS IS"
basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See
the License for the specific language governing rights and limitations
under the License.

The Original Code is XIL 1.4.1 (Open Source Version).
The Initial Developer of the Original Code is: Sun Microsystems, Inc..
Portions created by:_______________________________________________
are Copyright(C):__________________________________________________
All Rights Reserved.
Contributor(s):____________________________________________________


***********************************************************************/
//------------------------------------------------------------------------
//
//  File:   controlColormapOutput.cc
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:15:55, 03/10/00
//
//  Description:
//
//
//
//
//
//
//
//
//  MT-level:  <??????>
//
//------------------------------------------------------------------------
//  COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)controlColormapOutput.cc	1.2\t00/03/10  "

#include "XilDeviceCompressionCell.hh"

Xil_boolean
XilDeviceCompressionCell::controlColormapOutput()
{
    Xil_boolean encode_this_cmap = TRUE;

    compData.runningTotals[compData.currentTotal++] =
        compData.currentError =
        compData.cmapSelection.getColormapError();

    if(compData.currentTotal >= CELL_RUNNINGTOTAL_LENGTH)
        compData.currentTotal = 0;
    
    compData.currentRunningTotal = 0.0;
    for(int i=0; i<CELL_RUNNINGTOTAL_LENGTH; i++) {
        compData.currentRunningTotal += compData.runningTotals[i];
    }
    compData.currentRunningTotal /= CELL_RUNNINGTOTAL_LENGTH;

    if(compData.imageError > 200.0) {
        compData.adaptIsChanging = TRUE;
        compData.colormapControlMode = BigChanges;
    } else if(compData.currentRunningTotal > 9.0) {
        switch(compData.colormapControlMode)
        {
          case BigChanges:
        
            compData.adaptIsChanging = TRUE;

            if(compData.currentError < compData.previousError) {
                compData.numConsecutiveDown++;
            } else {
                compData.numConsecutiveDown = 0;
            }
        
            if(compData.numConsecutiveDown > 2) {
                compData.numConsecutiveDown = 0;
                compData.colormapControlMode = WaitingToSettle;
            }       
            break;

          case WaitingToSettle:

            if(compData.currentRunningTotal > compData.previousRunningTotal) {
                compData.colormapControlMode = Settled;
                compData.adaptIsChanging = FALSE;
            }
            break;

          case Settled:
            
            if((compData.currentError > 2.0*compData.previousRunningTotal) ||
               (compData.currentError < 0.5*compData.previousRunningTotal)) {
                compData.adaptIsChanging = TRUE;
                compData.colormapControlMode = BigChanges;
            } else {
                encode_this_cmap = FALSE;
            }
            break;

          default:
            break;
        }
    } else {
        if(compData.colormapControlMode != Settled) {
            compData.numConsecutiveDown = 0;
            compData.colormapControlMode = Settled;
            compData.adaptIsChanging = FALSE;
        }
    }

#ifdef VERBOSE
    fprintf(stderr, "chang:  %d;  prev: %6.3f;  curr: %6.3f;  cError: %8.3f  mode: %d\n",
            compData.adaptIsChanging,
            compData.previousRunningTotal, compData.currentRunningTotal,
         compData.currentError, compData.colormapControlMode);
#endif

    if(encode_this_cmap == TRUE) {
        compData.previousRunningTotal = compData.currentRunningTotal;
        compData.previousError = compData.currentError;
    }
    
    return encode_this_cmap;
}
