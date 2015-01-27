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
//  File:	XilRoi.cc
//  Project:	XIL
//  Revision:	1.66
//  Last Mod:	10:08:14, 03/10/00
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
//  MT-level:  UNsafe
//	
//------------------------------------------------------------------------
//	COPYRIGHT
 //------------------------------------------------------------------------
#pragma ident	"@(#)XilRoi.cc	1.66\t00/03/10  "

#ifdef _WINDOWS
//
// C4355 : 'this' : used in base member initializer list
#pragma warning	( disable : 4355 )
#endif

//
// XIL includes
//
#include "_XilDefines.h"
#include "_XilSystemState.hh"
#include "_XilRoi.hh"
#include "_XilBox.hh"

//
// Private XIL includes
//
#include "XiliUtils.hh"
#include "XiliRect.hh"

//------------------------------------------------------------------------
//
// inlines 
//
//------------------------------------------------------------------------

inline
Xil_boolean
XilRoi::nothingValid(XilRoi* roi)
{
    return ((!roi->roiRect.isValid()) && (!roi->roiBitmask.isValid())
             && (!roi->roiConvexRegion.isValid()));
}

//------------------------------------------------------------------------
//
//  Function:	XilRoi::XilRoi()
//
//  Description: Main constructor for ROI object
//	
//  MT-level:  <??????>
//	
//  Parameters:
//	
//	
//  Returns:
//	
//	
//  Side Effects:
//	
//	
//  Notes:
//	
//  Deficiencies/ToDo:
//	
//------------------------------------------------------------------------
XilRoi::XilRoi(XilSystemState* system_state) :
    XilNonDeferrableObject(system_state, XIL_ROI),
    roiRect(this),
    roiBitmask(this),
    roiConvexRegion(this)
{
    isOKFlag = TRUE;
}

XilRoi::XilRoi() :
    XilNonDeferrableObject(NULL, XIL_ROI),
    roiRect(this),
    roiBitmask(this),
    roiConvexRegion(this)
{
    isOKFlag = TRUE;
}

//------------------------------------------------------------------------
//
//  Function:	~XilRoi() 
//
//  Description: Destructor for ROI object
//	
//  MT-level:  <??????>
//	
//  Parameters:
//	
//	
//  Returns:
//	
//	
//  Side Effects:
//	
//	
//  Notes:
//	
//  Deficiencies/ToDo:
//	
//------------------------------------------------------------------------
XilRoi::~XilRoi()
{
}

//------------------------------------------------------------------------
//
//  Function:	XilRoi::clear()
//
//  Description: Returns ROI to "as new" state without expense of destruction/creation
//	
//  MT-level:  <??????>
//	
//  Parameters:
//	
//	
//  Returns:
//	
//	
//  Side Effects:
//	
//	
//  Notes:
//	
//  Deficiencies/ToDo:
//	
//------------------------------------------------------------------------
XilStatus
XilRoi::clear()
{
    isOKFlag = FALSE;

    if((roiRect.clear() != XIL_SUCCESS) ||
       (roiBitmask.clear() != XIL_SUCCESS) ||
       (roiConvexRegion.clear() != XIL_SUCCESS)) {
        return XIL_FAILURE;
    }

    isOKFlag = TRUE;
    return XIL_SUCCESS;
}

//------------------------------------------------------------------------
//
//  Function:	= operator
//
//  Description: Sets a second ROI to the first, including copies of all data
//	
//  MT-level:  <??????>
//	
//  Parameters:
//	
//	
//  Returns:
//	
//	
//  Side Effects:
//	
//	
//  Notes:
//	
//  Deficiencies/ToDo:
//	
//------------------------------------------------------------------------
const XilRoi&
XilRoi::operator =(XilRoi& from)
{
    isOKFlag = from.isOKFlag;

    if(from.roiRect.isValid()) {
        roiRect         = from.roiRect;
    }
    if(from.roiBitmask.isValid()) {
        roiBitmask      = from.roiBitmask;
    }
    if(from.roiConvexRegion.isValid()) {
        roiConvexRegion = from.roiConvexRegion;
    }

    //
    //  Copy the version information from the other ROI so they're 
    //  considered representing the same ROI.
    //
    copyVersionInfo(&from);
    
    return *this;
}

//------------------------------------------------------------------------
//
//  Function:	dump()
//
//  Description:
//	Dump the contents of the ROI to stderr. 
//	
//  Deficiencies/ToDo:
//	TODO: 4/9/96 jlf  Only works when roiRect is valid.
//	
//------------------------------------------------------------------------
#ifdef DEBUG
void
XilRoi::dump()
{
    if(roiRect.isValid()) {
        roiRect.dump();
    }
#if 0
    if(roiConvexRegion.isValid()) {
        roiConvexRegion.dump();
    }
#endif
}
#endif

//------------------------------------------------------------------------
//
//  Function:	XilRoi::addImage()
//
//  Description: 
//    Support for API call xil_roi_add_image().  The image is assumed 
//    to be 1-bit data, where non-zero values are added to the ROI.  
//    If the bitmask implementation is valid, add to that implementation 
//    (most straightforward).  If not, add to the rectlist implementation,
//    even if it requires translation to that type. 
//	
//  MT-level:  <??????>
//	
//  Parameters:
//	
//	
//  Returns:
//	
//	
//  Side Effects:
//	
//	
//  Notes:
//	
//  Deficiencies/ToDo:
//	
//------------------------------------------------------------------------
XilStatus              
XilRoi::addImage(XilImage*  image)
{
    //
    //  Update the version number prior to changing the contents of the ROI.
    //
    newVersion();
    
    if(roiBitmask.isValid()) {
        //
        //  Invalidate the other versions
        //
        roiRect.setValid(FALSE);
        roiConvexRegion.setValid(FALSE);

        return roiBitmask.addImage(image);
    }

    if(roiRect.isValid()) {
        //
        //  Invalidate the other versions
        //
        roiBitmask.setValid(FALSE);
        roiConvexRegion.setValid(FALSE);

        return roiRect.addImage(image);    
    }

    if(roiConvexRegion.isValid()) {
        //
        // Doesn't make sense to add an image to a convex region...too complicated.
        // translate to a rectlist implementation instead
        //
        if(roiRect.translateConvexRegion(&roiConvexRegion) == XIL_FAILURE) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-445", FALSE, this);
            return XIL_FAILURE;
        }

        //
        //  Invalidate the other versions
        //
        roiConvexRegion.setValid(FALSE);
        roiBitmask.setValid(FALSE);  

        return roiRect.addImage(image);
    }

    //
    // If the roi was empty, add the image to the rectlist implementation
    //
    if(roiRect.addImage(image) == XIL_FAILURE) {
        return XIL_FAILURE;
    }
    roiRect.setValid(TRUE);
    return XIL_SUCCESS;
}

