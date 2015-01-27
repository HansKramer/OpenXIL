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
//   File:	XilRoiPrivate.hh
//   Project:	XIL
//   Revision:	1.60
//   Last Mod:	10:20:48, 03/10/00
//  
//   Description:
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------

#ifdef _XIL_PRIVATE_INCLUDES

#include "XiliConvexRegion.hh"
#include "XiliList.hh"

#ifndef _XIL_ROI_HH

class XiliRoiBitmask;
class XiliRoiConvexRegion;

class XiliRoiRect {
public:
    //
    // Add the specified rectangle to the current rectlist representation of the ROI.
    //
    XilStatus	      addRect(int          x,
                              int          y,
                              unsigned int width,
                              unsigned int height);

    //
    // Fill in an XiliRoiRect with copied data and it's associated rectlist.
    //
    XilStatus         getCopyRoiRect(XiliRoiRect* copy);

    //
    // Return a reference to the current reclist representation of the ROI
    //
    XiliRectInt*      getRectList();

    //
    // Return number of rects in the rectlist
    //
    unsigned int      numRects();

    //
    // Translate the bitmask roi representation to a rectlist representation
    //
    XilStatus         translateBitmask(XiliRoiBitmask* bitmaskroi);

    //
    // Translate the convex region representation to a rectlist representation
    //
    XilStatus         translateConvexRegion(XiliRoiConvexRegion* convexregionroi);

    //
    // return the bounding box of the rectlist representation of the roi
    //
    XilStatus         intBoundingBox(int* x1,
                                     int* y1,
                                     unsigned int* xsize,
                                     unsigned int* ysize);
    //
    // An alternate method for retrieving the rectlist bbox
    //
    XiliRect*          getBbox();

    //
    // Add a bitmask image to the rectlist implementation of the roi
    //
    XilStatus         addImage(XilImage* image);

    //
    // Retrieve the rectlist implementation of the roi as a bitmask image
    //
    XilImage*         getAsImage();

    //
    // Subtract a rect from the rectlist implementation of the roi
    //
    XilStatus         subtractRect(int x,
				   int y,
				   unsigned int xsize,
				   unsigned int ysize);

    //
    // intersect two rectlist rois - the calling roi and other_rl. Return the
    // results in "intersect" 
    //
    XilStatus         intersect(XiliRoiRect* other_rl,
				XiliRoiRect* intersect);
    //
    // Unite "this" and "other_rl" rectlists - Return the results in "unite_rl"
    //
    XilStatus         unite(XiliRoiRect* other_rl,
			    XiliRoiRect* unite_rl);

    XilStatus         unite_disjoint(XiliRoiRect* other_rl,
				     XiliRoiRect* unite_rl);
    //
    // Translate "this" rectlist by x and y - Return the results in "unite_rl"
    //
    XilStatus         translate(int          x,
				int          y,
				XiliRoiRect* translation);
    //
    // translate "this" rectlist roi in place
    //
    XilStatus         translate_inplace(int x,
                                        int y);
    //
    // Scale "this" rectlist roi and return the results in "scale_copy"
    //
    XilStatus         scale(float xscale,
                            float yscale,
                            float xorigin,
                            float yorigin,
                            XiliRoiRect* scale_copy);
    //
    // Transpose "this" rectlist roi and return the results in "tpose_copy"
    //
    XilStatus          transpose(XilFlipType fliptype,
                                 float xorigin,
                                 float yorigin,
                                 XiliRoiRect* tpose_copy);

#if defined(_XIL_HAS_X11WINDOWS) || defined(_WINDOWS)
    //-----------------------------------------------------------------------
    //
    // X11/Windows Region supporting routines
    //
    //----------------------------------------------------------------------
    //
    // Add a Region to the reclist
    //
    XilStatus         addRegion(Region region);
    //
    // Retrieve the rectlist as a Region
    //
    Region            getAsRegion();
    //
    //----------------------------------------------------------------------
#endif /* _XIL_HAS_X11WINDOWS || _WINDOWS */
    

    //
    // Used to query validity of this implementation, or to set it
    //
    Xil_boolean       isValid() const;
    void              setValid(Xil_boolean valid_flag);

    //
    // Each roi implementation keeps track of the overall Roi
    //
    void              setCallingRoi(XilRoi* top_roi);

#ifdef DEBUG
    void              dump();
#endif

