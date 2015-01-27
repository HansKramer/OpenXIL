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
//  File:	XilDeviceIOcg6.hh
//  Project:	XIL
//  Revision:	1.19
//  Last Mod:	10:22:33, 03/10/00
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
//  MT-level:  <SAFE>
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilDeviceIOcg6.hh	1.19\t00/03/10  "

#ifndef _XIL_DEVICE_IO_CG6_HH
#define _XIL_DEVICE_IO_CG6_HH

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/cg6reg.h>
#include <sys/cg6fbc.h>
#include <dga/dga.h>

//
// C++ Includes
//
#include <xil/xilGPI.hh>
#include "XilDeviceManagerIOcg6.hh"

const unsigned int lut_bands = 3;
const unsigned int lut_levels = 256;

class XilDeviceIOcg6 : public XilDeviceIO {
public:
    //
    //  Constructor.
    //
              XilDeviceIOcg6(XilSystemState* state,
			     Display* dpy,
			     Window   win,
			     XilDeviceManagerIOcg6* mgr);

    //
    //  Destructor.
    //
              ~XilDeviceIOcg6();

    //
    // Display an image on the device
    //
    XilStatus display(XilOp*       op,
                      unsigned int op_count,
		      XilRoi*      roi,
		      XilBoxList*  bl);

    //
    // Capture an image from the device
    //
    XilStatus capture(XilOp*       op,
                      unsigned int op_count,
		      XilRoi*      roi,
		      XilBoxList*  bl);

    // ----  Begin Molecules ----
    //
    //  Add const to an image and display on the device
    //
    XilStatus addConstDisplay(XilOp*       op,
                              unsigned int op_count,
                              XilRoi*      roi,
                              XilBoxList*  bl);
    XilStatus addConstDisplayPre(XilOp*        op,
                                 unsigned int  op_count,
                                 XilRoi*       roi,
                                 void**        compute_data,
                                 unsigned int* func_ident);
    XilStatus addConstDisplayPost(XilOp*       op,
                                  void*        compute_data);

    //
    //  Copy from area to other area on screen.
    //
    XilStatus captureCopyDisplay(XilOp*       op,
                                 unsigned int op_count,
                                 XilRoi*      roi,
                                 XilBoxList*  bl);

    //
    //  Copy from area to other area on screen.
    //
    XilStatus cast1to8Display(XilOp*       op,
                              unsigned int op_count,
                              XilRoi*      roi,
                              XilBoxList*  bl);

    //
    //  Accelerate copy of image onto display.
    //
    XilStatus copyDisplay(XilOp*       op,
                          unsigned int op_count,
                          XilRoi*      roi,
                          XilBoxList*  bl);

    //
    //  Accelerate copy of BIT image onto display.
    //
    XilStatus lookup1to8Display(XilOp*       op,
                                unsigned int op_count,
                                XilRoi*      roi,
                                XilBoxList*  bl);

    //
    //  Accelerate lookup of BYTE image onto display.
    //
    XilStatus lookup8Display(XilOp*       op,
                             unsigned int op_count,
                             XilRoi*      roi,
                             XilBoxList*  bl);
    XilStatus lookup8DisplayPre(XilOp*        op,
                                unsigned int  op_count,
                                XilRoi*       roi,
                                void**        compute_data,
                                unsigned int* func_ident);
    XilStatus lookup8DisplayPost(XilOp*       op,
                                 void*        compute_data);

    //
    //  Multiply a const to an image and display on the device
    //
    XilStatus mulConstDisplay(XilOp*       op,
                              unsigned int op_count,
                              XilRoi*      roi,
                              XilBoxList*  bl);
    XilStatus mulConstDisplayPre(XilOp*        op,
                                 unsigned int  op_count,
                                 XilRoi*       roi,
                                 void**        compute_data,
                                 unsigned int* func_ident);
    XilStatus mulConstDisplayPost(XilOp*       op,
                                  void*        compute_data);


    //
    //  Accelerate rescale of image onto display.
    //
    XilStatus rescaleDisplay(XilOp*       op,
                             unsigned int op_count,
                             XilRoi*      roi,
                             XilBoxList*  bl);
    XilStatus rescaleDisplayPre(XilOp*        op,
                                unsigned int  op_count,
                                XilRoi*       roi,
                                void**        compute_data,
                                unsigned int* func_ident);
    XilStatus rescaleDisplayPost(XilOp*       op,
                                 void*        compute_data);

    //
    //  Set value into the display
    //
    XilStatus setValueDisplay(XilOp*       op,
                              unsigned int op_count,
                              XilRoi*      roi,
                              XilBoxList*  bl);

    //
    //  Scale nearest neighbor into the display
    //
    XilStatus scaleNearestDisplay(XilOp*       op,
				  unsigned int op_count,
				  XilRoi*      roi,
				  XilBoxList*  bl);

