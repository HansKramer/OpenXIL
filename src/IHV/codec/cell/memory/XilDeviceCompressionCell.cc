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
//  File:   XilDeviceCompressionCell.cc
//  Project:    XIL
//  Revision:   1.7
//  Last Mod:   10:10:48, 02/11/98
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
#pragma ident   "@(#)XilDeviceCompressionCell.cc	1.7\t98/02/11  "

#include "XilDeviceCompressionCell.hh"
#include "XiliUtils.hh"

XilDeviceCompressionCell::XilDeviceCompressionCell(
    XilDeviceManagerCompression* xdct,
    XilCis*  xcis)
: XilDeviceCompression(xdct, xcis, 0, FRAMES_PER_BUFFER),
  decompData(xcis->getSystemState()),
  compData()
{
    isOKFlag = FALSE;

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

int
XilDeviceCompressionCell::initValues()
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

    return XIL_SUCCESS;
}

void
XilDeviceCompressionCell::reset()
{
    if (inputType != outputType) {
      outputType->destroy();
    }

    inputType->destroy();

    initValues();
    compData.reset();
    decompData.reset();
    cellAttribs.reset();
    XilDeviceCompression::reset();
}

//
// Destructor
//
XilDeviceCompressionCell::~XilDeviceCompressionCell() {
    //
    // Destroy the ImageFormat object(s).
    //
    if(inputType != outputType) {
        outputType->destroy();
    }

    inputType->destroy();
}

//
//   The 3000 is a (possibly) overly large expansion of the maximum Cell frame
//   size.  A more exact computation may be useful here, but probably not
//   necessary.
//
//   An additional 8196 is added because of the new USER_DATA attribute which
//   can (for 1.0) be as large as 8192+4bytes of escape codes.
//

//
// TODO : Explore these values further. For right now assume 1.2 is right.
//
int XilDeviceCompressionCell::getMaxFrameSize(void) {
    return ((int)inputType->getWidth() *
            (int)inputType->getHeight() / 4) + 3000 + 8196;
}

XilStatus
XilDeviceCompressionCell::deriveOutputType()
{
    XilImageFormat* newtype;

    const unsigned int numbands = 3;

    decompressHeader();

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

    if (decompData.initialize(system_state, imageWidth, imageHeight)
               == XIL_FAILURE) {
      return XIL_FAILURE;
    }

    setInputType(newtype);  // sets outputType as a side effect
    newtype->destroy();
    return XIL_SUCCESS;
}

//
//  Set/Get Member Functions....
//

//
//  EncodingType
//
void
XilDeviceCompressionCell::setEncodingType(XilCellEncodingType  t) 
{
    if (t == BTC || t == DITHER) {
      cellAttribs.encodingType = t;
    } else {
      XIL_ERROR(system_state, XIL_ERROR_USER, "di-261", TRUE);
    }
    cellAttribs.version++;
}

XilCellEncodingType
XilDeviceCompressionCell::getEncodingType(void) 
{
    return cellAttribs.encodingType;
}

//
//  ColorMapAdaptionMode
//
void
XilDeviceCompressionCell::setColorMapAdaptionMode(Xil_boolean val) 
{
    // Ensure val is TRUE or FALSE and not something else
    if (val == TRUE || val == FALSE) {
      cellAttribs.colorMapAdaption = val;
    } else {
      //  Bad value for Xil_boolean type
      XIL_ERROR(system_state, XIL_ERROR_USER,"di-305",TRUE);
    }
    cellAttribs.version++;
}

Xil_boolean
XilDeviceCompressionCell::getColorMapAdaptionMode(void) 
{
    return cellAttribs.colorMapAdaption;
}