    //-----------------------------------------------------------------------
    //
    // Operators/Constructors/Destructors
    //
    //----------------------------------------------------------------------

    //
    // = operator; sets one rectlist from another
    //
    const XiliRoiRect& operator =(XiliRoiRect& from);

    //
    // Empty the contents of the rectlist without destroying it
    //
    XilStatus         clear();

                      XiliRoiRect(XilRoi* calling_roi);
                      ~XiliRoiRect();
private:
    //
    // Run through rectlist and join rects where appropriate - change to singleRect representation if possible
    //
    void              consolidateRects(); 
    //
    // Run through rectlist to bring bbox up to date
    //
    void              updateBoundingBox();
    //
    // Internal routine used to intersect two bands from two separate rects with overlap
    //
    XiliRectInt*      intersectBands(XiliRectInt* rect1, 
                                     XiliRectInt* rect2, 
                                     int y1, 
                                     int y2, 
                                     int* num_rects);

    //-----------------------------------------------------------------------
    //
    // Rectlist implementation of ROI - data members
    //
    //----------------------------------------------------------------------
    unsigned int        num_rects; // number of rectangles in rectlist

    //
    // TODO : maynard
    //        This should change to XiliRectInt. XilBox  holds more information than needed.
    //
    XiliRectInt            bbox; // high/low values in rectlist

    //
    // When num_rects is 1, store the rect in the singleRect static member for efficiency.
    //
    XiliRectInt            singleRect;

    //
    // The rectlist implementation - a linked list of XiliRectInts.
    // TODO: maynard
    //       the following will go away upon new implementation - see TODO below
    //
    XiliRectInt*           first;

    //
    // TODO: maynard
    //       when "first" goes away, the rectlist internal representation should be stored as
    //       and XiliList template. This will require some algorithm changes since the interface
    //       to the ROI should not change.
    //
    XiliList<XiliRectInt>*  rectList;

    //
    // The ROI that owns this rectlist. This is used to get access to the SystemState for errors
    //
    XilRoi*             myRoi;
    //
    // Indicates whether this implementation is currently valid
    //
    Xil_boolean         valid;

};

class XiliRoiBitmask {
public:
    //
    // Add a bitmask image to the bitmask implementation of the roi
    //
    XilStatus         addImage(XilImage* image);
    //
    // Retrieve the bitmask implementation of the roi as a bitmask image
    //
    XilImage*         getAsImage();

    //
    // Add the specified rectangle to the current bitmask representation of the ROI.
    //
    XilStatus	      addRect(int x, int y, unsigned int width, unsigned int height);
    //
    // Subtract the specified rectangle from the current bitmask respresentation of the ROI
    //
    XilStatus         subtractRect(int x,
				   int y,
				   unsigned int xsize,
				   unsigned int ysize);
    //
    // Retrieve the bounding box of the bitmask respresentation of the roi
    //
    XilStatus         intBoundingBox(int* x1,
                                     int* y1,
                                     unsigned int* xsize,
                                     unsigned int* ysize);
    //
    // An alternate method for retrieving the bounding box of the roi
    //
    XiliRect*          getBbox();

    //
    // Retrieve the data member that represents the bitmask data
    //
    XilImage*         getBitmask();
    
    //
    // Fill in an XiliRoiBitmask with copied data and it's associated bitmask
    //
   XilStatus         getCopyRoiBitmask(XiliRoiBitmask* copy);

    //
    // Translate a rectlist implementation of the roi to a bitmask representation
    //
    XilStatus         translateRectList(XiliRoiRect* rl);
    //
    // Intersect "this" bitmask and "other_bm" store the results in "intersect"
    //
    XilStatus         intersect(const XiliRoiBitmask* other_bm,
                                XiliRoiBitmask* intersect);
    //
    // Unit "this" bitmask and "other_bm" and store the results in "unite_bm".
    //
    XilStatus         unite(const XiliRoiBitmask* other_bm,
			    XiliRoiBitmask* unite_bm);


    //
    // Used to query validity of this implementation, or to set it
    //
    Xil_boolean       isValid() const;
    void              setValid(Xil_boolean valid_flag);

    //
    // Each roi implementation keeps track of the overall Roi
    //
    void              setCallingRoi(XilRoi* top_roi);
    //-----------------------------------------------------------------------
    //
    // Operators/Constructors/Destructors
    //
    //----------------------------------------------------------------------

