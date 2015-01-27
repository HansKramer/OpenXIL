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
//  File:	XilScanlineList.cc
//  Project:	XIL
//  Revision:	1.34
//  Last Mod:	10:08:54, 03/10/00
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
//  MT-level:  UNSAFE
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilScanlineList.cc	1.34\t00/03/10  "

//
//  System Includes
//
#ifndef _WINDOWS
#include <values.h>
#endif
#include <math.h>

//
//  XIL Includes
//
#include "_XilDefines.h"
#include "_XilSystemState.hh"
#include "_XilScanlineList.hh"
#include "_XilBox.hh"

//
//  Internal Includes
//
#include "XiliUtils.hh"
#include "XiliConvexRegion.hh"

//#define SCAN_DEBUG 1

XilScanlineList::XilScanlineList(XilBox*       dst_box,
                                 const double* x_array,
                                 const double* y_array,
                                 unsigned int  num_points)
{
    unsigned int i;
    if( (unsigned)this % 8 != 0)
        xili_print_debug("XilScanlineList - invalid object address: %08x\n",
            this);

#ifdef SCAN_DEBUG
    fprintf(stderr, "INCOMING CONVEX REGION:  %d points\n", num_points);
    fprintf(stderr, "--------\n");
    for(i=0; i<num_points; i++) {
        fprintf(stderr, "(%.20f, %.20f)\n", x_array[i], y_array[i]);
    }
    fprintf(stderr, "--------\n");
    fprintf(stderr, "dst_box = %p : %p\n",
            dst_box, dst_box->getPrivateData());
#endif

    if(dst_box != NULL && dst_box->getPrivateData() != NULL) {
        x1PixelAdj = ((XiliConvexRegion*)dst_box->getPrivateData())->xPixelAdj;
        y1PixelAdj = ((XiliConvexRegion*)dst_box->getPrivateData())->yPixelAdj;

        if(x1PixelAdj < 0.0) {
            x2PixelAdj = 9.999999999e-12;
        } else {
            x2PixelAdj = x1PixelAdj;
            x1PixelAdj = -9.999999999e-12;
        }

        if(y1PixelAdj < 0.0) {
            y2PixelAdj = 9.999999999e-12;
        } else {
            y2PixelAdj = y1PixelAdj;
            y1PixelAdj = -9.999999999e-12;
        }
    } else {
        x1PixelAdj = -XILI_PIXEL_ADJACENCY;
        y1PixelAdj = -XILI_PIXEL_ADJACENCY;
        x2PixelAdj = XILI_PIXEL_ADJACENCY;
        y2PixelAdj = XILI_PIXEL_ADJACENCY;
    }

#ifdef SCAN_DEBUG
    fprintf(stderr, "SCANLINE LIST X1ADJ= %23.20f X2ADJ=%23.20f\n",
            x1PixelAdj, x2PixelAdj);
    fprintf(stderr, "SCANLINE LIST Y1ADJ= %23.20f Y2ADJ=%23.20f\n",
            y1PixelAdj, y2PixelAdj);
#endif

    switch(num_points) {
      case 0:
        scanList   = NULL;
        curEntry   = 0;
        numEntries = 0;
        return;

      case 1:
        //
        //  Only one entry makes for just one point.
        //
        initAsRect(x_array[0],
                   y_array[0],
                   x_array[0],
                   y_array[0]);

        return;

      case 2:
        //
        //  Look for special case of a horizontal or vertical line.
        //
        if(XILI_DBL_EQ(y_array[0] - y_array[1], 0.0)) {
            //
            //  Horizontal line.
            //
            initAsRect(_XILI_MIN(x_array[0], x_array[1]),
                       y_array[0],
                       _XILI_MAX(x_array[0], x_array[1]),
                       y_array[0]);

            return;
        } else if(XILI_DBL_EQ(x_array[0] - x_array[1], 0.0)) {
            //
            //  Vertical line.
            //
            initAsRect(x_array[0],
                       _XILI_MIN(y_array[0], y_array[1]),
                       x_array[0],
                       _XILI_MAX(y_array[0], y_array[1]));

            return;
        }
        break;

      case 4:
        XiliConvexRegion cr(num_points, x_array, y_array);
        if(cr.isRect()) {
            initAsRect(cr.lowX, cr.lowY, cr.highX, cr.highY);

            //
            //  We're done.
            //
            return;
        }
    }

    //
    //  Calculate the y extent of the arrays.
    //    
    double min_y =  XIL_MAXDOUBLE;
    double max_y = -XIL_MAXDOUBLE;

    for(i=0; i<num_points; i++) {
        if(y_array[i] < min_y) {
            min_y = y_array[i];
        }

        if(y_array[i] > max_y) {
            max_y = y_array[i];
        }
    }

    //
    //  Force them to integer values within the convex region.
    //
    min_y = ceil(min_y + y1PixelAdj);
    max_y = floor(max_y + y2PixelAdj);

#ifdef SCAN_DEBUG
    fprintf(stderr, "min_y= %.20f, max_y = %.20f\n",min_y, max_y);
#endif
    
    //
    //  Initialize the array based upon the maximum number of scanlines we'll
    //  need to store.
    //
    if(initValues((unsigned int)(max_y - min_y + 1)) == XIL_FAILURE) {
        return;
    }

    double y;
    int j = 0;
    for(i=0, y=min_y; i<numEntries; i++, y++) {
        double min_x =  XIL_MAXDOUBLE;
        double max_x = -XIL_MAXDOUBLE;

        for(unsigned int n1=0; n1<num_points; n1++) {
            int n2 = (n1 == (num_points - 1)) ? 0 : n1 + 1;

            //
            //  Test to see if y intersects this line segment.  Since we don't
            //  know the order, check both directions.
            //
            if((_XILI_MIN(y_array[n1], y_array[n2]) + y1PixelAdj <= y) &&
               (_XILI_MAX(y_array[n1], y_array[n2]) + y2PixelAdj >= y)) {
                //
                //  Ok, it crosses the line segment.  Now find out where.
                //
                if(XILI_DBL_EQ(y_array[n2] - y_array[n1], 0.0)) {
                    //
                    //  Horizontal line -- slope == 0
                    //
                    if(x_array[n1] < min_x) {
                        min_x = x_array[n1];
                    }
                    if(x_array[n2] < min_x) {
                        min_x = x_array[n2];
                    }

                    if(x_array[n1] > max_x) {
                        max_x = x_array[n1];
                    }
                    if(x_array[n2] > max_x) {
                        max_x = x_array[n2];
                    }
                } else if(XILI_DBL_EQ(x_array[n2] - x_array[n1], 0.0)) {
                    //
                    //  Vertical line -- slope == Inf
                    //
                    if(x_array[n1] < min_x) {
                        min_x = x_array[n1];
                    }
                    if(x_array[n1] > max_x) {
                        max_x = x_array[n1];
                    }
                } else {
                    //
                    //  Slanted line...
                    //
                    double p1_x = x_array[n1];
                    double p1_y = y_array[n1];
                    double p2_x = x_array[n2];
                    double p2_y = y_array[n2];

                    double x =
                        -(y*(p2_x - p1_x) + p1_x*p2_y - p1_y*p2_x)/(p1_y - p2_y);

                    if(x < min_x) {
                        min_x = x;
                    }
                    if(x > max_x) {
                        max_x = x;
                    }
                }
            }
        }

        if(min_x == XIL_MAXDOUBLE || max_x == -XIL_MAXDOUBLE) {
#ifdef DEBUG	    
            fprintf(stderr, "y= %f doesn't intersect with convex region\n", y);
#endif
            continue;
        }

#ifdef SCAN_DEBUG
        fprintf(stderr, "y=%3d : min=%22.20f : %-3d : max_x=%22.20f : %-3d\n",
                (int)y,
                min_x, (int)ceil(min_x  + x1PixelAdj),
                max_x, (int)floor(max_x + x2PixelAdj));
#endif

        //
        //  Store the scanlines generated by this algorithm.
        //
	//  Using ceil() and floor() ensures the integer points remain inside
        //  the convex region.
	//
        scanList[j].y  = (int)y;
        scanList[j].x1 = (unsigned int)ceil(min_x + x1PixelAdj);
        scanList[j].x2 = (unsigned int)floor(max_x + x2PixelAdj);

        //
        //  There are scenarios (especially when ROI's are used)
        //  that could produce scanlines where lower coordinate is
        //  greater than higher coordinate (from previous 2 assignments)
        //  For example (some possible scenarios) :
        //    1. f_x1 = f_x2 = 43.22 then *x1 = 44 and *x2 = 43
        //    2. f_x1 = 42.01 & f_x2 = 42.99, then *x1 = 43 and *x2 = 42
        //
        //  This is inconsistent with the XIL GPI which states that start is
        //  less than end.
        //
        //  To handle this, we first look for small convex regions that land on
        //  or very near a real (integer) pixel and return that...a point.  If
        //  the convex region doesn't consume or touch a pixel, then we just
        //  move onto the next point since there is no pixel on the scanline
        //  for the given convex region.
        //
        //  May not be the best approach, but letting this stand until a
        //  more acceptable soluton is reached.
        //
        if(scanList[j].x1 > scanList[j].x2) {
            if(_XILI_PIXEL_EQ_INT(min_x, x1PixelAdj)) {
                //
                //  It's really a point.
                //
                scanList[j].x1 = scanList[j].x2 = _XILI_NEAREST_PIXEL(min_x);
            } else if(_XILI_PIXEL_EQ_INT(max_x, x2PixelAdj)) {
                //
                //  It's really a point.
                //
                scanList[j].x1 = scanList[j].x2 = _XILI_NEAREST_PIXEL(max_x);
            } else {
                //
                //  It's a point or line that lands between two destination
                //  pixels.  We skip onto the next scanline because the convex
                //  region doesn't touch a pixel.
                //
                continue;
            }
        }

        //
        //  Increment to the next entry.
        //
        j++;
    }

    numEntries = j;
}

