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
//  File:	XilSystemState.cc
//  Project:	XIL
//  Revision:	1.115
//  Last Mod:	10:08:11, 03/10/00
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
#pragma ident	"@(#)XilSystemState.cc	1.115\t00/03/10  "

//
//  System Includes
//
#include <stdio.h>
#include <stdlib.h>
#if !defined(_WINDOWS) && !defined(IRIX) && !defined(HPUX)
#include <libintl.h>
#endif
#if !defined(_WINDOWS)
#if defined(HPUX)
#include <dl.h>
#else
#include <dlfcn.h>
#endif
#include <unistd.h>
#endif

#ifdef _XIL_HAS_LIBDGA
//
//  DGA includes
//
#include <dga/dga.h>
#include <sys/fbio.h>
#include <sys/visual_io.h>
#endif

#ifdef XIL_XINERAMA
#include "XiliXPanoramix.hh"
#endif

//
//  XIL Includes
//
#include "_XilDefines.h"
#include "_XilGlobalState.hh"
#include "_XilSystemState.hh"
#include "_XilDeviceManagerIO.hh"
#include "_XilDeviceIO.hh"

#include "_XilObject.hh"
#include "_XilDeferrableObject.hh"
#include "_XilCis.hh"
#include "_XilColorspace.hh"
#include "_XilColorspaceList.hh"
#include "_XilDevice.hh"
#include "_XilDitherMask.hh"
#include "_XilImageFormat.hh"
#include "_XilImage.hh"
#include "_XilInterpolationTable.hh"
#include "_XilHistogram.hh"
#include "_XilKernel.hh"
#include "_XilLookupSingle.hh"
#include "_XilLookupCombined.hh"
#include "_XilLookupColorcube.hh"
#include "_XilSel.hh"

//
//  libxil Private Includes
//
#include "XilStorageAPI.hh"

XilErrorFunc      XilSystemState::defaultErrorHandler = XiliDefaultErrorHandler;
XilMutex          XilSystemState::defaultErrorHandlerMutex;

XilSystemState::XilSystemState() {
    isOKFlag = FALSE;
    
    isSynchronized   = FALSE;

    //
    // Create the XiliObjectHashTable
    //
    namedObjectTable = new XiliObjectHashTable(_XILI_DEFAULT_HASH_TABLE_SIZE, 
                                               this, FALSE);
    //
    //  Default to entire image.
    //
    xDefaultTileSize = 0;
    yDefaultTileSize = 0;

    //
    //  Default to a tiling mode of what's in the environment.
    //
    defaultTilingMode = XilGlobalState::theXGS->getTilingMode();

    //
    //  Initialize the iterpolation tables to NULL.  The NULL interpolation
    //  tables are recognized by the op and cause "nearest" interpolation to
    //  be used.
    //
    horizInterpTable    = NULL;
    verticalInterpTable = NULL;

    //
    //  Initially set the show_action flag to what's in the environment
    //
    showActionFlag = XilGlobalState::theXGS->getShowAction();

    //
    //  Default is not to provide warnings.
    //
    provideWarningsFlag = FALSE;

    this->unsharp_mode  = XIL_UNSHARP_EDGE_OPT;

    this->determine_no_cpus();

    isOKFlag = TRUE;
}

XilSystemState::~XilSystemState()
{
    horizInterpTable->destroy();
    verticalInterpTable->destroy();
    delete namedObjectTable;
}


#include <set>
#include <string>
                                                                                
void
XilSystemState::determine_no_cpus()
{
    this->no_cpus = 1;   // well you gotta have at least one
                                                                                
    FILE *fp = fopen("/proc/cpuinfo", "r");
                                                                                
    if (fp == NULL)
        return;
                                                                                
    std::set<std::string> cpus;
                                                                                
    char buffer[20];
    while (fgets(buffer, 20, fp)) {
        if (strncmp(buffer, "physical id", 11) == 0)
            cpus.insert(buffer);
        int len = strlen(buffer);
        if (len > 0 && buffer[len - 1] != '\n') {
            int c;
            while ((c = fgetc(fp)) != EOF)
                if (c == '\n')
                    break;
        }
    }
                                                                                
    fclose(fp);
                                                                                
    this->no_cpus = cpus.size();
}


void
XilSystemState::set_unsharp_mode(XilUnsharpMasking mode)
{
    this->unsharp_mode = mode;
}
                                                                                
                                                                                
XilUnsharpMasking
XilSystemState::get_unsharp_mode()
{
    return this->unsharp_mode;
}
                                                                                

Xil_boolean
XilSystemState::getSynchronized() const
{
    _XIL_TEST_FOR_NULL_THIS(FALSE, "di-260");

    if(XilGlobalState::theXGS->getSetSynchronize()) {
        return TRUE;
    } else {
        return isSynchronized;
    }
}
    
void
XilSystemState::setSynchronized(Xil_boolean onoff)
{
    _XIL_TEST_FOR_NULL_THIS_VOID("di-260");

    this->lock();

    isSynchronized = onoff;

    if(isSynchronized) {
        //
        //  Per-XIL rules, we flush any outstanding deferred operations.
        //
        //  We can't hold the lock on the system state while syncing the
        //  deferrable objects.  So build temporary list of deferrable objects
        //  and then sync() them after releasing the system state lock.
        //
        XiliSLList<XilObject*> tmplist(deferredObjectList);

        this->unlock();

        XilObject*                     defobj;
        XiliSLListIterator<XilObject*> tmp_li(&tmplist);
        while(tmp_li.getNext(defobj) == XIL_SUCCESS) {
            ((XilDeferrableObject*)defobj)->sync();
        }
    } else {
        this->unlock();
    }
}

//------------------------------------------------------------------------
//
//  Function:   addDefObject()/removeDefObject()
//
//  Description:
//      Adds or removes an object to or from the system state's
//      object list.
//      
//      Remove should not be called with an invalid position because
//      the condition is not tested.
//      
//      
//      
//      
//  MT-level:  SAFE
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
//      
//  Deficiencies/ToDo:
//      
//      
//------------------------------------------------------------------------
XiliSLListPosition
XilSystemState::addDefObject(XilObject* object)
{
    this->lock();

    XiliSLListPosition pos = deferredObjectList.append(object);

    this->unlock();

    return pos;
}

void
XilSystemState::removeDefObject(XilObject*       ,
                                XiliListPosition list_position)
{
    this->lock();

    deferredObjectList.remove(list_position);

    this->unlock();
}

//------------------------------------------------------------------------
//
//  Function:   addNamedObject()/removeNamedObject()
//
//  Description:
//      Adds or removes an named object to or from the system state's
//      hash table.
//      
//  MT-level:  SAFE
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
//      
//  Deficiencies/ToDo:
//      
//      
//------------------------------------------------------------------------
void
XilSystemState::addNamedObject(const char* name,
                               XilObject*  object)
{
    this->lock();

    //
    //  Only insert if there isn't an object there by the given name.
    //
    XilObject* table_object;
    if(namedObjectTable->lookup(name, table_object) == XIL_FAILURE) {
        namedObjectTable->insert(name, object);
    }
    

    this->unlock();
}

void
XilSystemState::removeNamedObject(const char* name,
                                  XilObject*  object)
{
    this->lock();

    //
    //  Verify the object we're being asked to remove is the one that's
    //  actually in the table and not a copy of the one in the table.
    //
    XilObject* table_object;
    if(namedObjectTable->lookup(name, table_object) == XIL_FAILURE) {
        table_object = NULL;
    }

    if(object == table_object) {
        namedObjectTable->remove(name, table_object);
    }

    this->unlock();
}

XilObject*
XilSystemState::getObjectByName(const char*   name,
                                XilObjectType type)
{
    _XIL_TEST_FOR_NULL_THIS(NULL, "di-260");

    //
    //  NULL names are not allowed.
    //
    if(name == NULL || name[0] == '\0') {
        return NULL;
    }

    //
    //  We need a check for a bogus object type at this time because the
    //  function is available at the GPI layer.  If it were only available
    //  through the API, then this check would not be necessary.
    //
    switch(type) {
      case XIL_IMAGE:
      case XIL_IMAGE_TYPE:
      case XIL_LOOKUP:
      case XIL_CIS:
      case XIL_DITHER_MASK:
      case XIL_KERNEL:
      case XIL_SEL:
      case XIL_ROI:
      case XIL_ROI_LIST:      // OBSOLETE - not in XIL 1.3
      case XIL_HISTOGRAM:
      case XIL_COLORSPACE:
      case XIL_ATTRIBUTE:
      case XIL_INTERPOLATION_TABLE:
        //
        //  New objects for XIL 1.3
        //
      case XIL_STORAGE:
      case XIL_DEVICE:
      case XIL_COLORSPACE_LIST:
        //
        //  Ok type.
        //
        break;

      default:
        //
        //  An unknown type which is unacceptable.
        //
        XIL_ERROR(this, XIL_ERROR_USER, "di-414", TRUE);
        return NULL;
    }

    //
    //  Ok, now that we're ok with the arguments, search the list of the
    //  specific type.
    //
    XilObject* object = NULL;

    //
    //  Search our object list to see if we can find an object with the
    //  given name.
    //
    this->lock();

    static XilMutex stdobj_mutex;

    stdobj_mutex.lock();

    if(namedObjectTable->lookup(name, object) == XIL_FAILURE) {
        object = NULL;
    }

    this->unlock();

    //
    //  After we've checked all those that have already been created, we'll go
    //  check to see if the name is a standard object that has not been
    //  created yet.
    //
    if(object == NULL) {
        object = getStandardObject(name, type);
    }

    stdobj_mutex.unlock();

    return object;
}

