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
//  File:       XiliColormapGenerator.cc
//  Project:    XIL
//  Revision:   1.6
//  Last Mod:   10:16:36, 03/10/00
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
#pragma ident   "@(#)XiliColormapGenerator.cc	1.6\t00/03/10  "

#include <xil/xilGPI.hh>
#include "XiliUtils.hh"
#include "XiliColormapGenerator.hh"

unsigned int XiliColormapGenerator::sqr[256] = {
        0,     1,     4,     9,    16,    25,    36,    49,    64,    81,
      100,   121,   144,   169,   196,   225,   256,   289,   324,   361,
      400,   441,   484,   529,   576,   625,   676,   729,   784,   841,
      900,   961,  1024,  1089,  1156,  1225,  1296,  1369,  1444,  1521,
     1600,  1681,  1764,  1849,  1936,  2025,  2116,  2209,  2304,  2401,
     2500,  2601,  2704,  2809,  2916,  3025,  3136,  3249,  3364,  3481,
     3600,  3721,  3844,  3969,  4096,  4225,  4356,  4489,  4624,  4761,
     4900,  5041,  5184,  5329,  5476,  5625,  5776,  5929,  6084,  6241,
     6400,  6561,  6724,  6889,  7056,  7225,  7396,  7569,  7744,  7921,
     8100,  8281,  8464,  8649,  8836,  9025,  9216,  9409,  9604,  9801,
    10000, 10201, 10404, 10609, 10816, 11025, 11236, 11449, 11664, 11881,
    12100, 12321, 12544, 12769, 12996, 13225, 13456, 13689, 13924, 14161,
    14400, 14641, 14884, 15129, 15376, 15625, 15876, 16129, 16384, 16641,
    16900, 17161, 17424, 17689, 17956, 18225, 18496, 18769, 19044, 19321,
    19600, 19881, 20164, 20449, 20736, 21025, 21316, 21609, 21904, 22201,
    22500, 22801, 23104, 23409, 23716, 24025, 24336, 24649, 24964, 25281,
    25600, 25921, 26244, 26569, 26896, 27225, 27556, 27889, 28224, 28561,
    28900, 29241, 29584, 29929, 30276, 30625, 30976, 31329, 31684, 32041,
    32400, 32761, 33124, 33489, 33856, 34225, 34596, 34969, 35344, 35721,
    36100, 36481, 36864, 37249, 37636, 38025, 38416, 38809, 39204, 39601,
    40000, 40401, 40804, 41209, 41616, 42025, 42436, 42849, 43264, 43681,
    44100, 44521, 44944, 45369, 45796, 46225, 46656, 47089, 47524, 47961,
    48400, 48841, 49284, 49729, 50176, 50625, 51076, 51529, 51984, 52441,
    52900, 53361, 53824, 54289, 54756, 55225, 55696, 56169, 56644, 57121,
    57600, 58081, 58564, 59049, 59536, 60025, 60516, 61009, 61504, 62001,
    62500, 63001, 63504, 64009, 64516, 65025 };

//
// Constructor
//
XiliColormapGenerator::XiliColormapGenerator()
{
    //
    // Establish ptrs to simplify treating the 3D vectors as 1D
    //
    vwt = &wt[0][0][0];
    vmr = &mr[0][0][0];
    vmg = &mg[0][0][0];
    vmb = &mb[0][0][0];
    vm2 = &m2[0][0][0];

    // Zero the histogram vectors
    //
    xili_memset(vwt, 0, 33*33*33*sizeof(unsigned int));
    xili_memset(vmr, 0, 33*33*33*sizeof(unsigned int));
    xili_memset(vmg, 0, 33*33*33*sizeof(unsigned int));
    xili_memset(vmb, 0, 33*33*33*sizeof(unsigned int));
    xili_memset(vm2, 0, 33*33*33*sizeof(float));

}

//
// Destructor
//
XiliColormapGenerator::~XiliColormapGenerator() 
{ 
}

#define INDEX33(A,B,C) (((A)<<10) + ((A)<<6) + (A) + ((B)<<5) + (B) + (C))

// At conclusion of the histogram step, we can interpret
//   wt[r][g][b] = sum over voxel of P(c)
//   mr[r][g][b] = sum over voxel of r*P(c)  ,  similarly for mg, mb
//   m2[r][g][b] = sum over voxel of c^2*P(c)
// Actually each of these should be divided by 'size' to give the usual
// interpretation of P() as ranging from 0 to 1, but we needn't do that here.
 

