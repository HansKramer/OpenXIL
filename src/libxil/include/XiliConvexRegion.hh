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
//  File:	XiliConvexRegion.hh
//  Project:	XIL
//  Revision:	1.34
//  Last Mod:	10:22:04, 03/10/00
//
//  Description:
//	A type to hold information of a convex region, up to a maximum
//	of 8 pts
//	
//  MT-level:  <SAFE>
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XiliConvexRegion.hh	1.34\t00/03/10  "

#ifndef _XILI_CONVEX_REGION_H
#define _XILI_CONVEX_REGION_H

#include "_XilDefines.h"
#include "XiliRect.hh"
#include "XiliUtils.hh"
#include "xili_geom_utils.hh"

const int MAX_VERTEX_PTS = 32;
const int MAX_CR_PTS     = 16;
const int RECT_CR_PTS    = 4;

class XiliConvexRegion {
public:
    double          xPtArray[MAX_CR_PTS];
    double          yPtArray[MAX_CR_PTS];
    unsigned int    pointCount;
    double          lowX;
    double          lowY;
    double          highX;
    double          highY;

    //
    //  Estimated pixel adjacencies for determining if one pixel is next to
    //  the other.  it's used by the intersect routine and should be used when
    //  scan converting a convex region.
    //
    double          xPixelAdj;
    double          yPixelAdj;
    
    //
    // Constructors/Destructors
    //
    XiliConvexRegion()
    {
        pointCount = 0;
    }

    // WARNING WARNING WARNING
	// If you modify the code below you must modify ../lib/XiliConvexRegion.cc
#if !defined(HPUX)
    XiliConvexRegion(XiliConvexRegion* region)
    {
        pointCount = region->pointCount;

        lowX  = region->lowX;
        lowY  = region->lowY;
        highX = region->highX;
        highY = region->highY;
    
        for(unsigned int i=0; i<pointCount; i++) {
            xPtArray[i] = region->xPtArray[i];
            yPtArray[i] = region->yPtArray[i];
        }
    }
#else
	XiliConvexRegion(XiliConvexRegion* region);
#endif

    //
    //  Setting with a rect.
    //
    XiliConvexRegion(XiliRect* rect)
    {
        rect->get(&lowX, &lowY, &highX, &highY);

        pointCount = RECT_CR_PTS;
        xPtArray[0] = xPtArray[3] = lowX;
        xPtArray[1] = xPtArray[2] = highX;
        yPtArray[0] = yPtArray[1] = lowY;
        yPtArray[2] = yPtArray[3] = highY;
    }

    XiliConvexRegion(XiliRectInt* rect)
    {
        pointCount = RECT_CR_PTS;
        xPtArray[0] = xPtArray[3] = lowX  = rect->getX1();
        yPtArray[0] = yPtArray[1] = lowY  = rect->getY1();
        xPtArray[1] = xPtArray[2] = highX = rect->getX2();
        yPtArray[2] = yPtArray[3] = highY = rect->getY2();
    }

    XiliConvexRegion(XiliRectDbl* rect)
    {
        pointCount = RECT_CR_PTS;
        xPtArray[0] = xPtArray[3] = lowX  = rect->getX1();
        yPtArray[0] = yPtArray[1] = lowY  = rect->getY1();
        xPtArray[1] = xPtArray[2] = highX = rect->getX2();
        yPtArray[2] = yPtArray[3] = highY = rect->getY2();
    }

    XiliConvexRegion(double x1,
		     double y1,
		     double x2,
		     double y2)
    {
        set(x1, y1, x2, y2);
    }
    
    // WARNING WARNING WARNING
	// If you modify the code below you must modify ../lib/XiliConvexRegion.cc
#if !defined(HPUX)
    //
    //  Create from point list, points need to be clockwise ordered
    //
    XiliConvexRegion(unsigned int  npts,
		     const double* x,
		     const double* y)
    {
        pointCount = npts;
        lowX = highX = x[0];
        lowY = highY = y[0];

        for(unsigned int i=0; i<pointCount; i++) {
            xPtArray[i] = x[i];
            yPtArray[i] = y[i];

            lowX  = _XILI_MIN(x[i], lowX);
            lowY  = _XILI_MIN(y[i], lowY);
            highX = _XILI_MAX(x[i], highX);
            highY = _XILI_MAX(y[i], highY);
        }
    }
#else
	XiliConvexRegion(unsigned int  npts,
                     const double* x,
                     const double* y);
#endif

