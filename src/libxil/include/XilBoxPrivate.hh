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
//  File:	XilBoxPrivate.hh
//  Project:	XIL
//  Revision:	1.25
//  Last Mod:	10:21:57, 03/10/00
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
#pragma ident	"@(#)XilBoxPrivate.hh	1.25\t00/03/10  "
#ifdef _XIL_PRIVATE_INCLUDES

#include "XiliRect.hh"

#endif

#ifdef _XIL_PRIVATE_DATA
public:
    //
    //  XilBox Constructors
    //

    //
    //  Initialize using corners 
    //
                         XilBox(int init_x1,
                                int init_y1,
                                int init_x2,
                                int init_y2)
    {
        //
        //  Convert corners to rect...
        //
        x     = init_x1;
        y     = init_y1;
        xSize = init_x2 - init_x1 + 1;
        ySize = init_y2 - init_y1 + 1;

        //
        //  NULL the private data pointer, indicate that storage info is
        //  not initialized yet and mark the box as valid.
        //
        privateData    = NULL;
        storageSetFlag = FALSE;
        isValidFlag    = TRUE;
    }
    

    //
    //  Initialize using a rectangle
    //
                         XilBox(int          init_x,
                                int          init_y,
                                unsigned int init_xsize,
                                unsigned int init_ysize)
    {
        //
        //  Store the rectangle
        //
        x     = init_x;
        y     = init_y;
        xSize = init_xsize;
        ySize = init_ysize;

        //
        //  NULL the private data pointer, indicate that storage info is
        //  not initialized yet and mark the box as valid.
        //
        privateData    = NULL;
        storageSetFlag = FALSE;
        isValidFlag    = TRUE;
    }

    //
    //  Initialize using an XiliRect
    //
                         XilBox(XiliRect* rect)
    {
        //
        //  NULL the private data pointer, indicate that storage info is
        //  not initialized yet and mark the box as valid.
        //
        privateData    = NULL;
        storageSetFlag = FALSE;
        isValidFlag    = TRUE;

        //
        //  Just get the rect as a Box...
        //
        rect->get(this);
    }

    //
    //  Initialize using storage info
    //
                         XilBox(int          init_x,
                                int          init_y,
                                unsigned int init_xsize,
                                unsigned int init_ysize,
                                int          init_band)
    {
        //
        //  Store the rectangle and storage info.
        //
        storageX     = x     = init_x;
        storageY     = y     = init_y;
        storageXSize = xSize = init_xsize;
        storageYSize = ySize = init_ysize;

        storageBand  = init_band;

        //
        //  NULL the private data pointer, indicate that storage info is
        //  now initialized and mark the box as valid.
        //
        privateData    = NULL;
        storageSetFlag = TRUE;
        isValidFlag    = TRUE;
    }

                         XilBox(XilBox& init_box)
    {
        *this = init_box;
    }
                         XilBox(XilBox* init_box)
    {
        *this = *init_box;
    }

    //
    //  Provide the ability to create an array of these
    //
                         XilBox()
    {
        //
        //  Set values to indicate the box is uninitialized
        //
        privateData    = NULL;
        storageSetFlag = FALSE;
        isValidFlag    = FALSE;
    }

    //
    //  Methods for obtaining the mebmers individually.
    //
    int                  getX()
    {
        return x;
    }

    int                  getY()
    {
        return y;
    }

    unsigned int         getXSize()
    {
        return xSize;
    }

    unsigned int         getYSize()
    {
        return ySize;
    }
    
    //
    //  Methods for obtaining the storage mebmers individually.
    //
    int                  stGetX()
    {
        if(storageSetFlag) {
            return storageX;
        } else {
            return x;
        }
    }

    int                  stGetY()
    {
        if(storageSetFlag) {
            return storageY;
        } else {
            return y;
        }
    }

    unsigned int         stGetXSize()
    {
        if(storageSetFlag) {
            return storageXSize;
        } else {
            return xSize;
        }
    }

    unsigned int         stGetYSize()
    {
        if(storageSetFlag) {
            return storageYSize;
        } else {
            return ySize;
        }
    }
    
    unsigned int         stGetBand()
    {
        if(storageSetFlag) {
            return storageBand;
        } else {
            return 0;
        }
    }
    
    //
    //  Set the information in the box as corners.
    //
    void                 setAsCorners(int  new_x1,
                                      int  new_y1,
                                      int  new_x2,
                                      int  new_y2)
    {
        //
        //  Convert corners to rect...
        //
        x     = new_x1;
        y     = new_y1;
        xSize = new_x2 - new_x1 + 1;
        ySize = new_y2 - new_y1 + 1;

        //
        //  Indicate storage is no longer valid, but the box info is now
        //  valid...
        //
        isValidFlag    = TRUE;
        storageSetFlag = FALSE;
    }

    //
    //  Set the information in the box as a rectangle.
    //
    void                 setAsRect(int           new_x,
                                   int           new_y,
                                   unsigned int  new_xsize,
                                   unsigned int  new_ysize)
    {
        x         = new_x;
        y         = new_y;
        xSize     = new_xsize;
        ySize     = new_ysize;

        //
        //  Indicate storage is no longer valid, but the box info is now
        //  valid...
        //
        isValidFlag    = TRUE;
        storageSetFlag = FALSE;
    }

    //
    //  Get and Set the secondary box that represents the actual storage 
    //  to aquire for the area described by the box.
    //
    void                 getStorageLocation(int*          ret_x,
                                            int*          ret_y,
                                            unsigned int* ret_xsize,
                                            unsigned int* ret_ysize,
                                            int*          ret_band)
    {
        if(!storageSetFlag) {
            *ret_x     = x;
            *ret_y     = y;
            *ret_xsize = xSize;
            *ret_ysize = ySize;
            *ret_band  = 0;
        } else {
            *ret_x     = storageX;
            *ret_y     = storageY;
            *ret_xsize = storageXSize;
            *ret_ysize = storageYSize;
            *ret_band  = storageBand;
        }
    }

    
    void                 setStorageLocation(int           new_x,
                                            int           new_y,
                                            unsigned int  new_xsize,
                                            unsigned int  new_ysize,
                                            int           new_band)
    {
        storageX     = new_x;
        storageY     = new_y;
        storageXSize = new_xsize;
        storageYSize = new_ysize;
        storageBand  = new_band;

        storageSetFlag = TRUE;
    }

    
    void                 getStorageAsCorners(int* ret_x1,
                                             int* ret_y1,
                                             int* ret_x2,
                                             int* ret_y2,
                                             int* ret_band)
    {
        if(! storageSetFlag) {
            *ret_band  = 0;
            *ret_x1    = x;
            *ret_y1    = y;
            *ret_x2    = *ret_x1 + xSize - 1;
            *ret_y2    = *ret_y1 + ySize - 1;
        } else {
            *ret_band  = storageBand;
            *ret_x1    = storageX;
            *ret_y1    = storageY;
            *ret_x2    = *ret_x1 + storageXSize - 1;
            *ret_y2    = *ret_y1 + storageYSize - 1;
        }
    }

    void                 setStorageAsCorners(int new_x1,
                                             int new_y1,
                                             int new_x2,
                                             int new_y2,
                                             int new_band)
    {
        storageX       = new_x1;
        storageY       = new_y1;
        storageXSize   = new_x2 - new_x1 + 1;
        storageYSize   = new_y2 - new_y1 + 1;
        storageBand    = new_band;

        storageSetFlag = TRUE;
    }

    //
    //  Copy the storage information from the given box but only if the given
    //  box has storage set on it.
    //
    void                 copyStorageLocation(XilBox* box)
    {
        if(box->storageSetFlag) {
            storageSetFlag   = TRUE;
            storageX         = box->storageX;
            storageY         = box->storageY;
            storageXSize     = box->storageXSize;
            storageYSize     = box->storageYSize;
            storageBand      = box->storageBand;
        }
    }

    //
    //  Offset both the front and back box by given x and y.
    //
    void                 offset(int  x_offset,
                                int  y_offset)
    {
        x += x_offset;
        y += y_offset;

        if(storageSetFlag) {
            storageX += x_offset;
            storageY += y_offset;
        }
    }

    //
    //  Offset only the front box by given x and y.
    //
    void                 offsetFront(int  x_offset,
                                     int  y_offset)
    {
        x += x_offset;
        y += y_offset;
    }

    //
    //  Offset only the storage box by given x and y -- remember, if the
    //  storage box is not set the front box is the storage box.
    //
    void                 offsetStorage(int  x_offset,
                                       int  y_offset)
    {
        if(storageSetFlag) {
            storageX += x_offset;
            storageY += y_offset;
        } else {
            x += x_offset;
            y += y_offset;
        }
    }

    //
    //  Clip this box against the given box.
    //
    //  TODO: 4/29/96 jlf  This is wrong -- it's really an intersect.
    //
    Xil_boolean          clip(int clip_x1,
                              int clip_y1,
                              int clip_x2,
                              int clip_y2);
    
    Xil_boolean          clip(int           clip_x1,
                              int           clip_y1,
                              unsigned int  clip_xsize,
                              unsigned int  clip_ysize);

    Xil_boolean          clip(XilBox* box);

    //
    //  Intersect this box with the given box.
    //
    Xil_boolean          intersect(XilBox* box);

    //
    //  Return whether this box intersects the given box.
    //
    //  Does not modify the contents of this box.
    //
    Xil_boolean          intersects(XilBox* box);

    //
    //  Calls to set/get privateData
    //
    void                 setPrivateData(void* data)
    {
        privateData = data;
    }

    void*                getPrivateData()
    {
        return privateData;
    }

    //
    //  Mark storage information as invalid.
    //
    void                 invalidateStorageInfo()
    {
        storageSetFlag = FALSE;
    }

    //
    //  Set the tag associated with this box.
    //
    void                 setTag(void* new_tag)
    {
        tag = new_tag;
    }

    //
    //  Is the storage box set?
    //
    Xil_boolean          isStorageSet()
    {
        return storageSetFlag;
    }

    //
    //  Is there valid information in this box?
    //
    Xil_boolean          isValid()
    {
        return isValidFlag;
    }

    //
    //  Operator for less than test.
    //
    int                  operator < (XilBox& rval)
    {
        return (xSize < rval.xSize || ySize < rval.ySize) ? TRUE : FALSE;
    }

    //
    //  Operator for setting this box from a rect.
    //
    XilBox&              operator = (XiliRect& rect)
    {
        //
        //  Just get the rect as a Box...
        //
        rect.get(this);

        return *this;
    }

    //
    //  Is the box empty -- i.e. it represents no pixels.
    //
    Xil_boolean          isEmpty()
    {
        return (xSize == 0 || ySize == 0) ? TRUE : FALSE;
    }

    //
    //  Dump the contents of the box to a single line on stderr
    //
    void                 dump()
    {
        if(storageSetFlag) {
            fprintf(stderr, "box %7p: (%2d %2d %2d %2d : %dX%d) (%2d %2d %2dX%2d %2d -- %d) %p\n",
                    this,
                    x, y, x+xSize-1, y+ySize-1,
                    xSize, ySize,
                    storageX, storageY, storageXSize, storageYSize, storageBand,
                    storageSetFlag,
                    privateData);
        } else {
            fprintf(stderr, "box %7p: (%2d %2d %2d %2d : %dX%d) NO_STORE %p\n",
                    this,
                    x, y, x+xSize-1, y+ySize-1,
                    xSize, ySize,
                    storageX, storageY, storageXSize, storageYSize, storageBand,
                    storageSetFlag,
                    privateData);
        }
    }

private:
    //
    //  Rectangle
    //
    int           x;
    int           y;
    unsigned int  xSize;
    unsigned int  ySize;

    //
    //  Storage
    //
    int           storageX;
    int           storageY;
    int           storageXSize;
    int           storageYSize;
    int           storageBand;

    //
    //  Inidicate what's valid.
    //
    Xil_boolean   isValidFlag;
    Xil_boolean   storageSetFlag;

    //
    //  Arbitrary box tag
    //
    void*          tag;

    //
    //  Private data pointer used to store information initially for geometric
    //  operations so that we can store the convex region associated with the
    //  box for later use. 
    //
    void*          privateData; 
#endif // _XIL_PRIVATE_DATA