//------------------------------------------------------------------------
//
//  Function:	getStandardObject()
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
//  MT-level:  SAFE
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
//	
//  Deficiencies/ToDo:
//	
//	
//------------------------------------------------------------------------
XilObject*
XilSystemState::getStandardObject(const char*   name,
                                  XilObjectType type)
{
    unsigned int offset;
    XilObject* object = NULL;

    //
    // Get the start offset based on number of static colors
    // Windows uses the first and last half of the NUMCOLORS
    //
#ifdef _WINDOWS
    HDC hDC = GetDC(NULL);
    if((offset = GetDeviceCaps(hDC, NUMCOLORS) / 2) < 0) {
        offset = 0;
    }
    ReleaseDC(NULL, hDC);
#endif

    switch(type) {
      case XIL_IMAGE:
        break;

      case XIL_IMAGE_TYPE:
        break;

      case XIL_LOOKUP:
        if(strcmp(name, "cc496") == 0) {
            int          multipliers[] = { 1, 4, 36 };
            unsigned int dimensions[]  = { 4, 9,  6 };
#ifndef _WINDOWS
            offset = 38;          // unix uses this offset
#endif
            object = createXilLookupColorcube(XIL_BYTE, XIL_BYTE, 3, offset,
                                              multipliers, dimensions);
            object->setName("cc496");
        }

        if(strcmp(name, "cc855") == 0) {
            int          multipliers[] = { 1, 8, 40 };
            unsigned int dimensions[]  = { 8, 5,  5 };
#ifndef _WINDOWS
            offset = 54;
#endif
            object = createXilLookupColorcube(XIL_BYTE, XIL_BYTE, 3, offset,
                                              multipliers, dimensions);
            object->setName("cc855");
        }

        if(strcmp(name, "yuv_to_rgb") == 0) {
            //
            // Note:
            // This code converts each of the values in the colorcube
            // from YUV to RGB709. The dimensions are set up as 855
            // (8 for Y) to get fine Y detail. Dither ops will
            // treat this as if it were a YCC cube, but the output
            // will be converted to RGB
            //

            int          multipliers[] = { 1, 8, 40 };
            unsigned int dimensions[]  = { 8, 5,  5 };
#ifndef _WINDOWS
            offset = 54;
#endif
            object = createXilLookupColorcube(XIL_BYTE, XIL_BYTE, 3, offset,
                                              multipliers, dimensions);
            if(object == NULL) {
                return NULL;
            }

            //
            // Get the values from the colorcube
            //
            Xil_unsigned8 cmap[256*3];
            ((XilLookupColorcube*)object)->getValues(offset, 200, (void*)cmap);

            //
            // Treat each entry as YUV and convert it to rgb709
            //
            Xil_unsigned8* entry = cmap;
            for(int i=0; i<200; i++) {
                float y = (float)entry[0];
                float u = (float)entry[1];
                float v = (float)entry[2];

                //
                // Rescale - no need to clamp here,
                // since results won't exceed 255.
                //
                y = y * (219.0F/255.0F) + 16.5F;
                u = u * (224.0F/255.0F) + 16.5F;
                v = v * (224.0F/255.0F) + 16.5F;


                // Normalize to CCIR-601 ranges
                y = (y - 16.0F) / 219.0F;
                u = (u - 128.0F) / 126.0F;
                v = (v - 128.0F) / 160.0F;

                // Convert to RGB709
                float r = y + v;
                float g = y - 0.194F*u - 0.509F*v;
                float b = y + u;

                //
                // Round and place back in the array.
                // Note that they must be put back in BGR order.
                //
                entry[0] = _XILI_ROUND_U8(b * 255.0F);
                entry[1] = _XILI_ROUND_U8(g * 255.0F);
                entry[2] = _XILI_ROUND_U8(r * 255.0F);

                entry += 3;
            }

            // Place new RGB values back in colorcube lookup
            ((XilLookupColorcube*)object)->setValues(offset, 200, (void*)cmap);
            object->setName("yuv_to_rgb");
        }
        break;

      case XIL_CIS:
        break;

      case XIL_DITHER_MASK:
        if(strcmp(name, "dm441") == 0) {
            float dithermask_4x4x1[] = { 0.9375, 0.4375, 0.8125, 0.3125,
                                         0.1875, 0.6875, 0.0625, 0.5625,
                                         0.7500, 0.2500, 0.8750, 0.3750,
                                         0.0000, 0.5000, 0.1250, 0.6250 };
            object = createXilDitherMask(4, 4, 1, dithermask_4x4x1);
            object->setName("dm441");
        }

        if(strcmp(name, "dm443") == 0) {
            float dithermask_4x4x3[] = { 0.0000, 0.5000, 0.1250, 0.6250,
                                         0.7500, 0.2500, 0.8750, 0.3750,
                                         0.1875, 0.6875, 0.0625, 0.5625,
                                         0.9375, 0.4375, 0.8125, 0.3125,
                        
                                         0.6250, 0.1250, 0.5000, 0.0000,
                                         0.3750, 0.8750, 0.2500, 0.7500,
                                         0.5625, 0.0625, 0.6875, 0.1875,
                                         0.3125, 0.8125, 0.4375, 0.9375,
                         
                                         0.9375, 0.4375, 0.8125, 0.3125,
                                         0.1875, 0.6875, 0.0625, 0.5625,
                                         0.7500, 0.2500, 0.8750, 0.3750,
                                         0.0000, 0.5000, 0.1250, 0.6250 };

            object = createXilDitherMask(4, 4, 3, dithermask_4x4x3);
            object->setName("dm443");
        }

        if(strcmp(name, "dm881") == 0) {
            float dithermask_8x8x1[] = { 
0.000000F, 0.250980F, 0.501961F, 0.752941F, 0.031373F, 0.282353F, 0.533333F, 0.784314F,
0.596078F, 0.847059F, 0.062745F, 0.313726F, 0.564706F, 0.815686F, 0.094118F, 0.345098F,
0.125490F, 0.376471F, 0.627451F, 0.878431F, 0.156863F, 0.407843F, 0.658824F, 0.909804F,
0.721569F, 0.972549F, 0.188235F, 0.439216F, 0.690196F, 0.941177F, 0.219608F, 0.470588F,
0.047059F, 0.298039F, 0.549020F, 0.800000F, 0.015686F, 0.266667F, 0.517647F, 0.768628F,
0.580392F, 0.831373F, 0.109804F, 0.360784F, 0.611765F, 0.862745F, 0.078431F, 0.329412F,
0.172549F, 0.423529F, 0.674510F, 0.925490F, 0.141176F, 0.392157F, 0.643137F, 0.894118F,
0.705882F, 0.956863F, 0.235294F, 0.486275F, 0.737255F, 0.988235F, 0.203922F, 0.454902F
};

            object = createXilDitherMask(8, 8, 1, dithermask_8x8x1);
            object->setName("dm881");
        }

        if(strcmp(name, "dm883") == 0) {
            float dithermask_8x8x3[] = {
0.784314F, 0.533333F, 0.282353F, 0.031373F, 0.752941F, 0.501961F, 0.250980F, 0.000000F,
0.345098F, 0.094118F, 0.815686F, 0.564706F, 0.313726F, 0.062745F, 0.847059F, 0.596078F,
0.909804F, 0.658824F, 0.407843F, 0.156863F, 0.878431F, 0.627451F, 0.376471F, 0.125490F,
0.470588F, 0.219608F, 0.941177F, 0.690196F, 0.439216F, 0.188235F, 0.972549F, 0.721569F,
0.768628F, 0.517647F, 0.266667F, 0.015686F, 0.800000F, 0.549020F, 0.298039F, 0.047059F,
0.329412F, 0.078431F, 0.862745F, 0.611765F, 0.360784F, 0.109804F, 0.831373F, 0.580392F,
0.894118F, 0.643137F, 0.392157F, 0.141176F, 0.925490F, 0.674510F, 0.423529F, 0.172549F,
0.454902F, 0.203922F, 0.988235F, 0.737255F, 0.486275F, 0.235294F, 0.956863F, 0.705882F,

0.988235F, 0.737255F, 0.486275F, 0.235294F, 0.956863F, 0.705882F, 0.454902F, 0.203922F,
0.392157F, 0.141176F, 0.925490F, 0.674510F, 0.423529F, 0.172549F, 0.894118F, 0.643137F,
0.862745F, 0.611765F, 0.360784F, 0.109804F, 0.831373F, 0.580392F, 0.329412F, 0.078431F,
0.266667F, 0.015686F, 0.800000F, 0.549020F, 0.298039F, 0.047059F, 0.768628F, 0.517647F,
0.941177F, 0.690196F, 0.439216F, 0.188235F, 0.972549F, 0.721569F, 0.470588F, 0.219608F,
0.407843F, 0.156863F, 0.878431F, 0.627451F, 0.376471F, 0.125490F, 0.909804F, 0.658824F,
0.815686F, 0.564706F, 0.313726F, 0.062745F, 0.847059F, 0.596078F, 0.345098F, 0.094118F,
0.282353F, 0.031373F, 0.752941F, 0.501961F, 0.250980F, 0.000000F, 0.784314F, 0.533333F,
 
0.000000F, 0.250980F, 0.501961F, 0.752941F, 0.031373F, 0.282353F, 0.533333F, 0.784314F,
0.596078F, 0.847059F, 0.062745F, 0.313726F, 0.564706F, 0.815686F, 0.094118F, 0.345098F,
0.125490F, 0.376471F, 0.627451F, 0.878431F, 0.156863F, 0.407843F, 0.658824F, 0.909804F,
0.721569F, 0.972549F, 0.188235F, 0.439216F, 0.690196F, 0.941177F, 0.219608F, 0.470588F,
0.047059F, 0.298039F, 0.549020F, 0.800000F, 0.015686F, 0.266667F, 0.517647F, 0.768628F,
0.580392F, 0.831373F, 0.109804F, 0.360784F, 0.611765F, 0.862745F, 0.078431F, 0.329412F,
0.172549F, 0.423529F, 0.674510F, 0.925490F, 0.141176F, 0.392157F, 0.643137F, 0.894118F,
0.705882F, 0.956863F, 0.235294F, 0.486275F, 0.737255F, 0.988235F, 0.203922F, 0.454902F
};
            
            object = createXilDitherMask(8, 8, 3, dithermask_8x8x3);
            object->setName("dm883");
        }
        break;

      case XIL_KERNEL:
        if(strcmp(name, "floyd-steinberg") == 0) {
            float floyd_steinbergdata[]={ 0.0,      0.0,      7.0/16.0,
                                          3.0/16.0, 5.0/16.0, 1.0/16.0};

            object = createXilKernel(3, 2, 1, 0, floyd_steinbergdata);
            object->setName("floyd-steinberg");
        }
        break;

      case XIL_SEL:
        break;

      case XIL_ROI:
        break;

      case XIL_ROI_LIST:      // OBSOLETE - not in XIL 1.3
        break;

      case XIL_HISTOGRAM:
        break;

      case XIL_COLORSPACE:
        //
        // Standard Colorspaces
        //
        if(strcmp(name, "rgb709") == 0) {
            object = createXilColorspace("rgb709", XIL_CS_RGB709, 3);
        }

        if(strcmp(name, "rgblinear") == 0) {
            object = createXilColorspace("rgblinear", XIL_CS_RGBLINEAR, 3);
        }

        if(strcmp(name, "ycc709") == 0) {
            object = createXilColorspace("ycc709", XIL_CS_YCC709, 3);
        }

        if(strcmp(name, "y709") == 0) {
            object = createXilColorspace("y709", XIL_CS_Y709, 1);
        }

        if(strcmp(name, "ylinear") == 0) {
            object = createXilColorspace("ylinear", XIL_CS_YLINEAR, 1);
        }

        if(strcmp(name, "photoycc") == 0) {
            object = createXilColorspace("photoycc", XIL_CS_PHOTOYCC, 3);
        }

        if(strcmp(name, "ycc601") == 0) {
            object = createXilColorspace("ycc601", XIL_CS_YCC601, 3);
        }

        if(strcmp(name, "y601") == 0) {
            object = createXilColorspace("y601", XIL_CS_Y601, 1);
        }

        if(strcmp(name, "cmy") == 0) {
            object = createXilColorspace("cmy", XIL_CS_CMY, 3);
        }

        if(strcmp(name, "cmyk") == 0) {
            object = createXilColorspace("cmyk", XIL_CS_CMYK, 4);
        }
        break;

      case XIL_ATTRIBUTE:
        break;

      case XIL_INTERPOLATION_TABLE:
        break;

        //
        //  New objects for XIL 1.3
        //
      case XIL_STORAGE:
      case XIL_DEVICE:
      case XIL_COLORSPACE_LIST:
        break;
    }        

    return object;
}