    ~XiliConvexRegion()
    {
    }
    
    //
    // Extend the convex region to extent space from coordinate space.
    // TODO: maynard 10/1/96 - this currently only works
    // for rectangular regions.
    //
    void              extend(XiliConvexRegion* cr);

    //
    //  Set the convex region from another convex region
    //
    void              set(XiliConvexRegion* region)
    {
        pointCount = region->pointCount;

        lowX  = region->lowX;
        lowY  = region->lowY;
        highX = region->highX;
        highY = region->highY;
    
        for(unsigned int i=0; i<pointCount; i++) {
            xPtArray[i] = region->xPtArray[i];
            yPtArray[i] = region->yPtArray[i];
        }
    }

    //
    //  Set the convex region from a rectangle
    //
    void              set(XiliRect* rect)
    {
        rect->get(&lowX, &lowY, &highX, &highY);

        set(lowX, lowY, highX, highY);
    }

    void              set(XiliRectInt* rect)
    {
        set(rect->getX1(), rect->getY1(), rect->getX2(), rect->getY2());
    }

    void              set(XiliRectDbl* rect)
    {
        set(rect->getX1(), rect->getY1(), rect->getX2(), rect->getY2());
    }

    //
    //  Set the convex region from four double points.
    //
    void              set(double x1,
                          double y1,
                          double x2,
                          double y2);

    //
    //  Set the convex region from an array of points, clockwise ordered.
    //
    void              set(unsigned int  npts,
                          const double* x,
                          const double* y)
    {
        pointCount = npts;
        lowX = highX = x[0];
        lowY = highY = y[0];

        for(unsigned int i=0; i<pointCount; i++) {
            xPtArray[i] = x[i];
            yPtArray[i] = y[i];

            lowX  = _XILI_MIN(x[i], lowX);
            lowY  = _XILI_MIN(y[i], lowY);
            highX = _XILI_MAX(x[i], highX);
            highY = _XILI_MAX(y[i], highY);
        }
    }

    //
    // Methods to get arrays and num points
    //
    const double*     getXPtArray()
    {
        return xPtArray;
    }

    const double*     getYPtArray()
    {
        return yPtArray;
    }

    int               getPointCount()
    {
        return pointCount;
    }

    //
    // Get the bounding box for the convex region
    //
    void              getBoundingBox(double* low_x,
				     double* low_y,
				     double* high_x,
				     double* high_y)
    {
        *low_x  = lowX;
        *low_y  = lowY;
        *high_x = highX;
        *high_y = highY;
    }
    
    //
    //  A convex region may be a representation of a simpler integer rectangle
    //  region.  This does not test for integer only values.
    //
    Xil_boolean       isRect();

    //
    //  Used to figure out if a convex region touches a given tile
    //
    Xil_boolean       touchesRect(int x1,
                                  int y1,
                                  unsigned int xsize,
                                  unsigned int ysize);

    //
    //  Given a rect clip the convex region to that rect to produce a
    //  different convex region.
    //
    XilStatus         clip(double x1,
                           double y1,
                           double x2,
                           double y2);
    
    // WARNING WARNING WARNING
	// If you modify the code below you must modify ../lib/XiliConvexRegion.cc
#if !defined(HPUX)
    XilStatus         clip(XiliRect* rect)
    {
        if(rect->getType() == XILI_RECT_TYPE_DOUBLE) {
            XiliRectDbl* r = (XiliRectDbl*)rect;
            return clip(r->getX1(), r->getY1(), r->getX2(), r->getY2());
        } else {
            XiliRectInt* r = (XiliRectInt*)rect;
            return clip(r->getX1(), r->getY1(), r->getX2(), r->getY2());
        }
    }
#else
    XilStatus         clip(XiliRect* rect);
#endif
    
    //
    //  Intersect a convex region with this one and
    //  put the new convex region in the given region.
    //
    XilStatus        intersect(XiliConvexRegion* intersect,
                               XiliConvexRegion* result);

    //
    // Intersect a convexRegion that's a line with
    // another convexRegion
    //
    XilStatus        intersect_line(XiliConvexRegion* region1,
                                    XiliConvexRegion* region2,
                                    XiliConvexRegion* result);
    
    //
    //  Do the intersection and leave the result in this convex region.
    //
    XilStatus        intersect(XiliConvexRegion* intersect);