XilScanlineList::XilScanlineList(unsigned int x,
                                 unsigned int y,
                                 unsigned int xsize,
                                 unsigned int ysize)
{
    if(initValues(ysize) == XIL_FAILURE) {
        return;
    }

    for(unsigned int i=0; i<ysize; i++) {
        scanList[i].y  = y + i;
        scanList[i].x1 = x;
        scanList[i].x2 = x + xsize;
    }
}

//
//  Destructor destroy the scanline list created
//
XilScanlineList::~XilScanlineList()
{
    delete [] scanList;
}

XilStatus
XilScanlineList::initValues(unsigned int ysize)
{
    scanList = new XilScanline[ysize];
    if(scanList == NULL) {
        XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }

    curEntry   = 0;
    numEntries = ysize;

    return XIL_SUCCESS;
}

void
XilScanlineList::initAsRect(double x1,
                            double y1,
                            double x2,
                            double y2)
{
#ifdef SCAN_DEBUG
    fprintf(stderr, "initAsRect:  %23.20f, %23.20f, %23.20f, %23.20f\n",
            x1, y1, x2, y2);
#endif

    //
    //  Special case of a rectangle.
    //
    //  Force them to integer values within the convex region.
    //
    unsigned int min_y = (unsigned int)ceil(y1 + y1PixelAdj);
    unsigned int max_y = (unsigned int)floor(y2 + y2PixelAdj);

    //
    //  Initialize the array based upon the maximum number of
    //  scanlines we'll need to store.
    //
    if(initValues(max_y - min_y + 1) == XIL_FAILURE) {
        return;
    }

    unsigned int ystart = min_y;
    unsigned int xstart = (unsigned int)ceil(x1 + x1PixelAdj);
    unsigned int xend   = (unsigned int)floor(x2 + x2PixelAdj);

#ifdef SCAN_DEBUG
    fprintf(stderr, "y= %d --> %d : x=  %d --> %d\n",
            min_y, max_y, xstart, xend);
#endif
    
    //
    //  See more detailed comments below on what's going on here.
    //
    if(xstart > xend) {
        if(_XILI_PIXEL_EQ_INT(x1, x1PixelAdj)) {
            //
            //  It's really a point.
            //
            xstart = xend = _XILI_NEAREST_PIXEL(x1);
        } else if(_XILI_PIXEL_EQ_INT(x2, x2PixelAdj)) {
            //
            //  It's really a point.
            //
            xstart = xend = _XILI_NEAREST_PIXEL(x2);
        } else {
            //
            //  It's a rect that lands between two destination pixels.
            //
            //  There is nothing to scan convert.  Set numEntries to 0
            //  and return.
            //
            numEntries = 0;
            return;
        }
    }

    //
    //  Fill in the list.
    //
    for(unsigned int i=0; i<numEntries; i++) {
        scanList[i].y  = ystart++;
        scanList[i].x1 = xstart;
        scanList[i].x2 = xend;
    }
}

//
//  These are the GPI-exposed calls.
//
Xil_boolean
XilScanlineList::getNext(unsigned int* y,
                         unsigned int* x1,
                         unsigned int* x2)
{
    if(curEntry < numEntries) {
	*y  = scanList[curEntry].y;
	*x1 = scanList[curEntry].x1;
	*x2 = scanList[curEntry].x2;
	
        curEntry++;

        return TRUE;
    } else {
        return FALSE;
    }
}

unsigned int
XilScanlineList::getNumScanlines()
{
    return numEntries;
}

XilScanline*
XilScanlineList::getScanlines(unsigned int* num_scanlines)
{
    *num_scanlines = numEntries;

    return scanList;
}