//------------------------------------------------------------------------
//
//  Function:	XilRoi::addRect()
//
//  Description: support for the API routine xil_roi_add_rect()
//               If the rectlist implementation is valid, add directly.
//               Otherwise add to whichever implementation is currently valid
//
//	
//  MT-level:  <??????>
//	
//  Parameters:
//	
//	
//  Returns:
//	
//	
//  Side Effects:
//	
//	
//  Notes: It may not make sense to add to a bitmask implementation. That need may better
//         be served by translating to a rectlist first.
//	
//  Deficiencies/ToDo:
//	
//------------------------------------------------------------------------
XilStatus
XilRoi::addRect(int          x,
		int          y,
		unsigned int xsize,
		unsigned int ysize)
{
    //
    //  Update the version number prior to changing the contents of the ROI.
    //
    newVersion();
    
    if(roiRect.isValid()) {
        //
        //  Invalidate the other implementations
        //
        roiBitmask.setValid(FALSE);
        roiConvexRegion.setValid(FALSE);

        return roiRect.addRect(x,y,xsize,ysize);    
    }

    if(roiBitmask.isValid()) {
        //
        // Invalidate the other implementations
        //
        roiRect.setValid(FALSE);
        roiConvexRegion.setValid(FALSE);

        return roiBitmask.addRect(x,y,xsize,ysize);
    }

    if(roiConvexRegion.isValid()) {
        //
        // Invalidate the other implementations
        //
        roiRect.setValid(FALSE);
        roiBitmask.setValid(FALSE);

        return roiConvexRegion.addRect(x,y,xsize,ysize);
    }

    //
    //  If the roi is empty, add to the rectlist
    //
    if(roiRect.addRect(x, y, xsize, ysize) == XIL_FAILURE) {
        return XIL_FAILURE;
    }
    roiRect.setValid(TRUE);
    return XIL_SUCCESS;
}

XilStatus
XilRoi::addRect(double x,
		double y,
		double xsize,
		double ysize)
{
    //
    //  Update the version number prior to changing the contents of the ROI.
    //
    newVersion();
    
    if(roiConvexRegion.isValid()) {
        //
        //  Invalidate the other implementations
        //
        roiRect.setValid(FALSE);
        roiBitmask.setValid(FALSE);

        return roiConvexRegion.addRect(x, y, xsize, ysize);
    }

    if(roiRect.isValid()) {
        //
        //  Translate to convex region and add the rectangle. It's
        //  the only way to maintain the double precision.
        //
        roiBitmask.setValid(FALSE);

        if(roiConvexRegion.translateRectList(&roiRect) == XIL_FAILURE) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-445", FALSE, this);
            return XIL_FAILURE;
        }
        roiRect.setValid(FALSE);

        return roiConvexRegion.addRect(x, y, xsize, ysize);
    }

    if(roiBitmask.isValid()) {
        //
        //  Invalidate the other implementations
        //
        roiRect.setValid(FALSE);
        roiConvexRegion.setValid(FALSE);

        //
        //  TODO: maynard 3/22/97 This is bogus - but okay until we implement bitmask.
        //        This should be translate to convex region and added in
        //        order to maintain precision. However this can remain for
        //        now because we don't actually implement bitmask rois.
        //
        return roiBitmask.addRect(_XILI_ROUND(x),
                                  _XILI_ROUND(y),
                                  (unsigned int)xsize,
                                  (unsigned int)ysize);
    }

    //
    //  If the ROI is empty then add the rect to the convex region implementation.
    //
    if(roiConvexRegion.addRect(x, y, xsize, ysize) == XIL_FAILURE) {
        return XIL_FAILURE;
    }
    roiConvexRegion.setValid(TRUE);
    return XIL_SUCCESS;
}


#if defined(_XIL_HAS_X11WINDOWS) || defined(_WINDOWS)
//------------------------------------------------------------------------
//
//  Function:	XilRoi::addRegion()
//
//  Description: Support for the API call xil_roi_add_region() for 
//               Xregion support.  Only add to a rectlist implementation
//               - other versions gettranslated to rectlist before 
//               operation.
//
//  MT-level:  <??????>
//	
//  Parameters:
//	
//	
//  Returns:
//	
//	
//  Side Effects:
//	
//	
//  Notes:
//	
//  Deficiencies/ToDo:
//	
//------------------------------------------------------------------------
XilStatus
XilRoi::addRegion(Region region)
{
    //
    //  Update the version number prior to changing the contents of the ROI.
    //
    newVersion();
    
    if(roiRect.isValid()) {
        //
        // Invalidate the other versions
        //
        roiBitmask.setValid(FALSE);
        roiConvexRegion.setValid(FALSE);

        return roiRect.addRegion(region);    
    }

    //
    //  Translate to a roiRect....
    //
    if(roiBitmask.isValid() || roiConvexRegion.isValid()){
        if(roiBitmask.isValid()) {
            //
            // doesn't make sense to add a region here... translate to rectlist
            //
            if(roiRect.translateBitmask(&roiBitmask) == XIL_FAILURE) {
                XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-445", FALSE, this);
                return XIL_FAILURE;
            }
            roiBitmask.setValid(FALSE);
        } else {
            //
            // doesn't make sense to add a region here... translate to rectlist
            //
            if(roiRect.translateConvexRegion(&roiConvexRegion) == XIL_FAILURE) {
                XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-445", FALSE, this);
                return XIL_FAILURE;
            }
            roiConvexRegion.setValid(FALSE);
        }
        return roiRect.addRegion(region);
    }

    //
    // If we're here then we know that the ROI was empty.
    //
    //
    //  Set roiRect valid in case it got to this point because it
    //  was empty.
    //
    if(roiRect.addRegion(region) == XIL_FAILURE) {
        return XIL_FAILURE;
    }
    roiRect.setValid(TRUE);
    return XIL_SUCCESS;
}
#endif /* _XIL_HAS_X11WINDOWS || _WINDOWS */
			       
//
//------------------------------------------------------------------------
//
//  Function:	XilRoi::createCopy()
//
//  Description: return a new ROI which is a copy of this ROI
//	
//  MT-level:  <??????>
//	
//  Parameters:
//	
//	
//  Returns:
//	
//	
//  Side Effects:
//	
//	
//  Notes:
//	
//  Deficiencies/ToDo:
//	
//------------------------------------------------------------------------
XilObject*
XilRoi::createCopy()
{
    XilRoi* copy = getSystemState()->createXilRoi();

    if(copy == NULL) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-145", FALSE, this);
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-14", FALSE, this);
        return NULL;
    }

    //
    // Now copy over all representations of the roi.
    //
    copy->roiRect = roiRect;
    copy->roiBitmask = roiBitmask;
    copy->roiConvexRegion = roiConvexRegion;

    //
    // update the myRoi fields in the implementations
    //
    copy->roiRect.setCallingRoi(copy);
    copy->roiBitmask.setCallingRoi(copy);
    copy->roiConvexRegion.setCallingRoi(copy);

    copy->copyVersionInfo(this);

    return copy;
}

