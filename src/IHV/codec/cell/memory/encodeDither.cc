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
//  File:   encodeDither.cc
//  Project:    XIL
//  Revision:   1.5
//  Last Mod:   10:15:55, 03/10/00
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
//  COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)encodeDither.cc	1.5\t00/03/10  "

#include "XilDeviceManagerCompressionCell.hh"
#include "XilDeviceCompressionCell.hh"

//
//  Some macros and inline functions
//
#define FMUL(A, B) ((float)(A) * (float)(B))
#define SETBIT(M, B) ((M) | (1 << ((BLOCKSIZE-1)-(B))))

#ifdef XIL_LITTLE_ENDIAN

#define   RED(cval)  (((*(const int*)&cval)>>8) &0xff)
#define   GRN(cval)  (((*(const int*)&cval)>>16)&0xff)
#define   BLU(cval)  (((*(const int*)&cval)>>24)&0xff)

#define   REDI(cval)  (((cval)>>8) &0xff)
#define   GRNI(cval)  (((cval)>>16)&0xff)
#define   BLUI(cval)  (((cval)>>24)&0xff)

#define   REDP(cval)  (((*(const int*)cval)>>8) &0xff)
#define   GRNP(cval)  (((*(const int*)cval)>>16)&0xff)
#define   BLUP(cval)  (((*(const int*)cval)>>24)&0xff)

#else

#define   RED(cval)  (((*(const int*)&cval)>>16)&0xff)
#define   GRN(cval)  (((*(const int*)&cval)>>8) &0xff)
#define   BLU(cval)  (((*(const int*)&cval)&0xff))

#define   REDI(cval)  (((cval)>>16)&0xff)
#define   GRNI(cval)  (((cval)>>8) &0xff)
#define   BLUI(cval)  (((cval)&0xff))

#define   REDP(cval)  (((*(const int*)cval)>>16)&0xff)
#define   GRNP(cval)  (((*(const int*)cval)>>8) &0xff)
#define   BLUP(cval)  (((*(const int*)cval)&0xff))

#endif

//
//  Our dither mask
//
char ditherBlock[16] = {
     0,  8,  2, 10,
    12,  4, 14,  6,
     3, 11,  1,  9,
    15,  7, 13,  5
};


//
//    Compute the average 4x4 Block of Color Values
//    
ColorValue
blockAvg(ColorValue* blk)
{
    int r = 0, g = 0, b = 0;

    ColorValue* pCV = blk;
    for(int i=0; i<4; i++) {
      for(int j=0; j<4; j++) {
        r += pCV->band0();
        g += pCV->band1();
        b += pCV->band2();
        pCV++;
      }
    }

    r = (r + 16/2)>>4;
    g = (g + 16/2)>>4;
    b = (b + 16/2)>>4;

    ColorValue tmp;
    tmp.band0() = r;
    tmp.band1() = g;
    tmp.band2() = b;
    
    return tmp;
}

//
//    Compute the variance 4x4 Block of Color Values
//    
int
blockVar(ColorValue* blk)
{
    int mr, mg, mb;
    int vr = 0, vg = 0, vb = 0;

    ColorValue mean = blockAvg(blk);
    
    mr = mean.band0();
    mg = mean.band1();
    mb = mean.band2();
    
    ColorValue* pCV = blk;
    for(int i=0; i<4; i++) {
      for(int j=0; j<4; j++) {
        ColorValue c = *pCV;
        pCV++;
        
#ifdef TODO
      // Replace after sqrs_table is in
        vr += sqrs_table[c.band0() - mr];
        vg += sqrs_table[c.band1() - mg];
        vb += sqrs_table[c.band2() - mb];
#endif
        vr += ((c.band0() - mr) * (c.band0() - mr));
        vg += ((c.band1() - mg) * (c.band1() - mg));
        vb += ((c.band2() - mb) * (c.band2() - mb));
      }
    }

    vr = (vr + 8)>>4;
    vg = (vg + 8)>>4;
    vb = (vb + 8)>>4;

    return (vr + vg + vb);
}


//------------------------------------------------------------------------
//
//  Class:    DistMap
//
// Description:
//    This class is used by the dither encoding algorithm to
//    maintain a sorted list of colors, their indices, and their
//    distance to the mean.
//    
//------------------------------------------------------------------------
struct DistMapEntry {
    ColorValue  color;
    int         index;
    int         dist;
};

class DistMap {
public:
    DistMap(CmapTable& t, ColorValue mean);
    ~DistMap(void) { delete distMap; }
    
    
    ColorValue& color(int i) {
        return distMap[i].color;
    }
    
    int&        index(int i) {
        return distMap[i].index;
    }
    
    int&        distance(int i) {
        return distMap[i].dist;
    }
    
    int         getSize(void) const {
        return size;
    }

private:
    DistMapEntry* distMap;
    const int     size;

    void heapSort(void);
    
