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
//  File:	XilColorspace.cc
//  Project:	XIL
//  Revision:	1.24
//  Last Mod:	10:08:57, 03/10/00
//
//  Description:
//      Implementation of the XilColorspace class
//
//
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------

#pragma ident	"@(#)XilColorspace.cc	1.24\t00/03/10  "

//
//  System Includes
//
#include <stdio.h>
#include <stdlib.h>
#if !defined(_WINDOWS)
#if defined(HPUX)
#include <dl.h>
#else
#include <dlfcn.h>
#endif
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//
//  XIL Includes
//
#include "_XilDefines.h"
#include "_XilColorspace.hh"
#include "_XilSystemState.hh"
#include "XiliUtils.hh"

#ifdef _XIL_HAS_KCMS
static struct kcsProcs {
    void*       dlhandle;
    
    KcsStatusId (*KcsLoadProfile)  (KcsProfileId*,
				    KcsProfileDesc*,
				    KcsLoadHints);
	
    KcsStatusId (*KcsFreeProfile)  (KcsProfileId);
    
    KcsStatusId (*KcsGetAttribute) (KcsProfileId,
				    KcsAttributeName,
				    KcsAttributeValue*);
	
    KcsStatusId (*KcsSaveProfile)  (KcsProfileId,
				    KcsProfileDesc*);
} kcsProcs;
#endif // _XIL_HAS_KCMS

//
// Constructors for a colorspace object
//
XilColorspace::XilColorspace(XilSystemState*     system_state,
                             const char*         name,
                             XilColorspaceOpCode op,
                             unsigned int        bands) :
    XilNonDeferrableObject(system_state, XIL_COLORSPACE)
{
   isOKFlag = FALSE;

   opcode   = op;
   nbands   = bands;

   type     = XIL_COLORSPACE_NAME;
   data     = NULL;

   setName(name);

   profileLoadedFlag = FALSE;
   profileData = NULL;
   
   isOKFlag = TRUE;
}

