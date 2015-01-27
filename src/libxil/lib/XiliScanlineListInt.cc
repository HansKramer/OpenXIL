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
//  File:	XiliScanlineListInt.cc
//  Project:	XIL
//  Revision:	1.6
//  Last Mod:	10:08:58, 03/10/00
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
#pragma ident	"@(#)XiliScanlineListInt.cc	1.6\t00/03/10  "

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
#include "_XilBox.hh"

//
//  Internal Includes
//
#include "XiliUtils.hh"
#include "XiliScanlineListInt.hh"
#include "XiliConvexRegion.hh"

//#define SCAN_DEBUG 1

XiliScanlineListInt::XiliScanlineListInt(const double* x_array,
                                         const double* y_array,
                                         unsigned int  num_points)
{
    unsigned int i;

#ifdef SCAN_DEBUG
    fprintf(stderr, "INCOMING CONVEX REGION\n");
    fprintf(stderr, "--------\n");
    for(i=0; i<num_points; i++) {
        fprintf(stderr, "(%f, %f)\n", x_array[i], y_array[i]);
    }
    fprintf(stderr, "--------\n");
#endif

    if(num_points == 1) {
        //
        //  Only one entry makes for just one point.
        //
        initAsRect(x_array[0],
                   y_array[0],
                   x_array[0],
                   y_array[0]);

        return;
    } else if(num_points == 2) {
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
    } else if(num_points == 4) {
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
    min_y = ceil(min_y - XILI_PIXEL_ADJACENCY);
    max_y = floor(max_y + XILI_PIXEL_ADJACENCY);

#ifdef SCAN_DEBUG
    fprintf(stderr, "min_y = %f, max_y = %f\n",min_y, max_y);
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
            if((_XILI_MIN(y_array[n1], y_array[n2]) - XILI_PIXEL_ADJACENCY <= y) &&
               (_XILI_MAX(y_array[n1], y_array[n2]) + XILI_PIXEL_ADJACENCY >= y)) {
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
    fprintf(stderr, "y = %4d : min= %12.6f : %-4d -- max_x = %12.6f : %-4d\n",
            (int)y,
            min_x, (int)ceil(min_x  - XILI_PIXEL_ADJACENCY),
            max_x, (int)floor(max_x + XILI_PIXEL_ADJACENCY));
#endif

        //
        //  Store the scanlines generated by this algorithm.
        //
	//  Using ceil() and floor() ensures the integer points remain inside
        //  the convex region.
	//
        scanList[j].y  = (int)y;
        scanList[j].x1 = (int)ceil(min_x - XILI_PIXEL_ADJACENCY);
        scanList[j].x2 = (int)floor(max_x + XILI_PIXEL_ADJACENCY);

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
            if(_XILI_PIXEL_EQ_INT(min_x)) {
                //
                //  It's really a point.
                //
                scanList[j].x1 = scanList[j].x2 = _XILI_NEAREST_PIXEL(min_x);
            } else if(_XILI_PIXEL_EQ_INT(max_x)) {
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

//
//  Destructor destroy the scanline list created
//
XiliScanlineListInt::~XiliScanlineListInt()
{
    delete [] scanList;
}

XilStatus
XiliScanlineListInt::initValues(unsigned int ysize)
{
    scanList = new XiliScanlineInt[ysize];
    if(scanList == NULL) {
        XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }

    numEntries = ysize;

    return XIL_SUCCESS;
}

void
XiliScanlineListInt::initAsRect(double x1,
                                double y1,
                                double x2,
                                double y2)
{
    //
    //  Special case of a rectangle.
    //
    //  Force them to integer values within the convex region.
    //
    int min_y = (int)ceil(y1 - XILI_PIXEL_ADJACENCY);
    int max_y = (int)floor(y2 + XILI_PIXEL_ADJACENCY);

    //
    //  Initialize the array based upon the maximum number of
    //  scanlines we'll need to store.
    //
    if(initValues(max_y - min_y + 1) == XIL_FAILURE) {
        return;
    }

    int ystart = (int)min_y;
    int xstart = (int)ceil(x1 - XILI_PIXEL_ADJACENCY);
    int xend   = (int)floor(x2 + XILI_PIXEL_ADJACENCY);

    //
    //  See more detailed comments below on what's going on here.
    //
    if(xstart > xend) {
        if(_XILI_PIXEL_EQ_INT(x1)) {
            //
            //  It's really a point.
            //
            xstart = xend = _XILI_NEAREST_PIXEL(x1);
        } else if(_XILI_PIXEL_EQ_INT(x2)) {
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

XiliScanlineInt*
XiliScanlineListInt::getScanlines(unsigned int* num_scanlines)
{
    *num_scanlines = numEntries;

    return scanList;
}