//
// Convert histogram into moments so that we can rapidly calculate
// the sums of the above quantities over any desired box.
//
void 
XiliColormapGenerator::M3d()
{
    int    ind1, ind2;
    int    line, line_r, line_g, line_b;
    int    area[33], area_r[33], area_g[33], area_b[33];
    float  line2;
    float  area2[33];

    for(int r=1; r<=32; ++r) {
        for(int i=0; i<=32; ++i) {
            area[i]=area_r[i]=area_g[i]=area_b[i]=0;
            area2[i]=0.0;
        }
        for(int g=1; g<=32; ++g) {
            line = line_r = line_g = line_b = 0;
            line2 = 0.0;
            for(int b=1; b<=32; ++b) {
                ind1 = INDEX33(r,g,b);
                line += vwt[ind1];
                line_r += vmr[ind1]; 
                line_g += vmg[ind1]; 
                line_b += vmb[ind1];
                line2 += vm2[ind1];
                area[b] += line;
                area_r[b] += line_r;
                area_g[b] += line_g;
                area_b[b] += line_b;
                area2[b] += line2;
                ind2 = ind1 - 1089; // [r-1][g][b] 
                vwt[ind1] = vwt[ind2] + area[b];
                vmr[ind1] = vmr[ind2] + area_r[b];
                vmg[ind1] = vmg[ind2] + area_g[b];
                vmb[ind1] = vmb[ind2] + area_b[b];
                vm2[ind1] = vm2[ind2] + area2[b];
            }
        }
    }
}


// Compute sum over a box of any given statistic 
int 
XiliColormapGenerator::Vol(CmapGenCube* cube,
                         unsigned int   mmt[33][33][33])
{
    return( mmt[cube->r1][cube->g1][cube->b1] 
           -mmt[cube->r1][cube->g1][cube->b0]
           -mmt[cube->r1][cube->g0][cube->b1]
           +mmt[cube->r1][cube->g0][cube->b0]
           -mmt[cube->r0][cube->g1][cube->b1]
           +mmt[cube->r0][cube->g1][cube->b0]
           +mmt[cube->r0][cube->g0][cube->b1]
           -mmt[cube->r0][cube->g0][cube->b0] );
}

// The next two routines allow a slightly more efficient calculation
// of Vol() for a proposed subbox of a given box.  The sum of Top()
// and Bottom() is the Vol() of a subbox split in the given direction
// and with the specified new upper bound.
 

// Compute part of Vol(cube, mmt) that doesn't depend on r1, g1, or b1 
// (depending on dir) 
int 
XiliColormapGenerator::Bottom(CmapGenCube* cube,
                            XILI_CCUBE_DIR   dir,
                            unsigned int   mmt[33][33][33])
{
    switch(dir) {
      case CCUBE_RED_DIR:
        return( mmt[cube->r0][cube->g1][cube->b0]
                    -mmt[cube->r0][cube->g1][cube->b1]
                    +mmt[cube->r0][cube->g0][cube->b1]
                    -mmt[cube->r0][cube->g0][cube->b0] );
      case CCUBE_GRN_DIR:
        return( mmt[cube->r1][cube->g0][cube->b0]
                    -mmt[cube->r1][cube->g0][cube->b1]
                    +mmt[cube->r0][cube->g0][cube->b1]
                    -mmt[cube->r0][cube->g0][cube->b0] );
      case CCUBE_BLU_DIR:
        return( mmt[cube->r1][cube->g0][cube->b0]
                    -mmt[cube->r1][cube->g1][cube->b0]
                    +mmt[cube->r0][cube->g1][cube->b0]
                    -mmt[cube->r0][cube->g0][cube->b0] );
      default :
        return(-1);
    }
}


