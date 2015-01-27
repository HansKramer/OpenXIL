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
//  File:	ColorCorrect.cc
//  Project:	XIL
//  Revision:	1.8
//  Last Mod:	10:13:37, 03/10/00
//
//  Description:
//	Using the color space (profile) array passed in
//      color correct the images through that profile chain.
//	
//	
//  MT-level:  Safe
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)ColorCorrect.cc	1.8\t00/03/10  "

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <string.h>
#include <kcms/kcs.h>
#include "XilDeviceManagerComputeSUNWkcms.hh"

//#define KCMS_DEBUG 1

//
// A way to convert a default color space to a
// a profile name. The order of the names matches
// the #defines used for these at the GPI level
//
static char* cspace_to_profile[] = {
    "NotUsed",            // 0 XIL_CS_INVALID
    "kcmsSUNWRGBL",       // 1 XIL_CS_RGBLINEAR
    "kcmsEKRGB709",       // 2 XIL_CS_RGB709
    "kcmsSUNWPhotoYCC",   // 3 XIL_CS_PHOTOYCC
    "kcmsSUNWYCC601",     // 4 XIL_CS_YCC601
    "kcmsSUNWYCC709",     // 5 XIL_CS_YCC709
    "kcmsSUNWYlinear",    // 6 XIL_CS_YLINEAR
    "kcmsSUNWYCC601L",    // 7 XIL_CS_Y601
    "kcmsSUNWYCC709L",    // 8 XIL_CS_Y709
    "kcmsSUNWCMY",        // 9 XIL_CS_CMY
    "kcmsSUNWCMYK"        // 10 XIL_CS_CMYK
};

//
// Used to save information about the colorspace
// list.
//
typedef struct {
    KcsProfileId  completeProfile;
    Xil_boolean   srcRGB;
    Xil_boolean   dstRGB;
} KCMSComputeData;