//------------------------------------------------------------------------
//
//  Function:	setInterpolationTables()/getInterpolationTables()
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
//	
//  Deficiencies/ToDo:
//	
//	
//------------------------------------------------------------------------
XilStatus
XilSystemState::setInterpolationTables(XilInterpolationTable* horiz_table,
                                       XilInterpolationTable* vertical_table)
{
    _XIL_TEST_FOR_NULL_THIS(XIL_FAILURE, "di-260");

    horizInterpTable->destroy();
    verticalInterpTable->destroy();

    horizInterpTable    = (XilInterpolationTable*)horiz_table->createCopy();
    verticalInterpTable = (XilInterpolationTable*)vertical_table->createCopy();

    return XIL_SUCCESS;
}

XilStatus
XilSystemState::getInterpolationTables(XilInterpolationTable** horiz_table,
                                       XilInterpolationTable** vertical_table)
{
    _XIL_TEST_FOR_NULL_THIS(XIL_FAILURE, "di-260");

    *horiz_table    = horizInterpTable;
    *vertical_table = verticalInterpTable;

    return XIL_SUCCESS;
}

//------------------------------------------------------------------------
//
//  Function:	setShowActionFlag()/getShowActionFlag()
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
//	
//  Deficiencies/ToDo:
//	
//	
//------------------------------------------------------------------------
void
XilSystemState::setShowActionFlag(int env_on_off)
{
    _XIL_TEST_FOR_NULL_THIS_VOID("di-260");

    if(env_on_off < 0) {
        showActionFlag = XilGlobalState::theXGS->getShowAction();
    } else {
        showActionFlag = env_on_off;
    }
}

Xil_boolean
XilSystemState::getShowActionFlag()
{
    _XIL_TEST_FOR_NULL_THIS(FALSE, "di-260");

    return showActionFlag;
}

//------------------------------------------------------------------------
//
//  Function:	setProvideWarningsFlag()/getProvideWarningsFlag()
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
//	
//  Deficiencies/ToDo:
//	
//	
//------------------------------------------------------------------------
void
XilSystemState::setProvideWarningsFlag(Xil_boolean on_off)
{
    _XIL_TEST_FOR_NULL_THIS_VOID("di-260");

    provideWarningsFlag = on_off;
}

Xil_boolean
XilSystemState::getProvideWarningsFlag()
{
    _XIL_TEST_FOR_NULL_THIS(FALSE, "di-260");

    if(XilGlobalState::theXGS->getProvideWarnings()) {
        return TRUE;
    } else {
        return provideWarningsFlag;
    }
}

//------------------------------------------------------------------------
//
//  Function:	completeObjectCreation()
//
//  Description:
//	Once an object has been created by one of the object create
//      methods below, it calls this method to complete the process.
//      This method takes care of all common steps.
//	
//  MT-level:  SAFE
//	
//  Parameters:
//	
//	
//  Returns:
//	XIL_SUCCESS or XIL_FAILURE
//	
//  Side Effects:
//	
//	
//  Notes:
//	
//	
//  Deficiencies/ToDo:
//	
//	
//------------------------------------------------------------------------
XilStatus
XilSystemState::completeObjectCreation(XilObject* object)
{
    if(object == NULL) {
        XIL_ERROR(this, XIL_ERROR_RESOURCE,"di-1",TRUE);
        return XIL_FAILURE;
    }
    if(object->isOK() == FALSE) {
        return XIL_FAILURE;
    }

    return XIL_SUCCESS;
}