//
//  CompressorColorMap
//
void
XilDeviceCompressionCell::setCompressorColorMap(XilLookupSingle* tmp_lookup)
{
    if (tmp_lookup) {
      if (tmp_lookup->getOutputNBands() != 3 ||
          tmp_lookup->getOutputDataType() != XIL_BYTE) {
        // "Lookup type mismatch"
        XIL_OBJ_ERROR(system_state, XIL_ERROR_USER,"di-132",TRUE, getCis());
      } else if(tmp_lookup->getNumEntries() < 2) {
        // XilCis: Cell colormaps must contain at least 2 colors
        XIL_OBJ_ERROR(system_state, XIL_ERROR_RESOURCE, "di-306",TRUE,getCis());
      } else {
        Xil_unsigned8* data = (Xil_unsigned8*)tmp_lookup->getData();

        //
        //  RGB colormaps via the Lookup are stored in BGR byte-order.
        //  So, when creating a UsedCmapTable, it's in BGR order.
        //
        if (data == NULL) {
          // Invalid NULL data pointer
          XIL_OBJ_ERROR(system_state, XIL_ERROR_USER,"di-259",TRUE,getCis());
        } else {
          if (cellAttribs.maxCompressorCmapSize < 0 ||
              tmp_lookup->getNumEntries() <=
                  (unsigned int)cellAttribs.maxCompressorCmapSize) {
            cellAttribs.compressorCmap =
                       new UsedCmapTable(data, BGR, 256,
                                         tmp_lookup->getNumEntries());

            if (cellAttribs.compressorCmap==NULL) {
              // out of memory
              XIL_OBJ_ERROR(system_state,
                            XIL_ERROR_RESOURCE,
                            "di-1",
                            TRUE,
                            getCis());
            }

            if (cellAttribs.compressorCmap->ok() == NULL) {
              // Couldn't create internal Cell compressor object
              XIL_OBJ_ERROR(system_state,
                            XIL_ERROR_SYSTEM,
                            "di-275",
                            FALSE,
                            getCis());
              delete cellAttribs.compressorCmap;
              cellAttribs.compressorCmap=NULL;
            }
          } else {
            cellAttribs.compressorCmap =
                        new UsedCmapTable(data, BGR, 256,
                              (unsigned int) cellAttribs.maxCompressorCmapSize);

            if (cellAttribs.compressorCmap==NULL) {
              // out of memory
              XIL_OBJ_ERROR(system_state,
                            XIL_ERROR_RESOURCE,
                            "di-1",
                            TRUE,
                            getCis());
            }

            if (cellAttribs.compressorCmap->ok() == NULL) {
              // Couldn't create internal Cell compressor object
              XIL_OBJ_ERROR(system_state,
                            XIL_ERROR_SYSTEM, 
                            "di-275",
                            FALSE,
                            getCis());
              delete cellAttribs.compressorCmap;
              cellAttribs.compressorCmap=NULL;
            }
          }
        }
      }
    } else {
    // "Invalid NULL lookup"
        XIL_OBJ_ERROR(system_state,
                      XIL_ERROR_USER,
                      "di-131",
                      TRUE,
                      getCis());
    }
    cellAttribs.version++;
}

//
//  rebuildRemap
//  called when DECOMPRESSOR_COLORMAP or RDWR_INDICES is set
//
void
XilDeviceCompressionCell::rebuildRemap(void)
{
    int j = 0;
    for (int i = 0; i < (int)XIL_MAXBYTE+1; i++) {
      if (decompData.rdwrIndices[i]) {
        decompData.remap[j] = i;
        decompData.remapExp[j] = (i << 24) | (i << 16) | (i << 8) | i;
        j++;
      }
    }
}