    //
    // = operator; sets one bitmask from another
    //
    const XiliRoiBitmask& operator =(XiliRoiBitmask& from);
    //
    // Empty the contents of the rectlist without destroying it
    //
    XilStatus         clear();

                      XiliRoiBitmask(XilRoi* calling_roi);
                      ~XiliRoiBitmask();
private:
    //-----------------------------------------------------------------------
    //
    // Bitmask implementation of ROI - data members
    //
    //----------------------------------------------------------------------
    //
    // 1-band, BIT data
    // origin stores x,y location of roi bbox
    // image w,h store dimensions of roi bbox
    //
    XilImage*           bitmask;

    //
    // TODO : maynard
    //        This should change to XiliRectInt. XilBox  holds more information than needed.
    //
    XiliRectInt              bbox;

    //
    // The ROI that owns this Bitmask.  This is used to get access to the
    // XilSystemState for errors 
    //
    XilRoi*             myRoi;

    //
    // Indicates whether this implementation is currently valid
    //
    Xil_boolean         valid;

};

class XiliRoiConvexRegion {
public:
    //
    // Retrieve the list of convex regions representing the ROI
    //
    XiliList<XiliConvexRegion>* getRegionList(); 
    //
    // Translate the bitmask respresentation of the ROI to a convex region representation
    //
    XilStatus                   translateBitmask(XiliRoiBitmask* bmr);
    //
    // Translate the rectlist representation of the ROI to a convex region representation
    //
    XilStatus                   translateRectList(XiliRoiRect* rl);

    //
    // Add the specified rectangle to the convex region representation of the ROI
    //
    XilStatus	                addRect(double x,
                                        double y,
                                        double width,
                                        double height);


    //
    // Subtract the specified rectangle from the convex region representation of the ROI
    //
    XilStatus                   subtractRect(int x,
					     int y,
					     unsigned int xsize,
					     unsigned int ysize);
    //
    // Retrieve the bounding box of the convex region ROI - enlarged to ints
    //
    XiliRect*                   getBbox();

    //
    // Fill in an XiliRoiConvexRegion with copied data and it's associated
    // convex region list 
    //
    XilStatus                   getCopyRoiConvexRegion(XiliRoiConvexRegion* copy);

    //
    // Translate "this" ConvexRegion roi by x and y - Return the results
    // in translation.
    //
    XilStatus                   translate(double               x,
					  double               y,
					  XiliRoiConvexRegion* translation);
    
    //
    // translate "this" rectlist roi in place
    //
    XilStatus                   translate_inplace(double x,
						  double y);
    
    //
    // Intersect "this" convex region list and "other_cr" and store the
    // results in "intersect" 
    //
    XilStatus                   intersect(XiliRoiConvexRegion* other_cr,
					  XiliRoiConvexRegion* intersect);
    //
    // Intersect "this" convex region list and "rectlist" and store the
    // results in "intersect"
    //
    // TODO: maynard 2/26/96
    //       NOTE that this simplified case of intersecting two convex region
    //       lists may not be worth a separate implementation
    //
    XilStatus                   intersect(XiliRoiRect*         rectlist,
					  XiliRoiConvexRegion* intersect);


    //
    // intersect two region rois - the calling roi and other_rl. Return the
    // results in "intersect".
    // Put the results of intersect in pixel extent space on the fly during
    // the intersection.
    // Currently this only works for convex region lists that contain only
    // rectangular convex regions.
    //
    XilStatus         extentIntersect(XiliRoiConvexRegion* other_rl,
                                      XiliRoiConvexRegion* intersect);
    //
    // Unite "this" convex region loist and "other_cr" and store the results
    // in "unite_cr"
    //
    XilStatus                   unite(XiliRoiConvexRegion* other_cr,
				      XiliRoiConvexRegion* unite_cr);

    //
    // Add a single convex region to an existing convex region list.
    // This does not connect up to the API
    //
    XilStatus                   addConvexRegion(XiliConvexRegion* region);

    //
    // Evaluates the convex region list to see if it could be represented more
    // simply as integer rectangles
    //
    Xil_boolean                 isSimple();

    //
    // How many regions are in the Roi
    //
    unsigned int           getNumRegions();

    //
    // Return a pointer to the single region
    //
    XiliConvexRegion*      getRegion();