//------------------------------------------------------------------------
//
//  Function:	setDefaultTileSize()/getDefaultTileSize()
//
//  Description:
//	Sets or gets the default tile size for all objects created from this
//      system state.  The default tile size is used by the core when the user
//      has not already specified a tile size for an image.
//	
//	If the environment variable XIL_DEBUG has txsize and tysize set and the
//      default tile size has not already been set by the application, the values
//      contained in the environment variable will be used.  A call to
//      setDefaultTileSize will perminantly override the values in
//      XIL_DEBUG txsize and tysize.
//	
//  MT-level:  SAFE
//	
//  Parameters:
//	
//	
//  Returns:
//	XIL_SUCCESS or XIL_FAILURE
//	
//  Side Effects:
//	
//	
//  Notes:
//	
//	
//  Deficiencies/ToDo:
//	
//	
//------------------------------------------------------------------------
XilStatus
XilSystemState::setDefaultTileSize(unsigned int txsize,
                                   unsigned int tysize)
{
    _XIL_TEST_FOR_NULL_THIS(XIL_FAILURE, "di-260");

    XilStatus ret_status = XIL_SUCCESS;

    this->lock();

    //
    //  Don't test defaultTilingMode here.  We treat it as orthogonal to the
    //  default tilesize settings and will be taken into account for each
    //  image.
    //
    xDefaultTileSize = txsize;
    yDefaultTileSize = tysize;

    this->unlock();

    return ret_status;
}

XilStatus
XilSystemState::getDefaultTileSize(unsigned int* txsize,
                                   unsigned int* tysize)
{
    _XIL_TEST_FOR_NULL_THIS(XIL_FAILURE, "di-260");

    this->lock();

    //
    //  Check whether the default tile size has been set by the application,
    //  if not, then check the XIL_DEBUG environment variable to see if
    //  txsize and tysize have been set and get the default from there.
    //
    //  If the application hasn't set the tilesize and the XIL_DEBUG
    //  values aren't set, then calculate a reasonable,
    //  configuration specific default tilesize.
    //
    if(xDefaultTileSize == 0 && yDefaultTileSize == 0) {
        xDefaultTileSize = XilGlobalState::theXGS->getTileSizeX();
        yDefaultTileSize = XilGlobalState::theXGS->getTileSizeY();
    }

    this->unlock();

    *txsize = xDefaultTileSize;
    *tysize = yDefaultTileSize;

    return XIL_SUCCESS;
}


//------------------------------------------------------------------------
//
//  Function:	setDefaultTilingMode()/getDefaultTilingMode()
//
//  Description:
//	These methods alter the default tiling mode of NEW objects
//      created from this system state.  Changing the default mode
//      does not retoactively reformat all of the existing images.
//	
//  MT-level:  SAFE
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
//	
//  Deficiencies/ToDo:
//	
//	
//------------------------------------------------------------------------
XilStatus
XilSystemState::setDefaultTilingMode(XilTilingMode new_tiling_mode)
{
    _XIL_TEST_FOR_NULL_THIS(XIL_FAILURE, "di-260");

    this->lock();

    //
    //  The application overrides what the environment sets.
    //
    defaultTilingMode = new_tiling_mode;

    this->unlock();

    return XIL_SUCCESS;
}

XilTilingMode
XilSystemState::getDefaultTilingMode()
{
    _XIL_TEST_FOR_NULL_THIS(XIL_WHOLE_IMAGE, "di-260");

    this->lock();

    XilTilingMode ret_mode = defaultTilingMode;

    this->unlock();

    return ret_mode;
}

//------------------------------------------------------------------------
//
//  Function:	installErrorHandler()
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
//	
//  Deficiencies/ToDo:
//	
//	
//------------------------------------------------------------------------
XilStatus
XilSystemState::installErrorHandler(XilErrorFunc error_function)
{
    //
    //  Not well documented, but if we receive a NULL system state, then we're
    //  expected to replace the default error handler with the provided error
    //  handler.
    //
    if(this == NULL) {
        defaultErrorHandlerMutex.lock();

        defaultErrorHandler = error_function;

        defaultErrorHandlerMutex.unlock();

        return XIL_SUCCESS;
    }

    XilStatus ret_val = XIL_SUCCESS;

    this->lock();

    if(errorFunctionList.insertBefore(error_function,
                                      errorFunctionList.head()) == _XILI_LIST_INVALID_POSITION) {
        XIL_ERROR(this, XIL_ERROR_OTHER, "di-256", FALSE);
        ret_val = XIL_FAILURE;
    }

    this->unlock();

    return ret_val;
}

void
XilSystemState::removeErrorHandler(XilErrorFunc error_function)
{
    _XIL_TEST_FOR_NULL_THIS_VOID("di-260");

    this->lock();

    XiliListPosition location = errorFunctionList.find(error_function);

    if(location == _XILI_LIST_INVALID_POSITION) {
        this->unlock();
        XIL_ERROR(this, XIL_ERROR_OTHER, "di-208", FALSE);
        return;
    }

    errorFunctionList.remove(location);

    this->unlock();
}

Xil_boolean
XilSystemState::callNextErrorHandler(XilError* error_obj)
{
    _XIL_TEST_FOR_NULL_THIS(FALSE, "di-260");

    this->lock();

    XilErrorFunc     err_func = NULL;
    XiliListPosition location = error_obj->getHandlerPosition();

    if(location != _XILI_LIST_INVALID_POSITION) {
        err_func = errorFunctionList.reference(location);

        location = errorFunctionList.next(location);

        error_obj->setHandlerPosition(location);
    }

    this->unlock();

    if(err_func != NULL) {
        return (*err_func)(error_obj);
    } else {
        return FALSE;
    }
}

void
XilSystemState::callError(XilError* error)
{
    //
    //  If the error report has a NULL system state, then we attempt to
    //  use the first system state on the global state's list.
    //
    //
    XilSystemState* sys_state;
    if(this == NULL) {
        sys_state = XilGlobalState::getXilGlobalState()->getFirstSystemState();
    } else {
        sys_state = this;
    }
    error->setSystemState(sys_state);

    //
    //  If we have an object, then its lock is currently held by the library.
    //  When reporting an error, we must unlock the object so error handlers
    //  can use the object to make calls.
    //
    XilObject*  object = error->getObject();
    Xil_boolean relock_object = FALSE;
    if(object != NULL) {
        //
        //  If the object is locked (i.e. trylock fails), then unlock the
        //  object and relock it when returning from callError().  If it's no
        //  locked (i.e. trylock succeeds), then unlock the lock we just
        //  aquired but don't relock it when returning from callError().
        //
        if(object->trylock() == XIL_FAILURE) {
            relock_object = TRUE;
        }

        object->unlock();
    }

    //
    //  If we still don't have a system state, then call the default error
    //  handler to get the error reported anyway.
    //
    if(sys_state == NULL) {
        //
        //  If XIL_DEBUG has provide_warnings set, then we print out XIL warnings.
        //  Otherwise, we ignore them. 
        //
        if(error->isWarning() &&
           !XilGlobalState::theXGS->getProvideWarnings()) {
            return;
        }

        defaultErrorHandlerMutex.lock();

        XilErrorFunc def_handler = defaultErrorHandler;

        defaultErrorHandlerMutex.unlock();
        
        (*def_handler)(error);
    } else {
        sys_state->lock();

        if(error->isWarning() && !sys_state->getProvideWarningsFlag()) {
            sys_state->unlock();
            return;
        }

        if(sys_state->errorFunctionList.length() != 0) {
            error->setHandlerPosition(sys_state->errorFunctionList.head());

            sys_state->unlock();

            sys_state->callNextErrorHandler(error);
        } else {
            sys_state->unlock();
    
            defaultErrorHandlerMutex.lock();

            XilErrorFunc def_handler = defaultErrorHandler;

            defaultErrorHandlerMutex.unlock();
        
            (*def_handler)(error);
        }
    }        

    //
    //  If we have an object, then we unlocked the lock held by the library.
    //  We must reaquire the lock prior to returning control to the library.
    //
    if(relock_object) {
        object->lock();
    }
}

void
XilSystemState::notifyError(XilErrorCategory category,
                            const char*      error_id,
                            Xil_boolean      primary,
                            unsigned int     line_number,
                            const char*      filename,
                            XilObject*       xil_object,
                            void*            arg)
{
    XilError error;

    error.setErrorCategory(category);
    error.setId(error_id);
    error.setPrimaryFlag(primary);
    error.setLine(line_number);
    error.setFilename(filename);
    error.setObject(xil_object);
    error.setIsWarningFlag(FALSE);
    error.setArg(arg);

    callError(&error);
}

void
XilSystemState::notifyWarning(XilErrorCategory category,
                              const char*      error_id,
                              Xil_boolean      primary,
                              unsigned int     line_number,
                              const char*      filename,
                              XilObject*       xil_object,
                              void*            arg)
{
    XilError error;

    error.setErrorCategory(category);
    error.setId(error_id);
    error.setPrimaryFlag(primary);
    error.setLine(line_number);
    error.setFilename(filename);
    error.setObject(xil_object);
    error.setIsWarningFlag(TRUE);
    error.setArg(arg);

    callError(&error);
}