//
//  DecompressorColorMap
//
void
XilDeviceCompressionCell::setDecompressorColorMap(XilLookupSingle* lkup)
{
    if (lkup) {
      if (lkup->getOutputNBands() != 3 ||
          lkup->getOutputDataType() != XIL_BYTE) {
        // Lookup type mismatch.
        XIL_ERROR(system_state, XIL_ERROR_USER,"di-132",TRUE);
      } else {
        cellAttribs.decompressorCmap = lkup;

        // Get the version info
        lkup->getVersion(&decompData.currentLookupVersion);

        //
        // In case this is not the first time that the user is setting
        // this, and in case the range on this new lookup is smaller,
        // flag as read-only any indices outside of the range of the
        // lookup
        //
        for (int i = lkup->getOffset() - 1; i >= 0; i--) {
          decompData.rdwrIndices[i] = 0;
        }
        for (i = lkup->getOffset() + lkup->getNumEntries();
                  i<(int)XIL_MAXBYTE+1; i++) {
          decompData.rdwrIndices[i] = 0;
        }

        //
        // Rebuild the remap after the above changes to rdwrIndices.
        // If no rdwrIndices are set, then this call will have no effect.
        //
        rebuildRemap();

        //
        // Indicate to decompressHeader that a user colormap update is
        // pending since the colormap has changed.
        //
        decompData.updateUserColormapPending = 1;
      }
    } else {
      cellAttribs.decompressorCmap = NULL;
      // decompData.currentLookupVersion.versionNumber = 0;
    }
    cellAttribs.version++;
}

XilLookup*
XilDeviceCompressionCell::getDecompressorColorMap(void) {
    return cellAttribs.decompressorCmap;
}

//
//  Decompressor Read-Write Indices
//
void
XilDeviceCompressionCell::setReadWriteIndices(XilIndexList* ilist)
{
    if (cellAttribs.decompressorCmap == NULL) {
      // To test this error, set RDWR_INDICES before DECOMPRESSOR_COLORMAP
      // "Must set DECOMPRESSOR_COLORMAP before RDWR_INDICES"
      XIL_ERROR(system_state, XIL_ERROR_USER,"di-63",TRUE);
    } else if (ilist == NULL) {
      // To test this error, pass a NULL for the indexlist address
      // "RDWR_INDICES: Invalid NULL indexlist"
      XIL_ERROR(system_state, XIL_ERROR_USER, "di-64", TRUE);
    } else if (ilist->ncolors > cellAttribs.decompressorCmap->getNumEntries()) {
      // To test this error, set ncolors in the indexlist struct to 257
      // "RDWR_INDICES: Invalid indexlist count"
      XIL_ERROR(system_state, XIL_ERROR_USER, "di-65", TRUE);
    } else {
      //
      // Quoting from the man page, "Setting the list is not cumulative;
      // the list from any previously set attribute call is discarded."
      //
      for (int i = 0; i < (int)XIL_MAXBYTE+1; i++) {
        decompData.rdwrIndices[i] = 0;
      }
      //
      // Quoting from the man page, "Any indices outside the range of
      // the DECOMPRESSOR_COLORMAP look-up table are discarded."
      //
      int low_limit  = cellAttribs.decompressorCmap->getOffset();
      int high_limit = low_limit +
                         cellAttribs.decompressorCmap->getNumEntries() - 1;
      for (i = 0; i < (int)ilist->ncolors; i++) {
        int pix = ilist->pixels[i];
        if (pix >= low_limit && pix <= high_limit) {
          decompData.rdwrIndices[pix] = 1;
        }
      }

      decompData.numRdwrIndices = ilist->ncolors;

      //
      // Rebuild the remap after the above changes to rdwrIndices,
      // and update the user's colormap.
      //
      rebuildRemap();

      //
      // Indicate to decompressHeader that a user colormap update is
      // pending since the colormap has changed.
      //
      decompData.updateUserColormapPending = 1;
    }

    cellAttribs.version++;
}

