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
//  File:       Convolve.cc
//  Project:    XIL
//  Revision:   1.10
//  Last Mod:   10:13:46, 03/10/00
//
//  Description:
//
//    Convolve functions
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)Convolve.cc	1.10\t00/03/10  "

#include "XilDeviceManagerComputeMMXBYTE.hh"
#include "ComputeInfoMMX.hh"


#define LOG_OF_2   0.69314718F

typedef struct {
    char*          scaled_kdata;
    IplConvKernel* iplKernel;
} ConvolveData;

//
// Use a preprocess routine to analyze the kernel and
// produce a scaled integer version, using power of 2 scaling
// as required by the IPL convlve routine.
// 
XilStatus
XilDeviceManagerComputeMMXBYTE::ConvolvePreprocess(XilOp*       op,
                                                   unsigned     ,
                                                   XilRoi*      ,
                                                   void**       compute_data,
                                                   unsigned int*  )
{
    XilKernel*   kernel;
    op->getParam(1, (void**)&kernel);

    int kw = kernel->getWidth();
    int kh = kernel->getHeight();
    int ks = kw*kh;
    const float* kdata = kernel->getData();

    ConvolveData* cd   = new ConvolveData;
    MMX_MEMCHK(cd);

    cd->scaled_kdata = new char[ks];
    MMX_MEMCHK(cd->scaled_kdata);
    
    //
    // Determine the largest absolute value in the kernel
    //
    float kmax = (kdata[0] < 0) ? -kdata[0] : kdata[0];
    for(int i=1; i<ks; i++) {
        float kval =  (kdata[i] < 0) ? -kdata[i] : kdata[i];
        if(kval > kmax) {
            kmax = kval;
        }
    }

    //
    // Calculate the scale factor. This will be the largest
    // power of 2 which, when multiplied by kmax, will not 
    // exceed the range of a byte value. To prevent any
    // chance of overflow, we back off one level to 126.
    //
    float fscale = 126.0F / kmax;
    int   ishift = (int) (log(fscale) / LOG_OF_2);
    float fshift = (float)(1 << ishift);

    //
    // Multiply all of the kernel values by the scale factor
    //
    for(i=0; i<ks; i++) {
        cd->scaled_kdata[i] = (char) (kdata[i] * fshift);
    }

    //
    // Create an IplConvKernel and pass it to the 
    // compute routine via compute_data.
    //
    cd->iplKernel = iplCreateConvKernel(kh, kw, 
                                        kernel->getKeyX(), kernel->getKeyY(),
                                        cd->scaled_kdata, ishift);
    MMX_MEMCHK(cd->iplKernel);

    *compute_data = (void*)cd;

    return XIL_SUCCESS;
}

XilStatus
XilDeviceManagerComputeMMXBYTE::ConvolvePostprocess(XilOp*       ,
                                                    void*        compute_data)
{
    ConvolveData* cd = (ConvolveData*)compute_data;

    iplDeleteConvKernel(cd->iplKernel);
    delete [] cd->scaled_kdata;
    delete cd;

    return XIL_SUCCESS;
}


XilStatus
XilDeviceManagerComputeMMXBYTE::Convolve(XilOp*       op,
                                         unsigned     op_count,
                                         XilRoi*      roi,
                                         XilBoxList*  bl)
{
    ComputeInfoMMX  ci(op, op_count, roi, bl);

    ConvolveData* cd = (ConvolveData*)op->getPreprocessData(this);

    XilKernel*   kernel;
    op->getParam(1, (void**)&kernel);

    int kx = (int)kernel->getKeyX();
    int ky = (int)kernel->getKeyY();

    while(ci.hasMoreInfo()) {
        if(ci.isStorageType(XIL_PIXEL_SEQUENTIAL)) {

            //
            // Only process center boxes. Let memory do the rest.
            // Ipl can't handle all XIL edge conditions anyway.
            //
            XilBoxAreaType tag = ci.getBoxTag();
            if(tag != XIL_AREA_CENTER) {
                MMX_MARK_BOX;
            }

            if(ci.createIplImages(FALSE) == XIL_FAILURE) {
                MMX_MARK_BOX;
            }

            //
            // Compensate for the Kernel key offset
            //
            int stepBack =  ky * ci.src1ScanlineStride 
                          + kx * ci.src1PixelStride;
            ci.mmxSrc1.imageData -= stepBack;

            stepBack =  ky * ci.destScanlineStride 
                      + kx * ci.destPixelStride;
            ci.mmxDst.imageData -= stepBack;

            iplConvolve2D(&ci.mmxSrc1, &ci.mmxDst, &cd->iplKernel, 1, 
                          IPL_SUM, NULL);

            if(IPL_ERRCHK("Convolve2D", "Error in IPL Library")) {
                MMX_MARK_BOX;
            }

        } else {
            MMX_MARK_BOX;
        }

    }

    return ci.returnValue;

}



