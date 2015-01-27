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
//  File:	XiliRect.hh
//  Project:	XIL
//  Revision:	1.27
//  Last Mod:	10:20:58, 03/10/00
//
//  Description:
//	
//	A class representing four corners, either integer or double.
//	
//  MT-level:  UNSAFE
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XiliRect.hh	1.27\t00/03/10  "

#ifndef _XILI_RECT_HH
#define _XILI_RECT_HH

//
//  System includes
//
#include <stdio.h>
#include <math.h>

//
//  XIL Includes
//
#include "_XilDefines.h"
#include "_XilClasses.hh"

enum XiliRectType {
    XILI_RECT_TYPE_DOUBLE,
    XILI_RECT_TYPE_INT
};

//
//  The XiliRect class is used to represent a rectangular region of 
//  an image. The XiliRect has two sub-classes XiliRectInt and XiliRectDbl.
//
class XiliRect {
public:
    //
    //  Return the actual storage type for this rect.
    //
    virtual XiliRectType   getType()=0;

    //
    //  Access as Integer Values
    //
    virtual void           set(int x1,
                               int y1,
                               int x2,
                               int y2)=0;
    
    virtual void           get(int* x1,
                               int* y1,
                               int* x2,
                               int* y2)=0;

    //
    //  Access as Double Values
    //
    virtual void           set(double x1,
                               double y1,
                               double x2,
                               double y2)=0;
    
    virtual void           get(double* x1,
                               double* y1,
                               double* x2,
                               double* y2)=0;

    //
    //  Access as other XiliRect*
    //
    virtual void           set(XiliRect* other)=0;
    
    virtual void           get(XiliRect* other)=0;
    
    //
    //  Access as XilBox*
    //
    virtual void           set(XilBox* box)=0;
    
    virtual void           get(XilBox* box)=0;
    
    //
    //  Test for Intersection
    //
    virtual Xil_boolean    intersects(XiliRect* other)=0;

    //
    //  Clip this Rect against the given Rect.  Retuns whether the resultant
    //  Rect was clipped to nothing (FALSE).
    //
    virtual Xil_boolean    clip(XiliRect* other)=0;

    //
    //  Translate (offset) this rect by the given x and y.
    //
    virtual void           translate(double x,
                                     double y)=0;

    //
    //  Empty the rect so it represents no area...
    //
    virtual void           empty()=0;

    //
    //  Test whether this rect is empty...
    //
    virtual Xil_boolean    isEmpty()=0;

    //
    //  Dump the contents of the rect to stderr...
    //
    virtual void           dump()=0;

    //
    //  Next Pointer Access
    //
    void                   setNext(XiliRect* newnext)
    {
        next = newnext;
    }

    XiliRect*              getNext()
    {
        return next;
    }

    //
    //  Construction.
    //
    //  constructNew will create a new XiliRect of the same type as this one
    //  and set all of the corners to 0 whereas createCopy() will produce a
    //  copy of this rect.
    //
    virtual XiliRect*      constructNew()=0;
    virtual XiliRect*      createCopy()=0;

    //
    //  Destruction.
    //
    virtual void           destroy()=0;

protected:
    //
    //  Constructor needed in order to init next pointer to NULL
    //
                           XiliRect() : next(NULL)
    {
    }

    //
    //  Destructor
    //
                           ~XiliRect()
    {
    }

private:
    XiliRect* next;
};

//------------------------------------------------------------------------
//
//  Class:	XiliRectInt
//
//  Description:
//	Integer based implementation of XiliRect.
//
//  The points represent the pixels of a rectangular
//  area in an image.
//
//------------------------------------------------------------------------
class XiliRectInt : public XiliRect {
public:
    //
    //  Return the actual storage type for this rect.
    //
    XiliRectType   getType();

    //
    //  Access as Integer Values
    //
    void           set(int x1,
                       int y1,
                       int x2,
                       int y2);
    
    void           get(int* x1,
                       int* y1,
                       int* x2,
                       int* y2);

    //
    //  Access as Double Values
    //
    void           set(double x1,
                       double y1,
                       double x2,
                       double y2);

    void           get(double* x1,
                       double* y1,
                       double* x2,
                       double* y2);

