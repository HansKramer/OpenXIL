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
//  File:	XilDeviceIOxlib.hh
//  Project:	XIL
//  Revision:	1.18
//  Last Mod:	10:22:31, 03/10/00
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
#pragma ident	"@(#)XilDeviceIOxlib.hh	1.18\t00/03/10  "

#ifndef _XIL_DEVICE_IO_XLIB_HH
#define _XIL_DEVICE_IO_XLIB_HH

#include <valarray>

//
// Xlib includes
//
#include <X11/Xlib.h>
#include <X11/Xutil.h>

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


class XilDeviceIOxlib : public XilDeviceIO {

public:

	//  Mutex to serialize access to the entire X library
	//  The mutex and the body of the inlines maniupulating
	//  disappear unless X_NOT_MT_SAFE is defined
	//  See lockX and unlockX below
#if defined(X_NOT_MT_SAFE)
	static XilMutex      XMutex;
#endif

    //
    //  Constructor.
    //
    XilDeviceIOxlib(XilDeviceManagerIO* device_manager,
                    XilSystemState*     state,
		    Display*            dpy,
		    Window              win);

    //
    //  Destructor.
    //
    ~XilDeviceIOxlib();

    //
    //  Display an image on the device
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
    XilStatus  copyDisplayPreprocess(XilOp*        op,
                                     unsigned int  op_count,
                                     XilRoi*       roi,
                                     void**        compute_data,
                                     unsigned int* func_ident);
    XilStatus  copyDisplay(XilOp*       op,
                           unsigned int op_count,
                           XilRoi*      roi,
                           XilBoxList*  bl);

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

private:
    //
    // Local information
    //
    XilSystemState*      stateptr;    // System state passed in
    Display*             displayptr;  // Display * passed in
    Window               window;      // Window passed in
    XImage*              ximage;      // XImage for this device
    unsigned int         x_depth;     // X window depth
    unsigned int         parent_bands;// Number of bands in the parent image
    GC                   gc;          // Our graphics context
    XWindowAttributes    window_attr; // Window attributes
    Visual*              visual;      // Window visual
    XilImage*            parentImage; // Parent if we need a child
    XilImage*            controllingImage;    // Image for the device
    XilColorspace*       colorspace;  // Colorspace for the device
    Xil_boolean          destroyColorspace;
    XilMutex             mutex;
    Xil_boolean          allocXImageData; // True if we alloc ximage->data 
    int                  ximagePS;    // the ximage's pixel stride
    int                  rTrim;       // # of LSBs to remove from 8 bit color
    int                  gTrim;
    int                  bTrim;
    int                  rLSB;        // Position of LSB in a plane mask
    int                  gLSB;
    int                  bLSB;

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

    //
    //  Lookup representing the colormap of the device
    //
    XilLookupSingle*     deviceLookup;
    Xil_unsigned8        lookupData[lut_bands * lut_levels];


	void                 lockX(void) {
#if defined(X_NOT_MT_SAFE)
							 XMutex.lock();
#endif
						 }

	void                 unlockX(void) {
#if defined(X_NOT_MT_SAFE)
							 XMutex.unlock();
#endif
						 }

    void reformatToXImage(Xil_unsigned8* src,
                          unsigned int srcPS,
                          unsigned int srcSS,
                          Xil_unsigned8* dst,
                          unsigned int dstSS,
                          unsigned int width,
                          unsigned int height);

    void reformatFromXImage(Xil_unsigned8* src,
                            unsigned int srcSS,
                            Xil_unsigned8* dst,
                            unsigned int dstPS,
                            unsigned int dstSS,
                            unsigned int width,
                            unsigned int height);

    void reformatTo24MSB(Xil_unsigned8* src,
                         unsigned int srcPS,
                         unsigned int srcSS,
                         Xil_unsigned8* dst,
                         unsigned int dstSS,
                         unsigned int width,
                         unsigned int height);

    void reformatTo24LSB(Xil_unsigned8* src,
                         unsigned int srcPS,
                         unsigned int srcSS,
                         Xil_unsigned8* dst,
                         unsigned int dstSS,
                         unsigned int width,
                         unsigned int height);

    void reformatTo32MSB(Xil_unsigned8* src,
                         unsigned int srcPS,
                         unsigned int srcSS,
                         Xil_unsigned8* dst,
                         unsigned int dstSS,
                         unsigned int width,
                         unsigned int height);

    void reformatTo32LSB(Xil_unsigned8* src,
                         unsigned int srcPS,
                         unsigned int srcSS,
                         Xil_unsigned8* dst,
                         unsigned int dstSS,
                         unsigned int width,
                         unsigned int height);

    void reformatFrom24MSB(Xil_unsigned8* src,
                           unsigned int srcSS,
                           Xil_unsigned8* dst,
                           unsigned int dstPS,
                           unsigned int dstSS,
                           unsigned int width,
                           unsigned int height);

    void reformatFrom24LSB(Xil_unsigned8* src,
                           unsigned int srcSS,
                           Xil_unsigned8* dst,
                           unsigned int dstPS,
                           unsigned int dstSS,
                           unsigned int width,
                           unsigned int height);

    void reformatFrom32MSB(Xil_unsigned8* src,
                           unsigned int srcSS,
                           Xil_unsigned8* dst,
                           unsigned int dstPS,
                           unsigned int dstSS,
                           unsigned int width,
                           unsigned int height);

    void reformatFrom32LSB(Xil_unsigned8* src,
                           unsigned int srcSS,
                           Xil_unsigned8* dst,
                           unsigned int dstPS,
                           unsigned int dstSS,
                           unsigned int width,
                           unsigned int height);

    int findLSB(unsigned long planemask);

    int countBits(unsigned long mask,
                  int           lsb);


};

#endif // _XIL_DEVICE_IO_XLIB_HH

