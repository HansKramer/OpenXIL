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
//  File:       SepConvolve.cc
//  Project:    XIL
//  Revision:   1.6
//  Last Mod:   10:13:48, 03/10/00
//
//  Description:
//
//    Separable Convolve. Allows speedups by special casing
//    separable kernels.
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)SepConvolve.cc	1.6\t00/03/10  "


#include "XilDeviceManagerComputeMMXBYTE.hh"
#include "ComputeInfoMMX.hh"

#define LOG_OF_2   0.69314718F

typedef struct {
    char* scaled_x_kdata;
    char* scaled_y_kdata;
    IplConvKernel* xKernel;
    IplConvKernel* yKernel;
} SepConvolveData;

//
// Use a preprocess routine to analyze the kernel and
// produce a scaled integer version, using power of 2 scaling
// as required by the IPL convolve routine.
// 
XilStatus
XilDeviceManagerComputeMMXBYTE::Convolve_SeparablePreprocess(XilOp*       op,
                                                      unsigned     ,
                                                      XilRoi*      ,
                                                      void**       compute_data,
                                                      unsigned int*  )
{
    //
    // Get the kernel object from the op
    //
    XilKernel*   kernel;
    op->getParam(1, (void**)&kernel);

    int kw = kernel->getWidth();
    int kh = kernel->getHeight();

    SepConvolveData* cd = new SepConvolveData;
    MMX_MEMCHK(cd);

    //
    // Allocate memory for the converted separable kernel data
    //
    cd->scaled_x_kdata = new char[kw];
    MMX_MEMCHK(cd->scaled_x_kdata);

    cd->scaled_y_kdata = new char[kh];
    MMX_MEMCHK(cd->scaled_y_kdata);
    
    float *kx;
    float *ky;
    kernel->getSeparableData((const float **)&kx, (const float **)&ky);

    //
    // Determine the largest absolute value in the kernel
    //
    float kmax = (kx[0] < 0) ? -kx[0] : kx[0];
    for(int i=1; i<kw; i++) {
        float kval =  (kx[i] < 0) ? -kx[i] : kx[i];
        if(kval > kmax) {
            kmax = kval;
        }
    }
    for(i=0; i<kh; i++) {
        float kval =  (ky[i] < 0) ? -ky[i] : ky[i];
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
    for(i=0; i<kw; i++) {
        cd->scaled_x_kdata[i] = (char) (kx[i] * fshift);
    }
    for(i=0; i<kh; i++) {
        cd->scaled_y_kdata[i] = (char) (ky[i] * fshift);
    }

    //
    // Create x and y IplKernels and pass to the 
    // compute routine in the SepConvolveData structure
    //
    cd->xKernel = iplCreateConvKernel(1, kw, 
                                      kernel->getKeyX(), 0,
                                      cd->scaled_x_kdata, ishift);
    MMX_MEMCHK(cd->xKernel);

    cd->yKernel = iplCreateConvKernel(kh, 1, 
                                      0, kernel->getKeyY(),
                                      cd->scaled_y_kdata, ishift);
    MMX_MEMCHK(cd->yKernel);

    *compute_data = (void*)cd;

    return XIL_SUCCESS;
}

XilStatus
XilDeviceManagerComputeMMXBYTE::Convolve_SeparablePostprocess(XilOp*       ,
                                                    void*        compute_data)
{


    SepConvolveData* cd = (SepConvolveData*)compute_data;

    iplDeleteConvKernel(cd->xKernel);
    iplDeleteConvKernel(cd->yKernel);
    delete [] cd->scaled_x_kdata;
    delete [] cd->scaled_y_kdata;
    delete cd;

    return XIL_SUCCESS;
}


XilStatus
XilDeviceManagerComputeMMXBYTE::Convolve_Separable(XilOp*       op,
                                                   unsigned     op_count,
                                                   XilRoi*      roi,
                                                   XilBoxList*  bl)
{
    ComputeInfoMMX  ci(op, op_count, roi, bl);

    SepConvolveData* cd = (SepConvolveData*)op->getPreprocessData(this);

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

            iplConvolveSep2D(&ci.mmxSrc1, &ci.mmxDst, 
                             cd->xKernel, cd->yKernel, NULL);

            if(IPL_ERRCHK("SepConvolve2D", "Error in IPL Library")) {
                MMX_MARK_BOX;
            }

        } else {
            MMX_MARK_BOX;
        }

    }

    return ci.returnValue;

}