Xil_boolean
XiliDefaultErrorHandler(XilError* error)
{
    fprintf(stderr, xili_dgettext("xil", "XilDefaultErrorFunc:\n"));
    fprintf(stderr, xili_dgettext("xil", "   error category: %s\n"),
            error->getErrorCategoryString());
    fprintf(stderr, xili_dgettext("xil", "     error string: %s\n"),
            error->getString());
    fprintf(stderr, xili_dgettext("xil", "         error id: %s\n"),
            error->getId());
    fprintf(stderr, xili_dgettext("xil", "       is warning: %s\n"),
            error->isWarning() ? xili_dgettext("xil", "TRUE") : xili_dgettext("xil", "FALSE"));
    fprintf(stderr,
            xili_dgettext("xil","   %s error detected in file %s, line %d in XIL\n"),
            error->getPrimaryFlag() ? xili_dgettext("xil", "primary") : xili_dgettext("xil", "secondary"),
            error->getFilename(),
            error->getLine());

    if(error->getObject()) {
        char error_storage[BUFSIZ];
        error->getObject()->getErrorString(error_storage, BUFSIZ);
        if(error_storage[0])
            fprintf(stderr, xili_dgettext("xil","      object info: %s\n"), error_storage);
    }

    return TRUE;
}

//------------------------------------------------------------------------
//
//  Function:	XIL Object Creation Methods
//
//  Description:
//	Creates a specific XilObject.
//	
//	
//	
//	
//	
//	
//	
//  MT-level:  SAFE
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
//	
//  Deficiencies/ToDo:
//	
//	
//------------------------------------------------------------------------
//
// Create the Cis object for the requested compressor
//
XilCis*
XilSystemState::createXilCis(const char* compressor_name)
{
    _XIL_TEST_FOR_NULL_THIS(NULL, "di-260");

    XilCis* cis = new XilCis(this, compressor_name);

    if(completeObjectCreation(cis) == XIL_FAILURE) {
        XIL_ERROR(this, XIL_ERROR_SYSTEM, "di-282", FALSE);
        return NULL;
    }
    
    return cis;
}

//
//  Create the device object for the specified device
//
XilDevice*
XilSystemState::createXilDevice(const char* device_name)
{
    _XIL_TEST_FOR_NULL_THIS(NULL, "di-260");

    XilDevice* device = new XilDevice(this, device_name);

    if(completeObjectCreation(device) == XIL_FAILURE) {
        XIL_ERROR(this, XIL_ERROR_SYSTEM, "di-345", FALSE);
        return NULL;
    }

    return device;
}

XilImageFormat*
XilSystemState::createXilImageFormat(unsigned int width,
                                     unsigned int height,
                                     unsigned int nbands,
                                     XilDataType  datatype)
{
    _XIL_TEST_FOR_NULL_THIS(NULL, "di-260");

    XilImageFormat* image_format =
        new XilImageFormat(this, width, height, nbands, datatype);

    if(completeObjectCreation(image_format) == XIL_FAILURE) {
        XIL_ERROR(this, XIL_ERROR_SYSTEM, "di-188", FALSE);
        return NULL;
    }
    
    return image_format;
}

XilImage*
XilSystemState::createXilImage(unsigned int width,
                               unsigned int height,
                               unsigned int nbands,
                               XilDataType  datatype)
{
    _XIL_TEST_FOR_NULL_THIS(NULL, "di-260");

    XilImage* image =
        new XilImage(this, width, height, nbands, datatype);

    if(completeObjectCreation(image) == XIL_FAILURE) {
        XIL_ERROR(this, XIL_ERROR_SYSTEM, "di-147", FALSE);
        return NULL;
    }
    
    return image;
}

XilImage*
XilSystemState::createXilImage(XilImageFormat* image_format)
{
    _XIL_TEST_FOR_NULL_THIS(NULL, "di-260");

    _XIL_TEST_FOR_NULL_PTR(NULL, "di-448", image_format);

    XilImage* image =
        new XilImage(this, image_format);

    if(completeObjectCreation(image) == XIL_FAILURE) {
        XIL_ERROR(this, XIL_ERROR_SYSTEM, "di-147", FALSE);
        return NULL;
    }
    
    return image;
}

XilImage*
XilSystemState::createXilImageTemp(unsigned int width,
                                   unsigned int height,
                                   unsigned int nbands,
                                   XilDataType  datatype)
{
    _XIL_TEST_FOR_NULL_THIS(NULL, "di-260");

    XilImage* image =
        new XilImage(this, width, height, nbands, datatype, TRUE);

    if(completeObjectCreation(image) == XIL_FAILURE) {
        XIL_ERROR(this, XIL_ERROR_SYSTEM, "di-147", FALSE);
        return NULL;
    }
    
    return image;
}

XilImage*
XilSystemState::createXilImageTemp(XilImageFormat* image_format)
{
    _XIL_TEST_FOR_NULL_THIS(NULL, "di-260");

    _XIL_TEST_FOR_NULL_PTR(NULL, "di-448", image_format);

    XilImage* image =
        new XilImage(this, image_format, TRUE);

    if(completeObjectCreation(image) == XIL_FAILURE) {
        XIL_ERROR(this, XIL_ERROR_SYSTEM, "di-147", FALSE);
        return NULL;
    }
    
    return image;
}

XilImage*
XilSystemState::createXilImage(const char* device_name,
                               XilDevice*  device)
{
    _XIL_TEST_FOR_NULL_THIS(NULL, "di-260");

    //
    //  Verify the device_name is non-NULL.  If it's NULL, then try to get the
    //  name from the device object. 
    //
    if(device_name == NULL) {
        //
        //  Ok, try to get the name from the device object.
        //
        if(device != NULL) {
            device_name = device->getDeviceName();
        }

        if(device_name == NULL) {
            XIL_ERROR(this, XIL_ERROR_USER, "di-146", TRUE);
            return NULL;
        }
    }
    
    //
    //  Get the device manager for the given device.
    //
    XilGlobalState*        xgs = XilGlobalState::getXilGlobalState();
    XilDeviceManagerIO*    mgr =
        xgs->getDeviceManagerIO(device_name, TRUE);

    if(mgr == NULL) {
        //
        //  Couldn't find the device...
        //
        XIL_ERROR(this, XIL_ERROR_SYSTEM, "di-149", FALSE);
        return NULL;
    }

    //
    //  Construct a new device.
    //
    XilDeviceIO* io_device = mgr->constructFromDevice(this, device);
    if(io_device == NULL) {
        XIL_ERROR(this, XIL_ERROR_SYSTEM, "di-149", TRUE);
	return NULL;
    }

    //
    //  Construct the controlling image from the device.
    //
    XilImage*    image = io_device->constructControllingImage();
    if(image == NULL) {
        XIL_ERROR(this, XIL_ERROR_SYSTEM, "di-242", TRUE);
	return NULL;
    }

    //
    //  Associate the IO device with the image
    //
    image->setDeviceIO(io_device);

    //
    //  Set the controlling image on the device as well.
    //
    io_device->setControllingImage(image);

    return image;
}

#if defined(_XIL_HAS_X11WINDOWS) || defined(_WINDOWS)

#ifdef _XIL_HAS_LIBDGA
//
//  Convert a device name to a pipeline name.
//
//  Devices beyond FBYTPE_LASTPLUSONE use the
//  VIS_GETIDENTIFIER interface.
//
char* pipeline_name[FBTYPE_LASTPLUSONE] = {
    "",			// 0, SUN1BW
    "",			// 1, SUN1COLOR
    "SUNWbwtwo",	// 2, SUN2BW
    "SUNWcoltwo",	// 3, SUN2COLOR
    "",			// 4, SUN2GP
    "",			// 5, SUN5COLOR (Roadrunner)
    "SUNWcg3",		// 6, SUN3COLOR
    "SUNWxlib",		// 7, MEMCOLOR 24bit
    "SUNWcg4",		// 8, cg4 on 4/110
    "",			// 9, reserved 3rd party
    "",			// 10,
    "",			// 11,
    "SUNWcg6",		// 12, cg6
    "SUNWcg8",		// 13, cg8 with rop h/w
    "",			// 14, sunvideo
    "",			// 15, 
    "SUNWplasma",	// 16, plasma panel
    "SUNWgs",		// 17, GS
    "SUNWgt",		// 18, GT
    "SUNWleo",	        // 19, LEO
    "SUNWcg14"	        // 20, SX
};