//------------------------------------------------------------------------
//
//  Function:	XilRoi::getAsImage()
//
//  Description: Support for API call xil_roi_get_as_image()
//               return an XIL_BIT image which represents the ROI.  The image needs 
//               to encompass the entire extent of the ROI, with pixels within the 
//               ROI set to 1, pixels outside of the ROI set to 0.
//               Pull the bitmask from a bitmask implementation if possible, otherwise
//               from a rectlist implementation.
//	
//  MT-level:  <??????>
//	
//  Parameters:
//	
//	
//  Returns:
//	
//	
//  Side Effects:
//	
//	
//  Notes:
//	
//  Deficiencies/ToDo:
//	
//------------------------------------------------------------------------
XilImage*
XilRoi::getAsImage()
{
    if(roiBitmask.isValid()) {
        return roiBitmask.getAsImage();
    }

    if(roiRect.isValid()) {
        return roiRect.getAsImage();    
    }

    if(roiConvexRegion.isValid()) {
        //
        //  We haven't implemented getting an image from a convex region ROI
        //
        if(roiRect.translateConvexRegion(&roiConvexRegion) == XIL_FAILURE) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-445", FALSE, this);
            return NULL;
        }
        return roiRect.getAsImage();
    }
    
    //
    //  It was an empty ROI
    //
    return NULL;
}

#if defined(_XIL_HAS_X11WINDOWS) || defined(_WINDOWS)
//------------------------------------------------------------------------
//
//  Function:	XilRoi::getAsRegion()
//
//  Description: Support for API all xil_roi_get_as_region()
//               return an X Region which represents the ROI
//               Operate only on the rectlistimplementation of the ROI.
//
//  MT-level:  <??????>
//	
//  Parameters:
//	
//	
//  Returns:
//	
//	
//  Side Effects:
//	
//	
//  Notes:
//	
//  Deficiencies/ToDo:
//	
//------------------------------------------------------------------------
Region
XilRoi::getAsRegion()
{
    //
    //  If not a rectlist, make it one before generating region
    //  If the ROI is empty, return NULL
    //
    if(roiRect.isValid()) {
        return roiRect.getAsRegion();
    }

    if(roiBitmask.isValid()) {
        if(roiRect.translateBitmask(&roiBitmask) == XIL_FAILURE) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-445", FALSE, this);
            return NULL;
        }
        return roiRect.getAsRegion();
    }

    if(roiConvexRegion.isValid()) {
        if(roiRect.translateConvexRegion(&roiConvexRegion) == XIL_FAILURE) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-445", FALSE, this);
            return NULL;
        }
        return roiRect.getAsRegion();
    } 

    //
    //  It was an empty ROI
    //
    return NULL;
}
#endif /* _XIL_HAS_X11WINDOWS || _WINDOWS */

//------------------------------------------------------------------------
//
//  Function:	XilRoi::intersect()
//
//  Description: Support for API call xil_roi_intersect()
//               return a new ROI which is the result of an intersection between
//               the current ("this") ROI and the passed ROI
//               If both ROIs are of the same type, do the intersection in type,
//               otherwise if convex region and rectlist do as convex region,
//               otherwise translate and do as rectlist.
//	
//  MT-level:  <??????>
//	
//  Parameters:
//	
//	
//  Returns:
//	
//	
//  Side Effects:
//	
//	
//  Notes:
//	
//  Deficiencies/ToDo:
//	
//------------------------------------------------------------------------
XilRoi*
XilRoi::intersect(XilRoi* roi)
{

    XilRoi* intersect_roi = getSystemState()->createXilRoi();
    if(intersect_roi == NULL) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-145", FALSE, this);
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-7", FALSE, this);
        return NULL;
    }

    //
    //  If either of the ROI's is empty, then return an empty ROI
    // 
    if(nothingValid(this) || nothingValid(roi)) {
        return intersect_roi;
    }

    if(this->intersect(roi, intersect_roi) == XIL_FAILURE) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-7", FALSE, this);
        delete intersect_roi;
        return NULL;
    }

    return intersect_roi;
}

//------------------------------------------------------------------------
//
//  Function:	XilRoi::intersect()
//
//  Description: Same operation as intersect() above, but fills in passed in ROI ptr
//               instead of returning a ROI ptr.
//
//  MT-level:  <??????>
//	
//  Parameters:
//	
//	
//  Returns:
//	
//	
//  Side Effects:
//	
//	
//  Notes:
//	
//  Deficiencies/ToDo:
//  TODO: maynard 2/26/96        
//        This should be combined functionality with intersect() above.
//	
//------------------------------------------------------------------------
XilStatus
XilRoi::intersect(XilRoi* roi,
                  XilRoi* intersect_roi)
{

    //
    //  If either of the ROI's is empty return an empty ROI
    //
    if(nothingValid(this) || nothingValid(roi)) {
        intersect_roi->clear();
        return XIL_SUCCESS;
    }

    //
    //  If both ROI's are a rectlist implementation, then
    //  do the intersection as rects.
    //
    if(roiRect.isValid() && roi->roiRect.isValid()) {
        //
        //  Invalidate the other implementations
        //
        intersect_roi->roiBitmask.setValid(FALSE);
        intersect_roi->roiConvexRegion.setValid(FALSE);
        
        if(roiRect.intersect(&(roi->roiRect),&(intersect_roi->roiRect)) == XIL_SUCCESS) {
            intersect_roi->roiRect.setValid(TRUE);
            return XIL_SUCCESS;
        } else {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-7", FALSE, this);
            return XIL_FAILURE;
        }
    }

    //
    //  If both ROI's are convex region implementations, then
    //  do the intersection as convex regions.
    //
    if(roiConvexRegion.isValid() && roi->roiConvexRegion.isValid())  {
        if(roiConvexRegion.intersect(&(roi->roiConvexRegion),
                                     &(intersect_roi->roiConvexRegion)) == XIL_SUCCESS) {
            intersect_roi->roiConvexRegion.setValid(TRUE);
            return XIL_SUCCESS;
        } else {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-7", FALSE, this);
            return XIL_FAILURE;
        }
    }

    //
    //  Is one of the ROIs in convex region form? If so, we'll need to
    //  do the intersection in that form for precision.
    //
    if(roiConvexRegion.isValid()) {
        //
        //  We know that the other ROI is not a convex region
        //
        if(!roi->roiRect.isValid()) {
            //
            //  Well then it must be a bitmask. Make it a rectlist
            //
            if(roi->roiRect.translateBitmask(&roi->roiBitmask) == XIL_FAILURE) {
                XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-445", FALSE, this);
                return XIL_FAILURE;
            }
        }
        //
        //  Now we know we have a rectlist and a convex region
        //
        if(roiConvexRegion.intersect(&roi->roiRect,
                                     &intersect_roi->roiConvexRegion) == XIL_SUCCESS) {
            intersect_roi->roiConvexRegion.setValid(TRUE);
            return XIL_SUCCESS;
        } else {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-7", FALSE, this);
            return XIL_FAILURE;
        }
    }

    if(roi->roiConvexRegion.isValid()) {
        //
        //  We know that the other ROI is not a convex region
        //
        if(!roiRect.isValid()) {
            //
            //  Well then it must be a bitmask. Make it a rectlist
            //
            if(roiRect.translateBitmask(&roi->roiBitmask) == XIL_FAILURE) {
                XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-445", FALSE, this);
                return XIL_FAILURE;
            }
        }
        //
        //  Now we know we have a rectlist and a convex region
        //
        if(roi->roiConvexRegion.intersect(&roiRect,
                                          &intersect_roi->roiConvexRegion) == XIL_SUCCESS) {
            intersect_roi->roiConvexRegion.setValid(TRUE);
            return XIL_SUCCESS;
        } else {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-7", FALSE, this);
            return XIL_FAILURE;
        }
    }

    //
    //   At this point each one is either a roiRect or a roiBitmask.
    //   Ensure that they're a rectlist and intersect.
    //
    if(!roiRect.isValid()) {
        if(roiRect.translateBitmask(&roiBitmask) == XIL_FAILURE) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-445", FALSE, this);
            return XIL_FAILURE;
        }
    }
    if(!roi->roiRect.isValid()) {
        if(roi->roiRect.translateBitmask(&roi->roiBitmask) == XIL_FAILURE) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-445", FALSE, this);
            return XIL_FAILURE;
        }
    }    

    //
    //  They're both rectlists now :
    //
    if(roiRect.intersect(&(roi->roiRect),&(intersect_roi->roiRect)) == XIL_SUCCESS) {
        intersect_roi->roiRect.setValid(TRUE);
        return XIL_SUCCESS;
    } 

    return XIL_FAILURE;
}