    //
    // Used to query validity of this implementation, or to set it
    //
    Xil_boolean            isValid() const;
    void                   setValid(Xil_boolean valid_flag);

    
    //
    // Each roi implementation keeps track of the overall Roi
    //
    void                   setCallingRoi(XilRoi* top_roi);
    //-----------------------------------------------------------------------
    //
    // Operators/Constructors/Destructors
    //
    //----------------------------------------------------------------------

    //
    // = operator; sets one convex region ROI from another
    //
    const XiliRoiConvexRegion& operator =(XiliRoiConvexRegion& from);

    //
    // Empties convex region ROI without destroying it
    // 
    XilStatus         clear();

                          XiliRoiConvexRegion(XilRoi* calling_roi);
                          ~XiliRoiConvexRegion();
private:
    
    void              updateBoundingBox(double x1,
					double y1,
					double x2,
					double y2);

    //-----------------------------------------------------------------------
    //
    // Convex region implementation of ROI - data members
    //
    // A convex region is allowed to consist of up to 8 pts
    //
    //----------------------------------------------------------------------
    //
    //
    XiliRectDbl                         bbox;

    //
    // Indicates whether there is only one region in the ROI
    //
    unsigned int                        num_regions;
    
    //
    // When there is only one convex region representing the ROI, it is stored
    // in the following XiliConvexRegion object.n
    //
    XiliConvexRegion                    region;
    
    //
    // Indicates whether the convex region could be represented as a list
    // of integer rectangles (eg isSimple()) such as just after a translation
    // from rectlist.
    //
    Xil_boolean                          simpleRegions;

    //
    // When the roi contains more than one convex reigon,
    // the data is store in the following list
    //
    XiliList<XiliConvexRegion>           regionList; 
    
    //
    // The ROI that owns this convex region list.
    // This is used to get access to the SystemState for errors
    //
    XilRoi*                              myRoi;

    //
    // Indicates whether this implementation is currently valid
    //
    Xil_boolean                          valid;

};

#endif //_XIL_ROI_HH
#endif

#ifdef _XIL_PRIVATE_DATA

public:
    //-----------------------------------------------------------------------
    //
    // In support of API calls on the ROI
    //
    //----------------------------------------------------------------------
    //
    // add values that are non-zero in the passed image to the ROI.
    // The image is expected to be of type XIL_BIT.
    //
    XilStatus              addImage(XilImage*  image);

    //
    // add the specified rectangle to the ROI
    //
    XilStatus              addRect(int          x,
				   int          y,
				   unsigned int xsize,
				   unsigned int ysize);

    XilStatus              addRect(double x,
				   double y,
				   double xsize,
				   double ysize);

#if defined(_XIL_HAS_X11WINDOWS) || defined(_WINDOWS)
    //
    // add the specified X Region to the ROI
    //
    XilStatus              addRegion(Region region);
#endif /* _XIL_HAS_X11WINDOWS || _WINDOWS */

    //
    // return a new ROI that is the result of an affine transformation of
    // this ROI with the specified matrix
    //
    XilRoi*                affine(float* matrix);

    //
    // return an XIL_BIT image which represents the ROI.  The image needs 
    // to encompass the entire extent of the ROI, with pixels within the 
    // ROI set to 1, pixels outside of the ROI set to 0.
    //
    XilImage*		   getAsImage();

#if defined(_XIL_HAS_X11WINDOWS) || defined(_WINDOWS)
    //
    // return an X Region which represents the ROI
    //
    Region		   getAsRegion();
#endif /* _XIL_HAS_X11WINDOWS || _WINDOWS */

    //
    // return a new ROI which is the result of an intersection between
    // the current ("this") ROI and the passed ROI
    //
    XilRoi*		   intersect(XilRoi* roi);
    //
    // used to intersect "this" and roi, placing results in dest
    //
    XilStatus		   intersect(XilRoi* roi,XilRoi* dest);
    //
    // Intersect "this" roi and "dest_roi" filling "dest_roi" with the results
    //
    XilStatus		   intersect_inplace(XilRoi* dest_roi);

    //
    // Intersect "this" roi and "dest_roi" filling "dest_roi" with the results
    // But this is valid only for convex region ROI's and it will extend
    // the resulting roi to extent space while it does the intersection.
    //
    XilStatus		   intersect_extent(XilRoi* dest_roi);

    //
    // return a new ROI that is the result of a rotation of this ROI about
    // the specified origin
    //
    XilRoi*		   rotate(float angle,
				  float xorigin,
				  float yorigin);

    //
    // return a new ROI that is the result of a scaling of this ROI about
    // the specified origin
    //
    XilRoi*       	   scale(float  xscale,
				 float  yscale,
				 float  xorigin,
				 float  yorigin);


    //
    // remove the specified rectangle from the ROI
    //
    XilStatus		   subtractRect(int          x,
					int          y,
					unsigned int xsize,
					unsigned int ysize);

    //
    //  Construct a new ROI that is the result of a translation of this ROI by
    //  the specified x and y amounts.
    //
    XilRoi*		   translate(double x,
				     double y);
    XilRoi*		   translate(int   x,
				     int   y);
    //
    //  Translate this ROI in place.
    //
    XilStatus		   translate_inplace(int x,
                                             int y);

    XilStatus		   translate_inplace(double x,
                                             double y);

