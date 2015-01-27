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
//  File:        _XilCis.hh
//  Project:     XIL
//  Revision:    1.19
//  Last Mod:    10:21:06, 03/10/00
//
//  Description:
//        
//   Provides the compression independent interface for 
//   Compressed Image Sequences (CIS)
//   One of these objects is created when xil_cis_create() is called.
//   Most of these functions map directly to a function in the XIL API.
//   More information can be found in the man pages for xil_cis_*
//   (Convert the internal name to the API function by changing upper case
//   to lower case with a '_' prepended.  Then prepend xil_cis_ to the name,
//   e.g. getCompressor -> xil_cis_get_compressor)
//        
//        
//  MT Level:   UNsafe
//
//        
//------------------------------------------------------------------------
//        COPYRIGHT
//------------------------------------------------------------------------
#pragma ident        "@(#)_XilCis.hh	1.19\t00/03/10  "

#ifndef _XIL_CIS_HH
#define _XIL_CIS_HH

#include "_XilDeferrableObject.hh"
#include "_XilDeviceCompression.hh"
#include "_XilCisBufferManager.hh"

//
//  Private Includes
//
#ifdef  _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_INCLUDES
 
#include "XilCisPrivate.hh"
  
#undef  _XIL_PRIVATE_INCLUDES
#endif

class XilCis : public XilDeferrableObject {
public:
    //
    // Required implementations of virtual functions from XilObject
    //
    XilObject*                   createCopy();

    //
    // Get compressor name (e.g. "Jpeg") or type (e.g. "JPEG")
    //
    char*                        getCompressor();
    char*                        getCompressionType();

    //
    // True if backward seeks are possible
    //
    Xil_boolean                  getRandomAccess();

    //
    // Get the current state of the Cis
    //
    int                          getStartFrame();
    int                          getReadFrame();
    int                          getWriteFrame();
    int                          getKeepFrames();
    int                          getMaxFrames();
    Xil_boolean                  getReadInvalid();
    Xil_boolean                  getAutorecover();

    //
    // Set the state of the cis 
    //
    void                         setStartFrame(int frame);
    void                         setReadFrame(int frame);
    void                         setWriteFrame(int frame);
    void                         setKeepFrames(int k);
    void                         setMaxFrames(int m);
    Xil_boolean                  getWriteInvalid();
    void                         setAutorecover(Xil_boolean on_off);

    //
    // Get input type (compression) or output type (decompression)
    //
    XilImageFormat*              getInputType();
    XilImageFormat*              getOutputType();

    //
    // Test if frames are available for decompression
    //
    int                          hasData();
    Xil_boolean                  hasFrame();
    int                          numberOfFrames();

    //
    // Get/Set codec-specific attributes
    //
    XilStatus                    getAttribute(const char* attribute_name, 
                                              void**      value);
    XilStatus                    setAttribute(const char* attribute_name, 
                                              void*       value);

    //
    // Compression/decompression operations 
    // These put an op onto the DAG.
    // decompress() calls deviceCompression->decompressHeader() before
    // creating the op
    //
    void                         compress(XilImage* src);
    void                         decompress(XilImage* dst);

    //
    // Force any pending compressions to complete
    //
    void                         flush();

    //
    // Remove data from cis and restore its default state
    //
    void                         reset();

    //
    // Set the read_frame to a new location.
    // "relative_to" is analagous to the same argument as in lseek(2)
    //
    void                         seek(int framenumber, 
                                      int relative_to);

    //
    // Attempt recovery from a non-autorecoverable error by
    // processing ahead a maximum of nframes or nbytes
    //
    void                         attemptRecovery(unsigned int nframes, 
                                                 unsigned int nbytes);
    
    //
    // Let the user supply or extract compressed data to/from the Cis.
    // These operations cannot be deferred, since they interact with
    // data from outside of XIL's control.
    //
    void*                        getBitsPtr(int* nbytes, 
                                            int* nframes);

    void                         putBits(int   nbytes, 
                                         int   nframes, 
                                         void* data);

    void                         putBitsPtr(int   nbytes, 
                                            int   nframes,
                                            void* data,
                                            XIL_FUNCPTR_DONE_WITH_DATA 
                                            done_with_data = NULL);

    //
    //  Get the XilDeviceCompression pointer for this CIS
    //  XilDeviceCompression is the device porting interface object for
    //  codecs.  This function allows the device dependent layer to get
    //  a pointer to the XilDeviceCompression object associated with a cis.
    //
    XilDeviceCompression*        getDeviceCompression();

    XilDeviceManagerCompression* getDeviceManagerCompression(); 

    //
    // This routine is used by device compressions which do not have a 
    // reliable way to get back to deferred frames.  They MUST get back
    // to the current frame, but can use this routine to avoid having any
    // frames older than read_frame - 1.  It is in the public part instead
    // of the private in order to avoid having to install XilCisPrivate.h
    // in the install point.
    //
    void                         flushPriorDecompressOps(int frame_no);

private:
#ifdef _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_DATA
 
#include "XilCisPrivate.hh"
  
#undef  _XIL_PRIVATE_DATA
#else
    //
    //  Make it clear to the compiler that a destructor exists.  This is done
    //  so GPI users will get a compile-time error if they attempt to delete
    //  this class.
    //
                                 ~XilCis();
#endif // _XIL_PRIVATE_DATA
};

//
// Error macros
//
#define XIL_CIS_ERROR(category,id,primary,dc, read_invalid, write_invalid) \
do {                                                                      \
    if (!(dc)->inMolecule())                 \
        (dc)->generateError((category), (id), (primary),             \
        (read_invalid), (write_invalid), __LINE__, __FILE__);              \
} while (0); 

#define XIL_CIS_UNCOND_ERROR(category,id,primary,dc, read_invalid, write_invalid) \
    (dc)->generateError((category), (id), (primary), (read_invalid), \
        (write_invalid), __LINE__, __FILE__);


#endif // _XIL_CIS_HH