//------------------------------------------------------------------------
//
//  Function:	XilRoi::intersect_inplace()
//
//  Description: As a performance improvement, intersect the two ROI's storing the
//               result back in the second ROI.
//
//  MT-level:  <??????>
//	
//  Parameters:
//	
//	
//  Returns:
//	
//	
//  Side Effects:
//	
//	
//  Notes:
//	
//  Deficiencies/ToDo:
//  TODO: maynard 2/26/96             
//        This first implementation just calls new/delete at this level and uses the
//        other intersect routines. It needs to be rewritten to be a true in-place intersection.
//	
//------------------------------------------------------------------------
XilStatus
XilRoi::intersect_inplace(XilRoi* intersect_roi)
{
    //
    //  Update the version number prior to changing the contents of the ROI.
    //
    newVersion();
    
    XilRoi* roi = this->intersect(intersect_roi);
    if(roi == NULL) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-145", FALSE, this);
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-7", FALSE, this);
        return XIL_FAILURE;
    }

    intersect_roi->roiRect = roi->roiRect;
    intersect_roi->roiBitmask = roi->roiBitmask;
    intersect_roi->roiConvexRegion = roi->roiConvexRegion;

    roi->destroy();

    return XIL_SUCCESS;
}


//
// Intersect the convex region ROI in place and
// change each of the roi's into PIXEL EXTENT space on
// the fly while you intersect.
//
XilStatus
XilRoi::intersect_extent(XilRoi* intersect_roi)
{

    //
    //  Intersection with an empty ROI is empty.
    //
    if(nothingValid(intersect_roi)) {
        return XIL_SUCCESS;
    }
    if(nothingValid(this)) {
        intersect_roi->clear();
        return XIL_SUCCESS;
    }

    //
    //  Update the version number prior to changing the contents of the ROI.
    //
    newVersion();
    
    XilRoi* extent_roi = getSystemState()->createXilRoi();
    if(extent_roi == NULL) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-145", FALSE, this);
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-7", FALSE, this);
        return XIL_FAILURE;
    }

    //
    //  Make sure each ROI is a convex region ROI
    //  We know that at least one implementation is valid
    //
    if(!roiConvexRegion.isValid()) {
        if(!roiRect.isValid()) {
            if(roiRect.translateBitmask(&roiBitmask) == XIL_FAILURE) {
                XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-445", FALSE, this);
                return XIL_FAILURE;
            }
        }
        if(roiConvexRegion.translateRectList(&roiRect) == XIL_FAILURE) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-445", FALSE, this);
            return XIL_FAILURE;
        }                  
    }

    if(!intersect_roi->roiConvexRegion.isValid()) {
        if(!intersect_roi->roiRect.isValid()) {
            if(intersect_roi->roiRect.translateBitmask(&intersect_roi->roiBitmask) == XIL_FAILURE) {
                XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-445", FALSE, this);
                return XIL_FAILURE;
            }
        }
        if(intersect_roi->roiConvexRegion.translateRectList(&intersect_roi->roiRect) == XIL_FAILURE) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-445", FALSE, this);
            return XIL_FAILURE;
        }                  
    }

    //
    //  We know both ROI's are now convex region lists
    //
    if(roiConvexRegion.extentIntersect(&intersect_roi->roiConvexRegion,
                                       &extent_roi->roiConvexRegion) == XIL_FAILURE) {
        extent_roi->destroy();
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-7", FALSE, this);
        return XIL_FAILURE;
    }
    extent_roi->roiConvexRegion.setValid(TRUE);
    intersect_roi->roiConvexRegion = extent_roi->roiConvexRegion;
    extent_roi->destroy();
    
    return XIL_SUCCESS;
}

//------------------------------------------------------------------------
//
//  Function:	XilRoi::scale()
//
//  Description: Support for API call xil_roi_scale
//               return a new ROI that is the result of a scaling of this ROI about
//               the specified origin
//               Only supported for rectlist and convex region implementations
//	
//  MT-level:  <??????>
//	
//  Parameters:
//	
//	
//  Returns:
//	
//	
//  Side Effects:
//	
//	
//  Notes:
//	
//  Deficiencies/ToDo:
//  TODO: maynard 2/26/96
//        Add in support for convex region version of scale()
//------------------------------------------------------------------------
XilRoi*
XilRoi::scale(float  xscale,
	      float  yscale,
	      float  xorigin,
	      float  yorigin)
{

    XilRoi* scale_roi = getSystemState()->createXilRoi();

    if(scale_roi == NULL) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-145", FALSE, this);
        return NULL;
    }
    
    //
    //  If ROI is emptry, return empty ROI
    //
    if(nothingValid(this)) {
        return scale_roi;
    }

    //
    //  Convex Region does not currently support scale.
    //  Translate to rectlist and then do translate
    //
    if(!roiRect.isValid()) {
        if(roiBitmask.isValid()) {
            if(roiRect.translateBitmask(&roiBitmask) == XIL_FAILURE) {
                XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-445", FALSE, this);
                return NULL;
            }
        } else {
            //
            // TODO: maynard 2/1/96
            // This should be supported directly for roiConvexRegion
            //
            if(roiRect.translateConvexRegion(&roiConvexRegion) == XIL_FAILURE) {
                XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-445", FALSE, this);
                return NULL;
            }
        }
    }

    //
    //  Now we know that this is a roiRect.
    //
    if(roiRect.scale(xscale,yscale,xorigin,yorigin,
                     &(scale_roi->roiRect)) == XIL_SUCCESS) {
        scale_roi->roiRect.setValid(TRUE);
        return scale_roi;
    }
    return NULL;
}