// Compute remainder of Vol(cube, mmt), substituting pos for 
// r1, g1, or b1 (depending on dir) 
int 
XiliColormapGenerator::Top(CmapGenCube* cube,
                         XILI_CCUBE_DIR   dir,
                         int   pos,
                         unsigned int   mmt[33][33][33])
{
    switch(dir) {
      case CCUBE_RED_DIR:
        return( mmt[pos][cube->g1][cube->b1] 
                   -mmt[pos][cube->g1][cube->b0]
                   -mmt[pos][cube->g0][cube->b1]
                   +mmt[pos][cube->g0][cube->b0] );
      case CCUBE_GRN_DIR:
        return( mmt[cube->r1][pos][cube->b1] 
                   -mmt[cube->r1][pos][cube->b0]
                   -mmt[cube->r0][pos][cube->b1]
                   +mmt[cube->r0][pos][cube->b0] );
      case CCUBE_BLU_DIR:
        return( mmt[cube->r1][cube->g1][pos]
                   -mmt[cube->r1][cube->g0][pos]
                   -mmt[cube->r0][cube->g1][pos]
                   +mmt[cube->r0][cube->g0][pos] );
      default :
        return(-1);
    }
}


// Compute the weighted variance of a box 
// NB: as with the raw statistics, this is really the variance * size 
float 
XiliColormapGenerator::Var(CmapGenCube* cube)
{
    float dr = (float) Vol(cube, mr); 
    float dg = (float) Vol(cube, mg); 
    float db = (float) Vol(cube, mb);
    float xx = m2[cube->r1][cube->g1][cube->b1] 
                -m2[cube->r1][cube->g1][cube->b0]
                -m2[cube->r1][cube->g0][cube->b1]
                +m2[cube->r1][cube->g0][cube->b0]
                -m2[cube->r0][cube->g1][cube->b1]
                +m2[cube->r0][cube->g1][cube->b0]
                +m2[cube->r0][cube->g0][cube->b1]
                -m2[cube->r0][cube->g0][cube->b0];

    return( xx - (dr*dr+dg*dg+db*db)/(float)Vol(cube,wt) );    
}

// We want to minimize the sum of the variances of two subboxes.
// The sum(c^2) terms can be ignored since their sum over both subboxes
// is the same (the sum for the whole box) no matter where we split.
// The remaining terms have a minus sign in the variance formula,
// so we drop the minus sign and MAXIMIZE the sum of the two terms.
 


float 
XiliColormapGenerator::Maximize(CmapGenCube* cube,
                              XILI_CCUBE_DIR   dir,
                              int   first, 
                              int   last, 
                              int*  cut,
                              int   whole_r, 
                              int   whole_g, 
                              int   whole_b, 
                              int   whole_w)
{
    int    base_r = Bottom(cube, dir, mr);
    int    base_g = Bottom(cube, dir, mg);
    int    base_b = Bottom(cube, dir, mb);
    int    base_w = Bottom(cube, dir, wt);
    float max    = 0.0;
    float temp;

    *cut = -1;
    for(int i=first; i<last; ++i) {
        int half_r = base_r + Top(cube, dir, i, mr);
        int half_g = base_g + Top(cube, dir, i, mg);
        int half_b = base_b + Top(cube, dir, i, mb);
        int half_w = base_w + Top(cube, dir, i, wt);

        //
        // Now half_x is sum over lower half of box, if split at i
        //
        if(half_w == 0) {      // subbox could be empty of pixels!
            continue;             // never split into an empty box
        } else {
            temp = ((float)half_r*half_r + (float)half_g*half_g +
                   (float)half_b*half_b)/half_w;
        }

        half_r = whole_r - half_r;
        half_g = whole_g - half_g;
        half_b = whole_b - half_b;
        half_w = whole_w - half_w;
        if(half_w == 0) {      // subbox could be empty of pixels! 
            continue;             // never split into an empty box 
        } else {
            temp += ((float)half_r*half_r + (float)half_g*half_g +
                     (float)half_b*half_b)/half_w;
        }

        if(temp > max) {
            max  = temp; 
            *cut = i;
        }
    }
    return(max);
}