XilColorspace::XilColorspace(XilSystemState*   system_state,
                             XilColorspaceType cspace_type,
                             void*             cspace_data,
			     Xil_boolean       copy_flag) :
    XilNonDeferrableObject(system_state, XIL_COLORSPACE)
{
   isOKFlag = FALSE;

   profileLoadedFlag = FALSE;
   profileData       = NULL;
   type              = cspace_type;
   data              = NULL;
   
   if(cspace_type == XIL_COLORSPACE_NAME) {
       //
       // We are using the colorspace name
       //
       if(cspace_data == NULL) {
           XIL_ERROR(getSystemState(), XIL_ERROR_USER, "di-146", TRUE);
           return;
       }

       xili_cspace_name_to_opcode((const char*)cspace_data,
                                  &opcode,
				  &nbands);

       isOKFlag = TRUE;
   } 
#ifdef _XIL_HAS_KCMS
   else {
       //
       // Its either a filename or a KcsProfileId
       //
       opcode = XIL_CS_INVALID;
       nbands = 0;

       //
       // The copy flag tells us if we need to make a copy
       // of the data. By default we don't, usually only when
       // createCopy is called is the copy made.
       //

       //
       // But we always make a copy if the colorspace is a
       // XIL_COLORSPACE_FILENAME.
       //
       if(cspace_type == XIL_COLORSPACE_FILENAME) {
           data = (void*)strdup((char*)cspace_data);
	 
           if(data == NULL) {
               XIL_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-1", TRUE);
               return;
           }
       }
       else if(copy_flag) {
           //
           // Its a KcsId, in order to copy the data we need to
           // save the profile into a memory buffer. First we need
           // to find out how big the profile is. Then we can load
           // it back into our memory buffer. Its a bit convoluted....
           //
           KcsAttributeValue attr;
           KcsProfileId      inProfile = (KcsProfileId)data;
	       
           if(loadKcsProcs() == XIL_FAILURE) {
               return;
           }

           //
           // Get the header, it contains the size
           //
           if(((*kcsProcs.KcsGetAttribute)(inProfile,
                                           icSigHeaderTag,
                                           &attr)) != KCS_SUCCESS) {
               XIL_ERROR(getSystemState(), XIL_ERROR_USER, "di-381", TRUE);
               return;
           }

           profileData = new char[attr.val.icHeader.size];
           if(profileData == NULL) {
               XIL_ERROR(getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
               return;
           }

           KcsProfileDesc desc;

           desc.type = KcsMemoryProfile;
           desc.desc.memPtr.memPtr = (void*)profileData;
           desc.desc.memPtr.offset = 0;
           desc.desc.memPtr.size = attr.val.icHeader.size;

           //
           // Save the passed in profile into this buffer
           //
           if(((*kcsProcs.KcsSaveProfile)(inProfile, &desc))
              != KCS_SUCCESS) {
               delete profileData;
               profileData = NULL;
               XIL_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-381", TRUE);
               return;
           }

           //
           // Now load the saved profile to get a new profile id.
           //
           KcsProfileId outProfile;
           if(((*kcsProcs.KcsLoadProfile)(&outProfile,
                                          &desc,
                                          KcsLoadWhenNeeded))
              != KCS_SUCCESS) {
               delete profileData;
               profileData = NULL;
               XIL_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-381", TRUE);
               return;
           }

           data = (void*)outProfile;
           profileLoadedFlag = TRUE;
       } else {
           // Don't copy the data just keep a pointer to it
           data = cspace_data;
       }

       // Set nbands
       if(setProfileNBands() == XIL_FAILURE) {
	   if(profileLoadedFlag == TRUE) {
	       // Free the profile if we loaded one and associated data
	       (*kcsProcs.KcsFreeProfile)((KcsProfileId)data);
	       delete profileData;
	   }
	   return;
       }

       isOKFlag = TRUE;
   }
#endif // _XIL_HAS_KCMS      
}

//
// Create a KCMS profile id colorspace from X11 information.
// Window system profiles are created by either kcms_calibrate(1)
// or kcms_configure(1).
//
#ifdef _XIL_HAS_KCMS
XilColorspace::XilColorspace(XilSystemState*   system_state,
			     Display*          display,
			     int               screen,
			     Visual*           visual) :
#else
XilColorspace::XilColorspace(XilSystemState*   system_state,
			     Display*          ,
			     int               ,
			     Visual*           ) :
#endif    
    XilNonDeferrableObject(system_state, XIL_COLORSPACE)
{
    isOKFlag          = FALSE;
    profileLoadedFlag = FALSE;
    profileData       = NULL;
    data              = NULL;
    
#ifdef _XIL_HAS_KCMS
    type              = XIL_COLORSPACE_KCS_ID;

    KcsProfileDesc description;
    KcsProfileId   profile;

    description.type             = KcsWindowProfile;
    description.desc.xwin.dpy    = display;
    description.desc.xwin.screen = screen;
    description.desc.xwin.visual = visual;

    //
    //  Make sure the KCS procs we need are loaded
    //
    if(loadKcsProcs() == XIL_FAILURE) {
	return;
    }

    //
    // Try to load a profile
    //
    if(((*kcsProcs.KcsLoadProfile)(&profile,
                                   &description,
                                   KcsLoadWhenNeeded)) != KCS_SUCCESS) {
	return;
    }

    data = (void*)profile;
    profileLoadedFlag = TRUE;

    if(setProfileNBands() == XIL_FAILURE) {
	// Free the profile
	(*kcsProcs.KcsFreeProfile)(profile);
	return;
    }
    
    isOKFlag = TRUE;
#endif // _XIL_HAS_KCMS
}

//
// Destructor
//
XilColorspace::~XilColorspace()
{
    if(type == XIL_COLORSPACE_FILENAME) {
	//
	// We need to use free as data was allocated
	// using strdup
	//
	free((char*)data);
	return;
    }
    
#ifdef _XIL_HAS_KCMS
    if((type == XIL_COLORSPACE_KCS_ID) &&
       (profileLoadedFlag == TRUE)) {
	// 
	// There should be no way that the profile got loaded
	// without KCMS being dlopened ok, so don't check kcsLoadProcs
	//
	if((*kcsProcs.KcsFreeProfile)((KcsProfileId)data) != KCS_SUCCESS) {
	    // This shouldn't be disastrous do we need to flag it
	    XIL_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-381", TRUE);
	}

	// Delete the profileData that we may have created
	delete profileData;
    }
#endif    
}

XilObject*
XilColorspace::createCopy()
{
    XilColorspace* new_cs;
    
    if(type == XIL_COLORSPACE_NAME) {
	char*          name = getName();
	
	new_cs = getSystemState()->createXilColorspace(name, opcode, nbands);

	//
	//  Must use free here since XilObject::getName() uses
	//  strdup which uses malloc
	//
	free(name);
    } else {
	//
	//  The construction of the object makes a copy of data
	//  as we change the default value of copy_flag to TRUE.
	//
	new_cs = getSystemState()->createXilColorspace(type, data, TRUE);
    }

    if(new_cs == NULL) {
       XIL_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-297", FALSE);
       return NULL;
    }

    //
    //  Give the copy the same version number
    //
    new_cs->copyVersionInfo(this);

    return new_cs;
}

XilColorspaceOpCode
XilColorspace::getOpcode()
{
    return opcode;
}

unsigned int 
XilColorspace::getNBands()
{
    return nbands;
}

XilColorspaceType
XilColorspace::getColorspaceType()
{
    return type;
}

void*
XilColorspace::getColorspaceData()
{
    if(type == XIL_COLORSPACE_NAME) {
	//
	// Its a colorspace name data is NULL, seems
	// reasonable to return the name instead.
	//
	const char* name = getName();
	return (void*)name;
    }

    return data;
}

#ifdef _XIL_HAS_KCMS
//
// Assumes the base profile information has been set.
// used by the KCS_ID and FILENAME type to set the number
// of bands.
//
XilStatus
XilColorspace::setProfileNBands()
{
    if(type == XIL_COLORSPACE_NAME) {
	// Should have been set already
	return XIL_SUCCESS;
    }

    //
    // Verify that we can go ahead and make KCMS calls
    //
    if(loadKcsProcs() == XIL_FAILURE) {
	nbands = 0;
	return XIL_FAILURE;
    }
    
    KcsProfileId tmpProfile;
    if(type == XIL_COLORSPACE_FILENAME) {
	//
	// We need to load the profile temporarily
	//
	KcsProfileDesc desc;
	
	desc.type = KcsSolarisProfile;
	desc.desc.solarisFile.fileName = (char*)data;
	desc.desc.solarisFile.hostName = NULL;
	desc.desc.solarisFile.oflag = O_RDONLY;
	desc.desc.solarisFile.mode = 0;
	
	//
	// Try to load the profile
	//
	if(((*kcsProcs.KcsLoadProfile)(&tmpProfile, &desc, KcsLoadAttributesNow)) !=
	   KCS_SUCCESS) {
	    nbands = 0;
	    XIL_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-381", TRUE);
	    return XIL_FAILURE;
	}
    } else {
	tmpProfile = (KcsProfileId)data;
    }

    KcsAttributeValue attr;

    //
    // Get the header for the input profile, the attribute
    // structure union is big enough to hold the header no
    // need to new it
    //
    attr.base.type = (KcsAttributeType)icSigHeaderType;
    attr.base.sizeOfType = sizeof(icHeader);
    attr.base.countSupplied = 1;
    if(((*kcsProcs.KcsGetAttribute)(tmpProfile, icSigHeaderTag, &attr))
       != KCS_SUCCESS) {
	// If we can't load the profile just set nbands to 0
	nbands = 0;
	XIL_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-381", TRUE);
	return XIL_FAILURE;
    }

    switch (attr.val.icHeader.colorSpace) {
      case icSigCmykData:	// CMYK need 4 channels
	nbands = 4;
	break;
      case icSigGrayData:	// Grayscale only one
	nbands = 1;
	break;
      case icSigXYZData:
      case icSigLabData:
      case icSigLuvData:
      case icSigYCbCrData:
      case icSigYxyData:
      case icSigRgbData:
      case icSigHsvData:
      case icSigHlsData:
      case icSigCmyData:
	nbands = 3;
	break;
      default :			// Everything else is 3
	nbands = 3;
	break;
    }

    //
    // If we loaded a profile free it, hmm if we are going to
    // load it I wonder if its worth changing the type to a
    // KCS_ID and only presenting that type at the GPI.
    //
    if(type == XIL_COLORSPACE_FILENAME) {
	(*kcsProcs.KcsFreeProfile)(tmpProfile);
    }
    return XIL_SUCCESS;
}

//
// dlopen KCMS and set up all the function pointers
//
XilStatus
XilColorspace::loadKcsProcs()
{
    static XilMutex    kcsProcsMutex;
    static Xil_boolean kcsLoadedFlag = FALSE;
    static Xil_boolean kcsLoadAttempted = FALSE;

    kcsProcsMutex.lock();

    //
    // Is everything already loaded
    //
    if(kcsLoadedFlag == TRUE) {
	kcsProcsMutex.unlock();
	return XIL_SUCCESS;
    }

    //
    // Did we try to load KCMS and it failed
    //
    if(kcsLoadAttempted == TRUE) {
	kcsProcsMutex.unlock();
	return XIL_FAILURE;
    }

    kcsProcs.dlhandle = dlopen("libkcs.so.1", RTLD_LAZY);

    //
    //  dlopen the KCMS library libkcs.so.1
    //
    if(kcsProcs.dlhandle == NULL) {
	XIL_WARNING(getSystemState(), XIL_ERROR_CONFIGURATION, "di-379", TRUE);
	kcsLoadAttempted = TRUE;
	kcsProcsMutex.unlock();
	return XIL_FAILURE;
    }
    
    //
    // Load in functions, KcsLoadProfile, KcsFreeProfile,
    // KcsGetAttribute, KcsSaveProfile.
    //
    if((kcsProcs.KcsLoadProfile =
	(KcsStatusId (*) (KcsProfileId*, KcsProfileDesc*, KcsLoadHints))
	
	dlsym(kcsProcs.dlhandle, "KcsLoadProfile")) == NULL) {
	kcsLoadAttempted = TRUE;
	kcsProcsMutex.unlock();
	XIL_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-380", TRUE);
	return XIL_FAILURE;
    }

    if((kcsProcs.KcsFreeProfile =
	(KcsStatusId (*) (KcsProfileId))
	dlsym(kcsProcs.dlhandle, "KcsFreeProfile")) == NULL) {
	
	kcsLoadAttempted = TRUE;
	dlclose(kcsProcs.dlhandle);
	kcsProcsMutex.unlock();
	XIL_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-380", TRUE);
	return XIL_FAILURE;
    }
    
    if((kcsProcs.KcsGetAttribute =
	(KcsStatusId (*) (KcsProfileId, KcsAttributeName, KcsAttributeValue*))
	dlsym(kcsProcs.dlhandle, "KcsGetAttribute")) == NULL) {
	
	kcsLoadAttempted = TRUE;
	dlclose(kcsProcs.dlhandle);
	kcsProcsMutex.unlock();
	XIL_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-380", TRUE);
	return XIL_FAILURE;
    }

    if((kcsProcs.KcsSaveProfile = 
	(KcsStatusId (*) (KcsProfileId, KcsProfileDesc*))
	dlsym(kcsProcs.dlhandle, "KcsSaveProfile")) == NULL) {
	
	kcsLoadAttempted = TRUE;
	dlclose(kcsProcs.dlhandle);
	kcsProcsMutex.unlock();
	XIL_ERROR(getSystemState(), XIL_ERROR_SYSTEM, "di-380", TRUE);
	return XIL_FAILURE;
    }

    kcsLoadedFlag = TRUE;
    
    kcsProcsMutex.unlock();
}
#endif // _XIL_HAS_KCMS
    
	    



    

	