//------------------------------------------------------------------------
//
//  Function:	XilRoi::subtractRect()
//
//  Description: Support for the API call xil_roi_subtract_rect
//               Remove the specified rectangle from the ROI - only supported
//               in the rectlist implementation
//	
//  MT-level:  <??????>
//	
//  Parameters:
//	
//	
//  Returns:
//	
//	
//  Side Effects:
//	
//	
//  Notes:
//	
//  Deficiencies/ToDo:
//------------------------------------------------------------------------
XilStatus
XilRoi::subtractRect(int          x,
		     int          y,
		     unsigned int xsize,
		     unsigned int ysize)
{

    //
    //  If the roi is empty, simply return it unchanged.
    //
    if(nothingValid(this)) {
        return XIL_SUCCESS;
    }

    //
    //  Update the version number prior to changing the contents of the ROI.
    //
    newVersion();
    
    if(!roiRect.isValid()) {
        //
        //  If ROI not a rectlist, then make it one
        // 
        if(roiBitmask.isValid()) {
            if(roiRect.translateBitmask(&roiBitmask) == XIL_FAILURE) {
                XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-445", FALSE, this);
                return XIL_FAILURE;
            }
        } else {
            if(roiRect.translateConvexRegion(&roiConvexRegion) == XIL_FAILURE) {
                XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-445", FALSE, this);
                return XIL_FAILURE;
            }
        }
    }

    //
    //  Now we know that roiRect is valid
    //
    roiBitmask.setValid(FALSE);
    roiConvexRegion.setValid(FALSE);
    return roiRect.subtractRect(x,y,xsize,ysize);

}

//------------------------------------------------------------------------
//
//  Function:	XilRoi::tranlsate()
//
//  Description: Support for the API all xil_roi_translate
//              return a new ROI that is the result of a translation of this ROI of
//              the specified x and y amounts
//              Only supported for the rectlist implementation
//
//	
//  MT-level:  <??????>
//	
//  Parameters:
//	
//	
//  Returns:
//	
//	
//  Side Effects:
//	
//	
//  Notes:
//	
//  Deficiencies/ToDo:
//	
//------------------------------------------------------------------------
XilRoi*
XilRoi::translate(int x,
		  int y)
{

    XilRoi* translate_roi = getSystemState()->createXilRoi();
    if(translate_roi == NULL) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-145", FALSE, this);
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-16", FALSE, this);
        return NULL;
    }
    
    //
    //  If nothing is valid, simply return an empty roi
    //
    if(nothingValid(this)) {
        return translate_roi;
    }


    if(!roiRect.isValid()) {
        //
        //  If it's a convex region, do the translate directly
        //
        if(roiConvexRegion.isValid()) {
            if(roiConvexRegion.translate((double)x, (double)y,
                                         &(translate_roi->roiConvexRegion)) == XIL_FAILURE) {
                translate_roi->destroy();
                XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-16", FALSE, this);
                return NULL;
            }

            translate_roi->roiConvexRegion.setValid(TRUE);
            return translate_roi;

        } else { 
            //
            //  It must be a bitmask implementation, turn it into a rectlist
            //
            if(roiRect.translateBitmask(&roiBitmask) == XIL_FAILURE) {
                XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-445", FALSE, this);
                return NULL;
            }
        }
    } 

    //
    //  If we get here then we know it is a rectlist
    //
    if(roiRect.translate(x, y, &(translate_roi->roiRect)) == XIL_FAILURE) {
        translate_roi->destroy();
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-16", FALSE, this);
        return NULL;
    }
    
    translate_roi->roiRect.setValid(TRUE);
    return translate_roi;
}

XilRoi*
XilRoi::translate(double x,
		  double y)
{

    XilRoi* translate_roi = getSystemState()->createXilRoi();
    if(translate_roi == NULL) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-145", FALSE, this);
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-16", FALSE, this);
        return NULL;
    }
    
    //
    //  If nothing is valid, simply return an empty roi
    //
    if(nothingValid(this)) {
        return translate_roi;
    }


    if(!roiConvexRegion.isValid()) {
        //
        //  If it's a rectangle
        //  TODO: maynard - loss of precision.
        //  change this so it translates to a convex region and does the
        //  translate with double precision.
        //
        if(!roiRect.isValid()) {
            //
            //  It must be a bitmask implementation, turn it into a rectlist
            //
            if(roiRect.translateBitmask(&roiBitmask) == XIL_FAILURE) {
                XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-445", FALSE, this);
                return NULL;
            }
        }
        if(roiRect.translate(_XILI_ROUND(x),
                             _XILI_ROUND(y),
                             &(translate_roi->roiRect)) == XIL_FAILURE) {
            translate_roi->destroy();
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-16", FALSE, this);
            return NULL;
        }
        
        translate_roi->roiRect.setValid(TRUE);
        return translate_roi;
    }

    //
    //  If we get here then we know it is a convex region list
    //
    if(roiConvexRegion.translate(x, y,
                                 &(translate_roi->roiConvexRegion)) == XIL_FAILURE) {
        translate_roi->destroy();
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-16", FALSE, this);
        return NULL;
    }
    
    translate_roi->roiConvexRegion.setValid(TRUE);
    return translate_roi;
}

//------------------------------------------------------------------------
//
//  Function:	XilRoi::translate_inplace()
//
//  Description:
//      Same functionality as translate, but puts results back into
//      "this" ROI. 
//	
//------------------------------------------------------------------------
XilStatus
XilRoi::translate_inplace(int x,
                          int y)
{
    //
    //  Update the version number prior to changing the contents of the ROI.
    //
    newVersion();

    //
    //  If nothing is valid, simply return an empty roi
    //
    if(nothingValid(this)) {
        this->clear();
        return XIL_SUCCESS;
    }

    if(!roiRect.isValid()) {
        //
        //  If it's a convex region, do the translate directly
        //
        if(roiConvexRegion.isValid()) {
            roiBitmask.setValid(FALSE);
            roiRect.setValid(FALSE);
            
            return roiConvexRegion.translate_inplace(x, y);
        } else { 
            //
            //  It must be a bitmask implementation, turn it into a rectlist
            //
            if(roiRect.translateBitmask(&roiBitmask) == XIL_FAILURE) {
                XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-445", FALSE, this);
                return XIL_FAILURE;
            }
        }
    } 

    //
    //  If we get here then we know it is a rectlist
    //
    roiBitmask.setValid(FALSE);
    roiConvexRegion.setValid(FALSE);
    
    return roiRect.translate_inplace(x, y);
}