int 
XiliColormapGenerator::Cut(CmapGenCube* set1, 
                         CmapGenCube* set2)
{
    XILI_CCUBE_DIR dir;
    int cutr, cutg, cutb;
    float maxr, maxg, maxb;
    int whole_r, whole_g, whole_b, whole_w;

    whole_r = Vol(set1, mr);
    whole_g = Vol(set1, mg);
    whole_b = Vol(set1, mb);
    whole_w = Vol(set1, wt);

    maxr = Maximize(set1, CCUBE_RED_DIR, set1->r0+1, set1->r1, &cutr,
                    whole_r, whole_g, whole_b, whole_w);
    maxg = Maximize(set1, CCUBE_GRN_DIR, set1->g0+1, set1->g1, &cutg,
                    whole_r, whole_g, whole_b, whole_w);
    maxb = Maximize(set1, CCUBE_BLU_DIR, set1->b0+1, set1->b1, &cutb,
                    whole_r, whole_g, whole_b, whole_w);

    if( (maxr>=maxg)&&(maxr>=maxb) ) {
        dir = CCUBE_RED_DIR;
        if(cutr < 0)  {
            return 0; // can't split the box
        }
    } else if( (maxg>=maxr)&&(maxg>=maxb) )  {
        dir = CCUBE_GRN_DIR;
    } else {
        dir = CCUBE_BLU_DIR; 
    }

    set2->r1 = set1->r1;
    set2->g1 = set1->g1;
    set2->b1 = set1->b1;

    switch (dir) {
      case CCUBE_RED_DIR:
        set2->r0 = set1->r1 = cutr;
        set2->g0 = set1->g0;
        set2->b0 = set1->b0;
        break;
      case CCUBE_GRN_DIR:
        set2->g0 = set1->g1 = cutg;
        set2->r0 = set1->r0;
        set2->b0 = set1->b0;
        break;
      case CCUBE_BLU_DIR:
        set2->b0 = set1->b1 = cutb;
        set2->r0 = set1->r0;
        set2->g0 = set1->g0;
        break;
    }
    set1->vol=(set1->r1-set1->r0)*(set1->g1-set1->g0)*(set1->b1-set1->b0);
    set2->vol=(set2->r1-set2->r0)*(set2->g1-set2->g0)*(set2->b1-set2->b0);
    return 1;
}


//
//  Partitions the 3-D histogram until the desired number
//  of colormap entries are achieved, The resulting colormap
//  is written into the array "lut" provided by the caller.
//
void
XiliColormapGenerator::generateColormap(Xil_unsigned8*   lut,
                                        unsigned int     lut_size)
{
    //
    // Compute the histogram moments
    //
    M3d();

    //
    // Max size is only 256 elements,
    // so allocate these on stack.
    //
    CmapGenCube  cube[256];
    float vv[256];

    cube[0].r0 = cube[0].g0 = cube[0].b0 = 0;
    cube[0].r1 = cube[0].g1 = cube[0].b1 = 32;

    int next = 0;
    for(unsigned int i=1; i<lut_size; ++i) {
        if(Cut(&cube[next], &cube[i])) {
            // volume test ensures we won't try to cut one-cell box 
            vv[next] = (cube[next].vol>1) ? Var(&cube[next]) : (float)0.0;
            vv[i] = (cube[i].vol>1) ? Var(&cube[i]) : (float)0.0;
        } else {
            vv[next] = 0.0;   // don't try to split this box again 
            i--;              // didn't create box i 
        }
        next = 0; 
        float temp = vv[0];
        for(unsigned int j=1; j<=i; ++j) {
            if(vv[j] > temp) {
                temp = vv[j]; 
                next = j;
            }
        }
        if(temp <= 0.0) {
            lut_size = i+1;
            break;
        }
    }

    for(unsigned int k=0; k<lut_size; ++k) {
        int weight = Vol(&cube[k], wt);
        if(weight) {
            lut[k*3] = (Xil_unsigned8)(Vol(&cube[k], mr) / weight);
            lut[k*3+1] = (Xil_unsigned8)(Vol(&cube[k], mg) / weight);
            lut[k*3+2] = (Xil_unsigned8)(Vol(&cube[k], mb) / weight);
        }
        else{
            lut[k*3] = lut[k*3+1] = lut[k*3+2] = 0;
        }
    }

}

//
// Build 3-D color histogram of counts, r/g/b, c^2
// Histogram is in elements 1..HISTSIZE along each axis,
// element 0 is for base or marginal value
// NB: these must start out 0!
//

