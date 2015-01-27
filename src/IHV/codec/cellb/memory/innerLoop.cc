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
//  File:   innerLoop.cc
//  Project:    XIL
//  Revision:   1.4
//  Last Mod:   10:15:38, 03/10/00
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
#pragma ident   "@(#)innerLoop.cc	1.4\t00/03/10  "

#include <xil/xilGPI.hh>

#include "XilDeviceCompressionCellB.hh"

//
// TODO: This function was originally included in compress.cc .
//       Due to a compiler bug, the compile SEG faults when running
//       with the normal O4 optimization level (debug is fine).
//       Move this back when the compiler gets fixed
//

int 
XilDeviceCompressionCellB::inner_loop(Xil_unsigned8 *base,
                                      unsigned int pixel_stride,
                                      unsigned int scan_stride)
{
    Xil_unsigned8  yval[16];
    Xil_unsigned32 ymean = 0;
    Xil_unsigned32 uvmean;
    unsigned int   umean = 0;
    unsigned int   vmean = 0;
   
    int i = 0;
    Xil_unsigned8* pLine = base;
    for (int y=4; y!=0; y--) {
      Xil_unsigned8* pSamp = pLine;
      for (int x=4; x!=0; x--, i++) {
        ymean += (yval[i] = pSamp[0]);
        umean += pSamp[1];
        vmean += pSamp[2];
        pSamp += pixel_stride;
      }
      pLine += scan_stride;
    }

    // Note that ymean is normalized, but uvmean has assumed binary points
    ymean >>= 4;
    uvmean = (umean << 16) + vmean;
    return encodeCell(ymean,
                      uvmean, 
                      *(Xil_unsigned32 *)&yval[0], 
                      *(Xil_unsigned32 *)&yval[4], 
                      *(Xil_unsigned32 *)&yval[8], 
                      *(Xil_unsigned32 *)&yval[12]);
}
        