//
//  KeyFrameInterval
//
void
XilDeviceCompressionCell::setKeyFrameInterval(int interval)
{
    if ((int)interval < 0) {
      //  XilCis: Invalid Cell key frame interval -- negative
      XIL_OBJ_ERROR(system_state, XIL_ERROR_USER, "di-303",TRUE,getCis());
      return;
    }       

    int tmp = cellAttribs.keyFrameInterval;
    cellAttribs.keyFrameInterval = interval;

    if (computeBytesPerFrameGroup() == FALSE) {
      cellAttribs.keyFrameInterval = tmp;
      return;
    }

    //
    //  The CompressorData errorFrame must be recreated because it is
    //  dependent upon the keyFrameInterval to determine its array size.
    //
    if (compData.errorFrame) {
      if (cellAttribs.keyFrameInterval >
           CELL_MAX_KEYFRAME_INTERVAL_WITH_BIT_RATE) {
        cellAttribs.keyFrameInterval = tmp;
        // XilCis: internal error
        XIL_OBJ_ERROR(system_state, XIL_ERROR_SYSTEM, "di-95",TRUE,getCis());
        return;
      }
      delete compData.errorFrame;

      compData.errorFrame    =
            new ErrorInfoFrame(compData.cellWidth,
                               compData.cellHeight,
                               (1<<(cellAttribs.keyFrameInterval-1)));

      if (compData.errorFrame == NULL) {
        cellAttribs.keyFrameInterval = tmp;
        // Out of memory
        XIL_OBJ_ERROR(system_state, XIL_ERROR_RESOURCE, "di-1",TRUE,getCis());
        return;
      }
    }
    cellAttribs.version++;
}

int
XilDeviceCompressionCell::getKeyFrameInterval(void) 
{
    return cellAttribs.keyFrameInterval;
}


//
//  CompressorMaxCmapSize
//
void
XilDeviceCompressionCell::setCompressorMaxCmapSize(int newval) 
{
    if (newval < 2) {
      // XilCis: Cell colormaps must contain at least 2 colors
      XIL_OBJ_ERROR(system_state, XIL_ERROR_RESOURCE, "di-306", TRUE, getCis());
    } else if (newval > 256) {
      // XilCis: Cell colormaps can not contain more than 256 colors
      XIL_OBJ_ERROR(system_state, XIL_ERROR_RESOURCE, "di-307", TRUE, getCis());
    } else if(compData.initialized == TRUE) {
      //
      //  The initialization of compData will occur on the first call to
      //  xil_compress().  Once it is set, it is not changed for the life of
      //  the CIS or until xil_reset() is called.
      //
      // XilCis: Cell COMPRESSOR_MAX_CMAP_SIZE attribute can not be changed
      //           after first xil_compress
      XIL_OBJ_ERROR(system_state, XIL_ERROR_RESOURCE, "di-308", TRUE, getCis());
    } else {
      cellAttribs.maxCompressorCmapSize = newval;
    }
    cellAttribs.version++;
}

int     
XilDeviceCompressionCell::getCompressorMaxCmapSize(void) 
{
    return cellAttribs.maxCompressorCmapSize;
}

//
//  DecompressorMaxCmapSize
//
int     
XilDeviceCompressionCell::getDecompressorMaxCmapSize(void) 
{
    // Call getOutputType() to ensure we've read the first header
    getOutputType();
    return cellAttribs.maxDecompressorCmapSize;
}

//
//  DecompressorFrameRate
//
Xil_unsigned32     
XilDeviceCompressionCell::getDecompressorFrameRate(void) 
{
    // Call getOutputType() to ensure we've read the first header
    getOutputType();
    return cellAttribs.decompressorFrameRate;
}

//
//  CompressorFrameRate
//
void  
XilDeviceCompressionCell::setCompressorFrameRate(Xil_unsigned32 rate) 
{
    Xil_unsigned32 tmp = cellAttribs.compressorFrameRate;
    cellAttribs.compressorFrameRate = rate;
    if (computeBytesPerFrameGroup() == FALSE)
      cellAttribs.compressorFrameRate = tmp;
    cellAttribs.version++;
}

Xil_unsigned32  
XilDeviceCompressionCell::getCompressorFrameRate(void) 
{
    return cellAttribs.compressorFrameRate;
}

