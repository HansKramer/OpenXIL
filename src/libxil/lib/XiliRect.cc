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
//  File:	XiliRect.cc
//  Project:	XIL
//  Revision:	1.29
//  Last Mod:	10:08:30, 03/10/00
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
#pragma ident	"@(#)XiliRect.cc	1.29\t00/03/10  "

#include "_XilDefines.h"
#include "_XilMutex.hh"
#include "XiliUtils.hh"
#include "XiliRect.hh"
#include "_XilBox.hh"

//------------------------------------------------------------------------
//
//  Class:	XiliRectInt
//
//  Description:
//	Integer based implementation of XiliRect.
//      The integers represent the pixel values of a rectangular
//      region of an image.
//	
//------------------------------------------------------------------------

//
//  Expect that we may need a lot of rects at any one time so we create
//  64 at a time to a maximum of 8192.
//
_XIL_NEW_DELETE_OVERLOAD_CC_FILE(XiliRectInt, 8192, 64)

#if 0
XiliRectInt*       XiliRectInt::freeList= NULL;                              
XilMutex         XiliRectInt::freeListMutex;                               
unsigned int     XiliRectInt::freeListCount = 0;                           

void*                                                                    
XiliRectInt::operator new(size_t)                                          
{                                                                        
    XiliRectInt* entry;                                                    
                                                                         
    freeListMutex.lock();                                                
                                                                         
    if(freeListCount > 8192) {                                
        entry = ::new XiliRectInt;                                         
        entry->nextFree = (XiliRectInt*)-1;                                

        fprintf(stderr, "0: setting %p = %p\n",
                entry, -1);
    } else {                                                             
        entry = freeList;                                                
                                                                         
        if(entry == NULL) {                                              
            XiliRectInt* tmp = ::new XiliRectInt[64];                 

            fprintf(stderr, "allocated 64 @ %p\n", tmp);

            for(entry=freeList=&tmp[64-1]; tmp<entry; entry--) {
                fprintf(stderr, "1: setting %p = %p -- %p\n",
                        entry, entry-1, &entry->nextFree);

                entry->nextFree = entry-1;                               
            }                                                            
            fprintf(stderr, "2: setting %p = %p\n",
                    entry, NULL);
            entry->nextFree = NULL;                                      
                                                                         
            entry = freeList;                                            
                                                                         
            freeListCount += 64;                                  
        }                                                                
                                                                         
        freeList = entry->nextFree;                                      
    }                                                                    
                                                                         
    freeListMutex.unlock();                                              
                                                                         
    return entry;                                                        
}                                                                        
                                                                         
void                                                                     
XiliRectInt::operator delete(void* ptr, size_t)                            
{                                                                        
    freeListMutex.lock();                                                

    fprintf(stderr, "checking %p ((XiliRectInt*)ptr)->nextFree= %p\n",
            ptr, ((XiliRectInt*)ptr)->nextFree);
    if(((XiliRectInt*)ptr)->nextFree == (XiliRectInt*)-1) {                  
        ::delete ptr;                                                    
    } else {                                                             
        fprintf(stderr, "3: setting %p = %p\n",
                ((XiliRectInt*)ptr), freeList);
        ((XiliRectInt*)ptr)->nextFree   = freeList;                        
        freeList                      = (XiliRectInt*)ptr;                 
    }                                                                    
                                                                         
    freeListMutex.unlock();                                               
}
#endif


XiliRectType
XiliRectInt::getType()
{
    return XILI_RECT_TYPE_INT;
}

//
//  Access as Integer Values
//
void
XiliRectInt::set(int x1_n,
                 int y1_n,
                 int x2_n,
                 int y2_n)
{
    x1 = x1_n;
    y1 = y1_n;
    x2 = x2_n;
    y2 = y2_n;
}
    
void
XiliRectInt::get(int* x1_r,
                 int* y1_r,
                 int* x2_r,
                 int* y2_r)
{
    *x1_r = x1;
    *y1_r = y1;
    *x2_r = x2;
    *y2_r = y2;
}