void 
XiliColormapGenerator::hist3dPixSeq(Xil_unsigned8*   src,
                                    unsigned int     width,
                                    unsigned int     height,
                                    unsigned int     src_ps,
                                    unsigned int     src_ss)
{
    unsigned int ind;
    unsigned int r, g, b;
    unsigned int inr, ing, inb;
    unsigned int ps    = src_ps;
    unsigned int ss    = src_ss;
    unsigned int xsize = width;
    unsigned int ysize = height;

    Xil_unsigned8* src_scan = src;
    for(int y=ysize; y!=0; y--) {
        Xil_unsigned8* src_pixel = src_scan;
        for(int x=xsize; x!=0; x--) {
            r = *src_pixel;
            g = *(src_pixel + 1);
            b = *(src_pixel + 2);
            src_pixel += ps;

            inr = (r>>3)+1; 
            ing = (g>>3)+1; 
            inb = (b>>3)+1; 

            //
            // Replace [inr][ing][inb] with single index
            //
            ind = INDEX33(inr,ing,inb);
            ++vwt[ind];
            vmr[ind] += r;
            vmg[ind] += g;
            vmb[ind] += b;
            vm2[ind] += (float)(sqr[r] + sqr[g] + sqr[b]);

        }
        src_scan += ss;
    }
}

//
// Histogram routine for general and band-sequential
//

void 
XiliColormapGenerator::hist3dGeneral(Xil_unsigned8**  src,
                                     unsigned int     width,
                                     unsigned int     height,
                                     unsigned int*    src_ps,
                                     unsigned int*    src_ss)
{
    int  ind, r, g, b;
    int         inr, ing, inb;

    unsigned int red_ps = src_ps[0];
    unsigned int grn_ps = src_ps[1];
    unsigned int blu_ps = src_ps[2];

    unsigned int red_ss = src_ss[0];
    unsigned int grn_ss = src_ss[1];
    unsigned int blu_ss = src_ss[2];

    Xil_unsigned8* red_src_scan = src[0];
    Xil_unsigned8* grn_src_scan = src[1];
    Xil_unsigned8* blu_src_scan = src[2];

    unsigned int xsize = width;
    unsigned int ysize = height;

    for(unsigned int y=0; y<ysize; y++) {
        Xil_unsigned8* red_src_pixel = red_src_scan;
        Xil_unsigned8* grn_src_pixel = grn_src_scan;
        Xil_unsigned8* blu_src_pixel = blu_src_scan;
        for(unsigned int x=0; x<xsize; x++) {
            r = *red_src_pixel;
            g = *grn_src_pixel;
            b = *blu_src_pixel;

            inr = (r>>3)+1; 
            ing = (g>>3)+1; 
            inb = (b>>3)+1; 

            //
            // Replace [inr][ing][inb] with single index
            //
            ind = INDEX33(inr,ing,inb);
            ++vwt[ind];
            vmr[ind] += r;
            vmg[ind] += g;
            vmb[ind] += b;
            vm2[ind] += (float)(sqr[r] + sqr[g] +sqr[b]);

            red_src_pixel += red_ps;
            grn_src_pixel += grn_ps;
            blu_src_pixel += blu_ps;
        }
        red_src_scan += red_ss;
        grn_src_scan += grn_ss;
        blu_src_scan += blu_ss;
    }
}


//
// Accumulate results from one ColormapGerator into another
// One object maintains the grand total.
// All the other objects have individual sub-totals from
// different threads.
//
void 
XiliColormapGenerator::accumulateHistogram(XiliColormapGenerator* cmapGen)
{

    //
    // Assign ptrs to the sub-total quantities
    //
    unsigned int* sub_wtp = cmapGen->vwt;
    unsigned int* sub_mrp = cmapGen->vmr;
    unsigned int* sub_mgp = cmapGen->vmg;
    unsigned int* sub_mbp = cmapGen->vmb;
    float*        sub_m2p = cmapGen->vm2;

    //
    // Assign pointers to the grand-total quantities
    //
    unsigned int* wtp = this->vwt;
    unsigned int* mrp = this->vmr;
    unsigned int* mgp = this->vmg;
    unsigned int* mbp = this->vmb;
    float*        m2p = this->vm2;

    //
    // Accumulate the histogram moments from the cmapGen passed
    // as an argument into the equivalent quantities in"this" object.
    //
    for(int count=33*33*33; count!=0; count--) {
        *wtp++ += *sub_wtp++;
        *mrp++ += *sub_mrp++;
        *mgp++ += *sub_mgp++;
        *mbp++ += *sub_mbp++;
        *m2p++ += *sub_m2p++;
    }

}
