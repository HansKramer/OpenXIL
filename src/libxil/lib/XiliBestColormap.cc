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
//  File:       XiliBestColormap.cc
//  Project:    XIL
//  Revision:   1.9
//  Last Mod:   10:08:50, 03/10/00
//
//  Description:

//    Utility function to calculate a best colormap
//    from a histogram of a 3 band RGB image.
//    Uses a "median cut" type of algorithm.
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XiliBestColormap.cc	1.9\t00/03/10  "

#include <string.h>
#include "_XilSystemState.hh"
#include "XiliBestColormap.hh"

Xil_boolean
XiliBestColormap::isOK()
{
    _XIL_ISOK_TEST();
}

//
// Constructor
//
XiliBestColormap::XiliBestColormap(XilHistogram* histo,
                                   unsigned int  ncolors_arg) 
{
    isOKFlag = FALSE;

    ncolors  = ncolors_arg;

    //
    // Keep a local copy of the SystemState
    //
    bestcmapState = histo->getSystemState();

    //
    // Allocate the storage for the lut array
    //
    lut = new Xil_unsigned8[ncolors*3];
    if(lut == NULL) {
        XIL_ERROR(bestcmapState, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return;
    }
    xili_memset(lut, 0, ncolors*3);

    // Get the histogram data
    unsigned int* histo_data = histo->getDataPtr();
    if(histo_data == NULL) {
        delete [] lut;
        XIL_ERROR(bestcmapState, XIL_ERROR_RESOURCE, "di-259", TRUE);
        return;
    }

    //
    // Allocate a 5 x 33x33x33 array to hold the histogram data
    // and the moments calculated from the histogram
    // (count, red mean, green mean, blue mean, variance)
    //
    XiliArray3d<unsigned int>* awt = new XiliArray3d<unsigned int>(33, 33, 33);
    if(! awt->isOK()) {
        delete [] lut;
        XIL_ERROR(bestcmapState, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return;
    }
    awt->zero();

    XiliArray3d<unsigned int>* amr = new XiliArray3d<unsigned int>(33, 33, 33);
    if(! amr->isOK()) {
        delete [] lut;
        delete awt;
        XIL_ERROR(bestcmapState, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return;
    }
    amr->zero();

    XiliArray3d<unsigned int>* amg = new XiliArray3d<unsigned int>(33, 33, 33);
    if(! amg->isOK()) {
        delete [] lut;
        delete amr;
        delete awt;
        XIL_ERROR(bestcmapState, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return;
    }
    amg->zero();

    XiliArray3d<unsigned int>* amb = new XiliArray3d<unsigned int>(33, 33, 33);
    if(! amb->isOK()) {
        delete [] lut;
        delete amg;
        delete amr;
        delete awt;
        XIL_ERROR(bestcmapState, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return;
    }
    amb->zero();

    XiliArray3d<double>* am2 = new XiliArray3d<double>(33, 33, 33);
    if(! am2->isOK()) {
        delete [] lut;
        delete amb;
        delete amg;
        delete amr;
        delete awt;
        XIL_ERROR(bestcmapState, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return;
    }
    am2->zero();


    // Load the Type*** pointers, so we can index like a normal 3-D array
    wt = awt->getDataPtr(); // Count per color cell
    mr = amr->getDataPtr(); // Sum of red values per cell
    mg = amg->getDataPtr(); // Sum of green values per cell
    mb = amb->getDataPtr(); // Sum of blue values per cell
    m2 = am2->getDataPtr(); // Sum of squares values per cell

    //
    // Copy the histogram data into the new array
    // Skip the [0][0][0] element 
    // Note: The (8*base + 56)*base + 140 calculation represents the
    //       sum of the squares of the values betwewn base and base+7
    //
    unsigned int count;
    unsigned int base;
    for(int r=0; r<32; r++) {
        base = r * 8;
        float rr  = (float)base + 3.5;
        float rr2 = (base + 7)*base + 140.0/8.0;
        for(int g=0; g<32; g++) {
            base = g * 8;
            float gg  = (float)base + 3.5;
            float gg2 = (base + 7)*base + 140.0/8.0;
            unsigned int* wt_ptr = &wt[r+1][g+1][1];
            unsigned int* mr_ptr = &mr[r+1][g+1][1];
            unsigned int* mg_ptr = &mg[r+1][g+1][1];
            unsigned int* mb_ptr = &mb[r+1][g+1][1];
            double* m2_ptr = &m2[r+1][g+1][1];
            for(int b=0; b<32; b++) {
                base = b * 8;
                float bb  = (float)base + 3.5;
                float bb2 = (base + 7)*base + 140.0/8.0;
                count = *histo_data++;
                wt_ptr[b] = count;
                mr_ptr[b] = count * rr;
                mg_ptr[b] = count * gg;
                mb_ptr[b] = count * bb;
                m2_ptr[b] = count * (rr2 + gg2 + bb2);
            }
        }
    }
   

    //
    // At conclusion of the above histogram processing step, we can interpret
    //   wt[r][g][b] = sum over voxel of P(c), 
    //   mr[r][g][b] = sum over voxel of r*P(c),  similarly for mg, mb
    //   m2[r][g][b] = sum over voxel of c^2*P(c)
    // Actually each of these should be divided by 'size' to give the usual
    // interpretation of P() as ranging from 0 to 1, 
    // but we needn't do that here.
    //

    //
    // Now call the driver routine which to
    // generate the colormap
    //
    if (generateColormap8() == XIL_FAILURE) {
        // TODO: Do we need an error macro here
        return;
    }

    delete awt;
    delete amr;
    delete amg;
    delete amb;
    delete am2;

    isOKFlag = TRUE;
}

//
// Destructor
//
XiliBestColormap::~XiliBestColormap()
{
//    delete [] lut;
}

//
// Return the colormap to the user as a byte array
//
Xil_unsigned8*
XiliBestColormap::getColormap()
{
    return lut;
}

// Compute cumulative moments.
//
void
XiliBestColormap::calculateCumulativeMoments()
{
    int   area[33];
    int   area_r[33];
    int   area_g[33];
    int   area_b[33];
    float area2[33];

    unsigned int*  pwt;
    unsigned int*  pmr;
    unsigned int*  pmg;
    unsigned int*  pmb;
    double*        pm2;

    unsigned int*  pwt1;
    unsigned int*  pmr1;
    unsigned int*  pmg1;
    unsigned int*  pmb1;
    double*        pm21;

    int i, r, g, b;

    for(r=1; r<=32; r++){
        for(i=0; i<=32; i++) {
            area2[i] = area[i] = area_r[i] = area_g[i] = area_b[i] = 0;
        }
        for(g=1; g<=32; g++){
            int line     = 0;
            int line_r   = 0;
            int line_g   = 0;
            int line_b   = 0;
            float line2  = 0.0F;

            // Resolve address to b component ([r][g][0])
            pwt = wt[r][g];
            pmr = mr[r][g];
            pmg = mg[r][g];
            pmb = mb[r][g];
            pm2 = m2[r][g];

            // Resolve address to [r-1][g][0]
            pwt1 = wt[r-1][g];
            pmr1 = mr[r-1][g];
            pmg1 = mg[r-1][g];
            pmb1 = mb[r-1][g];
            pm21 = m2[r-1][g];

            for(b=1; b<=32; b++){
                line   += pwt[b];
                line_r += pmr[b]; 
                line_g += pmg[b]; 
                line_b += pmb[b];
                line2  += pm2[b];
                area[b] += line;
                area_r[b] += line_r;
                area_g[b] += line_g;
                area_b[b] += line_b;
                area2[b] += line2;
                pwt[b] = pwt1[b] + area[b];
                pmr[b] = pmr1[b] + area_r[b];
                pmg[b] = pmg1[b] + area_g[b];
                pmb[b] = pmb1[b] + area_b[b];
                pm2[b] = pm21[b] + area2[b];
            }
        }
    }
}


// Compute sum over a box of any given statistic 
int
XiliBestColormap::Vol(Cbox*          cube, 
                     unsigned int*** mmt)
{
    //
    //  TODO:  Remove this when the x86 compiler is fixed...
    //         (What does this mean ????? Link)
    //
    return( mmt[cube->r1][cube->g1][cube->b1] 
           -mmt[cube->r1][cube->g1][cube->b0]
           -mmt[cube->r1][cube->g0][cube->b1]
           +mmt[cube->r1][cube->g0][cube->b0]
           -mmt[cube->r0][cube->g1][cube->b1]
           +mmt[cube->r0][cube->g1][cube->b0]
           +mmt[cube->r0][cube->g0][cube->b1]
           -mmt[cube->r0][cube->g0][cube->b0] );
}

//
// The next two routines allow a slightly more efficient calculation
// of Vol() for a proposed subbox of a given box.  The sum of Top()
// and Bottom() is the Vol() of a subbox split in the given direction
// and with the specified new upper bound.
//
 

//
// Compute part of Vol(cube, mmt) that doesn't depend on r1, g1, or b1 
// (depending on dir) 
//
int
XiliBestColormap::Bottom(Cbox*           cube,
                         int             dir,
                         unsigned int*** mmt)
{
    switch(dir){
      case RED:
        return( -mmt[cube->r0][cube->g1][cube->b1]
                +mmt[cube->r0][cube->g1][cube->b0]
                +mmt[cube->r0][cube->g0][cube->b1]
                -mmt[cube->r0][cube->g0][cube->b0] );
      case GREEN:
        return( -mmt[cube->r1][cube->g0][cube->b1]
                +mmt[cube->r1][cube->g0][cube->b0]
                +mmt[cube->r0][cube->g0][cube->b1]
                -mmt[cube->r0][cube->g0][cube->b0] );
      case BLUE:
        return( -mmt[cube->r1][cube->g1][cube->b0]
                +mmt[cube->r1][cube->g0][cube->b0]
                +mmt[cube->r0][cube->g1][cube->b0]
                -mmt[cube->r0][cube->g0][cube->b0] );
    }
}


//
// Compute remainder of Vol(cube, mmt), substituting pos for 
// r1, g1, or b1 (depending on dir) 
//
int
XiliBestColormap::Top(Cbox*          cube,
                     int             dir,
                     int             pos,
                     unsigned int*** mmt)
{
    switch(dir){
      case RED:
        return( mmt[pos][cube->g1][cube->b1] 
               -mmt[pos][cube->g1][cube->b0]
               -mmt[pos][cube->g0][cube->b1]
               +mmt[pos][cube->g0][cube->b0] );
      case GREEN:
        return( mmt[cube->r1][pos][cube->b1] 
               -mmt[cube->r1][pos][cube->b0]
               -mmt[cube->r0][pos][cube->b1]
               +mmt[cube->r0][pos][cube->b0] );
      case BLUE:
        return( mmt[cube->r1][cube->g1][pos]
               -mmt[cube->r1][cube->g0][pos]
               -mmt[cube->r0][cube->g1][pos]
               +mmt[cube->r0][cube->g0][pos] );
    }
}


//
// Compute the weighted variance of a box 
// NB: as with the raw statistics, this is really the variance * size 
//
float
XiliBestColormap::Var(Cbox *cube )
{
    float dr = Vol(cube, mr); 
    float dg = Vol(cube, mg); 
    float db = Vol(cube, mb);
    float xx = m2[cube->r1][cube->g1][cube->b1] 
             - m2[cube->r1][cube->g1][cube->b0]
             - m2[cube->r1][cube->g0][cube->b1]
             + m2[cube->r1][cube->g0][cube->b0]
             - m2[cube->r0][cube->g1][cube->b1]
             + m2[cube->r0][cube->g1][cube->b0]
             + m2[cube->r0][cube->g0][cube->b1]
             - m2[cube->r0][cube->g0][cube->b0];

    return( xx - (dr*dr + dg*dg + db*db)/(float)Vol(cube,wt) );    
}

//
// We want to minimize the sum of the variances of two subboxes.
// The sum(c^2) terms can be ignored since their sum over both subboxes
// is the same (the sum for the whole box) no matter where we split.
// The remaining terms have a minus sign in the variance formula,
// so we drop the minus sign and MAXIMIZE the sum of the two terms.
// 


float
XiliBestColormap::Maximize(Cbox* cube,
                          int   dir,
                          int   first, 
                          int   last, 
                          int*  cut,
                          int   whole_r, 
                          int   whole_g, 
                          int   whole_b, 
                          int   whole_w )
{
    float temp;

    int base_r = Bottom(cube, dir, mr);
    int base_g = Bottom(cube, dir, mg);
    int base_b = Bottom(cube, dir, mb);
    int base_w = Bottom(cube, dir, wt);
    float max = 0.0;
    *cut = -1;
    for(int i=first; i<last; i++){
        int half_r = base_r + Top(cube, dir, i, mr);
        int half_g = base_g + Top(cube, dir, i, mg);
        int half_b = base_b + Top(cube, dir, i, mb);
        int half_w = base_w + Top(cube, dir, i, wt);

        // Now half_x is sum over lower half of box, if split at i 
        if (half_w == 0) {      // subbox could be empty of pixels! 
            continue;           // never split into an empty box 
        } else {
            temp = ((float)half_r*half_r + (float)half_g*half_g +
                    (float)half_b*half_b)/half_w;
        }

        half_r = whole_r - half_r;
        half_g = whole_g - half_g;
        half_b = whole_b - half_b;
        half_w = whole_w - half_w;
        if (half_w == 0) {      // subbox could be empty of pixels! 
            continue;           // never split into an empty box 
        } else {
            temp += ((float)half_r*half_r + (float)half_g*half_g +
                     (float)half_b*half_b)/half_w;
        }

        if (temp > max) {
            max  = temp; 
            *cut = i;
        }
    }
    return max;
}

Xil_boolean
XiliBestColormap::Cut(Cbox* set1, 
                      Cbox* set2 )
{
    int whole_r = Vol(set1, mr);
    int whole_g = Vol(set1, mg);
    int whole_b = Vol(set1, mb);
    int whole_w = Vol(set1, wt);

    int cutr, cutg, cutb;

    float maxr = Maximize(set1, RED, set1->r0+1, set1->r1, &cutr,
                          whole_r, whole_g, whole_b, whole_w);
    float maxg = Maximize(set1, GREEN, set1->g0+1, set1->g1, &cutg,
                          whole_r, whole_g, whole_b, whole_w);
    float maxb = Maximize(set1, BLUE, set1->b0+1, set1->b1, &cutb,
                          whole_r, whole_g, whole_b, whole_w);

    int dir;
    if( (maxr>=maxg)&&(maxr>=maxb) ) {
        dir = RED;
        if (cutr < 0) {
            return FALSE; // can't split the box 
        }
    } else {
        if( (maxg>=maxr)&&(maxg>=maxb) ) {
            dir = GREEN;
        } else {
            dir = BLUE; 
        }
    }

    set2->r1 = set1->r1;
    set2->g1 = set1->g1;
    set2->b1 = set1->b1;

    switch(dir) {
      case RED:
        set2->r0 = set1->r1 = cutr;
        set2->g0 = set1->g0;
        set2->b0 = set1->b0;
        break;
      case GREEN:
        set2->g0 = set1->g1 = cutg;
        set2->r0 = set1->r0;
        set2->b0 = set1->b0;
        break;
      case BLUE:
        set2->b0 = set1->b1 = cutb;
        set2->r0 = set1->r0;
        set2->g0 = set1->g0;
        break;
    }
    set1->vol=(set1->r1-set1->r0)*(set1->g1-set1->g0)*(set1->b1-set1->b0);
    set2->vol=(set2->r1-set2->r0)*(set2->g1-set2->g0)*(set2->b1-set2->b0);

    return TRUE;
}


XilStatus
XiliBestColormap::generateColormap8()
{
    //
    // Calculate all the cumulative moments
    //
    calculateCumulativeMoments();

    //
    // Do the partitioning
    //
    Cbox* cube = new Cbox[ncolors];
    if(cube == NULL) {
        XIL_ERROR(bestcmapState, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }

    float* vv   = new float[ncolors];
    if(vv == NULL) {
        delete cube;
        XIL_ERROR(bestcmapState, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }

    cube[0].r0 = cube[0].g0 = cube[0].b0 = 0;
    cube[0].r1 = cube[0].g1 = cube[0].b1 = 32;

    int next = 0;
    int k;
    for(int i=1; i<ncolors; i++) {
        if (Cut(&cube[next], &cube[i])) {
            // volume test ensures we won't try to cut one-cell box 
            vv[next] = (cube[next].vol>1) ? Var(&cube[next]) : 0.0;
            vv[i] = (cube[i].vol>1) ? Var(&cube[i]) : 0.0;
        } else {
            vv[next] = 0.0;   // don't try to split this box again 
            i--;              // didn't create box i 
        }
        next = 0; 
        float temp = vv[0];
        for(k=1; k<=i; k++) {
            if (vv[k] > temp) {
                temp = vv[k]; 
                next = k;
            }
        }
        if(temp <= 0.0) {
            ncolors = i+1;
            break;
        }
    }

    int weight;
    for(k=0; k<ncolors; k++){
        weight = Vol(&cube[k], wt);
        if (weight) {
            lut[k*3] = (unsigned char)(Vol(&cube[k], mr) / weight);
            lut[k*3+1] = (unsigned char)(Vol(&cube[k], mg) / weight);
            lut[k*3+2] = (unsigned char)(Vol(&cube[k], mb) / weight);
        } else {
            lut[k*3] = lut[k*3+1] = lut[k*3+2] = 0;
        }
    }

    delete cube;
    delete vv;

    return XIL_SUCCESS;
}
