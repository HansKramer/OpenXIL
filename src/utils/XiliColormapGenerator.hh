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
//  File:       XiliColormapGenerator.hh
//  Project:    XIL
//  Revision:   1.3
//  Last Mod:   10:24:02, 03/10/00
//
//  Description:
//
//    Internal utility functions for Best Colormap Generation
//    Used by xil_choose_colormap and Cell compression.
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XiliColormapGenerator.hh	1.3\t00/03/10  "

#ifndef _XILI_COLORMAP_GENERATOR_HH_
#define _XILI_COLORMAP_GENERATOR_HH_

#include <xil/xilGPI.hh>

struct CmapGenCube {
    int r0;                         // min value, exclusive 
    int r1;                         // max value, inclusive 
    int g0;  
    int g1;  
    int b0;  
    int b1;
    int vol;
};

enum XILI_CCUBE_DIR {
    CCUBE_BLU_DIR, CCUBE_GRN_DIR, CCUBE_RED_DIR
};

class XiliColormapGenerator {
public:

    XiliColormapGenerator();

    ~XiliColormapGenerator();

    void generateColormap(Xil_unsigned8* lut, 
                          unsigned int   lut_size);

    void hist3dPixSeq(Xil_unsigned8*   src,
                      unsigned int     width,
                      unsigned int     height,
                      unsigned int     src_ps,
                      unsigned int     src_ss);

    void hist3dGeneral(Xil_unsigned8**  src,
                       unsigned int     width,
                       unsigned int     height,
                       unsigned int*    src_ps,
                       unsigned int*    src_ss);

    void accumulateHistogram(XiliColormapGenerator*);

    //
    // The histogram moments are calculated in private arrays.
    // Since the arrays are a known size, we'll just make them part
    // of the object.
    //
    // WARNING: This object should be created with new() since
    // otherwise a huge stack space would be taken, which could 
    // create some problems.
    // 
    unsigned int        wt[33][33][33];
    unsigned int        mr[33][33][33];
    unsigned int        mg[33][33][33];
    unsigned int        mb[33][33][33];
    float               m2[33][33][33];

    unsigned int*       vwt;
    unsigned int*       vmr;
    unsigned int*       vmg;
    unsigned int*       vmb;
    float*              vm2;

private:
//
// Private member functions.
// These implement the histogram minimization step.
//
    void  M3d();

    int   Vol(CmapGenCube*         cube,
              unsigned int         mmt[33][33][33]);

    int   Bottom(CmapGenCube*      cube,
                 XILI_CCUBE_DIR    dir,
                 unsigned int      mmt[33][33][33]);

    int   Top(CmapGenCube*         cube,
              XILI_CCUBE_DIR       dir,
              int                  pos,
              unsigned int         mmt[33][33][33]);

    float Var(CmapGenCube*         cube);

    float Maximize(CmapGenCube*   cube,
                   XILI_CCUBE_DIR dir,
                   int            first, 
                   int            last, 
                   int*           cut,
                   int            whole_r, 
                   int            whole_g, 
                   int            whole_b, 
                   int            whole_w);

    int   Cut(CmapGenCube*        set1, 
              CmapGenCube*        set2);

//
// Private data members
//

    unsigned int        cmap_size;

    //
    // Squares table and flag.
    // One copy shared by all instances.
    //
    static unsigned int sqr[256];

};

#endif // _XILI_COLORMAP_GENERATOR_HH_
