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
//This line lets emacs recognize this as -*- C++ -*- Code
//------------------------------------------------------------------------
//
//  File:       findNextFrameBoundary.cc
//  Project:    XIL
//  Revision:   1.4
//  Last Mod:   10:14:35, 03/10/00
//
//  Description:
//
//    Jpeg's implementation of findNextFrameBoundary()
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)findNextFrameBoundary.cc	1.4\t00/03/10  "


#include "xil/xilGPI.hh"
#include "JpegDecompressorData.hh"
#include "JpegMacros.hh"

XilStatus 
JpegDecompressorData::findNextFrameBoundary()
{
    int nbytes;
    Xil_unsigned8* ptr;
    Xil_boolean boundary_found = FALSE;
    XilStatus status = XIL_FAILURE;
  
    if ((ptr = cbm->getNextBytes(&nbytes)) == NULL) {
        return XIL_FAILURE;
    }
  
    while(!boundary_found) {

        while(nbytes != 0 && !boundary_found) {
            if(*ptr == MARKER) {

                if(nbytes == 1) {
                    // we need more bytes to examine
                    if((ptr = cbm->getNextBytes(&nbytes)) == NULL) {
                        return XIL_FAILURE;
                    }
                } else {
                    // increment pointer to next byte
                    ptr++;
                    nbytes--;
                }

                if(*ptr == EOI) {
                    ptr++;
                    boundary_found = TRUE;
                    status = cbm->foundNextFrameBoundary(ptr);
                } else if(*ptr != MARKER) {
                    // If MARKER, process next time in loop
                    ptr++;
                    nbytes--;
                }
            } else {
                ptr++;
                nbytes--;
            }
        }

        if(!boundary_found) {
            if((ptr = cbm->getNextBytes(&nbytes)) == NULL) {
                return XIL_FAILURE;
            }
        }

    }

    return status;
}