//
// Preprocess the colorspace list to arrive at a complete
// connected profile
//
XilStatus
XilDeviceManagerComputeSUNWkcms::ColorCorrectPreprocess(XilOp*        op,
							unsigned      ,
							XilRoi*       ,
							void**        compute_data,
							unsigned int* )
{
    XilImage*          dest = op->getDstImage(1);
    XilImage*          src = op->getSrcImage(1);
    
    XilColorspaceList* cspaceList; 
    op->getParam(1, (void**)&cspaceList);

    //
    // Has the information in the list already been used
    // if so use the cached version.
    //
    KCMSComputeData* kcms_data = (KCMSComputeData*)cspaceList->getCachedData();
    if(kcms_data != NULL) {
	*compute_data = kcms_data;

#ifdef KCMS_DEBUG
	fprintf(stderr, "using cached data %p %d %d %d\n",
		kcms_data, kcms_data->completeProfile,
		kcms_data->srcRGB, kcms_data->dstRGB);
#endif    
	return XIL_SUCCESS;
    }

    KcsProfileId  completeProfile;

    //
    // Load the profiles in the list
    //
    int           nspaces = cspaceList->getNumColorspaces();
    if(nspaces == 0) {
        //
        // The application might be foolish enough to do this
        // set an error and return. The op should catch this
        // but just to be on the safe side.
        //
        XIL_ERROR(dest->getSystemState(), XIL_ERROR_RESOURCE, "di-372", TRUE);
	return XIL_FAILURE;
    }
    
    KcsProfileId* profileSequence = new KcsProfileId[nspaces];
    if(profileSequence == NULL) {
	XIL_ERROR(dest->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
	return XIL_FAILURE;
    }
 
    //
    // If there are 2 colorspaces, they are not of type
    // XIL_COLORSPACE_NAME (these are handled in the op via color_convert)
    //
    for(int i=0; i<nspaces; i++) {
	//
	// Load the profiles
	//
	KcsProfileDesc*   desc;
	XilColorspace*    cspace = cspaceList->getColorspace(i);
	XilColorspaceType type = cspace->getColorspaceType();

	if(type != XIL_COLORSPACE_KCS_ID) {
	    desc = new KcsProfileDesc;
	    if(desc == NULL) {
		XIL_ERROR(dest->getSystemState(),
                          XIL_ERROR_RESOURCE,
                          "di-1",
                          TRUE);
		delete profileSequence;
		return XIL_FAILURE;
	    }
	    desc->type = KcsSolarisProfile;

	    if(type == XIL_COLORSPACE_NAME) {
		unsigned int opcode = cspace->getOpcode();
		desc->desc.solarisFile.fileName =
		    strdup(cspace_to_profile[cspace->getOpcode()]);
	    } else {
		desc->desc.solarisFile.fileName =
		    strdup((char*)cspace->getColorspaceData());
	    }
	    desc->desc.solarisFile.hostName = NULL;
	    desc->desc.solarisFile.oflag = O_RDONLY;
	    desc->desc.solarisFile.mode = 0;
				
	    if(KcsLoadProfile(&profileSequence[i],
                              desc,
                              KcsLoadWhenNeeded) != KCS_SUCCESS) {
		//
		// Free all those profiles we already loaded
		//
		if(i != 0) {
		    for(int j=0; j<(i+1); j++) {
			if(cspaceList->getType() != XIL_COLORSPACE_KCS_ID) {
			    KcsFreeProfile(profileSequence[j]);
			}
		    }
		}
		delete desc;
		delete profileSequence;
                
                XIL_ERROR(dest->getSystemState(),
                          XIL_ERROR_RESOURCE,
                          "di-381",
                          TRUE);
		return XIL_FAILURE;
	    }

            //
	    // Delete the description we newed...
            //
	    free(desc->desc.solarisFile.fileName);
	    delete desc;
	} else {
	    profileSequence[i] = (KcsProfileId)cspace->getColorspaceData();
	}
    }

    if(nspaces == 1) {
        //
        // If 1 colorspace is passed on the list, it's assumed that
        // this is a device link (or a previously concatenated list of
        // profiles/colorspaces). In which case we need not connect the
        // profiles in the list.
        //
        completeProfile = profileSequence[0];
    } else {
        //
        // Connect the sequence of profiles together
        //

        //
        // TODO : Will not work with Solaris-64. But KCMS does not
        //        like anything but unsigned long at this point.
        //
        unsigned long failedProfile;

        if(KcsConnectProfiles(&completeProfile,
                              nspaces,
                              profileSequence,
                              KcsOpForward+KcsContImage,
                              &failedProfile) != KCS_SUCCESS) {
            //
            // Profile connection failed. Free up profiles.
            //
            for(int j=0; j<nspaces; j++) {
                XilColorspace* cspace_del = cspaceList->getColorspace(j);
                if(cspace_del->getColorspaceType() != XIL_COLORSPACE_KCS_ID) {
                    KcsFreeProfile(profileSequence[j]);
                }
            }
            delete profileSequence;

            XIL_ERROR(dest->getSystemState(),
                      XIL_ERROR_RESOURCE,
                      "di-381",
                      TRUE);
            return XIL_FAILURE;
        }
    }
    
    //
    // We need to save the type of the end profiles for
    // use in the compute routine, as well as the completeProfile.
    //
    kcms_data = new KCMSComputeData;
    if(kcms_data == NULL) {
        XIL_ERROR(dest->getSystemState(),
                  XIL_ERROR_RESOURCE,
                  "di-1",
                  TRUE);
        //
        // delete all allocated resources
        //
        for(i=0; i<nspaces; i++) {
            XilColorspace* cspace_del = cspaceList->getColorspace(i);
            if(cspace_del->getColorspaceType() != XIL_COLORSPACE_KCS_ID) {
                if(KcsFreeProfile(profileSequence[i]) != KCS_SUCCESS) {
#ifdef KCMS_DEBUG		
fprintf(stderr, "free profile %d failed\n", profileSequence[i]);
#endif
                }
            }
        }

        delete profileSequence;
    }

    kcms_data->completeProfile = completeProfile;
    
    //
    // Check if the profiles are RGB. 0 is src image, 1 is dst image
    //
    kcms_data->srcRGB = isRGB(profileSequence[0], 0);
    kcms_data->dstRGB = isRGB(profileSequence[nspaces-1], 1);

    //
    // Set up the compute data pointer, cache the data
    // on the colorspace list and set up the
    // callback to delete the data when the list is deleted
    //
    *compute_data = (void*)kcms_data;

    extern void freeCachedProfile(void*);
    cspaceList->setCachedData((void*)kcms_data, freeCachedProfile);

#ifdef KCMS_DEBUG
    fprintf(stderr, "cached data created %p %d %d %d\n",
	    kcms_data, kcms_data->completeProfile,
	    kcms_data->srcRGB, kcms_data->dstRGB);
#endif    

    //
    // Free all the profiles we loaded in the list
    // we don't need them any more
    //
    for(i=0; i<nspaces; i++) {
	XilColorspace* cspace_del = cspaceList->getColorspace(i);
	if(cspace_del->getColorspaceType() != XIL_COLORSPACE_KCS_ID) {
	    if(KcsFreeProfile(profileSequence[i]) != KCS_SUCCESS) {
#ifdef KCMS_DEBUG		
fprintf(stderr, "free profile %d failed\n", profileSequence[i]);
#endif
	    }
	}
    }
    delete profileSequence;
    return XIL_SUCCESS;
}

XilStatus
XilDeviceManagerComputeSUNWkcms::ColorCorrect(XilOp*       op,
					      unsigned int   ,
					      XilRoi*      roi,
					      XilBoxList*  bl)
{
    //
    //  Split the list of XilBoxes to take tile boundaries into account.  This
    //  will work to ensure that no cobbling of the data is required because
    //  the boxes will not cross tile boundaries in the source images.
    //
    if(op->splitOnTileBoundaries(bl) == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    //
    //  Get the images for our operation.
    //
    XilImage*          src  = op->getSrcImage(1);
    XilImage*          dest = op->getDstImage(1);

    //
    // Colorspace list
    //
    XilColorspaceList* cspaceList;
    op->getParam(1, (void**)&cspaceList);

    unsigned int       nspaces = cspaceList->getNumColorspaces(); 
    
    //
    //  Get the connected profile id that was created in the preprocess
    //  method
    //
    KCMSComputeData* kcms_data = (KCMSComputeData*)op->getPreprocessData(this);
    KcsProfileId     cprofile = kcms_data->completeProfile;
    Xil_boolean      srcIsRGB = kcms_data->srcRGB;
    Xil_boolean      destIsRGB = kcms_data->dstRGB;

    //
    //  The is a counter which keeps track of how many boxes we have
    //  processed.
    //
    unsigned int boxcount = 0;
    
    //
    //  Store away the number of bands for this operation.
    //
    unsigned int src_bands   = src->getNumBands();
    unsigned int dest_bands  = dest->getNumBands();

    //
    // See if either image is RGB
    //

    //
    //  Loop over each of the boxes of storage we are to process.
    //
    XilBox* src_box;
    XilBox* dest_box;
    while(bl->getNext(&src_box, &dest_box)) {
        //
        //  Aquire our storage from the images.  The storage returned is valid
        //  for the box given.  Thus, any origins or child offsets have been
        //  taken into account.
        //
        XilStorage  src_storage(src);
        XilStorage  dest_storage(dest);
        if((src->getStorage(&src_storage, op, src_box, "XilMemory",
                             XIL_READ_ONLY)  == XIL_FAILURE) ||
           (dest->getStorage(&dest_storage, op, dest_box, "XilMemory",
                             XIL_WRITE_ONLY) == XIL_FAILURE)) {
            //
            //  Mark this box entry as having failed.  If marking the box
            //  returns XIL_FAILURE, then we return XIL_FAILURE.
            //
            if(bl->markAsFailed() == XIL_FAILURE) {
                return XIL_FAILURE;
            } else {
                continue;
            }
        }

	//
	// Set up the KcsPixelLayout structures from the storage
	// object. We do the setup the same way for all of the storage
	// types.
	//

	//
	// The KcsPixelLayout structure has space for up to four
	// channels (KcsExtendablePixelLayout) and this handles the most
	// common cases eg YCC, RGB, CMYK. If more than four channels become
	// accepted by the ICC we need to new the KcsPixelLayouts using the
	// extensibility capability of the layout structure.
	//
	KcsPixelLayout srcLayout;
	KcsPixelLayout destLayout;
	
	unsigned int   src_pixel_stride;
	unsigned int   src_scanline_stride;
	Xil_unsigned8* src_data;

	unsigned int   dest_pixel_stride;
	unsigned int   dest_scanline_stride;
	Xil_unsigned8* dest_data;
	char*          base_src_data[KcsExtendablePixelLayout];
	char*          base_dest_data[KcsExtendablePixelLayout];

	srcLayout.numOfComp = src_bands;
	destLayout.numOfComp = dest_bands;
	
	if(srcIsRGB == TRUE) {
	    //
	    // Reverse the order of the bands
	    //
	    for(int i=0; i<src_bands; i++) {
		src_storage.getStorageInfo(2-i,
					   &src_pixel_stride,
					   &src_scanline_stride,
					   NULL,
					   (void**)&src_data);
	    
		srcLayout.component[i].bitOffset = 0;
		srcLayout.component[i].compType= KcsCompUFixed;
		srcLayout.component[i].compDepth = 8;
		srcLayout.component[i].rangeStart = 0;
		srcLayout.component[i].rangeEnd= 255.0;
		srcLayout.component[i].colOffset = src_pixel_stride;
		srcLayout.component[i].rowOffset = src_scanline_stride;
		base_src_data[i] = (char*)src_data;
	    }
	} else {
	    for(int i=0; i<src_bands; i++) {
		src_storage.getStorageInfo(i,
					   &src_pixel_stride,
					   &src_scanline_stride,
					   NULL,
					   (void**)&src_data);
	    
		srcLayout.component[i].bitOffset = 0;
		srcLayout.component[i].compType= KcsCompUFixed;
		srcLayout.component[i].compDepth = 8;
		srcLayout.component[i].rangeStart = 0;
		srcLayout.component[i].rangeEnd= 255.0;
		srcLayout.component[i].colOffset = src_pixel_stride;
		srcLayout.component[i].rowOffset = src_scanline_stride;
		base_src_data[i] = (char*)src_data;
	    }
	}

	if(destIsRGB == TRUE) {
	    //
	    // Reverse the order of the bands
	    //
	    for(int i=0; i<dest_bands; i++) {
		dest_storage.getStorageInfo(2-i,
					    &dest_pixel_stride,
					    &dest_scanline_stride,
					    NULL,
					    (void**)&dest_data);
		
		destLayout.component[i].bitOffset = 0;
		destLayout.component[i].compType= KcsCompUFixed;
		destLayout.component[i].compDepth = 8;
		destLayout.component[i].rangeStart = 0;
		destLayout.component[i].rangeEnd= 255.0;
		destLayout.component[i].colOffset = dest_pixel_stride;
		destLayout.component[i].rowOffset = dest_scanline_stride;
		base_dest_data[i] = (char*)dest_data;
	    }
	} else {
	    for(int i=0; i<dest_bands; i++) {
		dest_storage.getStorageInfo(i,
					    &dest_pixel_stride,
					    &dest_scanline_stride,
					    NULL,
					    (void**)&dest_data);
		
		destLayout.component[i].bitOffset = 0;
		destLayout.component[i].compType= KcsCompUFixed;
		destLayout.component[i].compDepth = 8;
		destLayout.component[i].rangeStart = 0;
		destLayout.component[i].rangeEnd= 255.0;
		destLayout.component[i].colOffset = dest_pixel_stride;
		destLayout.component[i].rowOffset = dest_scanline_stride;
		base_dest_data[i] = (char*)dest_data;
	    }
	}

	//
	//  Create a list of rectangles to loop over.  The resulting list
	//  of rectangles is the area left by intersecting the ROI with
	//  the destination box.
	//
	XilRectList    rl(roi, dest_box);
	    
	int            x;
	int            y;
	unsigned int   xsize;
	unsigned int   ysize;
	while(rl.getNext(&x, &y, &xsize, &ysize)) {
	    for(int i=0; i<src_bands; i++) {
		// Adjust source pixel layout
		srcLayout.component[i].maxRow = ysize;
		srcLayout.component[i].maxCol = xsize;
		srcLayout.component[i].addr = (base_src_data[i] +
		    y*srcLayout.component[i].rowOffset +
		    x*srcLayout.component[i].colOffset);
	    }

	    for(i=0; i<dest_bands; i++) {
		// Adjust dest pixel layout
		destLayout.component[i].maxRow = ysize;
		destLayout.component[i].maxCol = xsize;
		destLayout.component[i].addr = (base_dest_data[i] + 
		    y*destLayout.component[i].rowOffset + 
		    x*destLayout.component[i].colOffset);
	    }

	    if(KcsEvaluate(cprofile,
                           KcsOpForward+KcsContImage,
			   &srcLayout,
                           &destLayout) != KCS_SUCCESS) {
                XIL_ERROR(dest->getSystemState(),
                          XIL_ERROR_RESOURCE,
                          "di-381",
                          TRUE);
		return XIL_FAILURE;
	    }
        }
        boxcount++;
    }

    return XIL_SUCCESS;
}

Xil_boolean
XilDeviceManagerComputeSUNWkcms::isRGB(KcsProfileId profile, int key)
{
    KcsAttributeValue attr;
    
    // Get the header for the input profile
    attr.base.type = (KcsAttributeType)icSigHeaderType;
    attr.base.sizeOfType = sizeof(icHeader);
    attr.base.countSupplied = 1;
    KcsGetAttribute(profile, icSigHeaderTag, &attr);

    if(attr.val.icHeader.deviceClass == icSigLinkClass) {
        //
        // It's a devicelink profile
        //

        if(key == 0) {
           //
           // Source image
           //
           
           if(attr.val.icHeader.colorSpace == icSigRgbData) {
	           return TRUE;
           }
        } else {
           //
           // Destination image
           //
           if(attr.val.icHeader.pcs == icSigRgbData) {
	           return TRUE;
           }
        }
    } else {
        //
        // It's not a device link profile
        // use normal approach to determine if it's RGB
        //
        if(attr.val.icHeader.colorSpace == icSigRgbData) {
	        return TRUE;
        }
    }
    
    return FALSE;
}

//
// Free the profile and delete the KCMSData structure
//
void
freeCachedProfile(void* data)
{
    KCMSComputeData* kcms_data = (KCMSComputeData*)data;

#ifdef KCMS_DEBUG
    fprintf(stderr, "freeing cached data %p %d %d %d\n",
	    kcms_data, kcms_data->completeProfile,
	    kcms_data->srcRGB, kcms_data->dstRGB);
#endif    

    if(KcsFreeProfile(kcms_data->completeProfile) != KCS_SUCCESS) {
#ifdef KCMS_DEBUG
	fprintf(stderr, "Failed to free complete profile in callback\n");
#endif
    }
    delete kcms_data;
}