//
//  (Re)Compute bytesPerFrameGroup
//  MUST BE CALLED IF ANY OF THE COMPONENTS CHANGE
//
Xil_boolean  
XilDeviceCompressionCell::computeBytesPerFrameGroup(void) 
{
    //
    //  If bit-rate control has been enabled and the user has a key-frame
    //  interval greater than 15, then bit-rate control will not work.
    //
    if (cellAttribs.bitsPerSecond>0 &&
        cellAttribs.keyFrameInterval>CELL_MAX_KEYFRAME_INTERVAL_WITH_BIT_RATE) {
      //  XilCis: Cell key frame interval is too large for bit rate control
      XIL_ERROR(system_state, XIL_ERROR_USER, "di-231", TRUE);
      return FALSE;
    }

    cellAttribs.bytesPerFrameGroup = int(
                   (cellAttribs.bitsPerSecond/8.0) *       // bytes/sec
                   (cellAttribs.compressorFrameRate/1e6) * // sec/frame
                   (cellAttribs.keyFrameInterval));        // frames/frame_group

    //
    //  If we're trying to control the output bit-rate and the computed number
    //  of bytes per frame group is greater than the maximum allowable, then
    //  it is an error.  I subtract 2000 from the maxFrameSize because the
    //  value is slightly inflated for the CMB's benefit.  It won't kill the
    //  bit-rate control if this assumption is wrong, the number of bytes output
    //  will still be one key-frame's worth.  It's possible that this
    //  key-frame is greater than the number of bytes the user asked us to
    //  provide.
    //
    if (cellAttribs.bitsPerSecond>0 &&
        cellAttribs.bytesPerFrameGroup < (getMaxFrameSize() - 2000)) {
      XIL_ERROR(system_state, XIL_ERROR_USER, "di-230", TRUE);
      return FALSE;
    }

    return TRUE;
}

//
//  bitsPerSecond
//
void  
XilDeviceCompressionCell::setBitsPerSecond(int bps) 
{
    int tmp = cellAttribs.bitsPerSecond;
    cellAttribs.bitsPerSecond = bps;

    if (computeBytesPerFrameGroup() == FALSE) {
      cellAttribs.bitsPerSecond = tmp;
      return;
    }

    //
    //  The CompressorData errorFrame must be created if it hasn't already
    //  because it can become VERY large depending upon the keyFrameInterval.
    //  So, it's creation is deferred until bit-rate control is enabled.
    //

    if (cellAttribs.bytesPerFrameGroup &&
        compData.errorFrame == NULL    && compData.initialized == TRUE)
    {
      compData.errorFrame    =
            new ErrorInfoFrame(compData.cellWidth,
                               compData.cellHeight,
                               (1<<(cellAttribs.keyFrameInterval-1)));

      if (compData.errorFrame == NULL) {
        cellAttribs.bitsPerSecond = tmp;
        XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return;
      }
    } else if(cellAttribs.bytesPerFrameGroup == 0) {
      delete compData.errorFrame;
      compData.errorFrame = NULL;
    }
    cellAttribs.version++;
}

int     
XilDeviceCompressionCell::getBitsPerSecond(void) 
{
    return cellAttribs.bitsPerSecond;
}

//
//  TemporalFiltering
//
void    
XilDeviceCompressionCell::setTemporalFiltering(Xil_boolean newval) 
{
    // Ensure val is TRUE or FALSE and not something else
    if (newval == TRUE || newval == FALSE) {
      if (cellAttribs.temporalFiltering == TRUE && newval == FALSE) {
        delete [] compData.previousImage;
        compData.previousImage = NULL;
      }

      cellAttribs.temporalFiltering = newval;
    } else {
      //  Bad value for Xil_boolean type
      XIL_ERROR(system_state, XIL_ERROR_USER,"di-305",TRUE);
    }
    cellAttribs.version++;
}