static int
xili_fbtype_to_devname(int   fbtype,
                       char* devname)
{
    if((fbtype < 0) || (fbtype >= FBTYPE_LASTPLUSONE)) {
	strcpy(devname,"");
	return 1;
    } else {
	strcpy(devname, pipeline_name[fbtype]);
#ifdef DEBUG
	fprintf(stderr,"FBTYPE %d : Pipeline name <%s>\n",fbtype,devname);
#endif
	return 0;
    }
}
#endif //_XIL_HAS_LIBDGA

XilDeviceManagerIO*
XilSystemState::getDeviceManagerForWindow(Display*    display,
                                          Window      window,
                                          const char* specific_device)
{
    //
    //  Check that its a valid display pointer
    //
#ifndef _WINDOWS
    if(display == NULL) {
	XIL_ERROR(this, XIL_ERROR_USER, "di-146", TRUE);
	return NULL;
    }
#endif

    //
    //  Check to see if we're supposed to use a specific_device via
    //  environment variable, XIL_DISPLAY. 
    //
    if(specific_device == NULL) {
        specific_device = getenv("XIL_DISPLAY");
    }

#ifdef XIL_XINERAMA
    //
    // Check if Xinerama is present and active.
    // If so, force Xlib to be used, since the XIL
    // DGA pipelines are not Xinerama aware
    //
    {
        XPanoramiXInfo xinfo;
        int major_opcode;
        int event_base;
        int error_base;

        if(XQueryExtension(display, "XINERAMA", 
                           &major_opcode, &event_base, &error_base)) {
            fprintf(stderr, "Xinerama is present\n");
            if(XPanoramiXGetState(display, window, &xinfo) &&
               xinfo.State == 1) {
                fprintf(stderr, "Xinerama is active - using xlib\n");
                specific_device = _XILI_DEFAULT_IO_DISPLAY;
            }
        }
    }
#endif

    //
    //  No way to check for a valid window since all unsigned values are
    //  considered valid.
    //

#ifdef _XIL_HAS_LIBDGA
    //
    //  Here we fill in a static table with the DGA funcions we need
    //  to implement this function.  We don't directly link in libdga because
    //  we don't want to have to increase our memory footprint for libraries
    //  that are only used by this one function.  So, we only load it if this
    //  function is called.
    //
    static XilMutex    dga_procs_table_mutex;
    static struct dga_procs {
        void*           dlhandle;

        void            (*dga_init_version)   (int);
        Dga_drawable    (*XDgaGrabDrawable)   (Display*, Drawable);
        int             (*dga_draw_devfd)     (Dga_drawable);
        int             (*XDgaUnGrabDrawable) (Dga_drawable);
    } xili_dga_procs;
    static Xil_boolean dga_loaded_flag = FALSE;
    static Xil_boolean dga_load_failed = FALSE;

    dga_procs_table_mutex.lock();
    if((dga_loaded_flag == FALSE) && (dga_load_failed== FALSE)) {
        xili_dga_procs.dlhandle = xili_dlopen("libdga.so.1");

        if(xili_dga_procs.dlhandle == NULL) {
            XIL_WARNING(this, XIL_ERROR_CONFIGURATION, "di-369", TRUE);
	    goto dga_load_problem;
	}

        if((xili_dga_procs.dga_init_version =
            (void (*) (int))
            xili_dlsym(xili_dga_procs.dlhandle, "dga_init_version")) == NULL) {

	    xili_dlclose(xili_dga_procs.dlhandle);
            XIL_WARNING(this, XIL_ERROR_SYSTEM, "di-370", TRUE);
	    goto dga_load_problem;
        }
            
        if((xili_dga_procs.XDgaGrabDrawable =
            (Dga_drawable (*) (Display*, Drawable))
            xili_dlsym(xili_dga_procs.dlhandle, "XDgaGrabDrawable")) == NULL) {
	    xili_dlclose(xili_dga_procs.dlhandle);
            XIL_WARNING(this, XIL_ERROR_SYSTEM, "di-370", TRUE);
	    goto dga_load_problem;
        }

        if((xili_dga_procs.dga_draw_devfd = (int (*) (Dga_drawable))
            xili_dlsym(xili_dga_procs.dlhandle, "dga_draw_devfd")) == NULL) {

	    xili_dlclose(xili_dga_procs.dlhandle);
            XIL_WARNING(this, XIL_ERROR_SYSTEM, "di-370", TRUE);
	    goto dga_load_problem;
        }

        if((xili_dga_procs.XDgaUnGrabDrawable =
            (int (*) (Dga_drawable))
             xili_dlsym(xili_dga_procs.dlhandle, "XDgaUnGrabDrawable")) == NULL) {

	    xili_dlclose(xili_dga_procs.dlhandle);
            XIL_WARNING(this, XIL_ERROR_SYSTEM, "di-370", TRUE);
	    goto dga_load_problem;
        }

        dga_loaded_flag = TRUE;
	
dga_load_problem:
        if(dga_loaded_flag == FALSE) {
            dga_load_failed = TRUE;
        }
    }
    dga_procs_table_mutex.unlock();
#endif // _XIL_HAS_LIBDGA

    char displayname[256];

    displayname[0] = '\0';

    //
    // Work out the device type if we're not given one.
    //
    if(specific_device == NULL) {
#ifdef _XIL_HAS_LIBDGA
        if(dga_loaded_flag == TRUE) {
            //
            // Only need to call DGA_INIT once
            //
            static XilMutex    dga_flag_mutex;
            static Xil_boolean dga_initialized = FALSE;

            dga_flag_mutex.lock();
            if(dga_initialized == FALSE) {
                //
                //  I've replaced the call to DGA_INIT() to match what's in
                //  the dga.h header file because it calls dag_init_version()
                //  which we load via dlopen().
                //
                (*xili_dga_procs.dga_init_version)(DGA_CLIENT_VERSION);

                dga_initialized = TRUE;
            }
            dga_flag_mutex.unlock();

            //
            //  Connect to DGA -- DGA is MT-SAFE
            //
            Dga_drawable dga_draw =
                (*xili_dga_procs.XDgaGrabDrawable)(display, window);
            if(dga_draw == NULL) {
                //
                //  Grab failed use the default device
                //
                strncpy(displayname, _XILI_PRIMARY_IO_DISPLAY, 256);
            } else {
                fbgattr        attr;
                fbtype         type;
                vis_identifier vid;
                int            real_type = -1;
                int            devfd     =
                    (*xili_dga_procs.dga_draw_devfd)(dga_draw);

                if(ioctl(devfd, VIS_GETIDENTIFIER, &vid) < 0) {
                    if(ioctl(devfd, FBIOGATTR, &attr) < 0) {
                        if(ioctl(devfd, FBIOGTYPE, &type) < 0) {
                            strncpy(displayname, _XILI_PRIMARY_IO_DISPLAY, 256);
                        } else {
                            real_type = type.fb_type;
                        }
                    } else {
                        real_type = attr.real_type;
                    }
                } else {
                    strncpy(displayname, vid.name, 256);
                }

                (*xili_dga_procs.XDgaUnGrabDrawable)(dga_draw);
                if(real_type >= 0) {
                    if(xili_fbtype_to_devname(real_type, displayname) < 0) {
                        strncpy(displayname, _XILI_PRIMARY_IO_DISPLAY, 256);
                    }
                }
            }
        } else {
            strncpy(displayname, _XILI_PRIMARY_IO_DISPLAY, 256);
        }
#else
        strncpy(displayname, _XILI_PRIMARY_IO_DISPLAY, 256);
#endif // _XIL_HAS_LIBDGA
    } else {
        //
        //  Use the device name provided.
        //
        strncpy(displayname, specific_device, 256);
    }

    //
    // Get a device manager
    //
    XilGlobalState*        xgs = XilGlobalState::getXilGlobalState();
    XilDeviceManagerIO*    mgr =
        xgs->getDeviceManagerIO(displayname,
                                specific_device == NULL ? FALSE : TRUE);

    //
    // Couldn't find the one we really wanted try the
    // other defaults.
    //
    if(mgr == NULL) {
        //
        //  If we're not given a specific device to open, attempt to fall back
        //  to the default display device.
        //
        if(specific_device == NULL) {
            //
            //  Try ioxlib and generate errors if it fails.
            //
            if((mgr = xgs->getDeviceManagerIO(_XILI_PRIMARY_IO_DISPLAY,
                                              FALSE)) == NULL) {
                return NULL;
            }
        } else {
            return NULL;
        }
    }

    return mgr;
}

