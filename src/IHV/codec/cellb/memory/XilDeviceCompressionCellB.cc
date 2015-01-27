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
//  File:   XilDeviceCompressionCellB.cc
//  Project:    XIL
//  Revision:   1.5
//  Last Mod:   10:15:30, 03/10/00
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
#pragma ident   "@(#)XilDeviceCompressionCellB.cc	1.5\t00/03/10  "

#include "XilDeviceCompressionCellB.hh"
#include "XilDeviceManagerCompressionCellB.hh"

XilDeviceCompressionCellB::XilDeviceCompressionCellB(
    XilDeviceManagerCompressionCellB* xdct,
    XilCis*                           xcis)
: XilDeviceCompression(xdct, xcis, 0, FRAMES_PER_BUFFER),
  decompData(&xdct->compmgrData),
  compData()
{
    isOKFlag = FALSE;

    ditherCvt = NULL;

    if(! deviceCompressionValid()) {
        //  Couldn't create internal base XilDeviceCompression object
        XIL_ERROR(system_state, XIL_ERROR_SYSTEM, "di-278", FALSE);
        return;
    }

    if (initValues() == XIL_FAILURE) {
        //  Couldn't create internal CellB compressor object
        XIL_ERROR(system_state, XIL_ERROR_SYSTEM, "di-275", FALSE);
        return;
    }

    isOKFlag = TRUE;
}

//
// Destructor
//
XilDeviceCompressionCellB::~XilDeviceCompressionCellB() 
{
    delete ditherCvt;

    //
    // Destroy the ImageFormat object(s).
    //
    if(inputType != outputType) {
        outputType->destroy();
    }

    inputType->destroy();

}


