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
//  File:       CellDecompressorData.cc
//  Project:    XIL
//  Revision:   1.4
//  Last Mod:   10:16:02, 03/10/00
//
//  Description:
//
//    Class to hold the data for cell compression.
//
//    TODO: Is this class really needed? It seems like
//          all of this info could just reside in the 
//          XilDeviceCompressionCell object.
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)CellDecompressorData.cc	1.4\t00/03/10  "

#include "XilDeviceCompressionCell.hh"

//------------------------------------------------------------------------
//
//  Function:    CellDecompressorData::reset
//
//  Description:
//    Parts of the constructor that need to be redone on cis_reset
//
//------------------------------------------------------------------------
void
CellDecompressorData::reset()
{
    delete cellFrame;
    cellFrame = NULL;

    delete y2bpArray;
    y2bpArray = NULL;

    colormapEntries = 0;

    for (int i=0; i<(int)XIL_MAXBYTE+1; i++) {
      remap[i]    = i;
      remapExp[i] = (i<<24)|(i<<16)|(i<<8)|i;
    }
      
    numRdwrIndices = 0;
  
    for (i=0; i<(int)XIL_MAXBYTE+1; i++) {
      rdwrIndices[i] = 0;
    }

    delete dith_multi_table;
    dith_multi_table = NULL;

    updateUserColormapPending = 0;
    updateUserColormapEnabled = 1;

    headerFrameNumber = -1;
    bp                = NULL;
    redrawNeeded      = 0;

#ifdef HAS_LIBDGA
  lastDGA_Win = 0;
#endif
}

//------------------------------------------------------------------------
//
//  Function:    CellDecompressorData::CellDecompressorData
//
//  Description:
//    Constructor for the CellDecompressorData class.
//    
//------------------------------------------------------------------------
CellDecompressorData::CellDecompressorData(XilSystemState* systemState)
{
    isok = FALSE;
    
    //
    // Make sure it is safe to call the destructor by setting to NULL all
    // the pointers that are used with 'delete'
    //
    cellFrame    = NULL;
    rdwrIndices  = NULL;
    remap        = NULL;
    remapExp     = NULL;

    dith_multi_table = NULL;
    y2bpArray    = NULL;

    //
    // 'colormap' will point to an XilLookup object where we keep a copy of
    // the RGB colormap from the Cell bytestream; create this object with 
    // enough space for the largest possible colormap in the bytestream (256).
    // Passing NULL as the last argument to createLookup instructs it to
    // allocate space for the data.
    //
    colormap = systemState->createXilLookupSingle(XIL_BYTE,
                                                  XIL_BYTE,
                                                  (unsigned int)3,
                                                  256,
                                                  0,
                                                  NULL);
    if(colormap == NULL) {
        XIL_ERROR(systemState, XIL_ERROR_RESOURCE,"di-112",FALSE);
        return;
    }

    rdwrIndices    = new Xil_unsigned8[XIL_MAXBYTE+1];
    if(rdwrIndices == NULL) {
        XIL_ERROR(systemState, XIL_ERROR_RESOURCE,"di-1",TRUE);
        return;
    }

    remap           = new Xil_unsigned8[XIL_MAXBYTE+1];
    if(remap == NULL) {
        XIL_ERROR( systemState, XIL_ERROR_RESOURCE,"di-1",TRUE);
        return;
    }

    remapExp        = new Xil_unsigned32[XIL_MAXBYTE+1];
    if(remapExp == NULL) {
        XIL_ERROR( systemState, XIL_ERROR_RESOURCE,"di-1",TRUE);
        return;
    }

    reset();

    isok = TRUE;
}

//  Destructor
CellDecompressorData::~CellDecompressorData(void)
{
    colormap->destroy();

    delete cellFrame;
    delete rdwrIndices;
    delete remap;
    delete remapExp;
    delete dith_multi_table;
    delete y2bpArray;
}

//------------------------------------------------------------------------
//
//  Function:    CellDecompressorData::initialize()
//
//  Description:
//  Initializes any data that can not be initialized without knowing the
//  image dimentions. 
//    
//------------------------------------------------------------------------
int CellDecompressorData::initialize(XilSystemState* systemState,
                                     unsigned int    imageWidth,
                                     unsigned int    imageHeight)
{
    // If the CellFrame object has not already been constructed in
    // burnFrames(), then we'll construct it here.
    if(cellFrame == NULL) {
        cellFrame = new CellFrame(imageWidth/4, imageHeight/4);
        if(cellFrame == NULL) {
            XIL_ERROR(systemState, XIL_ERROR_RESOURCE,"di-1",TRUE);
            return XIL_FAILURE;
        }
    }

    if(y2bpArray == NULL) {
        y2bpArray = new (Xil_unsigned8*[(imageHeight/4)+1]);
        if(y2bpArray == NULL) {
            XIL_ERROR(systemState, XIL_ERROR_RESOURCE,"di-1",TRUE);
            return XIL_FAILURE;
        }
    }
    
    return XIL_SUCCESS;
}