XilStatus
XilRoi::translate_inplace(double x,
                          double y)
{
    //
    //  Update the version number prior to changing the contents of the ROI.
    //
    newVersion();

    //
    //  If nothing is valid, simply return an empty roi
    //
    if(nothingValid(this)) {
        this->clear();
        return XIL_SUCCESS;
    }

    if(!roiConvexRegion.isValid()) {
        //
        //  If it's a rectlist, do the translate directly
        //  TODO: maynard 3/24/97 - loss of precision
        //  In order to maintain the double values, change this to a convex region.
        //
        if(!roiRect.isValid()) {
            //
            //  It must be a bitmask implementation, turn it into a rectlist
            //
            if(roiRect.translateBitmask(&roiBitmask) == XIL_FAILURE) {
                XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-445", FALSE, this);
                return XIL_FAILURE;
            }
        }
        roiBitmask.setValid(FALSE);
        roiConvexRegion.setValid(FALSE);
        
        return roiRect.translate_inplace(_XILI_ROUND(x),
                                         _XILI_ROUND(y));
    } 

    //
    //  If we get here then we know it is a convex region
    //
    roiBitmask.setValid(FALSE);
    roiRect.setValid(FALSE);
    
    return roiConvexRegion.translate_inplace(x, y);
}

//------------------------------------------------------------------------
//
//  Function:	XilRoi::transpose()
//
//  Description: Support for API call xil_roi_transpose()
//               return a new ROI that is the result of transposing this ROI about
//               the specified origin
//               This is only supported for rectlist and convex_region implementations
//	
//  Deficiencies/ToDo:
//  TODO: maynard 2/26/96
//        This needs to be implemented for convex region
//	
//------------------------------------------------------------------------
XilRoi*
XilRoi::transpose(XilFlipType fliptype,
		  float       xorigin,
		  float       yorigin)
{

    XilRoi* transpose_roi = getSystemState()->createXilRoi();

    if(transpose_roi == NULL) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-145", FALSE, this);
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-432", FALSE, this);
        return NULL;
    }

    //
    // if this ROI isn't valid, return empty ROI.
    //
    if(nothingValid(this)) {
        return transpose_roi;
    }

    //
    //  TODO: 3/24/97 - needs to be supported directly for convex regions
    //  Especially because of the float origin values, this should be supported
    //  for the convex region version of the ROI and then should always be
    //  handled in convex regions.
    //

    if(!roiRect.isValid()) {
        if(roiBitmask.isValid()) {
            if(roiRect.translateBitmask(&roiBitmask) == XIL_FAILURE) {
                XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-445", FALSE, this);
                return NULL;
            }
        } else {
            //
            //  convex region must be valid
            //
            if(roiRect.translateConvexRegion(&roiConvexRegion) == XIL_FAILURE) {
                XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-445", FALSE, this);
                return NULL;
            }
        }
    }

    //
    //  If we're here then we know that the roiRect version is valid
    //
    if(roiRect.transpose(fliptype, xorigin, yorigin,
                         &(transpose_roi->roiRect)) == XIL_FAILURE) {
        transpose_roi->destroy();
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-432", FALSE, this);
        return NULL;
    }
    
    transpose_roi->roiRect.setValid(TRUE);
    return transpose_roi;
}

//------------------------------------------------------------------------
//
//  Function:	XilRoi::unite()
//
//  Description: Support for API call xil_roi_unite()
//               return a new ROI that is the union of this ROI with the specified ROI
//               If both of same type, do the unite in type, otherwise translate 
//               to rectlist.
//	      
//  MT-level:  <??????>
//	
//  Parameters:
//	
//	
//  Returns:
//	
//	
//  Side Effects:
//	
//	
//  Notes:
//	
//  Deficiencies/ToDo:
//	
//------------------------------------------------------------------------
XilRoi*
XilRoi::unite(XilRoi* roi)
{

    XilRoi* unite_roi = getSystemState()->createXilRoi();
    if(unite_roi == NULL) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-145", FALSE, this);
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-433", FALSE, this);
        return NULL;
    }

    //
    //  If either of the ROI's is empty then the "unite" is just the other ROI.
    //  This will work even if both ROI's are empty.
    //
    if(nothingValid(this)) {
        unite_roi->roiRect = roi->roiRect;
        unite_roi->roiBitmask = roi->roiBitmask;
        unite_roi->roiConvexRegion = roi->roiConvexRegion;
        return unite_roi;
    }
    if(nothingValid(roi)) {
        unite_roi->roiRect = roiRect;
        unite_roi->roiBitmask = roiBitmask;
        unite_roi->roiConvexRegion = roiConvexRegion;
        return unite_roi;
    }

    //
    //  TODO: maynard 3/24/97
    //  Unite is currently only implemented for a rectlist. So make sure both of
    //  the ROI's are in this form before doing the unite. Note that this may
    //  cause loss of fp precision.
    //

    //
    // if both ROIs are rects, do the unite
    //
    if(roiRect.isValid() && roi->roiRect.isValid()) {
        if(roiRect.unite(&(roi->roiRect),&(unite_roi->roiRect)) == XIL_SUCCESS) {

            unite_roi->roiRect.setValid(TRUE);
            return unite_roi;
        } else {

            unite_roi->destroy();
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-433", FALSE, this);
            return NULL;
        }
    }

    //
    // If not translate to rectlist before unite-ing
    //
    if(!roiRect.isValid()) {
        if(roiBitmask.isValid()) {
            if(roiRect.translateBitmask(&roiBitmask) == XIL_FAILURE) {
                XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-445", FALSE, this);
                return NULL;
            }
        } else {
            //
            //  We know that convex region must be valid
            //
            if(roiRect.translateConvexRegion(&roiConvexRegion) == XIL_FAILURE) {
                XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-445", FALSE, this);
                return NULL;
            }
        }
    }
    if(!roi->roiRect.isValid()) {
        if(roi->roiBitmask.isValid()) {
            if(roi->roiRect.translateBitmask(&roi->roiBitmask) == XIL_FAILURE) {
                XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-445", FALSE, this);
                return NULL;
            }
        } else {
            //
            //  We know that convex region must be valid
            //
            if(roi->roiRect.translateConvexRegion(&roi->roiConvexRegion) == XIL_FAILURE) {
                XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-445", FALSE, this);
                return NULL;
            }
        }
    }

    //
    //  They're both rectlists now
    //
    if(roiRect.unite(&(roi->roiRect),&(unite_roi->roiRect)) == XIL_SUCCESS) {
        unite_roi->roiRect.setValid(TRUE);
        return unite_roi;
    }

    //
    //  Unite failed
    //
    unite_roi->destroy();
    XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-433", FALSE, this);
    return NULL;
}