Xil_boolean     
XilDeviceCompressionCell::getTemporalFiltering(void) {
    return cellAttribs.temporalFiltering;
}

//
//  TemporalFilteringLow
//
void
XilDeviceCompressionCell::setTemporalFilterLow(unsigned int newval)
{
    cellAttribs.lowFilterThreshold = newval;
    cellAttribs.version++;
}

Xil_boolean     
XilDeviceCompressionCell::getTemporalFilterLow(void) 
{
    return cellAttribs.lowFilterThreshold;
}

//
//  TemporalFilteringHigh
//
void
XilDeviceCompressionCell::setTemporalFilterHigh(unsigned int newval)
{
    cellAttribs.highFilterThreshold = newval;
    cellAttribs.version++;
}

Xil_boolean     
XilDeviceCompressionCell::getTemporalFilterHigh(void) 
{
    return cellAttribs.highFilterThreshold;
}

//
//  Cell User Data
//
void    XilDeviceCompressionCell::setUserData(XilCellUserData* newval) 
{
    //
    //  If the size of the data is too large, its an error.
    //
    if (newval->length > CELL_MAX_USER_DATA_SIZE) {
      XIL_ERROR(system_state, XIL_ERROR_USER, "di-283", TRUE);
      return;
    }

    //
    //  If the size is 0, then we're done because there's no use in outputting
    //  the skip codes for no data.
    //
    if (newval->length == 0) return;

    cellAttribs.compressorUserData = new CellUserData;

    if (cellAttribs.compressorUserData == NULL) {
      XIL_OBJ_ERROR(system_state, XIL_ERROR_RESOURCE,
                      "di-1", TRUE, getCis());
      return;
    }

    //
    //  Currently, the data is copied because every frame is being buffered.
    //  So, a copy of the user data must be associated with each frame and
    //  then output when the iterframe compression is done.
    //
    //  In the case we're putting out every frame as a key-frame or every
    //  frame is interencoded, then it probably doesn't make sense to copy it
    //  since we're not buffering intraframes.
    //
    cellAttribs.compressorUserData->data = new Xil_unsigned8[newval->length];

    if (cellAttribs.compressorUserData->data == NULL) {
      XIL_OBJ_ERROR(system_state, XIL_ERROR_RESOURCE,
                      "di-1", TRUE, getCis());
      delete cellAttribs.compressorUserData;
      cellAttribs.compressorUserData=NULL;
      return;
    }

    if (newval->data == NULL) {
      XIL_OBJ_ERROR(system_state, XIL_ERROR_RESOURCE, "di-259", TRUE, getCis());
      delete cellAttribs.compressorUserData->data;
      delete cellAttribs.compressorUserData;
      cellAttribs.compressorUserData=NULL;
      return;
    }

    xili_memcpy(cellAttribs.compressorUserData->data, 
               newval->data, 
               newval->length);

    cellAttribs.compressorUserData->length = newval->length;
    cellAttribs.version++;
}

XilCellUserData*  
XilDeviceCompressionCell::getUserData(void) 
{
    static XilCellUserData retdata;

    if (cellAttribs.decompressorUserData) {
      retdata.data   = cellAttribs.decompressorUserData->data;
      retdata.length = cellAttribs.decompressorUserData->length;
    } else {
      retdata.data   = NULL;
      retdata.length = 0;
    }

    return &retdata;
}