    //
    //  Access as other XiliRects
    //
    void           set(XiliRect* other);

    void           get(XiliRect* other);

    //
    //  Access as XilBox*
    //
    void           set(XilBox* box);
    
    void           get(XilBox* box);
    
    //
    //  Test for Intersection
    //
    Xil_boolean    intersects(XiliRect* other);

    //
    //  Clip this Rect against the given Rect.  Retuns whether the resultant
    //  Rect was clipped to nothing (FALSE).
    //
    Xil_boolean    clip(XiliRect* other);

    //
    //  Translate (offset) this rect by the given x and y.
    //
    void           translate(double x,
                             double y);

    //
    //  Empty the rect so it represents no area...
    //
    void           empty();

    //
    //  Test whether this rect is empty...
    //
    Xil_boolean    isEmpty();

    //
    //  Dump the contents of the rect to stderr...
    //
    void           dump()
    {
        fprintf(stderr, "(%d, %d, %d, %d)\n", x1, y1, x2, y2);
    }


    //
    //  Speedy access to the individual corners of the rect
    //
    int            getX1()
    {
        return x1;
    }

    int            getX2()
    {
        return x2;
    }

    int            getY1()
    {
        return y1;
    }

    int            getY2()
    {
        return y2;
    }

    //
    //  Setting individual corners
    //
    void            setX1(int new_x1)
    {
        x1 = new_x1;
    }

    void            setX2(int new_x2)
    {
        x2 = new_x2;
    }

    void            setY1(int new_y1)
    {
        y1 = new_y1;
    }

    void            setY2(int new_y2)
    {
        y2 = new_y2;
    }

    //
    //  Test to see if one rect is smaller than another rect.
    //
    int                  operator < (XiliRectInt& rval)
    {
        return (((x2 - x1) < (rval.x2 - rval.x1)) ||
                ((y2 - y1) < (rval.y2 - rval.y1))) ? TRUE : FALSE;
    }
    
    //
    //  Construction.
    //
    //  constructNew will create a new XiliRect of the same type as this one
    //  and set all of the corners to 0 whereas createCopy() will produce a
    //  copy of this rect.
    //
    XiliRect*      constructNew();
    XiliRect*      createCopy();

                   XiliRectInt()
    {
        //
        //  Initialize to an invalid/empty rect.
        //
        x1 =  0;
        y1 =  0;
        x2 = -1;
        y2 = -1;
    }

                   XiliRectInt(int x1_n,
                               int y1_n,
                               int x2_n,
                               int y2_n)
    {
        x1 = x1_n;
        y1 = y1_n;
        x2 = x2_n;
        y2 = y2_n;
    }

                   XiliRectInt(XiliRect* other)
    {
        int other_x1;
        int other_y1;
        int other_x2;
        int other_y2;
        other->get(&other_x1, &other_y1, &other_x2, &other_y2);

        x1 = other_x1;
        y1 = other_y1;
        x2 = other_x2;
        y2 = other_y2;
    }

                   XiliRectInt(XiliRect& other)
    {
        int other_x1;
        int other_y1;
        int other_x2;
        int other_y2;
        other.get(&other_x1, &other_y1, &other_x2, &other_y2);

        x1 = other_x1;
        y1 = other_y1;
        x2 = other_x2;
        y2 = other_y2;
    }
        

    //
    //  For constructing a new rect from a box -- always grabs the front box,
    //  not he storage location box. 
    //
    //
    //  Since XilBoxPrivate.hh includes XiliRect.hh, we cannot put this in the
    //  include file for inlining. 
    //
                   XiliRectInt(XilBox* box);


    //
    //  Destruction.
    //
    void           destroy();

    _XIL_NEW_DELETE_OVERLOAD_PUBLIC(XiliRectInt)

#if defined(GCC) || defined(_WINDOWS) || defined(HPUX)
    //
    //  For placating explicit template instantiation.
    //
    int operator == (XiliRectInt&) {
        return TRUE;
    }
#endif
private:
    int            x1;
    int            y1;
    int            x2;
    int            y2;

    _XIL_NEW_DELETE_OVERLOAD_PRIVATE(XiliRectInt)
};