//
//  Access as Double Values
//
void
XiliRectInt::set(double x1_n,
                 double y1_n,
                 double x2_n,
                 double y2_n)
{
    //
    //  For converting a double precision rect into an integer representation,
    //  we can't just round the corners because it will increase the size of
    //  the rect.  So, we ceil the upper corner and floor the lower
    //  corner from the clamped width and height.
    //
    x1 = (int) ceil(x1_n - XILI_PIXEL_ADJACENCY);
    y1 = (int) ceil(y1_n - XILI_PIXEL_ADJACENCY);
    x2 = (int) floor(x2_n + XILI_PIXEL_ADJACENCY);
    y2 = (int) floor(y2_n + XILI_PIXEL_ADJACENCY);
}

void
XiliRectInt::get(double* x1_r,
                 double* y1_r,
                 double* x2_r,
                 double* y2_r)
{
    *x1_r = (double)x1;
    *y1_r = (double)y1;
    *x2_r = (double)x2;
    *y2_r = (double)y2;
}

//
//  Access as other XiliRect*
//
void
XiliRectInt::set(XiliRect* other)
{
    other->get(&x1,&y1,&x2,&y2);
}
    
void
XiliRectInt::get(XiliRect* other)
{
    other->set(x1,y1,x2,y2);
}

//
//  Access as XilBox
//
void
XiliRectInt::set(XilBox* box)
{
    box->getAsCorners(&x1, &y1, &x2, &y2);
}

void
XiliRectInt::get(XilBox* box)
{
    box->setAsCorners(x1, y1, x2, y2);
}

//
//  Test for Intersection
//
Xil_boolean
XiliRectInt::intersects(XiliRect* other)
{
    int other_x1;
    int other_y1;
    int other_x2;
    int other_y2;
    other->get(&other_x1, &other_y1, &other_x2, &other_y2);

    int tmp_x1 = _XILI_MAX(x1, other_x1);
    int tmp_x2 = _XILI_MIN(x2, other_x2);
    int tmp_y1 = _XILI_MAX(y1, other_y1);
    int tmp_y2 = _XILI_MIN(y2, other_y2);

    //
    //  If it's clipped to an empty rect, then return FALSE.
    //
    if((tmp_x2 < tmp_x1) || (tmp_y2 < tmp_y1)) {
	return FALSE;
    }

    return TRUE;
}

//
//  Clip against other rect...
//
Xil_boolean
XiliRectInt::clip(XiliRect* other)
{
    int other_x1;
    int other_y1;
    int other_x2;
    int other_y2;
    other->get(&other_x1, &other_y1, &other_x2, &other_y2);

    x1 = _XILI_MAX(x1, other_x1);
    x2 = _XILI_MIN(x2, other_x2);
    y1 = _XILI_MAX(y1, other_y1);
    y2 = _XILI_MIN(y2, other_y2);

    //
    //  If it's clipped to an empty rect, then return FALSE.
    //
    if((x2 < x1) || (y2 < y1)) {
	return FALSE;
    }

    return TRUE;
}

//
//  Translate (offset) this rect by the given x and y.
//
void
XiliRectInt::translate(double x,
                       double y)
{
    int x_trans = _XILI_ROUND(x);
    int y_trans = _XILI_ROUND(y);

    x1 += x_trans;
    y1 += y_trans;
    x2 += x_trans;
    y2 += y_trans;
}

//
//  Empty the rect so it represents no area...
//
void
XiliRectInt::empty()
{
    x1 =  0;
    y1 =  0;
    x2 = -1;
    y2 = -1;
}

//
//  Test whether its an empty rect.
//
Xil_boolean
XiliRectInt::isEmpty()
{
    if((x2 < x1) || (y2 < y1)) {
	return TRUE;
    }

    return FALSE;
}

XiliRect*
XiliRectInt::constructNew()
{
    return new XiliRectInt();
}

XiliRect*
XiliRectInt::createCopy()
{
    return new XiliRectInt(this);
}