    //
    //  Accelerate translation of image onto display.
    //
    XilStatus translateNearestDisplay(XilOp*       op,
                                      unsigned int op_count,
                                      XilRoi*      roi,
                                      XilBoxList*  bl);

    //
    //  Accelerate window level of SHORT image onto display.
    //
    XilStatus winlevDisplay(XilOp*       op,
                            unsigned int op_count,
                            XilRoi*      roi,
                            XilBoxList*  bl);
    XilStatus winlevDisplayPre(XilOp*        op,
                               unsigned int  op_count,
                               XilRoi*       roi,
                               void**        compute_data,
                               unsigned int* func_ident);
    XilStatus winlevDisplayPost(XilOp*       op,
                                void*        compute_data);

    //
    // ----  End Molecules ----

    //
    // Get a pixel from the device
    //
    XilStatus getPixel(unsigned int x,
		       unsigned int y,
		       float*       data,
		       unsigned int offset_band,
		       unsigned int nbands);
    
    //
    // Set a pixel on the device
    //
    XilStatus setPixel(unsigned int x,
		       unsigned int y,
		       float*       data,
    		       unsigned int offset_band,
		       unsigned int nbands);
    //
    // Set an attribute on the device
    //
    XilStatus setAttribute(const char* attribute_name,
                           void*       value);


    //
    // Get an attribute from the device
    //
    XilStatus getAttribute(const char* attribute_name,
                           void**      value);

    //
    // Is the device readable
    //
    Xil_boolean isReadable();

    //
    // Is the device writable
    //
    Xil_boolean isWritable();

    //
    // Return the image created by the device to the
    // core, which can then attach the device to it.
    //
    XilImage*            constructControllingImage();

    Xil_boolean          isOK()
    {
        return isOKFlag;
    }

private:
    Xil_boolean          isOKFlag;

    //
    //  Convert from BIT to BYTE display using given colors.
    //
    XilStatus            generic1to8(XilOp*       op,
                                     XilRoi*      roi,
                                     XilBoxList*  bl,
                                     unsigned int fcolor,
                                     unsigned int bcolor);

    //
    //  Lookup from BYTE to BYTE display using given colors.
    //
    XilStatus            lookup8to8(XilOp*         op,
                                    XilRoi*        roi,
                                    XilBoxList*    bl,
                                    Xil_unsigned8* data);

    //
    //  Device manager...
    //
    XilDeviceManagerIOcg6* deviceManager;

    XilImage*              controllingImage; // Image for the device
    
    //
    // Local information
    //
    XilSystemState*      stateptr;    // System state passed in
    Display*             displayptr;  // Display * passed in
    Window               window;      // Window passed in
    Dga_drawable         dga_draw;    // DGA drawable
    int                  fd;          // Device file descriptor
    int                  retained_grabbed;
    int                  retained_changed;
    
    //
    //  Colormap Installation Info
    //
    Dga_cmap             dga_cmap;
    Colormap             xcmap;

    //
    //  Colorspace attribute value
    //
    XilColorspace*       colorspace;
    Xil_boolean          destroyColorspace;
    
    //
    //  Lookup representing the colormap of the device for lookup aquisition.
    //
    XilMutex             deviceMutex;
    XilLookupSingle*     deviceLookup;
    Xil_unsigned8        lookupData[lut_bands * lut_levels];

    //
    //  CG6 Specific Info
    //
    short                fb_width;
    short                fb_height;
    int                  fb_size;
    unsigned char*       fb_mem;
    volatile fbc*        fb_fbc;
    int                  fhc_config;
    int*                 dac_base;
    int*                 tec_base;
    cg6_cmap*            cg6cmap;

    //
    //  We currently use a single static mutex to protect multiple threads
    //  from modifying the GX registers at the same time.
    //
    static XilMutex      dgaRegistersMutex;
    XilMutex             dgaLockMutex;
    unsigned int         dgaLockRefCnt;

    void                 lockDGA(Xil_boolean lock_retained_grab = TRUE,
                                 Xil_boolean lock_registers     = TRUE);
    void                 unlockDGA(Xil_boolean unlock_registers = TRUE);

    //
    //  DGA Window Information
    //
    short*               dgaClipList;
    int                  winX;
    int                  winY;
    int                  winWidth;
    int                  winHeight;

    //
    //  Retained window information
    //
    int                  modif_flag;
    int                  rtnchg_flag;
    int                  rtnactive_flag;
    short                rtn_width;
    short                rtn_height;
    u_int                rtn_linebytes;
    int                  rtn_cached;
    unsigned char*       bs_ptr;
};

#endif // _XIL_DEVICE_IO_CG6_HH