XilStatus
XilSystemState::finishIODeviceImage(Display*     ,
                                    Window       ,
                                    XilDeviceIO* io_device,
                                    XilImage*    image)
{
    //
    // Associate the IO device with the image
    //
    image->setDeviceIO(io_device);

    //
    //  Set the controlling image on the device as well.
    //
    io_device->setControllingImage(image);

    //
    //  Mark the device as being a "framebuffer" device.
    //
    io_device->markAsFramebufferDevice();
        
    //
    //  TODO:  10/4/95  jlf  Exported state for devices
    //
    //    I/O Devices should contain the information on whether their backing
    //    storage can be exported and it probably should not be a blanket
    //    statement which is made for all I/O devices as in XIL 1.2.
    //

    if(completeObjectCreation(image) == XIL_FAILURE) {
        XIL_ERROR(this, XIL_ERROR_SYSTEM, "di-147", FALSE);
        return XIL_FAILURE;
    }

    return XIL_SUCCESS;
}

//
//  Used by xil_create_from_window. This method creates
//  the device, which creates the image and then we ask
//  the device for the image it created and attach the
//  device to it. The core is the only thing to have access
//  to both the device and the image.
//
XilImage*
XilSystemState::createXilImage(Display*    display,
                               Window      window,
                               const char* specific_device)
{
    _XIL_TEST_FOR_NULL_THIS(NULL, "di-260");

    //
    //  Get the device manager for the given window and display -- or the
    //  named device.
    //
    XilDeviceManagerIO* mgr;
    XilDeviceIO*        io_device = NULL;
    XilImage*           image = NULL;

    mgr = getDeviceManagerForWindow(display, window, specific_device);

    if(mgr != NULL) {
        //
        //  Construct a device and the device image -- for single buffered windows.
        //
        io_device = mgr->constructDisplayDevice(this, display, window);

        //
        //  Construct the controlling image from the device.
        //
        if(io_device != NULL) {
            image = io_device->constructControllingImage();
        }
    }

    //
    //  Did we construct the controlling image on the device successfully?
    //
    //  If not, then we'll attempt to fallback to our DEFAULT display.
    //
    if(image == NULL) {
        //
        //  Specifically go after our default device instead of our primary
        //  device -- unless given a specific device.
        //
        if(specific_device == NULL) {
            //
            //  Delete the io_device since it's no longer needed.
            //
            delete io_device;

            mgr =  getDeviceManagerForWindow(display, window,
                                             _XILI_DEFAULT_IO_DISPLAY);
            if(mgr != NULL) {
                io_device = mgr->constructDisplayDevice(this,
                                                        display, window);

                if(io_device != NULL) {
                    image = io_device->constructControllingImage();
                } else {
                    XIL_ERROR(this, XIL_ERROR_SYSTEM, "di-241", FALSE);
                    return NULL;
                }
            }
        }

        if(image == NULL) {
            //
            //  Delete the io_device since it's no longer needed.
            //
            delete io_device;

            XIL_ERROR(this, XIL_ERROR_SYSTEM, "di-242", FALSE);
            return NULL;
        }
    }

    //
    //  Check the I/O device and setup the image.
    //
    if(finishIODeviceImage(display, window, io_device, image) == XIL_FAILURE) {
        XIL_ERROR(this, XIL_ERROR_SYSTEM, "di-242", FALSE);
        return NULL;
    }

    return image;
}


//
//  Used by xil_create_double_buffered_window.  Just like the above
//  createXilImage method above -- except it creates an I/O device for the
//  given window that supports double buffering if the flag is TRUE
//
XilImage*
XilSystemState::createXilImage(Display*    display,
                               Window      window,
                               Xil_boolean double_buffered)
{
    _XIL_TEST_FOR_NULL_THIS(NULL, "di-260");

    //
    //  Return a regular window with no specific device to be looked for...
    //
    if(! double_buffered) {
        return createXilImage(display, window, (const char*)NULL);
    }

    //
    //  Get the device manager for the given window and display -- or the
    //  named device.
    //
    XilDeviceManagerIO* mgr;
    XilDeviceIO*        io_device = NULL;
    XilImage*           image = NULL;

    mgr = getDeviceManagerForWindow(display, window, NULL);

    if(mgr != NULL) {
        //
        //  Construct a device and the device image -- for double buffered
        //  windows. 
        //
        io_device = mgr->constructDoubleBufferedDisplayDevice(this,
                                                              display,
                                                              window);

        //
        //  Construct the controlling image from the device.
        //
        if(io_device != NULL) {
            image = io_device->constructControllingImage();
        }
    }

    //
    //  Did we construct the controlling image on the device successfully?
    //
    //  If not, then we'll attempt to fallback to our DEFAULT display.
    //
    if(image == NULL) {
        //
        //  Specifically go after our default device instead of our primary
        //  device -- unless given a specific device.
        //
        //  Delete the io_device since it's no longer needed.
        //
        delete io_device;

        mgr =  getDeviceManagerForWindow(display, window,
                                         _XILI_DEFAULT_IO_DISPLAY);
        if(mgr != NULL) {
            io_device = mgr->constructDoubleBufferedDisplayDevice(this,
                                                                  display,
                                                                  window);

            if(io_device != NULL) {
                image = io_device->constructControllingImage();
            } else {
                return NULL;
            }
        }

        if(image == NULL) {
            //
            //  Delete the io_device since it's no longer needed.
            //
            delete io_device;
            return NULL;
        }
    }

    //
    //  Check the I/O device and setup the image.
    //
    if(finishIODeviceImage(display, window, io_device, image) == XIL_FAILURE) {
        return NULL;
    }

    //
    //  Mark the device as being a "double buffering" device.
    //
    io_device->markAsDoubleBufferingDevice();
        
    return image;
}

//
//  Used by xil_create_from_special_window. Just like the above
//  createXilImage method above -- except it creates an I/O device for the
//  given window that supports any enhanced capabilities specified
//  by the XilWindowCaps argument, such as stereo and/or DBE.
//
XilImage*
XilSystemState::createXilImage(Display*      display,
                               Window        window,
                               XilWindowCaps wincaps)
{
    _XIL_TEST_FOR_NULL_THIS(NULL, "di-260");

    Xil_boolean want_dbe    = ( (wincaps & XIL_DOUBLE_BUFFER) != 0 );
    Xil_boolean want_stereo = ( (wincaps & XIL_STEREO) != 0 );

    //
    //  Return a regular window with no specific device to be looked for...
    //
    if(! want_dbe && ! want_stereo) {
        return createXilImage(display, window, (const char*)NULL);
    } else if(want_dbe && ! want_stereo) {
        //
        // Vector a request for a non-stereo, double-buffered image to
        // the double buffer creation code.
        //
        return createXilImage(display, window, TRUE);
    }

    //
    // Now we know that the request is for a stereo image
    //

    //
    //  Get the device manager for the given window and display -- or the
    //  named device.
    //
    XilDeviceManagerIO* mgr;
    XilDeviceIO*        io_device = NULL;
    XilImage*           image = NULL;

    mgr = getDeviceManagerForWindow(display, window, NULL);

    if(mgr != NULL) {
        //
        //  Construct a device and the device image -- for stereo windows. 
        //
        io_device = mgr->constructSpecialDisplayDevice(
                             this, display, window, wincaps);

        //
        //  Construct the controlling image from the device.
        //
        if(io_device != NULL) {
            image = io_device->constructControllingImage();
        }
    }

    //
    //  Did we construct the controlling image on the device successfully?
    //
    //  If not, then we'll attempt to fallback to our DEFAULT display.
    //
    if(image == NULL) {
        //
        //  Specifically go after our default device instead of our primary
        //  device -- unless given a specific device.
        //
        //  Delete the io_device since it's no longer needed.
        //
        delete io_device;

        mgr =  getDeviceManagerForWindow(display, window,
                                         _XILI_DEFAULT_IO_DISPLAY);
        if(mgr != NULL) {
            io_device = mgr->constructSpecialDisplayDevice(
                                 this, display, window, wincaps);


            if(io_device != NULL) {
                image = io_device->constructControllingImage();
            } else {
                return NULL;
            }
        }

        if(image == NULL) {
            //
            //  Delete the io_device since it's no longer needed.
            //
            delete io_device;
            return NULL;
        }
    }

    //
    //  Check the I/O device and setup the image.
    //
    if(finishIODeviceImage(display, window, io_device, image) == XIL_FAILURE) {
        return NULL;
    }

    //
    //  Mark the device as being a stereo and/or double-buffering device
    //
    io_device->markAsStereoDevice();
    if(want_dbe) {
        io_device->markAsDoubleBufferingDevice();
    }

    return image;
}

#endif // _XIL_HAS_X11WINDOWS || _WINDOWS

//
//  Creation of the XilInterpolationTable Object
//
XilInterpolationTable*
XilSystemState::createXilInterpolationTable(unsigned int    kernel_size,
                                            unsigned int    num_subsamples,
                                            float*          init_data)
{
    _XIL_TEST_FOR_NULL_THIS(NULL, "di-260");
    
    XilInterpolationTable* interp_table =
        new XilInterpolationTable(this, kernel_size, num_subsamples, init_data);

    if(completeObjectCreation(interp_table) == XIL_FAILURE) {
        XIL_ERROR(this, XIL_ERROR_SYSTEM, "di-351", FALSE);
        return NULL;
    }

    return interp_table;
}