int
XilDeviceCompressionCellB::initValues()
{
    const unsigned int zero = 0;
    const unsigned int three = 3;

    XilImageFormat* t =
        system_state->createXilImageFormat(zero, zero, three, XIL_BYTE);

    if(t == NULL) {
        XIL_ERROR(system_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }

    inputType = outputType = t;
    imageWidth = 0;
    imageHeight = 0;
    setRandomAccess(FALSE);

    return XIL_SUCCESS;
}

void
XilDeviceCompressionCellB::reset()
{
    if (inputType != outputType) {
      outputType->destroy();
    }

    inputType->destroy();

    initValues();
    compData.reset();
    decompData.reset();
    XilDeviceCompression::reset();
}


//
//  Routines to get references to the Compressor, Decompressor and
//  Attribute specific classes.
//
CellBCompressorData* 
XilDeviceCompressionCellB::getCellBCompressorData()
{
    return &compData;
}

CellBDecompressorData*  
XilDeviceCompressionCellB::getCellBDecompressorData()
{
    return &decompData;
}

int 
XilDeviceCompressionCellB::getMaxFrameSize() 
{
    // If no interframe compression, each 4x4 block of pixels will require
    // 4 bytes of data, so max size = w/4 * h/4 * 4 = w * h / 4
    return (int)inputType->getWidth() * (int)inputType->getHeight() / 4;
}

XilStatus
XilDeviceCompressionCellB::deriveOutputType()  
{
    XilImageFormat* newtype;

    const unsigned int numbands = 3;

    if (imageWidth == 0 || imageHeight == 0) {
      return XIL_FAILURE;
    }

    newtype = system_state->createXilImageFormat(imageWidth,
                                                 imageHeight,
                                                 numbands,
                                                 XIL_BYTE);

    if (newtype == NULL) {
        // out of memory
        XIL_ERROR(system_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }

    setInputType(newtype);  // sets outputType as a side effect
    newtype->destroy();
    return XIL_SUCCESS;
}


//------------------------------------------------------------------------
//
//  Function:   XilDeviceCompressionCellB::seek
//
//  Description:
//      We allow 2 modes of seeking in CellB, selectable by the user.
//      The random_access attribute indicates if backward seeks are legal for
//      the cis; it is "gettable" only, and determined by the state of the
//      user "settable" ignore_history attribute. When ignore_history is FALSE,
//      the seek backward is disabled and any forward seeks keep the history
//      buffers accurate by burning through frames. When ignore_history is
//      TRUE, the seek backward is enabled, and any seek (either forward or
//      backward), takes you to the desired frame without forcing an update of
//      the history buffer.
//
//      DEFERRED EXECUTION NOTE:
//      If we are not ignoring history, then we can get two cases which
//      are not handled in a straightforward manner.  1) We need to decompress
//      the same frame we just finished decompressing.  Since we know that
//      the cellb_frame will be accurate for this one, we handle it by just
//      seeking to the desired location.  2) If we need to decompress a frame
//      further back than read_frame-1, we are sunk.  We have to handle that
//      elsewhere by actually flushing any such decompresses from the dag.
//      This is implemented using seekFlush() as the seek within each molecule
//      and atom.  It cannot be part of seek() because seek is called from
//      the non-deferred part of decompress as well.
//
//      NOTE:  We actually burn forward if necessary, since we have no faster
//             seeking method.
//
//      NOTE: We may want to later enable backwards seeking for non
//            IGNORE_HISTORY by actually processing the bitstream backwards
//            until we have a full frame.
//
//      If the compressor has a reasonable setting for maximum skip, then this
//      should not take too long.
//
//  Parameters:
//      int framenumber    desired framenumber
//
//  Returns:
//      void
//
//------------------------------------------------------------------------

#define NON_FRAME_TYPE (-(XIL_CIS_ANY_FRAME_TYPE))

void
XilDeviceCompressionCellB::seek(int framenumber, Xil_boolean history_update)
{
    int frames_to_burn;
    int seek_type;

    // cbm.seek guarentees to return the current frame if it can't
    // find a frame of the correct type.  Thus we use this to
    // determine how many frames to burn forward in the NON IGNORE_HISTORY
    // case.
    if (decompData.ignoreHistory) {
      seek_type = XIL_CIS_ANY_FRAME_TYPE;
    } else {
      if (history_update == TRUE)
        // must keep history updated
        if (framenumber == cbm->getNextDecompressId() - 1)
          // special case of seek backward by "1"
          seek_type = XIL_CIS_ANY_FRAME_TYPE;
        else
          seek_type = NON_FRAME_TYPE;
      else
        // history not imptr, looking for position only
        seek_type = XIL_CIS_NO_BURN_TYPE;
    }

    frames_to_burn = cbm->seek(framenumber, seek_type);

    if (frames_to_burn>0) {
      burnFrames(frames_to_burn);
    } else if (frames_to_burn<0) {
      // error in seeking
      XIL_ERROR(system_state, XIL_ERROR_SYSTEM, "di-317", TRUE);
      return;
    }

    return;
}

void
XilDeviceCompressionCellB::seekFlush(int framenumber)
{
    if (!decompData.ignoreHistory)
      cis->flushPriorDecompressOps(framenumber);
    seek(framenumber);
}

//
//  Set/Get Member Functions....
//

//
//  setWidth
//
void 
XilDeviceCompressionCellB::setWidth(int w) {
    // Image width has to be multiple of 4
    if (w % 4 || w == 0) {
      XIL_ERROR(system_state, XIL_ERROR_USER, "di-111", TRUE);
      return;
    }
    imageWidth = w;
}

//
//  setHeight
//
void 
XilDeviceCompressionCellB::setHeight(int h) {
    // Image width has to be multiple of 4
    if (h % 4 || h == 0) {
      XIL_ERROR(system_state, XIL_ERROR_USER,"di-111",TRUE);
      return;
    }
    imageHeight = h;
}

void 
XilDeviceCompressionCellB::setIgnoreHistoryFlag(Xil_boolean value)
{
    // If we are moving into a state where we do not care about the
    // history, but we have any decompresses which DO care, we need
    // to flush them.
    if (value)
      cis->flushPriorDecompressOps(cis->getReadFrame());
    decompData.ignoreHistory = value;
    setRandomAccess(value);
}

Xil_boolean 
XilDeviceCompressionCellB::getIgnoreHistoryFlag()
{
   return decompData.ignoreHistory;
}

//
// Verify that the constraints for decompressColorConvert Molecules
// are satisfied (ycc601->rgb709 color cvt)
//
Xil_boolean
XilDeviceCompressionCellB::validDecompressColorConvert(XilImage* src,
                                                      XilImage* dst)
{
    //
    // Color spaces must be Ycc601 src, Rgb709 dst
    //
    XilColorspace* src_cspace = src->refColorspace();
    XilColorspace* dst_cspace = dst->refColorspace();

    XilSystemState* state = getSystemState();
    XilColorspace*  desired_src_cspace =
        (XilColorspace*)state->getObjectByName("ycc601", XIL_COLORSPACE);
    XilColorspace* desired_dst_cspace =
        (XilColorspace*)state->getObjectByName("rgb709", XIL_COLORSPACE);

    if(src_cspace->getOpcode() != desired_src_cspace->getOpcode() ||
       dst_cspace->getOpcode() != desired_dst_cspace->getOpcode()) {
         return FALSE;
    }

    return TRUE;
}

//
// Verify that the constraints for decompressOrderedDither
// are satisfied (4x4 dither mask, 855 colorcube)
//
Xil_boolean
XilDeviceCompressionCellB::validDecompressOrderedDither(
          XilLookupColorcube* cube,
          XilDitherMask*      dmask)
{
    //
    // Dithermask must be 4x4 3-band, but contents don't matter
    //
    if(dmask->getWidth()    != 4 ||
       dmask->getHeight()   != 4 ||
       dmask->getNumBands() != 3) {
        return FALSE;
    }
    //
    // Colorcube must be the 855 variety, suitable for yuv2rgb conversion
    //

    const int*          mults = cube->getMultipliers();
    const unsigned int* dims  = cube->getDimensions();
    if(dims[0] != 8 || dims[1] != 5 || dims[2] != 5 ||
       mults[0] != 1 || mults[1] != 8 || mults[2] != 40) {
        return FALSE;
    }

    return TRUE;
}
//
// Function to retrieve or create dither tables.
// This is located in the DeviceCompression because
// different CIS'es may be using different dither matrices.
// So each CIS must have its own table.
//
XiliOrderedDitherLut*
XilDeviceCompressionCellB::getDitherTable(XilLookupColorcube* cmap,
                                         XilDitherMask*      dmask,
                                         float*              scale,
                                         float*              offset)
{
    //
    // We can potentially use the existing table.
    // But first, check if the cmap and dmask versions have changed.
    // If so, then we need to create a new table.
    // This is also true for the first call.
    // Use a mutex lock to serialize access here.
    //
    //
    mutex.lock();
    if(ditherCvt == NULL                        ||
       !cmap->isSameAs(&current_cmap_version)     ||
       !dmask->isSameAs(&current_dmask_version)   ||
       scale[0]  != current_scale[0]              ||
       scale[1]  != current_scale[1]              ||
       scale[2]  != current_scale[2]              ||
       offset[0] != current_offset[0]             ||
       offset[1] != current_offset[1]             ||
       offset[2] != current_offset[2] ) {

        ditherCvt = new XiliOrderedDitherLut(cmap, dmask, scale, offset);
        if(! ditherCvt->isOK()) {
            mutex.unlock();
            return NULL;
        }

        //
        // Record new version info
        //
        cmap->getVersion(&current_cmap_version);
        dmask->getVersion(&current_dmask_version);

        //
        // Record new scale, offset info
        //
        current_scale[0]  = scale[0];
        current_scale[1]  = scale[1];
        current_scale[2]  = scale[2];
        current_offset[0] = offset[0];
        current_offset[1] = offset[1];
        current_offset[2] = offset[2];
    }

    mutex.unlock();

    return ditherCvt;
}