//
//  Join two disjoint rois, it is assumed the calling party knows
//  this to be true
//  TODO: maynard 3/24/97 - This is currently only implemented for rectlists
//        It should be implemented for the other versions as well.    
//
XilRoi*
XilRoi::unite_disjoint(XilRoi* roi)
{


    XilRoi* unite_roi = getSystemState()->createXilRoi();
    if(unite_roi == NULL) {
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-145", FALSE, this);
        XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-433", FALSE, this);
        return NULL;
    }

    //
    //  If either of the ROI's is empty then the "unite" is just the other ROI.
    //  This will work even if both ROI's are empty
    //
    if(nothingValid(this)) {
        unite_roi->roiRect = roi->roiRect;
        unite_roi->roiBitmask = roi->roiBitmask;
        unite_roi->roiConvexRegion = roi->roiConvexRegion;
        return unite_roi;
    }
    if(nothingValid(roi)) {
        unite_roi->roiRect = roiRect;
        unite_roi->roiBitmask = roiBitmask;
        unite_roi->roiConvexRegion = roiConvexRegion;
        return unite_roi;
    }

    //
    //  If both of same type, do the unite in type      
    //
    if(roiRect.isValid() && roi->roiRect.isValid()) {
        if(roiRect.unite_disjoint(&(roi->roiRect),&(unite_roi->roiRect)) == XIL_SUCCESS) {
            unite_roi->roiRect.setValid(TRUE);
            return unite_roi;
        } else {
            unite_roi->destroy();
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-433", FALSE, this);
            return NULL;
        }
    }

    //
    // If not translate to rectlist before unite-ing
    //
    if(!roiRect.isValid()) {
        if(roiBitmask.isValid()) {
            if(roiRect.translateBitmask(&roiBitmask) == XIL_FAILURE) {
                XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-445", FALSE, this);
                return NULL;
            }
        } else {
            //
            //  We know that convex region must be valid
            //
            if(roiRect.translateConvexRegion(&roiConvexRegion) == XIL_FAILURE) {
                XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-445", FALSE, this);
                return NULL;
            }
        }
    }
    if(!roi->roiRect.isValid()) {
        if(roi->roiBitmask.isValid()) {
            if(roi->roiRect.translateBitmask(&roi->roiBitmask) == XIL_FAILURE) {
                XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-445", FALSE, this);
                return NULL;
            }
        } else {
            //
            //  We know that convex region must be valid
            //
            if(roi->roiRect.translateConvexRegion(&roi->roiConvexRegion) == XIL_FAILURE) {
                XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-445", FALSE, this);
                return NULL;
            }
        }
    }

    //
    //  They're both rectlists now
    //
    if(roiRect.unite_disjoint(&(roi->roiRect),&(unite_roi->roiRect)) == XIL_SUCCESS) {
        unite_roi->roiRect.setValid(TRUE);
        return unite_roi;
    }

    //
    //  Unite failed
    //
    unite_roi->destroy();
    XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-433", FALSE, this);
    return NULL;
}

//------------------------------------------------------------------------
//
//  Function:	XilRoi::unite_inplace()
//
//  Description: As a performance improvement, union the two ROI's storing the
//               result back in the second ROI.
//
//  MT-level:  <??????>
//	
//  Parameters:
//	
//	
//  Returns:
//	
//	
//  Side Effects:
//	
//	
//  Notes:
//	
//  Deficiencies/ToDo:
//        dtb 5/1/96 see intersect_inplace
//	
//------------------------------------------------------------------------
XilStatus
XilRoi::unite_inplace(XilRoi* unite_roi)
{
    //
    //  Update the version number prior to changing the contents of the ROI.
    //
    newVersion();
    
    XilRoi *roi = this->unite(unite_roi);
    unite_roi->roiRect = roi->roiRect;
    unite_roi->roiBitmask = roi->roiBitmask;
    unite_roi->roiConvexRegion = roi->roiConvexRegion;
    roi->destroy();
    return XIL_SUCCESS;
}

XilStatus
XilRoi::unite_disjoint_inplace(XilRoi* unite_roi)
{
    //
    //  Update the version number prior to changing the contents of the ROI.
    //
    newVersion();
    
    XilRoi *roi = this->unite_disjoint(unite_roi);
    unite_roi->roiRect = roi->roiRect;
    unite_roi->roiBitmask = roi->roiBitmask;
    unite_roi->roiConvexRegion = roi->roiConvexRegion;
    roi->destroy();

    return XIL_SUCCESS;
}

//----------------------------------------------------------------
//
// The following routines are for use by the XilRectList,
// XilBitmask and XilConvexRegionList
//
//----------------------------------------------------------------

//------------------------------------------------------------------------
//
//  Function:	XilRoi::getBoundingBox()
//
//  Description: Returns the bounding box of the first valid implementation encountered
//	
//  MT-level:  <??????>
//	
//  Parameters:
//	
//	
//  Returns:
//	
//	
//  Side Effects:
//	
//	
//  Notes:
//	
//  Deficiencies/ToDo:
//  TODO: maynard 2/26/96
//        consider making the bounding box data on the ROI. All implementation
//        should generate the same values, except maybe convex region which
//        could be slightly different due to float->int rounding.
//	
//------------------------------------------------------------------------
XilStatus
XilRoi::getIntBoundingBox(int*          clipx,
                          int*          clipy,
                          unsigned int* clip_xsize,
                          unsigned int* clip_ysize)
{
    if(roiRect.isValid()) {
        return roiRect.intBoundingBox(clipx, clipy,
                                      clip_xsize, clip_ysize);
    }

    if(roiBitmask.isValid()) {
        return roiBitmask.intBoundingBox(clipx, clipy,
                                         clip_xsize, clip_ysize);
    }

    if(roiConvexRegion.isValid()) {
        XiliRect* tmp;
        int clipx2,clipy2;

        //
        // Convex regions stores the bounding box as doubles.
        //
        tmp = roiConvexRegion.getBbox();
        // get as Integers
        tmp->get(clipx,clipy,&clipx2,&clipy2);
        *clip_xsize =(unsigned int)(clipx2-(*clipx)+1);
        *clip_ysize =(unsigned int)( clipy2-(*clipy)+1);
        return XIL_SUCCESS;
    }

    //
    //  Empty ROI
    //

    *clipx      = 0;
    *clipy      = 0;
    *clip_xsize = 0;
    *clip_ysize = 0;
    
    return XIL_SUCCESS;
}