//
//  For constructing a new rect from a box -- always grabs the front box,
//  not he storage location box.
//
//  Since XilBoxPrivate.hh includes XiliRect.hh, we cannot put this in the
//  include file for inlining. 
//
XiliRectInt::XiliRectInt(XilBox* box)
{
    unsigned int xsize;
    unsigned int ysize;
    box->getAsRect(&x1, &y1, &xsize, &ysize);

    x2 = x1 + xsize - 1;
    y2 = y1 + ysize - 1;
}

//
//  Destruction.
//
void
XiliRectInt::destroy()
{
    delete this;
}

//------------------------------------------------------------------------
//
//  Class:	XiliRectDbl
//
//  Description:
//	Double based implementation of XiliRect.
//	
//------------------------------------------------------------------------

//
//  Expect that we may need a lot of rects at any one time so we create
//  64 at a time to a maximum of 8192.
//
_XIL_NEW_DELETE_OVERLOAD_CC_FILE(XiliRectDbl, 8192, 64)

#if 0
XiliRectDbl*       XiliRectDbl::freeList= NULL;                              
XilMutex         XiliRectDbl::freeListMutex;                               
unsigned int     XiliRectDbl::freeListCount = 0;                           
                                                                         
void*                                                                    
XiliRectDbl::operator new(size_t)                                          
{                                                                        
    XiliRectDbl* entry;                                                    
                                                                         
    freeListMutex.lock();                                                
                                                                         
    if(freeListCount > 8192) {                                
        entry = ::new XiliRectDbl;                                         
        entry->nextFree = (XiliRectDbl*)-1;                                
    } else {                                                             
        entry = freeList;                                                
                                                                         
        if(entry == NULL) {                                              
            XiliRectDbl* tmp = ::new XiliRectDbl[64];                 
                                                                         
            for(entry=freeList=&tmp[64-1]; tmp<entry; entry--) {  
                entry->nextFree = entry-1;                               
            }                                                            
            entry->nextFree = NULL;                                      
                                                                         
            entry = freeList;                                            
                                                                         
            freeListCount += 64;                                  
        }                                                                
                                                                         
        freeList = entry->nextFree;                                      
    }                                                                    
                                                                         
    freeListMutex.unlock();                                              
                                                                         
    return entry;                                                        
}                                                                        
                                                                         
void                                                                     
XiliRectDbl::operator delete(void* ptr, size_t)                            
{                                                                        
    freeListMutex.lock();                                                
                                                                         
    if(((XiliRectDbl*)ptr)->nextFree == (XiliRectDbl*)-1) {                  
        ::delete ptr;                                                    
    } else {                                                             
        ((XiliRectDbl*)ptr)->nextFree   = freeList;                        
        freeList                      = (XiliRectDbl*)ptr;                 
    }                                                                    
                                                                         
    freeListMutex.unlock();                                               
}
#endif

XiliRectType
XiliRectDbl::getType()
{
    return XILI_RECT_TYPE_DOUBLE;
}

//
//  Access as Integer Values
//
void
XiliRectDbl::set(int x1_n,
                 int y1_n,
                 int x2_n,
                 int y2_n)
{
    x1 = (double)x1_n;
    y1 = (double)y1_n;
    x2 = (double)x2_n;
    y2 = (double)y2_n;
}
    
void
XiliRectDbl::get(int* x1_r,
                 int* y1_r,
                 int* x2_r,
                 int* y2_r)
{
    //
    //  For converting a double precision rect into an integer representation,
    //  we can't just round the corners because it will increase the size of
    //  the rect.  So, we ceil the upper corner and floor the lower
    //  corner from the clamped width and height.
    //
    *x1_r = (int) ceil(x1 - XILI_PIXEL_ADJACENCY);
    *y1_r = (int) ceil(y1 - XILI_PIXEL_ADJACENCY);
    *x2_r = (int) floor(x2 + XILI_PIXEL_ADJACENCY);
    *y2_r = (int) floor(y2 + XILI_PIXEL_ADJACENCY);
}

//
//  Access as Double Values
//
void
XiliRectDbl::set(double x1_n,
                 double y1_n,
                 double x2_n,
                 double y2_n)
{
    x1 = x1_n;
    y1 = y1_n;
    x2 = x2_n;
    y2 = y2_n;
}