    //
    //  Translate this into the given convex region.
    //
    void             translate(XiliConvexRegion* translated,
			       double            x,
			       double            y)
    {
        if(translated == NULL) {
            return;
        }
    
        for(unsigned int p=0; p<pointCount; p++) {
            translated->xPtArray[p] = xPtArray[p] + x;
            translated->yPtArray[p] = yPtArray[p] + y;
        }

        translated->lowX  = lowX  + x;
        translated->lowY  = lowY  + y;
        translated->highX = highX + x;
        translated->highY = highY + y;
    }

    //
    //  Translate this convex region by specified x and y.
    //
    void             translate(double x,
                               double y)
    {
        for(unsigned int p=0; p<pointCount; p++) {
            xPtArray[p] += x;
            yPtArray[p] += y;
        }

        lowX  += x;
        lowY  += y;
        highX += x;
        highY += y;
    }

    int              operator == (const XiliConvexRegion& rval) const
    {
        if(pointCount == rval.pointCount) {
            for(unsigned int i=0; i<pointCount; i++) {
                if(! XILI_PIXEL_EQ(xPtArray[i], rval.xPtArray[i]) ||
                   ! XILI_PIXEL_EQ(yPtArray[i], rval.yPtArray[i])) {
                    return FALSE;
                }
            }
	}

        return TRUE;
    }

    void             dump()
    {
        fprintf(stderr, "------ BEGIN ------\n");
        for(unsigned int i=0; i<pointCount; i++) {
            fprintf(stderr, "\t(%20.10f, %20.10f)\n", xPtArray[i], yPtArray[i]);
        }

        fprintf(stderr, "\t %20.10f %20.10f %20.10f %20.10f\n",
                lowX, lowY, highX, highY);
        fprintf(stderr, "------- END -------\n");
    }

private:
    //
    // Methods used by clip to convert from a Vertex list
    // in xili_geom_utils to a convexRegion and vice versa.
    // We need the vertex list to do the computation of the
    // clip and intersection so we store both inside the class
    //
    void             setupVertexArray(XiliConvexRegion* region,
                                      Vertex* vlist);
    void             setupPointArrays(Vertex* vlist);

    //
    // Is a point inside the region
    //
    Xil_boolean      point_inside(double            x,
				  double            y,
				  XiliConvexRegion* cr,
				  int               side);

    //
    // Is a point on the line
    //
    Xil_boolean      point_on_line(double a1x,
                                   double a1y,
                                   double a2x,
                                   double a2y,
                                   double x,
                                   double y);

    //
    // Coaelsce a convexregion
    //
    void             coaelsce(XiliConvexRegion* cr);
    
    //
    // Intersection of two points and the region
    //
    void             get_intersection(double            a1x,
				      double            a1y,
				      double            a2x,
				      double            a2y,
				      XiliConvexRegion* cr,
				      int               side,
				      double*           newx,
				      double*           newy);


    //
    // Used by the touches rect code
    //
    Xil_boolean intersect_segments(double   a1x,
				   double   a1y,
				   double   a2x,
				   double   a2y,
				   double   b1x,
				   double   b1y,
				   double   b2x,
				   double   b2y,
				   double*  det,
				   double*  det_ta,
				   double*  det_tb);
    
    Xil_boolean segment_intersect_vertical_segment(double  a1x,
						   double  a1y,
						   double  a2x,
						   double  a2y,
						   double  bx,
						   double  b1y,
						   double  b2y,
						   double* det,
						   double* det_ta,
						   double* det_tb);
    
    Xil_boolean segment_intersect_horizontal_segment(double  a1x,
						     double  a1y,
						     double  a2x,
						     double  a2y,
						     double  b1x,
						     double  b2x,
						     double  by,
						     double* det,
						     double* det_ta,
						     double* det_tb);
    
    Xil_boolean segment_intersect_rect(double  a1x,
				       double  a1y,
				       double  a2x,
				       double  a2y,
				       double  xlo,
				       double  ylo,
				       double  xhi,
				       double  yhi,
				       double* wrapstate);
    void        get_line(double  lx1,
                         double  ly1,
                         double  lx2,
                         double  ly2,
                         double  vx,
                         double  vy,
                         double  vnx,
                         double  vny,
                         double* rx1,
                         double* ry1,
                         double* rx2,
                         double* ry2,
                         int*    is_intersect);
};

#endif // _XILI_CONVEX_REGION_H