XiliRect*
XilRoi::getBoundingBox()
{
    if(roiRect.isValid()) {
        return roiRect.getBbox();
    } else if(roiBitmask.isValid()) {
        return roiBitmask.getBbox();
    } else if(roiConvexRegion.isValid()) {
        return roiConvexRegion.getBbox();
    } else {
        //
        //  Return an empty rect.
        //
        return roiRect.getBbox();
    }
}


//------------------------------------------------------------------------
//
//  Function:	XilRoi::numRects()
//
//  Description: Return the number of rectangles in the rectlist implementation
//	
//  MT-level:  <??????>
//	
//  Parameters:
//	
//	
//  Returns:
//	
//	
//  Side Effects:
//	
//	
//  Notes:
//	
//  Deficiencies/ToDo:
//	
//------------------------------------------------------------------------
unsigned int
XilRoi::numRects()
{
    if(roiRect.isValid()) {
        return roiRect.numRects();
    }

    if(roiBitmask.isValid()) {
        if(roiRect.translateBitmask(&roiBitmask) == XIL_FAILURE) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-445", FALSE, this);
            return 0;
        }
        return roiRect.numRects();
    }

    if(roiConvexRegion.isValid()) {
        if(roiRect.translateConvexRegion(&roiConvexRegion) == XIL_FAILURE) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-445", FALSE, this);
            return 0;
        }
        return roiRect.numRects();
    }
    return 0;
}

unsigned int
XilRoi::numRegions()
{
    if(roiConvexRegion.isValid()) {
        return roiConvexRegion.getNumRegions();
    }

    if(roiRect.isValid()) {
        if(roiConvexRegion.translateRectList(&roiRect) == XIL_FAILURE) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-445", FALSE, this);
            return 0;
        }
        return roiConvexRegion.getNumRegions();
    }

    if(roiBitmask.isValid()) {
	//
	// TODO: dtb 4/3/96 change when convex regions
	// can translate bitmasks
	//
        if(roiRect.translateBitmask(&roiBitmask) == XIL_FAILURE) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-445", FALSE, this);
            return 0;
        }
        if(roiConvexRegion.translateRectList(&roiRect) == XIL_FAILURE) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-445", FALSE, this);
            return 0;
        }
        return roiConvexRegion.getNumRegions();
    }

    //
    //  Empty ROI
    //
    return 0;
}

//------------------------------------------------------------------------
//
//  Function:	XilRoi::getRectList()
//
//  Description: Return the rectlist associated with the ROI as a linked list of XiliRect*
//               Translate to rectlist implementation if necessary.
//	
//  MT-level:  <??????>
//	
//  Parameters:
//	
//	
//  Returns:
//	
//	
//  Side Effects:
//	
//	
//  Notes:
//	
//  Deficiencies/ToDo:
//	
//------------------------------------------------------------------------
XiliRectInt*
XilRoi::getRectList()
{
    if(roiRect.isValid()) {
        return roiRect.getRectList();
    }

    if(roiBitmask.isValid()) {
        if(roiRect.translateBitmask(&roiBitmask) == XIL_FAILURE) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-445", FALSE, this);
            return NULL;
        }
        return roiRect.getRectList();
    }

    if(roiConvexRegion.isValid()) {
        if(roiRect.translateConvexRegion(&roiConvexRegion) == XIL_FAILURE) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-445", FALSE, this);
            return NULL;
        }
        return roiRect.getRectList();
    } 

    //
    //  Empty ROI
    //
    return NULL;
}

//------------------------------------------------------------------------
//
//  Function:	XilRoi::getConvexRegionList()
//
//  Description: Return the convex region list representing the ROI.
//               Translate to the convex region implementation if necessary.
//	
//  MT-level:  <??????>
//	
//  Parameters:
//	
//	
//  Returns:
//	
//	
//  Side Effects:
//	
//	
//  Notes:
//	
//  Deficiencies/ToDo:
//	
//------------------------------------------------------------------------
XiliList<XiliConvexRegion>*
XilRoi::getConvexRegionList()
{
    if(roiConvexRegion.isValid()) {
        return roiConvexRegion.getRegionList();
    }

    if(roiRect.isValid()) {
        if(roiConvexRegion.translateRectList(&roiRect) == XIL_FAILURE) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-445", FALSE, this);
            return NULL;
        }
        return roiConvexRegion.getRegionList();
    } 

    if(roiBitmask.isValid()) {
        if(roiRect.translateBitmask(&roiBitmask) == XIL_FAILURE) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-445", FALSE, this);
            return NULL;
        }
        if(roiConvexRegion.translateRectList(&roiRect) == XIL_FAILURE) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-445", FALSE, this);
            return NULL;
        }
        return roiConvexRegion.getRegionList();
    }
    //
    //  Empty ROI
    //
    return NULL;
}

XiliConvexRegion*
XilRoi::getConvexRegion()
{
    //
    // This is only valid if there is only one
    // region in the roi.
    // 
   if(numRegions() == 1) {
       //
       //  This will work because numRegions will force a translation to
       //  the convex region implementation.
       //
	return roiConvexRegion.getRegion();
    }

    return NULL;
}

//------------------------------------------------------------------------
//
//  Function:	XilRoi::addConvexRegion()
//
//  Description: Add a convex region to the ROI.
//               If the implementation is a rectlist, translate it to the convex region implementation
//               unless the region you're adding is really an integer rect in disguise.
//	
//  MT-level:  <??????>
//	
//  Parameters:
//	
//	
//  Returns:
//	
//	
//  Side Effects:
//	
//	
//  Notes:
//	
//  Deficiencies/ToDo:
//	
//------------------------------------------------------------------------
XilStatus
XilRoi::addConvexRegion(XiliConvexRegion* region)
{

    if(roiConvexRegion.isValid()) {
        roiRect.setValid(FALSE);
        roiBitmask.setValid(FALSE);

        return roiConvexRegion.addConvexRegion(region);
    }

    if(roiRect.isValid()) {
        if(roiConvexRegion.translateRectList(&roiRect) == XIL_FAILURE) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-445", FALSE, this);
            return XIL_FAILURE;
        }
        roiBitmask.setValid(FALSE);
        roiRect.setValid(FALSE);

        return roiConvexRegion.addConvexRegion(region);
    }

    if(roiBitmask.isValid()) {
        if(roiRect.translateBitmask(&roiBitmask) == XIL_FAILURE) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-445", FALSE, this);
            return XIL_FAILURE;
        }
        if(roiConvexRegion.translateRectList(&roiRect) == XIL_FAILURE) {
            XIL_OBJ_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-445", FALSE, this);
            return XIL_FAILURE;
        }
        roiBitmask.setValid(FALSE);
        roiRect.setValid(FALSE);

        return roiConvexRegion.addConvexRegion(region);
    }

    //
    // This ROI is empty - Add to the convex region and make it valid
    //
    roiConvexRegion.setValid(TRUE);
    return roiConvexRegion.addConvexRegion(region);
}
