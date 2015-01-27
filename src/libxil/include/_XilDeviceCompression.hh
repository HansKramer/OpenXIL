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
//  File:        _XilDeviceCompression.hh
//  Project:        XIL
//  Revision:        1.18
//  Last Mod:        10:22:03, 03/10/00
//
//  Description:
//        
//  Abstract class to provide the entry points for the various
//  compression devices.
//        
//------------------------------------------------------------------------
//        COPYRIGHT
//------------------------------------------------------------------------


#ifndef XIL_DEVICE_COMPRESSION_HH
#define XIL_DEVICE_COMPRESSION_HH

//
//  XIL Includes
//
#include "_XilDefines.h"

#include "_XilDeviceManager.hh"
#include "_XilCis.hh"
#include "_XilCisBufferManager.hh"
#include "_XilDeviceManagerCompression.hh"

class XilDeviceCompression {
public:
    //
    // Pure virtual functions for compress/decompress
    // These MUST be implemented in the derived class
    //
    virtual XilStatus             compress(XilOp*       op,
                                           unsigned int op_count,
                                           XilRoi*      roi,
                                           XilBoxList*  bl) =0;

    virtual XilStatus             decompress(XilOp*       op,
                                             unsigned int op_count,
                                             XilRoi*      roi,
                                             XilBoxList*  bl) =0;

    //
    // Give the deviceCompression an opportunity to decompress 
    // any Cis header before frame decompressions start.
    //
    virtual XilStatus            decompressHeader(void);

    virtual void                 seek(int         framenumber, 
                                      Xil_boolean history_update = TRUE);
    //
    // Flush pending frame compression ops
    //
    virtual void                 flush(void);
    virtual void                 reset(void);

    //
    //  KEEPFRAME/MAXFRAME management call to adjust the
    //    beginning of the CIS
    //
    virtual int                  adjustStart(int new_start_frame);

    //
    //  Getting external data into and out of the CIS
    //
    virtual void*                getBitsPtr(int* nbytes, 
                                            int* nframes);

    virtual void                 putBits(int   nbytes, 
                                         int   nframes, 
                                         void* data);

    virtual void                 putBitsPtr(int   nbytes,
                                            int   nframes,
                                            void* data,
                                            XIL_FUNCPTR_DONE_WITH_DATA 
                                            done_with_data = NULL);
    //
    //   The state of data in the cis
    //
    virtual int                  hasData();
    virtual int                  numberOfFrames();
    virtual Xil_boolean          hasFrame();

    //
    //  Partial frame handling
    //
    virtual int                  findNextFrameBoundary();

    //
    // Determine output type from bit stream (may require parsing)
    //
    virtual XilStatus            deriveOutputType(void);
    
    //
    // Called if the inputType is unknown, and the width & height are
    // derived from the first input image.  Can modify outputType as well.
    //
    virtual XilStatus            setInputType(XilImageFormat* image_format);

    //
    // Direct support of attributes from XilCis
    // get only attributes (set at create time or by compressor) 
    //
    char*                        getCompressor();
    char*                        getCompressionType();
    Xil_boolean                  getRandomAccess();
    void                         setRandomAccess(Xil_boolean random);
    XilImageFormat*              getInputType();
    XilImageFormat*              getOutputType();
    XilImageFormat*              getOutputTypeHoldTheDerivation();

    XilCisBufferManager*         getCisBufferManager();
    XilDeviceManager*            getDeviceManager();
    XilCis*                      getCis();
    
    void                         generateError(XilErrorCategory category, 
                                               char*            id,
                                               int              primary, 
                                               Xil_boolean      read_invalid,
                                               Xil_boolean      write_invalid,
                                               int              line, 
                                               char*            file);    
    

    // These reflect the actual state of the cis, as opposed to the state
    // the user sees (if operations are deferred).
    int                          getStartFrame();
    int                          getReadFrame();
    int                          getWriteFrame();

    //
    //  get/set attributes
    //
    XilStatus                    getAttribute(const char* name, 
                                              void**      value);

    XilStatus                    setAttribute(const char* name, 
                                              void*       value);

    void                         destroy();


    //
    // This is called from XilCis to generate errors.  It can be used to
    // save any state that may be necessary when a xil_cis_error_recovery 
    // is called.  The general approach to generate an error is to use the
    // XIL_CIS_ERROR macro in XilCis.h
    //
    virtual void                  generateError(XilErrorCategory category, 
                                                char*            id,
                                                int              primary, 
                                                int              line, 
                                                char*            file);

    virtual void                  attemptRecovery(unsigned int nframes, 
                                                  unsigned int nbytes,
                                                  Xil_boolean& read_invalid, 
                                                  Xil_boolean& write_invalid);

    Xil_boolean                   inMolecule(); 

    void                          setInMolecule(Xil_boolean on_off);

    XilSystemState*               getSystemState();
    
    //
    // Function to test for containment of cis in dst image
    //
    Xil_boolean                   cisFitsInDst(XilRoi*     roi);


protected:
    //
    // Derived classes will all inherit this method
    //
    Xil_boolean                   deviceCompressionValid();

    virtual void                  burnFrames(int nframes) =0;  
    virtual int                   getMaxFrameSize() =0;
    
    //
    // Constructor / Destructor
    //
                                  XilDeviceCompression(
                                      XilDeviceManagerCompression* xdct,
                                      XilCis*                      xcis,
                                      int                          frame_size,
                                      int                          nfpb);

    virtual                      ~XilDeviceCompression ();

    //
    // Flag updated by derived DeviceCompression constructors
    //
    Xil_boolean                   dcOKFlag;

    XilCisBufferManager*          cbm;
    XilCis*                       cis;
    XilDeviceManagerCompression*  mgr;
    XilSystemState*               system_state;
    XilImageFormat*               inputType;
    XilImageFormat*               outputType;
    Xil_boolean                   random_access;


private:
    void                          initValues();    

    int                           xil_data_type;
    Xil_boolean                   in_molecule;

    void*                         _extra_data[256];
};

#endif