    void swapDistMapEntries(DistMapEntry* e1, DistMapEntry* e2) {
      ColorValue ctmp;
      int        tmp;

      ctmp       =  e2->color;
      e2->color  =  e1->color;
      e1->color  =  ctmp;
    
      tmp        =  e2->index;
      e2->index  =  e1->index;
      e1->index  =  tmp;
    
      tmp        =  e2->dist;
      e2->dist   =  e1->dist;
      e1->dist   =  tmp;
    }
    
    int leftChildIndex(int i) { return ((i<<1) + 1); }
    
};

DistMap::DistMap(CmapTable& t, ColorValue mean) : size(t.getNumEntries())
{
    distMap = new DistMapEntry[size];
    
    if (distMap == NULL) {
      XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);
      return;
    }
    
    for (int i=0; i<size; i++) {
      distMap[i].color = t[i];
      distMap[i].index = i;
      distMap[i].dist  = t[i].distance(mean);
    }

    heapSort();
}

//------------------------------------------------------------------------
//
//  Function:    DistMap::heapSort
//  Created:    92/05/15
//
//  Description:
//    Uses the heap sort algorithm to sort the distance list in
//    increasing order.
//    
//  Parameters:    void.
//    
//  Returns:       void.
//    
//  Side Effects:  The distMap is sorted in increasing order based on
//             distance.
//    
//------------------------------------------------------------------------
void  DistMap::heapSort(void)
{
    DistMapEntry* pnode;
    DistMapEntry* lnode;
    DistMapEntry* rnode;
    int     firsthalf, index, child;
    int     csize = size;

    //  .... during the first half of the algorithm we convert
    //  the list to a linear heap, meaning each parent node (with
    //  array index N) is greater than its two children (which
    //  have indicies 2N+1 and 2N+2).... this relationship is
    //  maintained by the second while( ) loop .... after we have
    //  made the list into a heap we swap the top of the heap
    //  with the bottom and convert the remaining list back into
    //  a heap .... The result is sorted in increasing order ....

    firsthalf = csize>>1;
    while (csize > 1) {
      if (firsthalf > 0) {
        firsthalf--;
        index = firsthalf;
        pnode = &distMap[index];
      } else {
        csize--;
        pnode = &distMap[csize];
        swapDistMapEntries(distMap, pnode);
        index = 0;
        pnode = distMap;
      }

      while ((child = leftChildIndex(index)) < csize) {
        lnode = &distMap[child];
        if (child + 1 < csize) {
          rnode = lnode + 1;
          if (rnode->dist > lnode->dist) {
            lnode = rnode;
            child++;
          }
        }

        if (lnode->dist > pnode->dist) {
          swapDistMapEntries(pnode, lnode);
          index = child;
          pnode = lnode;
        } else
        break;
      }
    }
}

//------------------------------------------------------------------------
//
//  Function:    XilDevicecompressionCell::encodeDither
//
//  Description:
//    The routine that controls the dither encoding process for a
//    single 4x4 Block.  First, a DistMap class is created to
//    maintain the distances of each color in the colormap
//    to the mean of the block.  All of the candidates for the color
//    must be less than 2 times the variance of the block.  Each
//    color segment between every candidate is checked to find which
//    has the lowest error.  The Cell is constructed and returned.
//    
//  Parameters:    Block<ColorValue,4,4>&
//    
//  Returns:       Cell
//    
//  Side Effects:  None.
//    
//------------------------------------------------------------------------

const int  DITHERSCALE   = 16;
const int  BLOCKSIZE     = 16;
#ifdef NOCOMPILE
const int  MAPSIZE       = 256;
#endif
const int  MINCANDIDATES = 10;
const int  MAXSQR        = 46340;
const int  BIGERROR      = (BLOCKSIZE*256*256);

Cell XilDeviceCompressionCell::encodeDither(ColorValue* block)
{
    ColorValue mean = blockAvg(block);
    DistMap    dmap(compData.currentCmap, mean);

    //
    //  Determine which colors are candidates
    //
    int        vari = blockVar(block);
    for (int i=MINCANDIDATES; i<dmap.getSize(); i++)
      if (dmap.distance(i) > (2*vari))
        break;

      //
      //  Set this variable which is used in tryColorSegment
      //
      compData.ditherMinError  = (float)BIGERROR;

      ColorValue* cvp = block;
      Cell        cell;
      int         maxnoise   = 42;
      int         candidates = i;
      int         tmp, j;

      for (i=0; i<candidates; i++) {
        for (j=i+1; j<candidates; j++) {

        tmp = tryColorSegment(dmap.color(i),
                              dmap.color(j),
                              cvp,
                              maxnoise);
        
        if(tmp >= 0) {
          cell.setCell(u_short(tmp),
                       Xil_unsigned8(dmap.index(i)),
                       Xil_unsigned8(dmap.index(j)));
        }
      }
    }
    
    return cell;
}

int
XilDeviceCompressionCell::tryColorSegment(const ColorValue& color0,
                                          const ColorValue& color1,
                                          const ColorValue* block,
                                          int maxnoise)