void
XiliRectDbl::get(double* x1_r,
                 double* y1_r,
                 double* x2_r,
                 double* y2_r)
{
    *x1_r = x1;
    *y1_r = y1;
    *x2_r = x2;
    *y2_r = y2;
}

//
//  Access as other XiliRect
//
void
XiliRectDbl::set(XiliRect* other)
{
    other->get(&x1,&y1,&x2,&y2);
}

void
XiliRectDbl::get(XiliRect* other)
{
    other->set(x1,y1,x2,y2);
}

//
//  Access as XilBox
//
void
XiliRectDbl::set(XilBox* box)
{
    int bx1;
    int by1;
    int bx2;
    int by2;
    box->getAsCorners(&bx1, &by1, &bx2, &by2);

    x1 = bx1;
    y1 = by1;
    x2 = bx2;
    y2 = by2;
}

void
XiliRectDbl::get(XilBox* box)
{
    //
    //  For converting a double precision rect into a box representation, we
    //  just grab the pixels that fall inside the rect (within pixel adjacency).
    //
    int bx1 = (int) ceil(x1 - XILI_PIXEL_ADJACENCY);
    int by1 = (int) ceil(y1 - XILI_PIXEL_ADJACENCY);
    int bx2 = (int) floor(x2 + XILI_PIXEL_ADJACENCY);
    int by2 = (int) floor(y2 + XILI_PIXEL_ADJACENCY);

    box->setAsCorners(bx1, by1, bx2, by2);
}

//
//  Test for Intersection
//
Xil_boolean
XiliRectDbl::intersects(XiliRect* other)
{
    double other_x1;
    double other_y1;
    double other_x2;
    double other_y2;
    other->get(&other_x1, &other_y1, &other_x2, &other_y2);

    double tmp_x1 = _XILI_MAX(x1, other_x1);
    double tmp_x2 = _XILI_MIN(x2, other_x2);
    double tmp_y1 = _XILI_MAX(y1, other_y1);
    double tmp_y2 = _XILI_MIN(y2, other_y2);

    //
    //  If it's clipped to an empty rect (within XILI_PIXEL_ADJACENCY), then
    //  return FALSE. 
    //
    return XILI_CHECK_RECT_EMPTY(&tmp_x1, &tmp_y1, tmp_x2, tmp_y2);
}

//
//  Clip against other rect...
//
Xil_boolean
XiliRectDbl::clip(XiliRect* other)
{
    double other_x1;
    double other_y1;
    double other_x2;
    double other_y2;
    other->get(&other_x1, &other_y1, &other_x2, &other_y2);

    x1 = _XILI_MAX(x1, other_x1);
    x2 = _XILI_MIN(x2, other_x2);
    y1 = _XILI_MAX(y1, other_y1);
    y2 = _XILI_MIN(y2, other_y2);

    //
    //  If it's clipped to an empty rect (within XILI_PIXEL_ADJACENCY), then
    //  return FALSE. 
    //
    return XILI_CHECK_RECT_EMPTY(&x1, &y1, x2, y2);
}

//
//  Translate (offset) this rect by the given x and y.
//
void
XiliRectDbl::translate(double x,
                       double y)
{
    x1 += x;
    y1 += y;
    x2 += x;
    y2 += y;
}

//
//  Empty the rect so it represents no area...
//
void
XiliRectDbl::empty()
{
    x1 =  0.0;
    y1 =  0.0;
    x2 = -1.0;
    y2 = -1.0;
}

//
//  Test whether its an empty rect.
//
Xil_boolean
XiliRectDbl::isEmpty()
{
    if((x2 < x1) || (y2 < y1)) {
	return TRUE;
    }

    return FALSE;
}

XiliRect*
XiliRectDbl::constructNew()
{
    return new XiliRectDbl();
}

XiliRect*
XiliRectDbl::createCopy()
{
    return new XiliRectDbl(this);
}

//
//  Destruction.
//
void
XiliRectDbl::destroy()
{
    delete this;
}

