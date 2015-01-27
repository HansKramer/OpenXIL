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
//  File:	XilDeviceIOxshm.hh
//  Project:	XIL
//  Revision:	1.8
//  Last Mod:	10:22:34, 03/10/00
//  SID:        %Z% %F% %I% %U% %E%
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
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilDeviceIOxshm.hh	1.8\t00/03/10  "

#ifndef _XIL_DEVICE_IO_XSHM_HH
#define _XIL_DEVICE_IO_XSHM_HH

#include <valarray>

//
// Xlib includes
//
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>

//
// C++ Includes
//
#include <xil/xilGPI.hh>

const unsigned int lut_bands = 3;
const unsigned int lut_levels = 256;


struct RegionData {
    int src_x;
    int src_y;
    int dst_x;
    int dst_y;
    int x_size;
    int y_size;
};


class XilDeviceIOxshm : public XilDeviceIO {
public:
    //
    //  Constructor.
    //
    XilDeviceIOxshm(XilDeviceManagerIO* device_manager,
                    XilSystemState*     state,
		    Display*            dpy,
		    Window              win);

    //
    //  Destructor.
    //
    ~XilDeviceIOxshm();

    //
    // Display an image on the device
    //
    XilStatus display(XilOp*       op,
                      unsigned int op_count,
		      XilRoi*      roi,
		      XilBoxList*  bl);

    //
    //  Double buffer shit
    //
    XilStatus swapBuffers();

    //
    // Capture an image from the device
    //
    XilStatus capture(XilOp*       op,
                      unsigned int op_count,
		      XilRoi*      roi,
		      XilBoxList*  bl);
	
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
    //  Double buffered
    //
    Xil_boolean isDoubleBuffered();

    void setDoubleBuffered(Xil_boolean flag);
    
    //
    // Return the image created by the device to the
    // core, which can then attach the device to it.
    //
    XilImage*            constructControllingImage();

    //
    //  Assorted molecules.
    //
    XilStatus  setValueDisplayPreprocess(XilOp*        op,
                                         unsigned int  op_count,
                                         XilRoi*       roi,
                                         void**        compute_data,
                                         unsigned int* func_ident);
    XilStatus  setValueDisplay(XilOp*       op,
                               unsigned int op_count,
                               XilRoi*      roi,
                               XilBoxList*  bl);

    XilStatus  cast1to8DisplayPreprocess(XilOp*        op,
                                         unsigned int  op_count,
                                         XilRoi*       roi,
                                         void**        compute_data,
                                         unsigned int* func_ident);
    XilStatus  cast1to8Display(XilOp*       op,
                               unsigned int op_count,
                               XilRoi*      roi,
                               XilBoxList*  bl);

    Xil_boolean          isOK()
    {
        return isOKFlag;
    }
private:
    Xil_boolean          isOKFlag;

    //
    //  Local X Information
    //
    Display*             origDisplay;
    Display*             xDisplay;
    Window               xWindow;
    XImage*              xImage;
    unsigned int         xDepth;
    Visual*              xVisual;
    GC                   xGC;
    GC                   oGC;
    XWindowAttributes    xWindowAttribs;
    XShmSegmentInfo      xshmSegmentInfo;
    int                  xshmCompleteType;
    Xil_boolean          isXBGR;
    Xil_boolean          isXRGB;
    int                  rLSB;
    int                  gLSB;
    int                  bLSB;

    //
    //  Local XIL Information
    //
    XilSystemState*      systemState;
    XilImage*            parentImage;
    XilImage*            controllingImage;
    unsigned int         controllingImageBandOffset;
    XilColorspace*       colorspace;
    Xil_boolean          destroyColorspace;

    //
    //  Variable protection from multiple threads.
    //
    XilMutex             mutex;

    //
    //  Lookup representing the colormap of the device
    //
    XilLookupSingle*     deviceLookup;
    Xil_unsigned8        lookupData[lut_bands * lut_levels];

    //
    //  Support for cast1->8 molecule.
    //
    XImage*              bitXImage;
    Pixmap               bitPixmap;
    GC                   castGC;

    //
    //  Double Buffer Support
    //
    Pixmap                    offScreenBuffer;
    Xil_boolean               doubleBuffered;
    int                       nRegions;
    std::valarray<RegionData> regionData;
};

//
//  Structure to store error handling information...
//
struct XiliXShmAttachInfo {
    int attachFailed;
    int major_code;
    int (*oldHandler)(Display*, XErrorEvent*);
};

extern XiliXShmAttachInfo xili_xshm_attach_info;

#endif // _XIL_DEVICE_IO_XSHM_HH