//
//  XilLookup OBJECT CREATION FUNCTIONS
//
XilLookupColorcube* 
XilSystemState::createXilLookupColorcube(XilDataType  input_type,
                                         XilDataType  output_type,
                                         unsigned int nbands,
                                         short        offset,
                                         int          multipliers[],
                                         unsigned int dimensions[])
{
    _XIL_TEST_FOR_NULL_THIS(NULL, "di-260");
    
    XilLookupColorcube* lookup = 
        new XilLookupColorcube(this, input_type, output_type, nbands, offset,
                      multipliers, dimensions);

    if(completeObjectCreation(lookup) == XIL_FAILURE) {
        XIL_ERROR(this, XIL_ERROR_SYSTEM, "di-192", FALSE);
        return NULL;
    }
    
    return lookup;
}

//
// CreateCopy version
//
XilLookupColorcube* 
XilSystemState::createXilLookupColorcube(XilLookupColorcube* orig_cube)
{
    _XIL_TEST_FOR_NULL_THIS(NULL, "di-260");
    
    XilLookupColorcube* lookup = 
        new XilLookupColorcube(this, orig_cube);

    if(completeObjectCreation(lookup) == XIL_FAILURE) {
        XIL_ERROR(this, XIL_ERROR_SYSTEM, "di-192", FALSE);
        return NULL;
    }
    
    return lookup;
}

XilLookupSingle* 
XilSystemState::createXilLookupSingle(XilDataType  input_type,
                                      XilDataType  output_type,
                                      unsigned int nbands,
                                      unsigned int num_entries,
                                      short        offset,
                                      void*        data)
{
    _XIL_TEST_FOR_NULL_THIS(NULL, "di-260");

    XilLookupSingle* lookup =
        new XilLookupSingle(this, input_type, output_type,
                      nbands, num_entries, offset, data);

    if(completeObjectCreation(lookup) == XIL_FAILURE) {
        XIL_ERROR(this, XIL_ERROR_SYSTEM, "di-192", FALSE);
        return NULL;
    }
    
    return lookup;
}

XilLookupCombined* 
XilSystemState::createXilLookupCombined(XilLookupSingle*   lookup_list[],
                                        unsigned int       num_lookups)
{
    _XIL_TEST_FOR_NULL_THIS(NULL, "di-260");

    XilLookupCombined* lookup =
        new XilLookupCombined(this, lookup_list, num_lookups);

    if(completeObjectCreation(lookup) == XIL_FAILURE) {
        XIL_ERROR(this, XIL_ERROR_SYSTEM, "di-192", FALSE);
        return NULL;
    }
    
    return lookup;
}

XilKernel*
XilSystemState::createXilKernel(unsigned int width,
				unsigned int height,
				int	     key_x,
				int	     key_y,
				float*       input_data)
{
    _XIL_TEST_FOR_NULL_THIS(NULL, "di-260");

    XilKernel* kernel = 
	new XilKernel(this, width, height, key_x, key_y, input_data);

    if(completeObjectCreation(kernel) == XIL_FAILURE) {
        XIL_ERROR(this, XIL_ERROR_SYSTEM, "di-189", FALSE);
	return NULL;
    }

    return kernel;
}

XilKernel*
XilSystemState::createXilKernel(unsigned int width,
                                unsigned int height,
                                int          key_x,
                                int          key_y,
                                float*       x_data,
                                float*       y_data)
{
    _XIL_TEST_FOR_NULL_THIS(NULL, "di-260");

    XilKernel* kernel = 
	new XilKernel(this, width, height, key_x, key_y, x_data, y_data);

    if(completeObjectCreation(kernel) == XIL_FAILURE) {
        XIL_ERROR(this, XIL_ERROR_SYSTEM, "di-189", FALSE);
	return NULL;
    }

    return kernel;
}


XilSel*
XilSystemState::createXilSel(unsigned int  xsize,
			     unsigned int  ysize,
			     int           key_x,
			     int	   key_y,
			     unsigned int *input_data)
{
    _XIL_TEST_FOR_NULL_THIS(NULL, "di-260");

    XilSel* sel = 
	new XilSel(this, xsize, ysize, key_x, key_y, input_data);

    if(completeObjectCreation(sel) == XIL_FAILURE) {
        XIL_ERROR(this, XIL_ERROR_SYSTEM, "di-191", FALSE);
	return NULL;
    }

    return sel;
}

XilDitherMask*
XilSystemState::createXilDitherMask(unsigned int xsize,
				    unsigned int ysize,
				    unsigned int nbands,
				    float       *input_data)
{
    _XIL_TEST_FOR_NULL_THIS(NULL, "di-260");

    XilDitherMask* mask = 
	new XilDitherMask(this, xsize, ysize, nbands, input_data);

    if(completeObjectCreation(mask) == XIL_FAILURE) {
        XIL_ERROR(this, XIL_ERROR_SYSTEM, "di-190", FALSE);
	return NULL;
    }

    return mask;
}

XilHistogram*
XilSystemState::createXilHistogram(unsigned int  nbands,
                                   unsigned int* nbins,
                                   float*        low_value,
                                   float*        high_value)
{
    _XIL_TEST_FOR_NULL_THIS(NULL, "di-260");

    XilHistogram* histogram =
        new XilHistogram(this, nbands, nbins, low_value, high_value);

    if(completeObjectCreation(histogram) == XIL_FAILURE) {
        XIL_ERROR(this, XIL_ERROR_SYSTEM, "di-236", FALSE);
        return NULL;
    }
    
   return histogram;
}

XilRoi*
XilSystemState::createXilRoi()
{
    _XIL_TEST_FOR_NULL_THIS(NULL, "di-260");

    XilRoi* roi = new XilRoi(this);

    if(completeObjectCreation(roi) == XIL_FAILURE) {
        XIL_ERROR(this, XIL_ERROR_SYSTEM, "di-145", FALSE);
        return NULL;
    }

    return roi;
}


XilStorageAPI*
XilSystemState::createXilStorageAPI(XilImage* image)
{
    _XIL_TEST_FOR_NULL_THIS(NULL, "di-260");

    XilStorageAPI* storage = new XilStorageAPI(this, image);

    if(completeObjectCreation(storage) == XIL_FAILURE) {
        XIL_ERROR(this, XIL_ERROR_SYSTEM, "di-374", FALSE);
	return NULL;
    }

    return storage;
}

XilColorspace* 
XilSystemState::createXilColorspace(char*               name,
                                    XilColorspaceOpCode op, 
                                    unsigned int        bands)
{
    _XIL_TEST_FOR_NULL_THIS(NULL, "di-260");

    XilColorspace* colorspace = new XilColorspace(this, name, op, bands);

    if(completeObjectCreation(colorspace) == XIL_FAILURE) {
        XIL_ERROR(this, XIL_ERROR_SYSTEM, "di-298", FALSE);
	return NULL;
    }

    return colorspace;
}

XilColorspace*
XilSystemState::createXilColorspace(XilColorspaceType type,
                                    void*             data,
                                    Xil_boolean       copy_data)
{
    _XIL_TEST_FOR_NULL_THIS(NULL, "di-260");

    XilColorspace* colorspace = new XilColorspace(this, type, data, copy_data);

    if(completeObjectCreation(colorspace) == XIL_FAILURE) {
        XIL_ERROR(this, XIL_ERROR_SYSTEM, "di-298", FALSE);
	return NULL;
    }

    return colorspace;
}

//
// IO device convenience to create an XIL_COLORSPACE_KCS_ID
// colorspace.
//
XilColorspace*
XilSystemState::createXilColorspace(Display* display,
				    int      screen,
				    Visual*  visual)
{
    _XIL_TEST_FOR_NULL_THIS(NULL, "di-260");

    XilColorspace* colorspace = new XilColorspace(this, display, screen, visual);

    if(completeObjectCreation(colorspace) == XIL_FAILURE) {
	//
	// The above call can fail if no window profile is present
	// so we just return NULL.
	//
	return NULL;
    }

    return colorspace;
}

XilColorspaceList*
XilSystemState::createXilColorspaceList(XilColorspace** colorspace_array,
                                        unsigned int    num_colorspaces)
{
    _XIL_TEST_FOR_NULL_THIS(NULL, "di-260");

    XilColorspaceList* colorspacelist =
        new XilColorspaceList(this, colorspace_array, num_colorspaces);

    if(completeObjectCreation(colorspacelist) == XIL_FAILURE) {
        XIL_ERROR(this, XIL_ERROR_SYSTEM, "di-375", FALSE);
	return NULL;
    }

    return colorspacelist;
}