{
    //     .... the line segment between the two Colors is defined as:
    //         l(t) = C0 + t(C1 - C0)
    //     the vector from a point to that line is:
    //         d(t) = P - C0 - t(C1 - C0)
    //     using the following substitutions:
    //         X = C1 - C0    and    Y = P - C0
    //         d(t) = Y - tX
    //     the minimum distance from the point to the line can be found
    //     by finding t where d(t) is perpendicular to the line segment.
    //         X o d(t) =  X o (Y - tX) = 0
    //     yeilding:
    //         tmin = XoY / XoX
    //     and the distance from the point to the line is:
    //         d(tmin) o d(tmin) = YoY - (XoY * XoY) / XoX
    //     at the boundry conditions:
    //         d(0) o d(0) = YoY
    //         d(1) o d(1) = YoY - 2 XoY + XoX ....
    // 
    // 
    //     .... this better/bogus dither allows dithering between
    //     the two endpoints of the segment while the resulting added
    //     noise is constrained to be less than some maximum.  When
    //     the segment's length exceeds the maximum allowed noise the
    //     dither matrix is rescaled to some sub-segment of the line
    //     which is centered about the segment's midpoint ....
    // 
    //     When the segment's length is greater than maxnoise the dither
    //     noise function becomes:
    //         (t > ((dither[u][v]/SCALE - 1/2) * maxnoise/len(C1 - C0)) + 1/2)
    // 
    //     substituting tmin for t, X for (C1 - C0), and HSCALE for SCALE/2:
    //         (XoY / XoX) >
    //         ((dither[u][v]/SCALE - 1/2) * maxnoise/sqrt(XoX)) + 1/2
    //     
    //         (XoY / XoX) >
    //         ((dither[u][v] - HSCALE) * maxnoise + HSCALE * sqrt(XoX))
    //         --------------------------------------------------------
    //                       SCALE * sqrt(XoX)
    //     cross multiplying:
    //         (XoY * SCALE) >
    //         sqrt(XoX) * maxnoise * (dither[u][v] - HSCALE) + XoX * HSCALE
    // 
    //     substituting
    //         Da = sqrt(XoX) * maxnoise    and    Db = HSCALE * (XoX - Da)
    // 
    //     the dither noise function becomes:
    //         (XoY * SCALE) > (Da * dither[u][v] + Db)
    // 
    //     when the segments length is less than maxnoise the dither
    //     noise function is the normal:
    //         t > (dither[u][v] / SCALE)
    // 
    //     which becomes:
    //         (XoY / XoX) > (dither[u][v] / SCALE)
    //         (XoY * SCALE) > (XoX * dither[u][v]) ....
    //     

    int         Xr, Xg, Xb;
    int         Yr, Yg, Yb;
    float       XoY, XoX, YoY, iXoX;
    float       da, db;

    Xb  = BLU(color1) - BLU(color0);
    Xg  = GRN(color1) - GRN(color0);
    Xr  = RED(color1) - RED(color0);

#ifdef TODO
    // Replace after sqrs_table is in
    XoX = sqrs_table[Xr] + sqrs_table[Xg] + sqrs_table[Xb];
#endif
    XoX = (Xr*Xr) + (Xg*Xg) + (Xb*Xb);
    
    if (XoX != 0.0)
      iXoX = (float) 1.0 / XoX;


#ifdef TODO
    // Replace after sqrs_table is in
    if (sqrs_table[maxnoise] >= XoX) {
#endif
    if ((maxnoise*maxnoise) >= XoX) {
      da = XoX;
      db = 0;
    } else {
      da = sqrt((double) XoX) * maxnoise;
      db = (DITHERSCALE/2) * (XoX - da);
    }

    float       dist = 0.0;
    int         mask = 0;
    for (int i=0; i<16; i++) {
      int cv = *((int*)block + i);

      Yb = BLUI(cv) - BLU(color0);
      Yg = GRNI(cv) - GRN(color0);
      Yr = REDI(cv) - RED(color0);
#ifdef TODO
      // Replace after sqrs_table is in
      YoY = sqrs_table[Yr] + sqrs_table[Yg] + sqrs_table[Yb];
#endif
      YoY = (Yr*Yr) + (Yg*Yg) + (Yb*Yb);
    
      XoY = FMUL(Xr,Yr) + FMUL(Xg,Yg) + FMUL(Xb,Yb);
    
      if ((XoY <= 0.0) || (XoX == 0.0)) {
        //  color 0 is closest or degenerate segment
        dist += YoY;
      } else if (XoY >= XoX) {
        //  color 1 is closest
        dist += YoY - 2.0 * XoY + XoX;
        mask = SETBIT(mask, i);
      } else {
        dist += YoY - XoY * XoY * iXoX;
        if (DITHERSCALE * XoY > ditherBlock[i] * da + db)
          mask = SETBIT(mask, i);
      }
      if (dist > compData.ditherMinError)
        return -1;
    }
    
    if ((dist == compData.ditherMinError) &&
        (XoX > compData.ditherMinXoX)) {
      return -1;
    }
    
    compData.ditherMinError = dist;
    compData.ditherMinXoX   = XoX;
    
    return mask;
}
