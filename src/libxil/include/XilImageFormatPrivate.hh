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
//  File:	XilImageFormatPrivate.hh
//  Project:	XIL
//  Revision:	1.22
//  Last Mod:	10:21:05, 03/10/00
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
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------

#ifdef _XIL_PRIVATE_DATA
public:
    //
    //  Required virtual functions from XilObject
    //
    XilObject*      createCopy();

    //
    //  Test whether two formats are the same.
    //
    int             operator == (XilImageFormat& rval)
    {
        return (xSize     == rval.xSize  &&
                ySize     == rval.ySize  &&
                nBands    == rval.nBands &&
                dataType  == rval.dataType) ? TRUE : FALSE;
    }

    //
    //  These are required for deferrable object.  Since you can't do
    //  operations with XilImageFormat objects, they should never be called.
    //
    XilRoi*         getGlobalSpaceRoi()
    {
        return NULL;
    }

    XilRoi*         getGlobalSpaceRoiWithDoublePrecision()
    {
        return NULL;
    }

    XilRoi*         getExtentGlobalSpaceRoi()
    {
        return NULL;
    }

    XiliRect*       getGlobalSpaceRect()
    {
        return NULL;
    }

    unsigned int    getNumTiles()
    {
        return 1;
    }


                    XilImageFormat(XilSystemState* system_state,
                                   unsigned int    x_size, 
                                   unsigned int    y_size, 
                                   unsigned int    num_bands,
                                   XilDataType     data_type,
                                   XilObjectType   object_type = XIL_IMAGE_TYPE);

                    XilImageFormat(XilSystemState* system_state,
                                   XilImageFormat* image_format,
                                   XilObjectType   object_type = XIL_IMAGE_TYPE);

protected:
    unsigned int    xSize;
    unsigned int    ySize;
    unsigned int    nBands;
    XilDataType     dataType;
    XilColorspace*  colorspace;
    float           xPixelSize;
    float           yPixelSize;

                    XilImageFormat(XilSystemState* system_state,
                                   XilObjectType   object_type = XIL_IMAGE_TYPE);
                    ~XilImageFormat();
    
#endif // _XIL_PRIVATE_DATA
