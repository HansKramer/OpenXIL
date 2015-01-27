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
//  File:	XiliConvexRegion.cc
//  Project:	XIL
//  Revision:	1.57
//  Last Mod:	10:09:03, 03/10/00
//
//  Description:
//	
//	
//	
//  MT-level:  <SAFE>
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XiliConvexRegion.cc	1.57\t00/03/10  "

//
// C++ Includes
//
#include "_XilDefines.h"
#include "_XilMutex.hh"
#include "_XilSystemState.hh"

#include "XiliConvexRegion.hh"
#include "XiliUtils.hh"

//
// DUMP_DEBUG would just turn on the convexRegions and the resulting
//            intersection. Fast way to check the result.
// INTERSECT_DEBUG is more detailed. Shows every point's status.
//
//#define DUMP_DEBUG 1
//#define INTERSECT_DEBUG 1

// WARNING WARNING WARNING
// If you modify the code below you must modify ../lib/XiliConvexRegion.cc
#if defined(HPUX)
XiliConvexRegion::XiliConvexRegion(XiliConvexRegion* region)
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
#endif

//
// Extend from the pixel coordinate representation to the pixel
// extent representation. 
//
void
XiliConvexRegion::extend(XiliConvexRegion* extended_cr)
{
    
    //
    // TODO: maynard 10/1/96
    // This will currently only work correctly for a
    // 4pt rectangular region.
    // Given the way this is being used, this is
    // okay for now.
    // I didn't use the isRect check because of speed.
    // 
    
    if(pointCount != RECT_CR_PTS) {
        return;
    }

    extended_cr->pointCount = this->pointCount;

    extended_cr->xPtArray[0] = this->xPtArray[0] - XILI_TOP_LF_EXTENT;
    extended_cr->yPtArray[0] = this->yPtArray[0] - XILI_TOP_LF_EXTENT;
    
    extended_cr->xPtArray[1] = this->xPtArray[1] + XILI_BOT_RT_EXTENT;
    extended_cr->yPtArray[1] = this->yPtArray[1] - XILI_TOP_LF_EXTENT;
    
    extended_cr->xPtArray[2] = this->xPtArray[2] + XILI_BOT_RT_EXTENT;
    extended_cr->yPtArray[2] = this->yPtArray[2] + XILI_BOT_RT_EXTENT;
    
    extended_cr->xPtArray[3] = this->xPtArray[3] - XILI_TOP_LF_EXTENT;
    extended_cr->yPtArray[3] = this->yPtArray[3] + XILI_BOT_RT_EXTENT;
    
    extended_cr->lowX  = this->lowX  - XILI_TOP_LF_EXTENT;
    extended_cr->lowY  = this->lowY  - XILI_TOP_LF_EXTENT;
    extended_cr->highX = this->highX + XILI_BOT_RT_EXTENT;
    extended_cr->highY = this->highY + XILI_BOT_RT_EXTENT;
}

void
XiliConvexRegion::set(double x1,
                      double y1,
                      double x2,
                      double y2)
{
    if(XILI_PIXEL_EQ(x1, x2) && XILI_PIXEL_EQ(y1, y2)) {
        //
        // Special case : It's a point
        //
        pointCount = 1;
        xPtArray[0] = lowX = highX = x1;
        yPtArray[0] = lowY = highY = y1;
    } else if(XILI_PIXEL_EQ(x1, x2) || XILI_PIXEL_EQ(y1, y2)) {
        //
        // Special case : It's a vertical or a horizontal line
        //                Set from bounding box, so assumes x1 < x2
        //                and y1 < y2
        //
        pointCount = 2;
        xPtArray[0] = lowX = x1;
        yPtArray[0] = lowY = y1;
        xPtArray[1] = highX = x2;
        yPtArray[1] = highY = y2;
    } else {
        //
        // It's a rect
        // could be a diagonal line in which case convexRegion is
        // the bounding box of that line
        //
        pointCount = RECT_CR_PTS;
        xPtArray[0] = xPtArray[3] = lowX = x1;
        xPtArray[1] = xPtArray[2] = highX = x2;
        yPtArray[0] = yPtArray[1] = lowY = y1;
        yPtArray[2] = yPtArray[3] = highY = y2;
    }
}

//
//  Is it a rectangle
//
Xil_boolean
XiliConvexRegion::isRect()
{
    //
    //  It is okay that this doesn't also check for integer part because when
    //  this check is used, the bbox is used to get the values.
    //
    if(pointCount != RECT_CR_PTS) {
        return FALSE;
    }

    //
    // TODO : Venu 10/10/96
    //        Special case code specifically for rectangle checking
    //        Might do a different version later in time
    //

    //
    // Check all points against lowX, lowY, highX, highy
    // to verify that these are indeed rectangles
    //
    Xil_boolean check1, check2, check3, check4;

    //
    // Initialize to FALSE
    //
    check1 = check2 = check3 = check4 = FALSE;

    for(unsigned int i=0; i<RECT_CR_PTS; i++) {
        if((XILI_PIXEL_EQ(xPtArray[i], this->lowX)) &&
           (XILI_PIXEL_EQ(yPtArray[i], this->lowY))) {
            check1 = TRUE;
            break;
        }
    }

    if(check1 == FALSE) return FALSE;
    
    for(i=0; i<RECT_CR_PTS; i++) {
        if((XILI_PIXEL_EQ(xPtArray[i], this->lowX)) &&
           (XILI_PIXEL_EQ(yPtArray[i], this->highY))) {
            check2 = TRUE;
            break;
        }
    }

    if(check2 == FALSE) return FALSE;


    for(i=0; i<RECT_CR_PTS; i++) {
        if((XILI_PIXEL_EQ(xPtArray[i], this->highX)) &&
           (XILI_PIXEL_EQ(yPtArray[i], this->lowY))) {
            check3 = TRUE;
            break;
        }
    }

    if(check3 == FALSE) return FALSE;

    for(i=0; i<RECT_CR_PTS; i++) {
        if((XILI_PIXEL_EQ(xPtArray[i], this->highX)) &&
           (XILI_PIXEL_EQ(yPtArray[i], this->highY))) {
            check4 = TRUE;
            break;
        }
    }

    return check4;
}

//
// Does it touch a rectangle
//
Xil_boolean
XiliConvexRegion::touchesRect(int x1,
                              int y1,
                              unsigned int xsize,
                              unsigned int ysize)
{
    double      xRectLo = (double)x1;
    double      xRectHi = (double)(x1+xsize-1);
    double      yRectLo = (double)y1;
    double      yRectHi = (double)(y1+ysize-1);
    double      wrapState;
    double      wrapTest;
    Xil_boolean wrapStateChanged = FALSE;

    //
    //  Check for bounding box overlap first.
    //
    if((xRectLo > highX) || (xRectHi < lowX) || 
       (yRectLo > highY) || (yRectHi < lowY)) {
        return FALSE;
    }

    //
    //  From here, brute force intersections for now.
    //  There are enhancements we could do for performance 
    //  if needed:
    //    - make state for which "quadrant" around rect for each point in
    //        polygon; intersection can only occur when quadrants change
    //  Notes: 
    //        Simply checking for a cr point within the rect is insufficient.
    //        ULHC determinant is used for "entire rect within polygon" case.
    //

    if(segment_intersect_rect(xPtArray[0], yPtArray[0],
                              xPtArray[pointCount-1], yPtArray[pointCount-1],
                              xRectLo, yRectLo, xRectHi, yRectHi,
                              &wrapState)) {
        return TRUE;
    }

    for(unsigned int i=1; i<pointCount; i++) {
        //
        // compute intersection with each side of rect
        //
        if(segment_intersect_rect(xPtArray[i], yPtArray[i],
                                  xPtArray[i-1], yPtArray[i-1],
                                  xRectLo, yRectLo, xRectHi, yRectHi, &wrapTest)) {
            return TRUE;
        }
        
        if(wrapState * wrapTest < 0.0) {
            wrapStateChanged = TRUE;
        }
    }

    if(wrapStateChanged) {
        return FALSE; // rect outside region
    } else {
        return TRUE;
    }
}

//
//  Convert from ourselves to a vertex list
//
void
XiliConvexRegion::setupVertexArray(XiliConvexRegion* region,
                                   Vertex* vlist)
{ 
    for(unsigned int i=0; i<region->pointCount; i++) {
        vlist[i].x = region->xPtArray[i];
        vlist[i].y = region->yPtArray[i];
        vlist[i].next = &vlist[(i+1)%region->pointCount];
    }
}