//------------------------------------------------------------------------
//
//  Class:	XiliRectDbl
//
//  Description:
//	Double based implementation of XiliRect.
//      The doubles represent the pixel extent corners of 
//      a rectangular area in an image.
//
//	
//------------------------------------------------------------------------
class XiliRectDbl : public XiliRect {
public:
    //
    //  Return the actual storage type for this rect.
    //
    XiliRectType   getType();

    //
    //  Access as Integer Values
    //
    void           set(int x1,
                       int y1,
                       int x2,
                       int y2);
    
    void           get(int* x1,
                       int* y1,
                       int* x2,
                       int* y2);

    //
    //  Access as Double Values
    //
    void           set(double x1,
                       double y1,
                       double x2,
                       double y2);

    void           get(double* x1,
                       double* y1,
                       double* x2,
                       double* y2);


    //
    //  Access as other XiliRect
    //
    void           set(XiliRect* other);

    void           get(XiliRect* other);

    //
    //  Access as XilBox*
    //
    void           set(XilBox* box);
    
    void           get(XilBox* box);
    
    //
    //  Test for Intersection
    //
    Xil_boolean    intersects(XiliRect* other);

    //
    //  Clip this Rect against the given Rect.  Retuns whether the resultant
    //  Rect was clipped to nothing (FALSE).
    //
    Xil_boolean    clip(XiliRect* other);

    //
    //  Translate (offset) this rect by the given x and y.
    //
    void           translate(double x,
                             double y);

    //
    //  Empty the rect so it represents no area...
    //
    void           empty();

    //
    //  Test whether this rect is empty...
    //
    Xil_boolean    isEmpty();

    //
    //  Dump the contents of the rect to stderr...
    //
    void           dump()
    {
        fprintf(stderr, "(%f, %f, %f, %f)\n", x1, y1, x2, y2);
    }

    //
    //  Speedy access to the individual corners of the rect
    //
    double         getX1()
    {
        return x1;
    }

    double         getX2()
    {
        return x2;
    }

    double         getY1()
    {
        return y1;
    }

    double         getY2()
    {
        return y2;
    }

    //
    //  Setting individual corners
    //
    void           setX1(double new_x1)
    {
        x1 = new_x1;
    }

    void           setX2(double new_x2)
    {
        x2 = new_x2;
    }

    void           setY1(double new_y1)
    {
        y1 = new_y1;
    }

    void           setY2(double new_y2)
    {
        y2 = new_y2;
    }

    //
    //  Construction.
    //
    //  constructNew will create a new XiliRect of the same type as this one
    //  and set all of the corners to 0 whereas createCopy() will produce a
    //  copy of this rect.
    //
    XiliRect*      constructNew();
    XiliRect*      createCopy();

                   XiliRectDbl()
    {
        //
        //  Initialize to an invalid/empty rect.
        //
        x1 =  0.0F;
        y1 =  0.0F;
        x2 = -1.0F;
        y2 = -1.0F;
    }

                   XiliRectDbl(double x1_n,
                               double y1_n,
                               double x2_n,
                               double y2_n)
    {
        x1 = x1_n;
        y1 = y1_n;
        x2 = x2_n;
        y2 = y2_n;
    }

                   XiliRectDbl(XiliRect* other)
    {
        double other_x1;
        double other_y1;
        double other_x2;
        double other_y2;
        other->get(&other_x1, &other_y1, &other_x2, &other_y2);

        x1 = other_x1;
        y1 = other_y1;
        x2 = other_x2;
        y2 = other_y2;
    }

                   XiliRectDbl(XiliRect& other)
    {
        double other_x1;
        double other_y1;
        double other_x2;
        double other_y2;
        other.get(&other_x1, &other_y1, &other_x2, &other_y2);

        x1 = other_x1;
        y1 = other_y1;
        x2 = other_x2;
        y2 = other_y2;
    }

    //
    //  Destruction.
    //
    void           destroy();

    _XIL_NEW_DELETE_OVERLOAD_PUBLIC(XiliRectDbl)

private:
    double         x1;
    double         y1;
    double         x2;
    double         y2;

    _XIL_NEW_DELETE_OVERLOAD_PRIVATE(XiliRectDbl)
};


#endif // _XILI_RECT_H