//
//  updateUserColormap
//
//  In addition to updating any writable indices in DECOMPRESSOR_COLORMAP,
//  this function will adjust the remap arrays for Cell bytestream colors
//  that don't have corresponding writable indices.
//
void
XilDeviceCompressionCell::updateUserColormap(void) 
{
    if (decompData.colormap == NULL) {
      // This will only happen if the decompData constructor failed.
      return;
    }

    Xil_unsigned8 udata [256*3];

    Xil_unsigned8* cdata = (Xil_unsigned8*)decompData.colormap->getData();

    //
    // Get the number of entries in the user's colormap (unum) as well as
    // the Cell bytestream colormap (cnum).
    //
    int unum = cellAttribs.decompressorCmap->getNumEntries();
    int cnum = decompData.colormapEntries;

    int offset = cellAttribs.decompressorCmap->getOffset();
    Xil_unsigned8* remap = decompData.remap;
    int num_rdwr = decompData.numRdwrIndices;

    cellAttribs.decompressorCmap->getValues(offset, unum, udata);

    //
    // First we'll change the DECOMPRESSOR_COLORMAP values corresponding
    // to all of the RDWR_INDICES (if any).  For simplicity's sake (and
    // for speed), if there are N RDWR_INDICES, the first N entries of
    // the bytestream colormap will use those writable indices.  In other
    // words, if there are 3 RDWR_INDICES, 7, 10, & 213, the first three
    // values of the remap array will always be 7, 10, & 213.
    //
    int i = 0;
    int i3 = 0;

    while (i < num_rdwr) {
      int t = remap[i] - offset;
      t += t << 1;   // t = (remap[i] - offset) * 3
      //
      // The Cell bytestream colormap is in RGB order, so
      // decompData.colormap is kept in RGB order as well so it can be
      // quickly read from the bytestream.  However, from the user's point
      // of view, since RGB images are stored in BGR order,
      // colormaps are also in BGR order, so cellAttribs.decompressorCmap
      // is in BGR order as well.
      //
      udata[t+2] = cdata[i3];       // red
      udata[t+1] = cdata[i3+1];     // green
      udata[t]   = cdata[i3+2];     // blue
      i++;
      i3 += 3;
    }

    //
    // If there aren't enough RDWR_INDICES for every color in the Cell
    // bytestream colormap, then for the remaining bytestream colors we'll do
    // a nearest neighbor search on the known user colors (which at this point
    // is everything in the user's colormap, both read-only and read-write
    // colors), and adjust the remap array accordingly.
    //
    // TODO: we could benefit greatly here from having the previous colormap
    // to compare to and doing a nearest color search only on those colormap
    // entries which have changed.  However, if any of the
    // RDWR_INDICES color values changed, we should do the nearest neighbor
    // search again anyway.
    //
    while (i < cnum) {
      int nearest;

      int ir = cdata[i3];
      int ig = cdata[i3+1];
      int ib = cdata[i3+2];
      int dist_squared = 255 * 255 * 3;

      for (int j = 0, j3 = 0; j < unum; j++, j3 += 3) {
#ifdef TODO
      // Replace after sqrs_table is in
        int new_dist = sqrs_table[ir - udata[j3+2]] +
                       sqrs_table[ig - udata[j3+1]] +
                       sqrs_table[ib - udata[j3]] ;
#endif
        int new_dist = ( ( (ir - udata[j3+2]) * (ir - udata[j3+2]) ) +
                         ( (ig - udata[j3+1]) * (ig - udata[j3+1]) ) +
                         ( (ib - udata[j3]  ) * (ib - udata[j3]  ) ) ) ;

        if (new_dist < dist_squared) {
          dist_squared = new_dist;
          nearest = j;
        }
      }

      int new_remap = nearest + offset;

      if (remap[i] != new_remap) {
        remap[i] = new_remap;
        decompData.remapExp[i] = (new_remap << 24) | 
                                 (new_remap << 16) |
                                 (new_remap <<  8) |
                                  new_remap;
        //
        // We've updated the remap array, so set a flag for those
        // molecules which rely on frame buffer contents when processing
        // interframe skip codes.
        //
        decompData.redrawNeeded = 1;
      }

      i++;
      i3 += 3;
    }

    cellAttribs.decompressorCmap->setValues(offset, unum, udata);

    // Get version info
    cellAttribs.decompressorCmap->getVersion(&decompData.currentLookupVersion);
}


