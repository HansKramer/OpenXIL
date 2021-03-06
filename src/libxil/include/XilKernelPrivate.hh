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
//  File:	XilKernelPrivate.hh
//  Project:	XIL
//  Revision:	1.9
//  Last Mod:	10:21:33, 03/10/00
//
//  Description:
//	Definition of private elements of XilKernel Object
//	
//------------------------------------------------------------------------
//  COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XilKernelPrivate.hh	1.9\t00/03/10  "

#ifdef _XIL_PRIVATE_DATA

public:
    //
    //  Required methods from XilObject
    //
    XilObject*	    createCopy();

    //
    //  Invert the kernel -- used by the convolve op in the ops directory.
    //
    void            invert();

    //
    //  Constructor
    //
                    XilKernel(XilSystemState* system_state,
                              unsigned int    xsize,
                              unsigned int    ysize,
                              int             key_x,
                              int             key_y,
                              float*          data);

    //
    //  Constructor for separable convolution kernels
    //
                    XilKernel(XilSystemState* system_state,
                              unsigned int    xsize,
                              unsigned int    ysize,
                              int             key_x,
                              int             key_y,
                              float*          x_data,
                              float*          y_data);

    void            getData(float* data);

protected:
                    ~XilKernel();


private:
    //
    //  A function that runs through and tests to see if the data in the
    //  kernel is separable.  If so, it sets the xData and yData members
    //  correctly.
    //
    void            checkSeparable();

    unsigned int    width;
    unsigned int    height;
    int             keyX;
    int             keyY;
    float*          data;

    //
    //  For seperable kernels.
    //
    float*          xData;
    float*          yData;

    //
    // For internal locking around invert() and getData calls
    //
    XilMutex        kernelMutex;
    
#endif // _XIL_PRIVATE_DATA