//
// Convert from a vertex list to ourselves
//
void
XiliConvexRegion::setupPointArrays(Vertex* vlist)
{
    //
    // Copy the first point
    //
    int i = 0;
    xPtArray[i] = lowX = highX = vlist->x;
    yPtArray[i] = lowY = highY = vlist->y;

    //
    // Now loop round the vertex list until
    // we reach the start again.
    //
    i++;
    Vertex* start = vlist;
    while((vlist = vlist->next) != start) {
        xPtArray[i] = vlist->x;
        yPtArray[i] = vlist->y;
        
        // Set up the bounding box
        lowX = _XILI_MIN(lowX, xPtArray[i]);
        lowY = _XILI_MIN(lowY, yPtArray[i]);
        highX = _XILI_MAX(highX, xPtArray[i]);
        highY = _XILI_MAX(highY, yPtArray[i]);
        
        i++;
    }
    pointCount = i;
}

//
// Utility code to take a convex region and clip it
// some rectangle.
//
XilStatus
XiliConvexRegion::clip(double x1,
                       double y1,
                       double x2,
                       double y2)
{
    XiliConvexRegion region(x1, y1, x2, y2);
    XiliConvexRegion tmp_region;
    
    //
    // Intersecting a rect with a convex region should never
    // produce more than a single new region
    //
    if(intersect(&region, &tmp_region) == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    if(tmp_region.pointCount == 0) {
        return XIL_FAILURE;
    }

    //
    // Copy the intersected region into this region
    //
    *this = tmp_region;

    return XIL_SUCCESS;
}

//
// Code to intersect two convex regions (polygons).  Each polygon
// has maximum 8 sides; the intersected polygon has maximum 16 sides. 
// If resultant polygon has more than 8 sides, split it into
// smaller polygons.  It returns the number of new polygons.
// (There may be as many as three).
//
// Bounding boxes are part of polygon here, but they could be
// calculated as-needed instead.
//

//
// Intersection expects both convexRegions to be ordered
// clockwise.
//
XilStatus
XiliConvexRegion::intersect(XiliConvexRegion* intersect,
                            XiliConvexRegion* newRegion)
{
    unsigned int i;
    unsigned int p2side;
    int          verts_to_test;
    int          num_cur_vert;
    int          new_vertex_index;
    Vertex*      vptr;
    Vertex*      start_clip;
    Vertex*      end_clip;
    Vertex       va[MAX_VERTEX_PTS];
    Vertex       start_v;
    Vertex       end_v;
    
#ifdef DUMP_DEBUG
fprintf(stderr, "\nIntersecting 2 convexregions\n");
intersect->dump();
this->dump(); 
#endif
    
    //
    // SPECIAL CASE - both "this" and the "intersect" convex regions
    // are really rectangles. Do a much simpler intersection!
    //
    if(this->isRect() && intersect->isRect()) {
#ifdef DUMP_DEBUG
        fprintf(stderr, "Rectangular intersection\n");
#endif
        
        double tmp_x1 = _XILI_MAX(lowX, intersect->lowX);
        double tmp_x2 = _XILI_MIN(highX, intersect->highX);
        double tmp_y1 = _XILI_MAX(lowY, intersect->lowY);
        double tmp_y2 = _XILI_MIN(highY, intersect->highY);

        //
        //  If it's clipped to an empty rect, then return empty intersection;
        //
        if(! XILI_CHECK_RECT_EMPTY(&tmp_x1, &tmp_y1, tmp_x2, tmp_y2)) {
            newRegion->pointCount = 0;
            return XIL_SUCCESS;
        }
        
        //
        // This intersection could produce a point, line or a Rectangle
        //

        if((XILI_PIXEL_EQ(tmp_x1, tmp_x2)) &&
           (XILI_PIXEL_EQ(tmp_y1, tmp_y2))) {
            //
            // It's a point
            //
            newRegion->pointCount = 1;
            newRegion->xPtArray[0] = tmp_x1;
            newRegion->yPtArray[0] = tmp_y1;
            newRegion->lowX =
                newRegion->highX = tmp_x1;
            newRegion->lowY =
                newRegion->highY = tmp_y1;
        } else if((XILI_PIXEL_EQ(tmp_x1, tmp_x2)) ||
                  (XILI_PIXEL_EQ(tmp_y1, tmp_y2))) {
            //
            // It's a line
            //
            newRegion->pointCount = 2;

            if(XILI_PIXEL_EQ(tmp_x1, tmp_x2)) {
                //
                // Vertical line
                //
                newRegion->xPtArray[0] = tmp_x1;
                newRegion->yPtArray[0] = tmp_y1;
                newRegion->xPtArray[1] = tmp_x1;
                newRegion->yPtArray[1] = tmp_y2;
                newRegion->lowX =
                    newRegion->highX = tmp_x1;
                newRegion->lowY = tmp_y1;
                newRegion->highY = tmp_y2;
            } else {
                //
                // Horizontal line
                //
                newRegion->xPtArray[0] = tmp_x1;
                newRegion->yPtArray[0] = tmp_y1;
                newRegion->xPtArray[1] = tmp_x2;
                newRegion->yPtArray[1] = tmp_y1;
                newRegion->lowX = tmp_x1;
                newRegion->highX = tmp_x2;
                newRegion->lowY =
                    newRegion->highY = tmp_y1;
            }           
        } else {
            //
            // It's a rectangle
            //
            newRegion->pointCount = RECT_CR_PTS;
            newRegion->xPtArray[0] =
                newRegion->xPtArray[3] =
                newRegion->lowX = tmp_x1;
            newRegion->xPtArray[1] =
                newRegion->xPtArray[2] =
                newRegion->highX = tmp_x2;
            newRegion->yPtArray[0] =
                newRegion->yPtArray[1] =
                newRegion->lowY = tmp_y1;
            newRegion->yPtArray[2] =
                newRegion->yPtArray[3] =
                newRegion->highY = tmp_y2;
        }
          
           
#ifdef DUMP_DEBUG
                fprintf(stderr, "Resulting convexRegion :\n");
                newRegion->dump();
#endif
                
        return XIL_SUCCESS;
    }

#ifdef DUMP_DEBUG
    fprintf(stderr, "Convexregion intersection \n");
#endif
    
    //
    //  Another SPECIAL CASE : one of the convexRegion's is a point
    //
    if((intersect->pointCount == 1) ||
       (this->pointCount == 1) ) {
        //
        // It is a point.
        //
//#define POINT_DEBUG 1
#ifdef POINT_DEBUG
        fprintf(stderr, "It's a point\n");
#endif

        if(intersect->pointCount == 1) {
            //
            // Check and see if this point is "inside" of all
            // line segments of convexRegion *this
            //
            verts_to_test = this->pointCount;
            unsigned int side = 0;
            while((verts_to_test > 0) &&
                  (side < this->pointCount) &&
                  (point_inside(intersect->xPtArray[0],
                                intersect->yPtArray[0],
                                this,
                                side) == TRUE)) {
                verts_to_test--;
                //
                // Test against next side of convexRegion
                //
                side += 1;
            }

            if(verts_to_test == 0) {
                //
                // Since this point is inside of all individual lines
                // of a ConvexRegion, it is inside that ConvexRegion
                //
#ifdef POINT_DEBUG
                fprintf(stderr, "point is inside the Convexregion\n\n");
#endif

                newRegion->pointCount = 1;
                newRegion->xPtArray[0] =
                    newRegion->lowX =
                    newRegion->highX = intersect->xPtArray[0];
                newRegion->yPtArray[0] =
                    newRegion->lowY =
                    newRegion->highY = intersect->yPtArray[0];

#ifdef DUMP_DEBUG
                fprintf(stderr, "Resulting convexRegion :\n");
                newRegion->dump();
#endif

                return XIL_SUCCESS;
            } else {
                //
                // The point is on the outside of one of the line segments
                // of the convexRegions.
                //
#ifdef POINT_DEBUG
                fprintf(stderr, "point is outside the convexRegion\n\n");
#endif
                newRegion->pointCount = 0;

#ifdef DUMP_DEBUG
                fprintf(stderr, "Resulting convexRegion : 0 vertices \n");
#endif

                return XIL_SUCCESS;
            }
        } else if(this->pointCount == 1) {
            //
            // Check and see if this point is "inside" of all
            // line segments of convexRegion *this
            //
            verts_to_test = intersect->pointCount;
            unsigned int side = 0;
            while((verts_to_test > 0) &&
                  (side < intersect->pointCount) &&
                  (point_inside(this->xPtArray[0],
                                this->yPtArray[0],
                                intersect,
                                side) == TRUE)) {
                verts_to_test--;
                //
                // Test against next side of convexRegion
                //
                side += 1;
            }

            if (verts_to_test == 0) {
                //
                // Since this point is inside of all individual lines
                // of a ConvexRegion, it is inside that ConvexRegion
                //
#ifdef POINT_DEBUG
                fprintf(stderr, "point is inside the Convexregion\n\n");
#endif

                newRegion->pointCount = 1;
                newRegion->xPtArray[0] =
                    newRegion->lowX =
                    newRegion->highX = this->xPtArray[0];
                newRegion->yPtArray[0] =
                    newRegion->lowY =
                    newRegion->highY = this->yPtArray[0];

#ifdef DUMP_DEBUG
                fprintf(stderr, "Resulting convexRegion :\n");
                newRegion->dump();
#endif

                return XIL_SUCCESS;
            } else {
                //
                // The point is on the outside of one of the line segments
                // of the convexRegions.
                //
#ifdef POINT_DEBUG
                fprintf(stderr, "point is outside the convexRegion\n\n");
#endif

                newRegion->pointCount = 0;

#ifdef DUMP_DEBUG
                fprintf(stderr, "Resulting convexRegion : 0 vertices \n");
#endif

                return XIL_SUCCESS;
            }
        }
    }

    //
    // Special case line intersection
    //
    if((intersect->pointCount == 2) ||
       (this->pointCount == 2)) {
        //
        // Rearrange params
        //
        if(intersect->pointCount == 2) {
            return intersect_line(intersect, this, newRegion);
        } else {
            return intersect_line(this, intersect, newRegion);
        }
    }

    
    //
    // copy poly1 to initialize the vertex_info array
    //
    num_cur_vert = pointCount;
    setupVertexArray(this, va);
      
    //
    // start adding new vertices here
    //
    new_vertex_index = num_cur_vert;

    //
    // each side of poly2 describes a half plane which
    // can be used to clip the current contents of va
    //
    p2side = 0;
    vptr = &va[0];
        
    while((p2side < intersect->pointCount) && (num_cur_vert > 0)) {
        start_clip = NULL;
        verts_to_test = num_cur_vert;
        
#ifdef INTERSECT_DEBUG
        fprintf(stderr, "Side %d\n", p2side);
#endif

        //
        //  Look at first point
        //
        if(point_inside(vptr->x, vptr->y, intersect, p2side)) {
            //
            //  Look for a vertex outside
            //
            while((point_inside(vptr->next->x,
                                vptr->next->y,
                                intersect,
                                p2side) == TRUE) &&
                  (verts_to_test > 0)) {
                verts_to_test--;
                vptr = vptr->next;
            }

            //
            //  If there was a point outside, compute intersection
            //
            if(verts_to_test != 0) {
                start_clip = vptr;
                
#ifdef INTERSECT_DEBUG
                fprintf(stderr, "start_clip @ %.15f, %.15f\n",
                        start_clip->x, start_clip->y);
#endif

                get_intersection(vptr->x,
                                 vptr->y,
                                 vptr->next->x,
                                 vptr->next->y,
                                 intersect,
                                 p2side,
                                 &start_v.x,
                                 &start_v.y);
#ifdef INTERSECT_DEBUG
                fprintf(stderr, "Start Intersection @ %.15f, %.15f\n",
                        start_v.x, start_v.y);
#endif
                
                //
                // Move to next vertex
                //
                vptr = vptr->next;
                 
                // 
                // Now second crossover
                // 
                while(point_inside(vptr->next->x,
                                   vptr->next->y,
                                   intersect,
                                   p2side) == FALSE){
#ifdef INTERSECT_DEBUG
                    fprintf(stderr, "Clipping %.15f, %.15f\n",
                            vptr->next->x, vptr->next->y);
#endif

                    num_cur_vert--;
                    vptr = vptr->next;
                }
                
                //
                //  end_clip is the last vertex before crossover
                //
                end_clip = vptr;
                
#ifdef INTERSECT_DEBUG
                fprintf(stderr, "end_clip @ %.15f, %.15f\n",
                        end_clip->x, end_clip->y);
#endif
                
                get_intersection(vptr->x,
                                 vptr->y,
                                 vptr->next->x,
                                 vptr->next->y,
                                 intersect,
                                 p2side,
                                 &end_v.x,
                                 &end_v.y);

#ifdef INTERSECT_DEBUG
                fprintf(stderr, "end Intersection @ %.15f, %.15f\n",
                        end_v.x, end_v.y);
                fprintf(stderr, "Replacing %.15f, %.15f with %.15f, %.15f\n",
                        end_clip->x, end_clip->y, end_v.x, end_v.y);
#endif 
                
                //
                // Replace end_clip with end_v vertex values
                // 
                end_clip->x = end_v.x;
                end_clip->y = end_v.y;

                if((XILI_PIXEL_EQ(end_clip->x, end_clip->next->x)) &&
                   (XILI_PIXEL_EQ(end_clip->y, end_clip->next->y))) {
                    end_clip = end_clip->next;
                    num_cur_vert--;
                }

                if((XILI_PIXEL_EQ(start_clip->x, start_v.x)) &&
                   (XILI_PIXEL_EQ(start_clip->y, start_v.y))) {
                    // 
                    // No new vertex 
                    //   
#ifdef INTERSECT_DEBUG   
                    fprintf(stderr, "Start_clip & start_v identical\n");
#endif                       
                    start_clip->next = end_clip;
                    //
                    // point to valid vertex
                    //
                    vptr = start_clip;
                } else {
                    // 
                    // Add start_v while maintaining convexRegion 
                    // continuity    
                    //   
#ifdef INTERSECT_DEBUG   
                    fprintf(stderr, "Adding %.15f, %.15f\n",
                            start_v.x, start_v.y); 
#endif   
 
                    va[new_vertex_index] = start_v;
                    va[new_vertex_index].next = end_clip;
                    start_clip->next = &va[new_vertex_index];
                    new_vertex_index++;
                    num_cur_vert++;

                    // 
                    // Point to valid vertex 
                    //  
                    vptr = start_clip->next;
                }

            } else {
                //
                // all points on "inside" of this side
                //
            }
            
        } else {
            // 
            // first point is outside
            // Look for a vertex inside
            //
            while((point_inside(vptr->next->x,
                                vptr->next->y,
                                intersect,
                                p2side) == FALSE) &&
                  (verts_to_test > 0)) {
                verts_to_test--;
                vptr = vptr->next;
            }

            //
            //  If there was a point inside, compute intersection
            //
            if(verts_to_test != 0) {
                end_clip = vptr;
                
#ifdef INTERSECT_DEBUG
                fprintf(stderr, "end_clip @ %.15f, %.15f\n",
                        end_clip->x, end_clip->y);
#endif

                get_intersection(vptr->x,
                                 vptr->y,
                                 vptr->next->x,
                                 vptr->next->y,
                                 intersect,
                                 p2side,
                                 &end_v.x,
                                 &end_v.y);
                
#ifdef INTERSECT_DEBUG
                fprintf(stderr, "End Intersection @ %.15f, %.15f\n",
                        end_v.x, end_v.y);
#endif

                vptr = vptr->next;
                
                // 
                //  Now look for second crossover
                //                
                while(point_inside(vptr->next->x,
                                   vptr->next->y,
                                   intersect,
                                   p2side) == TRUE){
                    verts_to_test--;
                    vptr = vptr->next;
                }
                
                start_clip = vptr;
                
#ifdef INTERSECT_DEBUG
                fprintf(stderr, "start_clip @ %.15f, %.15f\n",
                        start_clip->x, start_clip->y);
#endif
 
                get_intersection(vptr->x,
                                 vptr->y,
                                 vptr->next->x,
                                 vptr->next->y,
                                 intersect,
                                 p2side,
                                 &start_v.x,
                                 &start_v.y);
                
#ifdef INTERSECT_DEBUG
                fprintf(stderr, "Start Intersection @ %.15f, %.15f\n",
                        start_v.x, start_v.y);
                fprintf(stderr, "Replacing %.15f, %.15f with %.15f, %.15f\n",
                        end_clip->x, end_clip->y, end_v.x, end_v.y);
#endif
 
                while(vptr->next != end_clip) {
#ifdef INTERSECT_DEBUG 
fprintf(stderr, "Clipping %.15f, %.15f\n", vptr->next->x, vptr->next->y); 
#endif  
                    vptr = vptr->next;
                    num_cur_vert--;
                }
                
                //
                // Replace end_clip with end_v coordinate values
                //
                end_clip->x = end_v.x;
                end_clip->y = end_v.y;

                if((XILI_PIXEL_EQ(end_clip->x, end_clip->next->x)) &&
                   (XILI_PIXEL_EQ(end_clip->y, end_clip->next->y))) {
                    //
                    // 2 consecutive vertices are ~1000 same
                    //
                    end_clip = end_clip->next;
                    num_cur_vert--;
                }

                if((XILI_PIXEL_EQ(start_clip->x, start_v.x)) &&
                   (XILI_PIXEL_EQ(start_clip->y, start_v.y))) {
                    //
                    // No new vertex
                    //  
#ifdef INTERSECT_DEBUG  
                    fprintf(stderr, "Start_clip & start_v identical\n");
#endif                      
                    start_clip->next = end_clip;
                    //
                    // point to valid vertex
                    //
                    vptr = start_clip;
                } else {
                    //
                    // Add start_v while maintaining convexRegion
                    // continuity   
                    //  
#ifdef INTERSECT_DEBUG  
                    fprintf(stderr, "Adding %.15f, %.15f\n",
                            start_v.x, start_v.y);
#endif  
 
                    va[new_vertex_index] = start_v;
                    va[new_vertex_index].next = end_clip;
                    start_clip->next = &va[new_vertex_index];
                    new_vertex_index++;
                    num_cur_vert++;

                    //
                    // Point to valid vertex
                    // 
                    vptr = start_clip->next;
                }
                
            } else {
                //
                // all points outside
                //
                num_cur_vert = 0;
            }
        }

#ifdef INTERSECT_DEBUG
        fprintf(stderr, "\nCurrent polygon %d vertices:\n", num_cur_vert);
        for(i=0; i<num_cur_vert; i++) {
            fprintf(stderr, "%.15f, %.15f\n", vptr->x, vptr->y);
            vptr = vptr->next;
        }

        fprintf(stderr, "\n");
#endif
        
        p2side++;
    }
        
    if(num_cur_vert > 1) {
        if(num_cur_vert > 16) {
            XIL_ERROR(NULL, XIL_ERROR_INTERNAL, "di-431", TRUE);
            return XIL_FAILURE;
        }

        newRegion->pointCount = num_cur_vert;
        newRegion->lowX = newRegion->highX = vptr->x;
        newRegion->lowY = newRegion->highY = vptr->y;

        for(i=0; i<newRegion->pointCount; i++) {
            if((XILI_PIXEL_EQ(vptr->x, vptr->next->x)) &&
               (XILI_PIXEL_EQ(vptr->y, vptr->next->y))) {
                //
                // Check if 2 consecutive vertices are the same
                // within a precision of 1/100.
                //
                newRegion->pointCount -= 1;
                vptr = vptr->next;
                i--;
            } else {
                newRegion->xPtArray[i] = vptr->x;
                newRegion->yPtArray[i] = vptr->y;

                newRegion->lowX  = _XILI_MIN(newRegion->lowX, vptr->x);
                newRegion->lowY  = _XILI_MIN(newRegion->lowY, vptr->y);
                newRegion->highX = _XILI_MAX(newRegion->highX, vptr->x);
                newRegion->highY = _XILI_MAX(newRegion->highY, vptr->y);
                vptr = vptr->next;
            }
        }

#ifdef DUMP_DEBUG
        fprintf(stderr, "Resulting convexRegion :\n");
        newRegion->dump();
#endif

        return XIL_SUCCESS;
    } else if(num_cur_vert == 1) {
        //
        // Intersection produced 1 point
        //
        newRegion->pointCount = 1;
        newRegion->xPtArray[0] = vptr->x;
        newRegion->yPtArray[1] = vptr->y;

        newRegion->lowX  = vptr->x;
        newRegion->lowY  = vptr->y;
        newRegion->highX = vptr->x;
        newRegion->highY = vptr->y;

#ifdef DUMP_DEBUG
        fprintf(stderr, "Resulting convexRegion :\n");
        newRegion->dump();
#endif

        return XIL_SUCCESS;
    } else {
        //
        // Null intersection
        //
        newRegion->pointCount = 0;

#ifdef DUMP_DEBUG
        fprintf(stderr, "Resulting ConvexRegion : 0 vertices\n");
#endif
       
        return XIL_SUCCESS;
    }
}

//
// PRIVATE METHODS
// used by intersection and touches rect code
//
Xil_boolean
XiliConvexRegion::point_on_line(double a1x,
                                double a1y,
                                double a2x,
                                double a2y,                        
                                double x,
                                double y)
{
#ifdef INTERSECT_DEBUG
    fprintf(stderr, "point_on_line():\n");
    fprintf(stderr, "\t(%.10lf, %.10lf) -> (%.10lf, %.10lf) w/ (%.10lf, %.10lf)\n",
            a1x, a1y, a2x, a2y, x, y);
#endif

    //
    // If the point is on the line (either end_point) within pixel adjacency
    // tolerance return TRUE
    //
    if((XILI_PIXEL_EQ(a1x, x) && XILI_PIXEL_EQ(a1y, y)) ||
       (XILI_PIXEL_EQ(a2x, x) && XILI_PIXEL_EQ(a2y, y))) {
        //  
        // If the point on or around the line then it's on that line  
        //
#ifdef INTERSECT_DEBUG
        fprintf(stderr, "\t ---- Point on line\n");
#endif
        return TRUE;
    }

    //
    // If the point is on the (extended) line then return TRUE
    //
    if((XILI_PIXEL_EQ(a1x, x) && XILI_PIXEL_EQ(a2x, x)) ||
       (XILI_PIXEL_EQ(a1y, y) && XILI_PIXEL_EQ(a2y, y))) {
        //   
        // If the point on or around the (extended) line   
        // then it's on that line   
        //
#ifdef INTERSECT_DEBUG
        fprintf(stderr, "\t ---- Point on extended line\n");
#endif
        return TRUE;
    }

    //
    //  If determinant is 0 (within 1/100th precision) return true.
    //
    double det = ((a2x - a1x)*(y - a1y)) - ((x - a1x)*(a2y - a1y));

#ifdef INTERSECT_DEBUG
    fprintf(stderr, "det = %30.20lf : %30.20lf : %d\n",
            det, fabs(det), fabs(det) < 0.00000001);
#endif

    if(XILI_DBL_EQ_ZERO(det)) {
#ifdef INTERSECT_DEBUG
        fprintf(stderr,
                "\t ---- Point on line (not horiz or vert) : %.10lf\n", det);
#endif
        return TRUE;
    }

#ifdef INTERSECT_DEBUG
    fprintf(stderr, "Point not on line : det = %-20.10lf\n", det);
#endif
    
    return FALSE;
}

//
// Compute whether v is "inside" of side of polygon, given
// counter-clockwise winding
//
Xil_boolean
XiliConvexRegion::point_inside(double            x,
                               double            y,
                               XiliConvexRegion* cr,
                               int               side)
{
    double a1x = cr->xPtArray[side];
    double a1y = cr->yPtArray[side];
    double a2x = cr->xPtArray[(side+1)%(cr->pointCount)];
    double a2y = cr->yPtArray[(side+1)%(cr->pointCount)];
    double det;

#ifdef INTERSECT_DEBUG
    fprintf(stderr, "x=%.15f, y=%.15f\n", x, y); 
    fprintf(stderr, "a1x=%.15f, a1y=%.15f, a2x=%.15f, a2y=%.15f\n",a1x, a1y, a2x, a2y);
#endif

    det = ((a2x - a1x)*(y - a1y)) - ((x - a1x)*(a2y - a1y));

    //
    //  If the det is 0.0, it means that the point is on the
    //  line. It's considered to be inside the line segment
    //
    if(det < 0.0 && !XILI_DBL_EQ_ZERO(det)) {
#ifdef INTERSECT_DEBUG
fprintf(stderr, " - outside, det = %.15f\n", det);
#endif
        return FALSE;
    } else {
#ifdef INTERSECT_DEBUG
fprintf(stderr, " - inside, det = %.15f\n", det);
#endif
        return TRUE;
    }
    
}
        
//
// calculate the intersection of a segment with a polygon side
//
void
XiliConvexRegion::get_intersection(double            a1x,
                                   double            a1y,
                                   double            a2x,
                                   double            a2y,
                                   XiliConvexRegion* cr,
                                   int               side,
                                   double*           newx,
                                   double*           newy)
{
    double b1x = cr->xPtArray[side];
    double b1y = cr->yPtArray[side];
    double b2x = cr->xPtArray[(side+1)%cr->pointCount];
    double b2y = cr->yPtArray[(side+1)%cr->pointCount];
    double det;
    double det_ta;

#ifdef INTERSECT_DEBUG
    fprintf(stderr, "INTERSECT:\n");
    fprintf(stderr, "\ta1x=%.10f, a1y=%.10f, a2x=%.10f, a2y=%.10f\n",
            a1x, a1y, a2x, a2y);
    fprintf(stderr, "\tb1x=%.10f, b1y=%.10f, b2x=%.10f, b2y=%.10f\n",
            b1x, b1y, b2x, b2y);
#endif 

    det    = (a2x - a1x)*(b1y - b2y) - (a2y - a1y)*(b1x - b2x);
    det_ta = (b1x - a1x)*(b1y - b2y) - (b1y - a1y)*(b1x - b2x);

    
#ifdef INTERSECT_DEBUG
    fprintf(stderr, "det_ta=%30.20lf --- det=%30.20lf\n", det_ta,det);
#endif

    //
    //  We still need to check the determinant being very, very small because
    //  we use a slightly different calculation for the determinant here than
    //  in point_on_line().
    //
    if(XILI_DBL_EQ_ZERO(det)) {
        //
        //  The lines effectively overlap.
        //
#ifdef INTERSECT_DEBUG
        fprintf(stderr,
                "Lines (effectively) overlap - det == 0: %23.20f, %23.20f\n",
                a2x, a2y);
#endif

        *newx = a2x;
        *newy = a2y;
    } else if(XILI_DBL_EQ_ZERO(det_ta)) {
        //
        //  The lines effectively overlap.
        //
#ifdef INTERSECT_DEBUG
        fprintf(stderr,
                "Lines (effectively) overlap - det_ta == 0: %23.20f, %23.20f\n",
                a1x, a1y);
#endif

        *newx = a1x;
        *newy = a1y;
    } else {
        *newx = a1x + (det_ta/det)*(a2x - a1x);
        *newy = a1y + (det_ta/det)*(a2y - a1y);

#if 0
        //
        //  These tests shouldn't be needed with our testing det and det_ta
        //  for values very, very close to 0.  If bad intersections occur,
        //  these tests will finish the job -- but at an obvious performance
        //  cost which is why they're not being added now.  The simpler test
        //  of checking det and det_ta seems to work for all the cases I've
        //  found so far. -- jlf
        //
        //  Make certain we didn't get a bad intersection that can occur if
        //  the lines are very close, but still not equal to each other.
        //
        if(*newx < a1x && *newx < a2x) {
            *newx = _XILI_MIN(a1x, a2x);
        }
        if(*newy < a1y && *newy < a2y) {
            *newy = _XILI_MIN(a1y, a2y);
        }
        if(*newx > a1x && *newx > a2x) {
            *newx = _XILI_MAX(a1x, a2x);
        }
        if(*newy > a1y && *newy > a2y) {
            *newy = _XILI_MAX(a1y, a2y);
        }
#endif
    }

#ifdef INTERSECT_DEBUG
    fprintf(stderr, "new -------> (%30.20lf, %30.20lf)\n", *newx, *newy);
#endif
}

//
// compute intersection of two arbitrary segments
//
Xil_boolean
XiliConvexRegion::intersect_segments(double  a1x,
                                     double  a1y,
                                     double  a2x,
                                     double  a2y,
                                     double  b1x,
                                     double  b1y,
                                     double  b2x,
                                     double  b2y,
                                     double* det,
                                     double* det_ta,
                                     double* det_tb)
{
    *det    = (a2x-a1x)*(b1y-b2y) - (a2y-a1y)*(b1x-b2x);
    *det_ta = (b1x-a1x)*(b1y-b2y) - (b1y-a1y)*(b1x-b2x);
    *det_tb = (a2x-a1x)*(b1y-a1y) - (a2y-a1y)*(b1x-a1x);

    if (XILI_DBL_EQ_ZERO(*det) && !XILI_DBL_EQ_ZERO(*det_ta)) {
        return FALSE;
    }
    
    if(*det > 0.0) {
        if((*det_ta > 0.0) && (*det_ta < *det) &&
           (*det_tb > 0.0) && (*det_tb < *det)) {
            return TRUE;
        } else {
            return FALSE;
        }
    } else {
        if((*det_ta < 0.0) && (*det_ta > *det) &&
           (*det_tb < 0.0) && (*det_tb > *det)) {
            return TRUE;
        } else {
            return FALSE;
        }
    }
}

Xil_boolean
XiliConvexRegion::segment_intersect_vertical_segment(double  a1x,
                                                     double  a1y,
                                                     double  a2x,
                                                     double  a2y,
                                                     double  bx,
                                                     double  b1y,
                                                     double  b2y,
                                                     double* det,
                                                     double* det_ta,
                                                     double* det_tb)
{
    *det    = (a2x-a1x)*(b1y-b2y);
    *det_ta = (bx-a1x)*(b1y-b2y);
    *det_tb = (a2x-a1x)*(b1y-a1y) - (a2y-a1y)*(bx-a1x);

    if(XILI_DBL_EQ_ZERO(*det) && !XILI_DBL_EQ_ZERO(*det_ta)) {
        return FALSE;
    }
    
    if(*det > 0.0) {
        if((*det_ta > 0.0) && (*det_ta < *det) &&
           (*det_tb > 0.0) && (*det_tb < *det)) {
            return TRUE;
        } else {
            return FALSE;
        }
    } else {
        if((*det_ta < 0.0) && (*det_ta > *det) &&
           (*det_tb < 0.0) && (*det_tb > *det)) {
            return TRUE;
        } else {
            return FALSE;
        }
    }
}

Xil_boolean
XiliConvexRegion::segment_intersect_horizontal_segment(double  a1x,
                                                       double  a1y,
                                                       double  a2x,
                                                       double  a2y,
                                                       double  b1x,
                                                       double  b2x,
                                                       double  by,
                                                       double* det,
                                                       double* det_ta,
                                                       double* det_tb)
{
    *det    = -(a2y-a1y)*(b1x-b2x);
    *det_ta = -(by-a1y)*(b1x-b2x);
    *det_tb = (a2x-a1x)*(by-a1y) - (a2y-a1y)*(b1x-a1x);

    if(XILI_DBL_EQ_ZERO(*det) && !XILI_DBL_EQ_ZERO(*det_ta)) {
        return FALSE;
    }
    if(*det > 0.0) {
        if((*det_ta > 0.0) && (*det_ta < *det) &&
           (*det_tb > 0.0) && (*det_tb < *det)) {
            return TRUE;
        } else {
            return FALSE;
        }
    } else {
        if((*det_ta < 0.0) && (*det_ta > *det) &&
           (*det_tb < 0.0) && (*det_tb > *det)) {
            return TRUE;
        } else {
            return FALSE;
        }
    }
}

Xil_boolean
XiliConvexRegion::segment_intersect_rect(double  a1x,
                                         double  a1y,
                                         double  a2x,
                                         double  a2y,
                                         double  xlo,
                                         double  ylo,
                                         double  xhi,
                                         double  yhi,
                                         double* wrapstate)
{
    double det;
    double det_a;
    double det_b;

    if(segment_intersect_vertical_segment(a1x, a1y, a2x, a2y,
                                          xlo, ylo, yhi, &det,
                                          &det_a, &det_b)) {
        return TRUE;
    }
    
    *wrapstate = det_b;
    if(segment_intersect_vertical_segment(a1x, a1y, a2x, a2y,
                                          xhi, ylo, yhi, &det,
                                          &det_a, &det_b)) {
        return TRUE;
    }
    
    if(segment_intersect_horizontal_segment(a1x, a1y, a2x, a2y,
                                            xlo, xhi, ylo,
                                            &det, &det_a, &det_b)) {
        return TRUE;
    }
    
    if(segment_intersect_horizontal_segment(a1x, a1y, a2x, a2y,
                                            xlo, xhi, yhi,
                                            &det, &det_a, &det_b)) {
        return TRUE;
    }

    return FALSE;
}

//
// Coaelsce a convexRegion
// If 3 points of the convexRegion fall on the same line
// remove the center point as it's redundant
//

void
XiliConvexRegion::coaelsce(XiliConvexRegion* region)
{
    XiliConvexRegion tmp_region;

    tmp_region.set(region->pointCount,
                   region->xPtArray,
                   region->yPtArray);
    
    double x1, y1, x2, y2, x3, y3;

    unsigned int points = tmp_region.pointCount;
    unsigned int new_points = points;
    unsigned int i, j, k;

    unsigned int *which_pt = new unsigned int [points];

    //
    // First pass for x coordinates
    //
    j = 0;
    for(i = 0; i < points; i++) {            
        //
        // Get 3 adjacent points
        //
        x1 = tmp_region.xPtArray[i];
        x2 = tmp_region.xPtArray[(i+1) % points];
        x3 = tmp_region.xPtArray[(i+2) % points];

        if(XILI_PIXEL_EQ(x1, x2)) {
            //
            // 2 Adjacent points have the same x coordinate
            //
            if(XILI_PIXEL_EQ(x2, x3)) {
                //
                // 3 Consecutive points have the same x coordinate
                // Keep track of which point to remove
                new_points--;
                which_pt[j] = i + 1;
                j++;
            }                            
        }
    }

    //
    // Only coalesce if any points have been removed
    //
    if(j > 0) {
        //
        // Now remake the new convexRegion
        //
    
        region->pointCount = new_points;

        //
        // Start checking against first point to be excluded
        //
        i = 0;
        j = 0;
    
        for(k = 0; k < points; k++) {
            if(k != which_pt[j]) {
                //
                // Valid point. Include in convexRegion
                //
                region->xPtArray[i] = tmp_region.xPtArray[k];
                region->yPtArray[i] = tmp_region.yPtArray[k];

                //
                // Next valid point
                //
                i++;
            } else {
                //
                // Point to be removed from convexRegion
                //
                j++;
            }
        }

        //
        // Reset the convexRegion to be the newly created convexRegion
        //
        tmp_region.set(region->pointCount,
                       region->xPtArray,
                       region->yPtArray);
    }

    //
    // Second pass for y Coordinates.
    // The region now is the new region if it got modified
    // otherwise process the original one
    //
    
    points = tmp_region.pointCount;
    new_points = points;
    j = 0;
    
    for(i = 0; i < points; i++) {            
        //
        // Get 3 adjacent points
        //
        y1 = tmp_region.yPtArray[i];
        y2 = tmp_region.yPtArray[(i+1) % points];
        y3 = tmp_region.yPtArray[(i+2) % points];

        if(XILI_PIXEL_EQ(y1, y2)) {
            //
            // 2 Adjacent points have the same x coordinate
            //
            if(XILI_PIXEL_EQ(y2, y3)) {
                //
                // 3 Consecutive points have the same x coordinate
                // Keep track of which point to remove
                new_points--;
                which_pt[j] = i + 1;
                j++;
            }                            
        }
    }

    //
    // Only coalesce if any points have been removed
    //
    if(j > 0) {
        //
        // Now remake the convexRegion
        //
    
        region->pointCount = new_points;

        //
        // Start checking against first point to be excluded
        //
        i = 0;
        j = 0;
    
        for(k = 0; k < points; k++) {
            if(k != which_pt[j]) {
                //
                // Valid point. Include in convexRegion
                //
                region->xPtArray[i] = tmp_region.xPtArray[k];
                region->yPtArray[i] = tmp_region.yPtArray[k];
                
                //
                // Next valid point
                //
                i++;
            } else {
                //
                // Point to be removed from convexRegion
                //
                j++;
            }
        }
    }

    //
    // delete allocated stuff
    //
    delete [] which_pt;
    
}

XilStatus
XiliConvexRegion::intersect_line(XiliConvexRegion* region1,
                                 XiliConvexRegion* region2,
                                 XiliConvexRegion* newRegion)
{
    //
    //  region1 is 2 points that form a horizontal or vertical line and
    //  region2 is a rectangle, then we can use the rectangle intersection
    //  algorithm. 
    //
    if((XILI_PIXEL_EQ(region1->lowX, region1->highX) ||
        XILI_PIXEL_EQ(region1->lowY, region1->highY)) && region2->isRect()) {
#ifdef INTERSECT_DEBUG
        fprintf(stderr, "Rect and Line intersection \n");
#endif
        
        //
        // Special case when the convexRegion is a rect
        // and the line is vertical or horizontal.
        // Seen commonly during translate and scale.
        //
        double tmp_x1 = _XILI_MAX(region2->lowX, region1->lowX);
        double tmp_x2 = _XILI_MIN(region2->highX, region1->highX);
        double tmp_y1 = _XILI_MAX(region2->lowY, region1->lowY);
        double tmp_y2 = _XILI_MIN(region2->highY, region1->highY);

#ifdef INTERSECT_DEBUG
        fprintf(stderr, "LINE/RECT INTERSECTION:  %.10f %.10f %.10f %.10f\n",
                tmp_x1, tmp_x2, tmp_y1, tmp_y2);
        fprintf(stderr, "%.10f == %.10f : %d; %.10f == %.10f : %d;\n",
                tmp_x1, tmp_x2, XILI_PIXEL_EQ(tmp_x1, tmp_x2),
                tmp_y1, tmp_y2, XILI_PIXEL_EQ(tmp_y1, tmp_y2));
#endif
                

        //
        //  If it's clipped to an empty rect, then return empty intersection;
        //
        if(! XILI_CHECK_RECT_EMPTY(&tmp_x1, &tmp_y1, tmp_x2, tmp_y2)) {
            newRegion->pointCount = 0;
            return XIL_SUCCESS;
        }

        //
        //  This intersection could produce a point, or a line
        //
        Xil_boolean xeq = XILI_PIXEL_EQ(tmp_x1, tmp_x2);
        Xil_boolean yeq = XILI_PIXEL_EQ(tmp_y1, tmp_y2);
        if(xeq && yeq) {
            //
            // It's a point
            //
            newRegion->pointCount = 1;
            newRegion->xPtArray[0] = tmp_x1;
            newRegion->yPtArray[0] = tmp_y1;
            newRegion->lowX =
                newRegion->highX = tmp_x1;
            newRegion->lowY =
                newRegion->highY = tmp_y1;
        } else if(xeq) {
            //
            // Vertical line
            //
            newRegion->pointCount  = 2;
            newRegion->xPtArray[0] = tmp_x1;
            newRegion->yPtArray[0] = tmp_y1;
            newRegion->xPtArray[1] = tmp_x1;
            newRegion->yPtArray[1] = tmp_y2;
            newRegion->lowX =
                newRegion->highX = tmp_x1;
            newRegion->lowY = tmp_y1;
            newRegion->highY = tmp_y2;
        } else {
            //
            // Horizontal line
            //
            newRegion->pointCount  = 2;
            newRegion->xPtArray[0] = tmp_x1;
            newRegion->yPtArray[0] = tmp_y1;
            newRegion->xPtArray[1] = tmp_x2;
            newRegion->yPtArray[1] = tmp_y1;
            newRegion->lowX = tmp_x1;
            newRegion->highX = tmp_x2;
            newRegion->lowY =
                newRegion->highY = tmp_y1;
        }

#ifdef DUMP_DEBUG
        fprintf(stderr, "Resulting convexRegion :\n");
        newRegion->dump();
#endif
        
        return XIL_SUCCESS;
    }
    
    //
    // Do the intersection for the other cases
    //

    //
    // set up vertex array with region2 points
    //
    int num_cur_vert = region2->pointCount;

    Vertex va[MAX_VERTEX_PTS];
    setupVertexArray(region2, va);

    int verts_to_test = num_cur_vert;

    //
    // Start adding new vertices here
    //
    int new_vertex_index = num_cur_vert;

    Vertex* vptr;
    Vertex* start_clip;
    Vertex* end_clip;
    Vertex  start_v;
    Vertex  end_v;

    vptr = &va[0];

    //
    // Number of passes around the algorithm.
    // In the case of line we'll have to make 2 passes
    //
    unsigned int pass = 0;

    while((pass < region2->pointCount) && (num_cur_vert > 0)) {
        start_clip = NULL;
        verts_to_test = num_cur_vert;
        
#ifdef INTERSECT_DEBUG
        fprintf(stderr, "pass %d\n", pass);
#endif

        //
        // Check first point
        //
        if(point_inside(vptr->x, vptr->y, region1, 0)) {
            //
            // First point is inside
            //

            //
            // Check if each point of region2 is "inside" of region1
            //
            while((point_inside(vptr->next->x,
                                vptr->next->y,
                                region1,
                                0) == TRUE) &&
                  (verts_to_test > 0)) {
                verts_to_test--;
                vptr = vptr->next;
            }

            if(verts_to_test != 0) {
                //
                // There was a point outside
                //
                start_clip = vptr;

#ifdef INTERSECT_DEBUG
                fprintf(stderr, "start_clip @ %.15f, %.15f\n",
                        start_clip->x, start_clip->y);
#endif

                get_intersection(vptr->x,
                                 vptr->y,
                                 vptr->next->x,
                                 vptr->next->y,
                                 region1,
                                 0,
                                 &start_v.x,
                                 &start_v.y);

#ifdef INTERSECT_DEBUG
                fprintf(stderr, "Start Intersection @ %.15f, %.15f\n",
                        start_v.x, start_v.y);
#endif

                //
                // Move to next vertex
                //
                vptr = vptr->next;
            
                //
                // Now second crossover
                //
                while(point_inside(vptr->next->x,
                                   vptr->next->y,
                                   region1,
                                   0) == FALSE){
#ifdef INTERSECT_DEBUG
                    fprintf(stderr, "Clipping %.15f, %.15f\n",
                            vptr->next->x, vptr->next->y);
#endif

                    num_cur_vert--;
                    vptr = vptr->next;
                }

                //
                //  end_clip is the last vertex before crossover
                //
                end_clip = vptr;

#ifdef INTERSECT_DEBUG
                fprintf(stderr, "end_clip @ %.15f, %.15f\n",
                        end_clip->x, end_clip->y);
#endif

                get_intersection(vptr->x,
                                 vptr->y,
                                 vptr->next->x,
                                 vptr->next->y,
                                 region1,
                                 0,
                                 &end_v.x,
                                 &end_v.y);

#ifdef INTERSECT_DEBUG
                fprintf(stderr, "end Intersection @ %.15f, %.15f\n",
                        end_v.x, end_v.y);
                fprintf(stderr, "Replacing %.15f, %.15f with %.15f, %.15f\n",
                        end_clip->x, end_clip->y, end_v.x, end_v.y);
#endif

                
                //
                // Replace end_clip with end_v vertex values
                //
                end_clip->x = end_v.x;
                end_clip->y = end_v.y;

                if((XILI_PIXEL_EQ(end_clip->x, end_clip->next->x)) &&
                   (XILI_PIXEL_EQ(end_clip->y, end_clip->next->y))) {
                    end_clip = end_clip->next;
                    num_cur_vert--;
                }

                if((XILI_PIXEL_EQ(start_clip->x, start_v.x)) &&
                   (XILI_PIXEL_EQ(start_clip->y, start_v.y))) {
                    //
                    // No new vertex
                    //
#ifdef INTERSECT_DEBUG
fprintf(stderr, "Start_clip & start_v identical\n");
#endif              
                    start_clip->next = end_clip;
                    //
                    // point to valid vertex
                    //
                    vptr = start_clip;
                } else {
                    //
                    // Add start_v while maintaining convexRegion
                    // continuity
                    //
#ifdef INTERSECT_DEBUG
fprintf(stderr, "Adding %.15f, %.15f\n", start_v.x, start_v.y);
#endif

                    va[new_vertex_index] = start_v;
                    va[new_vertex_index].next = end_clip;
                    start_clip->next = &va[new_vertex_index];
                    new_vertex_index++;
                    num_cur_vert++;

                    //
                    // Point to valid vertex
                    //
                    vptr = start_clip->next;
                }

            } else {
                //
                // All points are inside of region1
                // This means either of 2 things.
                //      1. region1 is outside of region2
                //      2. region1 is on or along 1 of region2's (remaining
                //         convexRegion's) lines.
                //         This means that if region1 is not on any of
                //         region2's line it's outside and hence no
                //         intersection
                //
#ifdef INTERSECT_DEBUG
                fprintf(stderr, "All points on inside of line\n");
                fprintf(stderr, "Either the line is on or along the");
                fprintf(stderr, "convexRegion or outside of it\n");
#endif

                Xil_boolean pt_on_line = FALSE;

                double x1 = region1->xPtArray[0];
                double y1 = region1->yPtArray[0];
                double x2 = region1->xPtArray[1];
                double y2 = region1->yPtArray[1];
                
                for(int i = 0; i < num_cur_vert; i++) {
                    //
                    // Get the first vertex not on line
                    // Then traverse across the convexRegion
                    // Reason being, traversal after this
                    // guarantees that 2 consecutive vertices
                    // vptr and vptr->next of the region will be
                    // on the (extended) line.
                    //
                    if(point_on_line(x1, y1, x2, y2, vptr->x, vptr->y)
                       == TRUE) {
                        //
                        // Go to next vertex
                        //
                        vptr = vptr->next;
                    } else {
                        //
                        // Got a vertex that's not on line
                        //
                        break;
                    }
                }

                //
                // Now check for vertices on the (extended) line
                //
                for(int j = 0; j < num_cur_vert; j++) {
                    //
                    // Check if vertex on line
                    //
                    if(point_on_line(x1, y1, x2, y2, vptr->x, vptr->y)
                       == TRUE) {

                        pt_on_line = TRUE;
                        
                        double ret_x1, ret_y1, ret_x2, ret_y2;
                        int is_intersect;
                        
                        //
                        // vertex on the (extended) line. Next vertex
                        // should also be on the (extended) line
                        //
                        get_line(x1,
                                 y1,
                                 x2,
                                 y2,
                                 vptr->x,
                                 vptr->y,
                                 vptr->next->x,
                                 vptr->next->y,
                                 &ret_x1,
                                 &ret_y1,
                                 &ret_x2,
                                 &ret_y2,
                                 &is_intersect);

                        if(is_intersect == 1) {
                            //
                            // We did intersect
                            //
                            if((XILI_PIXEL_EQ(ret_x1, ret_x2)) &&
                               (XILI_PIXEL_EQ(ret_y1, ret_y2))) {
                                //
                                // It's a point
                                //
                                num_cur_vert = 1;
                                vptr->x = ret_x1;
                                vptr->y = ret_y1;
                            } else {
                                //
                                // It's a line
                                //
                                num_cur_vert = 2;
                                vptr->x = ret_x1;
                                vptr->y = ret_y1;
                                vptr->next->x = ret_x2;
                                vptr->next->y = ret_y2;
                            }
                        } else {
                            //
                            // Null intersection
                            //
                            num_cur_vert = 0;
                        }
                        
                        //
                        // Get out of loop
                        //
                        break;
                    }

                    vptr = vptr->next;
                }

                if(pt_on_line == FALSE) {
                    //
                    // All vertices not on line or on extended line
                    // So Null intersection
                    //
                    num_cur_vert = 0;
                }

                //
                // break out of the while loop
                // intersection done, we already have the results
                //
                break;
            }

        } else {
            //
            // First point is outside
            //

            //
            // Look for a vertex inside
            //
            while((point_inside(vptr->next->x,
                                vptr->next->y,
                                region1,
                                0) == FALSE) &&
                  (verts_to_test > 0)) {
                //
                // Go to next vertex
                //
                verts_to_test--;
                vptr = vptr->next;
            }

            if(verts_to_test != 0) {
                end_clip = vptr;
                //
                // There was a point inside
                //
#ifdef INTERSECT_DEBUG
                fprintf(stderr, "end_clip @ %.15f, %.15f\n",
                        end_clip->x, end_clip->y);
#endif

                get_intersection(vptr->x,
                                 vptr->y,
                                 vptr->next->x,
                                 vptr->next->y,
                                 region1,
                                 0,
                                 &end_v.x,
                                 &end_v.y);

#ifdef INTERSECT_DEBUG
                fprintf(stderr, "End Intersection @ %.15f, %.15f\n",
                        end_v.x, end_v.y);
#endif

                vptr = vptr->next;

                //
                //  Now look for second crossover
                //
                while(point_inside(vptr->next->x,
                                   vptr->next->y,
                                   region1,
                                   0) == TRUE){
                    verts_to_test--;
                    vptr = vptr->next;
                }

                start_clip = vptr;

#ifdef INTERSECT_DEBUG
                fprintf(stderr, "start_clip @ %.15f, %.15f\n",
                        start_clip->x, start_clip->y);
#endif
                get_intersection(vptr->x,
                                 vptr->y,
                                 vptr->next->x,
                                 vptr->next->y,
                                 region1,
                                 0,
                                 &start_v.x,
                                 &start_v.y);

#ifdef INTERSECT_DEBUG
                fprintf(stderr, "Start Intersection @ %.15f, %.15f\n",
                        start_v.x, start_v.y);
                fprintf(stderr, "Replacing %.15f, %.15f with %.15f, %.15f\n",
                        end_clip->x, end_clip->y, end_v.x, end_v.y);
#endif

                while(vptr->next != end_clip) {
#ifdef INTERSECT_DEBUG
                    fprintf(stderr, "Clipping %.15f, %.15f\n",
                            vptr->next->x, vptr->next->y);
#endif 
                    vptr = vptr->next;
                    num_cur_vert--;
                }

                //
                // Replace end_clip with end_v coordinate values
                //
                end_clip->x = end_v.x;
                end_clip->y = end_v.y;

                if((XILI_PIXEL_EQ(end_clip->x, end_clip->next->x)) &&
                   (XILI_PIXEL_EQ(end_clip->y, end_clip->next->y))) {
                    //
                    // 2 consecutive vertices are ~1000 same
                    //
                    end_clip = end_clip->next;
                    num_cur_vert--;
                }

                if((XILI_PIXEL_EQ(start_clip->x, start_v.x)) &&
                   (XILI_PIXEL_EQ(start_clip->y, start_v.y))) {
                    //
                    // No new vertex
                    //
#ifdef INTERSECT_DEBUG
                    fprintf(stderr, "Start_clip & start_v identical\n");
#endif
                    start_clip->next = end_clip;
                    //
                    // point to valid vertex
                    //
                    vptr = start_clip;
                } else {
                    //
                    // Add start_v while maintaining convexRegion
                    // continuity
                    //
#ifdef INTERSECT_DEBUG
                    fprintf(stderr, "Adding %.15f, %.15f\n",
                            start_v.x, start_v.y);
#endif 

                    va[new_vertex_index] = start_v;
                    va[new_vertex_index].next = end_clip;
                    start_clip->next = &va[new_vertex_index];
                    new_vertex_index++;
                    num_cur_vert++;

                    //
                    // Point to valid vertex
                    //
                    vptr = start_clip->next;
                }
            } else {
                //
                // Null intersection. All points outside
                //
                num_cur_vert = 0;
            }        
        }

#ifdef INTERSECT_DEBUG
        fprintf(stderr, "\nCurrent polygon %d vertices:\n", num_cur_vert);
        Vertex* prt_ptr = vptr;
        for(int i=0; i<num_cur_vert; i++) {
            fprintf(stderr, "%.15f, %.15f\n",prt_ptr->x, prt_ptr->y);
            prt_ptr = prt_ptr->next;
        }

        fprintf(stderr, "\n");
#endif

        pass++;
    }

    if(num_cur_vert > 2) {
        XIL_ERROR(NULL, XIL_ERROR_INTERNAL, "di-431", TRUE);
        return XIL_FAILURE;
    } else if(num_cur_vert == 2) {
        //
        // Intersection produced a line
        //
        newRegion->pointCount = num_cur_vert;
        newRegion->lowX = newRegion->highX = vptr->x;
        newRegion->lowY = newRegion->highY = vptr->y;

        for(unsigned int i=0; i<newRegion->pointCount; i++) {
            
            newRegion->xPtArray[i] = vptr->x;
            newRegion->yPtArray[i] = vptr->y;

            newRegion->lowX = _XILI_MIN(newRegion->lowX, vptr->x);
            newRegion->lowY = _XILI_MIN(newRegion->lowY, vptr->y);
            newRegion->highX = _XILI_MAX(newRegion->highX, vptr->x);
            newRegion->highY = _XILI_MAX(newRegion->highY, vptr->y);
            vptr = vptr->next;
        }
        
#ifdef DUMP_DEBUG
        fprintf(stderr, "Resulting convexRegion :\n");
        newRegion->dump();
#endif

        return XIL_SUCCESS;
        
    } else if(num_cur_vert == 1) {
        //
        // Intersection produced 1 point
        //
        newRegion->pointCount = 1;
        newRegion->xPtArray[0] = vptr->x;
        newRegion->yPtArray[1] = vptr->y;

#ifdef DUMP_DEBUG
        fprintf(stderr, "Resulting convexRegion :\n");
        newRegion->dump();
#endif

        return XIL_SUCCESS;
    } else {
        //
        // Null intersection
        //
        newRegion->pointCount = 0;

#ifdef DUMP_DEBUG
        fprintf(stderr, "Resulting ConvexRegion : 0 vertices\n");
#endif
       
        return XIL_SUCCESS;
    }
}

//
// In this routine, calculate the intersected line between
// 2 line segments. Note that this routine is not generic
// but a special case within the context of our convexRegion
// mechanism (intersection, point being "inside" etc). Refer
// point_on_line routine in this file.
//
void
XiliConvexRegion::get_line(double  lx1,
                           double  ly1,
                           double  lx2,
                           double  ly2,
                           double  vx,
                           double  vy,
                           double  vnx,
                           double  vny,
                           double* ret_x1,
                           double* ret_y1,
                           double* ret_x2,
                           double* ret_y2,
                           int*    is_intersect)
{
    //
    // Only the following scenarios can occur
    //    1. Both horizontal lines
    //    2. Both vertical lines
    //    3. Both diagonal (or anti-diagonal) lines
    //
    double tmp_x1, tmp_y1, tmp_x2, tmp_y2;

    //
    //  Case 1 : Both Horizontal lines
    //
    if((XILI_PIXEL_EQ(ly1, vy)) &&
       (XILI_PIXEL_EQ(ly2, vy)) &&
       (XILI_PIXEL_EQ(ly1, vny)) &&
       (XILI_PIXEL_EQ(ly2, vny))) {
        *ret_x1 = _XILI_MAX(_XILI_MIN(lx1, lx2), _XILI_MIN(vx, vnx));
        *ret_y1 = vy;

        *ret_x2 = _XILI_MIN(_XILI_MAX(lx1, lx2), _XILI_MAX(vx, vnx));
        *ret_y2 = vy;

        *is_intersect = XILI_CHECK_RECT_EMPTY(ret_x1, ret_y1,
                                              *ret_x2, *ret_y2);
        return;
    }

    //
    // Case 2 : Both Vertical lines
    //
    if((XILI_PIXEL_EQ(lx1, vx)) &&
       (XILI_PIXEL_EQ(lx2, vx)) &&
       (XILI_PIXEL_EQ(lx1, vnx)) &&
       (XILI_PIXEL_EQ(lx2, vnx))) {
        *ret_x1 = vx;
        *ret_y1 = _XILI_MAX(_XILI_MIN(ly1, ly2), _XILI_MIN(vy, vny));

        *ret_x2 = vx;
        *ret_y2 = _XILI_MIN(_XILI_MAX(ly1, ly2), _XILI_MAX(vy, vny));

        *is_intersect = XILI_CHECK_RECT_EMPTY(ret_x1, ret_y1,
                                              *ret_x2, *ret_y2);
        return;
    }

    //
    // Other (diagonal or anti-diagonal lines)
    //
    // We can use the approach below only because the line
    // lies along 1 of the line segments of the convexRegion.
    // The convexRegion results from initial intersection between
    // region1 (a line) and region2 (a convexRegion of > 2 points)
    // This approach cannot be used for arbitrary line intersections.
    //

    //
    // There is a intersection
    //
    *is_intersect = 1;
    
    //
    // Order the vertex pairs so that they are directionally
    // ascending along y direction
    //    
    if(ly1 > ly2) {
        tmp_x1 = lx2;
        tmp_y1 = ly2;
        tmp_x2 = lx1;
        tmp_y2 = ly1;
    } else {
        tmp_x1 = lx1;
        tmp_y1 = ly1;
        tmp_x2 = lx2;
        tmp_y2 = ly2;
    }

    double tmp_vx1, tmp_vy1, tmp_vx2, tmp_vy2;
    
    if(vny < vy) {
        tmp_vx1 = vnx;
        tmp_vy1 = vny;
        tmp_vx2 = vx;
        tmp_vy2 = vy;
    } else {
        tmp_vx1 = vx;
        tmp_vy1 = vy;
        tmp_vx2 = vnx;
        tmp_vy2 = vny;
    }

    //
    // Now determine the (resulting) line
    //
    if(tmp_vy1 > tmp_y1) {
        *ret_y1 = tmp_vy1;
        *ret_x1 = tmp_vx1;
    } else {
        *ret_y1 = tmp_y1;
        *ret_x1 = tmp_x1;
    }

    if(tmp_vy2 < tmp_y2) {
        *ret_y2 = tmp_vy2;
        *ret_x2 = tmp_vx2;
    } else {
        *ret_y2 = tmp_y2;
        *ret_x2 = tmp_x2;
    }

    return;
}

// WARNING WARNING WARNING
// If you modify the code below you must modify ../lib/XiliConvexRegion.cc
#if defined(HPUX)
//
//  Create from point list, points need to be clockwise ordered
//
XiliConvexRegion::XiliConvexRegion(unsigned int  npts,
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
#endif
    // WARNING WARNING WARNING
	// If you modify the code below you must modify ../lib/XiliConvexRegion.cc
#if defined(HPUX)
XilStatus         
XiliConvexRegion::clip(XiliRect* rect)
{
	if(rect->getType() == XILI_RECT_TYPE_DOUBLE) {
		XiliRectDbl* r = (XiliRectDbl*)rect;
		return clip(r->getX1(), r->getY1(), r->getX2(), r->getY2());
	} else {
		XiliRectInt* r = (XiliRectInt*)rect;
		return clip(r->getX1(), r->getY1(), r->getX2(), r->getY2());
	}
}
#endif