#ifdef DEBUG
    void                   dump();
#endif

    //
    // return a new ROI that is the result of transposing this ROI about
    // the specified origin
    //
    XilRoi*		   transpose(XilFlipType fliptype,
				     float       xorigin,
				     float       yorigin);


    //
    // return a new ROI that is the union of this ROI with the specified ROI
    //
    XilRoi* 	           unite(XilRoi* roi);

    //
    // Unite this roi and leave the results in roi
    //
    XilStatus              unite_inplace(XilRoi* roi);


    //
    // return a new ROI that is the union of this ROI with the specified ROI
    //
    XilRoi* 	           unite_disjoint(XilRoi* roi);

    //
    // Unite this roi and leave the results in roi
    //
    XilStatus              unite_disjoint_inplace(XilRoi* roi);


    //-----------------------------------------------------------------------
    //
    // In support of internal manipulation of the ROI
    //
    //----------------------------------------------------------------------
    //
    // return a new ROI which is a copy of this ROI
    //
    XilObject*             createCopy();

    //
    // Retrieve the bounding box of the ROI
    //
    XilStatus              getIntBoundingBox(int*          clipx,
                                             int*          clipy,
                                             unsigned int* clipxsize,
                                             unsigned int* clipysize);

    XiliRect*              getBoundingBox();

    //
    // return the number of rects in the rectlist version of the ROI
    //
    unsigned int           numRects();

    //
    // return the number of regions in the convex region version of the ROI
    //
    unsigned int           numRegions();

    //
    // Indicate whether the ROI contains any info (or is it newly constructed)
    //
    Xil_boolean            isValid()
    {
        return
            roiRect.isValid() ||
            roiBitmask.isValid() ||
            roiConvexRegion.isValid() ? TRUE : FALSE;
    }

    Xil_boolean            isConvexRegionValid()
    {
        return roiConvexRegion.isValid();
    }

    //
    // Used by XilRectList
    //
    Xil_boolean            isRectValid()
    {
        return roiRect.isValid();
    }

    //
    // return the rectlist representing the ROI
    //
    XiliRectInt*              getRectList();

    //
    // return the convex region list of the ROI
    //
    XiliList<XiliConvexRegion>*      getConvexRegionList();

    //
    // return the single convex region of the ROI
    //
    XiliConvexRegion*                getConvexRegion();

    //
    // Add a convex region to the ROI
    //
    XilStatus              addConvexRegion(XiliConvexRegion* region);

    //-----------------------------------------------------------------------
    //
    // Operators/Constructors/Destructors
    //
    //----------------------------------------------------------------------
    const XilRoi& operator =(XilRoi& from);

    //
    //  This routine is used to reset the data in a roi to a just new state
    //
    XilStatus              clear();

    //
    //  Special constructor for XilOp.  This permits a ROI to be a member of
    //  XilOp since very executed op requires a ROI.
    //
                           XilRoi();

    //
    //  Standard CONSTRUCTOR/DESTRUCTOR
    //
                           XilRoi(XilSystemState* system_state);
   	                   ~XilRoi();
				   
private:
    Xil_boolean             nothingValid(XilRoi*);                       

    //-----------------------------------------------------------------------
    //
    // ROI - data members
    //
    //----------------------------------------------------------------------


    //
    // Internal representations of the ROI
    //
    XiliRoiRect              roiRect;
    XiliRoiBitmask           roiBitmask;
    XiliRoiConvexRegion      roiConvexRegion;
    
#endif  // _XIL_PRIVATE_DATA


