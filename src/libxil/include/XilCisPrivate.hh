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
//  File:	XilCisPrivate.hh
//  Project:	XIL
//  Revision:	1.20
//  Last Mod:	10:21:07, 03/10/00
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
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilCisPrivate.hh	1.20\t00/03/10  "

#ifdef    _XIL_PRIVATE_INCLUDES

#include "XiliRect.hh"

#endif

#ifdef    _XIL_PRIVATE_DATA
public:

    //
    // Constructor
    //
                        XilCis(XilSystemState* system_state,
                               const char*     compression_name);

    // error handling
    void                generateError(XilErrorCategory category,
                                      const char*      id,
                                      int              primary,
                                      Xil_boolean      read_invalid,
                                      Xil_boolean      write_invalid,
                                      int              line,
                                      const char*      file);

    //
    //  Some functions that are required for hooking into deferred
    //  execution.
    //
    XilRoi*             getGlobalSpaceRoi();
    XiliRect*           getGlobalSpaceRect();
    //
    // the following two are required by XilDeferrableObject but will not
    // be called on cis.
    //
    XilRoi*             getGlobalSpaceRoiWithDoublePrecision()
    {
        return NULL;
    }

    XilRoi*             getExtentGlobalSpaceRoi()
    {
        return NULL;
    }

    unsigned int        getNumTiles() { return 1; }

    XiliOpQueuePosition setOp(XilOp* new_op);

protected:

                        ~XilCis();
    

private:

    XilMutex    mutex;

    int read_frame;      // frame that cis thinks is next to decompress
    int write_frame;     // frame that cis thinks is the last one compressed.
    int start_frame;     // oldest seekable frame (if random_access)
    int keep_frames;	 // # of frames to keep prior to read_frame
    int max_frames;	 // max # of frames in the buffer

    Xil_boolean autorecover;  // autorecover from errors ??
    Xil_boolean read_invalid; // whether the cis is marked read invalid
    Xil_boolean write_invalid; // whether the cis is marked write invalid
    
    XilDeviceCompression*  deviceCompression;
    XilDeviceManagerCompression*  deviceManagerCompression;
    XilCisBufferManager* cbm;
    
    void    initValues(void);

    // deferred execution stuff
    XilOp**         dependents;
    XilOp*          compressOp;
    int             compressOpNumber;
    char*           compressFunctionName;
    int             decompressOpNumber;
    char*           decompressFunctionName;
    unsigned int    dependentCount;

    //
    //  Keep a copy of the globalSpaceRoi around so we don't have to generate
    //  a new one every time.
    //
    XilRoi*         globalSpaceRoi;
    XiliRectInt     globalSpaceRect;
#endif // _XIL_PRIVATE_DATA
