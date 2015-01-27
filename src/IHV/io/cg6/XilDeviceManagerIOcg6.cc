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
//  File:	XilDeviceManagerIOcg6.cc
//  Project:	XIL
//  Revision:	1.16
//  Last Mod:	10:13:53, 03/10/00
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
//  MT-level:  <SAFE>
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilDeviceManagerIOcg6.cc	1.16\t00/03/10  "

//
//  Utilitiy routines
//
#include "XiliUtils.hh"

//
//  Local includes
//
#include "XilDeviceManagerIOcg6.hh"
#include "XilDeviceIOcg6.hh"

//
//  Create a new instance of the device manager
//
XilDeviceManagerIO*
XilDeviceManagerIO::create(unsigned int  libxil_gpi_major,
                           unsigned int  libxil_gpi_minor,
                           unsigned int* devhandler_gpi_major,
                           unsigned int* devhandler_gpi_minor)
{
    XIL_BASIC_GPI_VERSION_TEST(libxil_gpi_major,
                               libxil_gpi_minor,
                               devhandler_gpi_major,
                               devhandler_gpi_minor);

    XilDeviceManagerIOcg6* mgr = new XilDeviceManagerIOcg6;
    if(mgr == NULL) {
        XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return NULL;
    }

    return mgr;
}

//
//  Constructor
//
XilDeviceManagerIOcg6::XilDeviceManagerIOcg6()
{
    int i;

    //
    //  Initialize the list of CG6 description structure to NULL
    //
    baseCG6Description=NULL;

    //
    //  Initialize the lookup, rescale and window level tables.
    //
    for(i = 0; i < _XILI_NUM_LOOKUP_TABLES; i++) {
        lookupCache[i]   = NULL;
        lookupRefCnts[i] = 0;
    }

    for(i = 0; i < _XILI_NUM_RESCALE_TABLES; i++) {
        rescaleCache[i]   = NULL;
        rescaleRefCnts[i] = 0;
    }

    for(i = 0; i < _XILI_NUM_WINLEV_TABLES; i++) {
        winlevCache[i]   = NULL;
        winlevRefCnts[i] = 0;
    }
}

//
//  Destructor
//
XilDeviceManagerIOcg6::~XilDeviceManagerIOcg6()
{
    int i;

    CG6Description* temp = baseCG6Description;
    while(baseCG6Description) {
        //
        //  Close the framebuffer file descriptor.
        //
        close(baseCG6Description->fd);

        //
        //  Unmap the fbc mappings.
        //
        munmap((char*)baseCG6Description->fb_fbc, CG6_FBCTEC_SZ);

        //
        //  Unmap the framebuffer.
        //
        munmap((char*)baseCG6Description->fb_mem,
               baseCG6Description->fb_size);

        //
        //  Move onto the next and delete this one.
        //
        baseCG6Description = baseCG6Description->next;
        delete temp;
        temp = baseCG6Description;
    }

    for(i = 0; i < _XILI_NUM_LOOKUP_TABLES; i++) {
        delete [] lookupCache[i];
    }

    for(i = 0; i < _XILI_NUM_RESCALE_TABLES; i++) {
        delete [] rescaleCache[i];
    }

    for(i = 0; i < _XILI_NUM_WINLEV_TABLES; i++) {
        if(winlevCache[i] != NULL) {
            delete [] (winlevCache[i] - 32768);
        }
    }
}


//
//  Create a new device
//
XilDeviceIO*
XilDeviceManagerIOcg6::constructDisplayDevice(XilSystemState* state,
                                              Display*        display,
                                              Window          window)
{
    XilDeviceIOcg6* device;

    device = new XilDeviceIOcg6(state, display, window, this);
    if(device == NULL) {
	XIL_ERROR(state, XIL_ERROR_RESOURCE, "di-1", TRUE);
	return NULL;
    }

    if(! device->isOK()) {
        delete device;
        return NULL;
    }

    return device;
}

//
//  Return the device name
//
const char*
XilDeviceManagerIOcg6::getDeviceName()
{
    return "SUNWcg6";
}

//
//  Describe the functions we implement to the XIL Core
//
XilStatus
XilDeviceManagerIOcg6::describeMembers()
{
    XilFunctionInfo*  func_info;

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "display_SUNWcg6");
    func_info->setFunction((XilIOFunctionPtr)XilDeviceIOcg6::display);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "capture_SUNWcg6");
    func_info->setFunction((XilIOFunctionPtr)XilDeviceIOcg6::capture);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "display_SUNWcg6");
    func_info->describeOp(XIL_STEP, 1, "add_const;8");
    func_info->setFunction((XilIOFunctionPtr)XilDeviceIOcg6::addConstDisplay,
                           "display_SUNWcg6(add_const;8))");
    func_info->setPreprocessFunction((XilIOPreprocessFunctionPtr)
                                     XilDeviceIOcg6::addConstDisplayPre);
    func_info->setPostprocessFunction((XilIOPostprocessFunctionPtr)
                                      XilDeviceIOcg6::addConstDisplayPost);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "display_SUNWcg6");
    func_info->describeOp(XIL_STEP, 1, "copy;8");
    func_info->describeOp(XIL_STEP, 1, "capture_SUNWcg6");
    func_info->setFunction((XilIOFunctionPtr)XilDeviceIOcg6::captureCopyDisplay,
                           "display_SUNWcg6(copy;8(capture_SUNWcg6()))");
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "display_SUNWcg6");
    func_info->describeOp(XIL_STEP, 1, "cast;1->8");
    func_info->setFunction((XilIOFunctionPtr)XilDeviceIOcg6::cast1to8Display,
                           "display_SUNWcg6(cast;1->8())");
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "display_SUNWcg6");
    func_info->describeOp(XIL_STEP, 1, "copy;8");
    func_info->setFunction((XilIOFunctionPtr)XilDeviceIOcg6::copyDisplay,
                           "display_SUNWcg6(copy;8())");
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "display_SUNWcg6");
    func_info->describeOp(XIL_STEP, 1, "lookup;1->8");
    func_info->setFunction((XilIOFunctionPtr)XilDeviceIOcg6::lookup1to8Display,
                           "display_SUNWcg6(lookup;1->8())");
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "display_SUNWcg6");
    func_info->describeOp(XIL_STEP, 1, "lookup;8->8");
    func_info->setFunction((XilIOFunctionPtr)XilDeviceIOcg6::lookup8Display,
                           "display_SUNWcg6(lookup;8->8())");
    func_info->setPreprocessFunction((XilIOPreprocessFunctionPtr)
                                     XilDeviceIOcg6::lookup8DisplayPre);
    func_info->setPostprocessFunction((XilIOPostprocessFunctionPtr)
                                      XilDeviceIOcg6::lookup8DisplayPost);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "display_SUNWcg6");
    func_info->describeOp(XIL_STEP, 1, "multiply_const;8");
    func_info->setFunction((XilIOFunctionPtr)XilDeviceIOcg6::mulConstDisplay,
                           "display_SUNWcg6(multiply_const;8))");
    func_info->setPreprocessFunction((XilIOPreprocessFunctionPtr)
                                     XilDeviceIOcg6::mulConstDisplayPre);
    func_info->setPostprocessFunction((XilIOPostprocessFunctionPtr)
                                      XilDeviceIOcg6::mulConstDisplayPost);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "display_SUNWcg6");
    func_info->describeOp(XIL_STEP, 1, "rescale;8");
    func_info->setFunction((XilIOFunctionPtr)XilDeviceIOcg6::rescaleDisplay,
                           "display_SUNWcg6(rescale;8())");
    func_info->setPreprocessFunction((XilIOPreprocessFunctionPtr)
                                     XilDeviceIOcg6::rescaleDisplayPre);
    func_info->setPostprocessFunction((XilIOPostprocessFunctionPtr)
                                      XilDeviceIOcg6::rescaleDisplayPost);
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "display_SUNWcg6");
    func_info->describeOp(XIL_STEP, 1, "scale_nearest;8");
    func_info->setFunction((XilIOFunctionPtr)
			   XilDeviceIOcg6::scaleNearestDisplay,
                           "display_SUNWcg6(scale_nearest;8))");
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "display_SUNWcg6");
    func_info->describeOp(XIL_STEP, 1, "set_value;8");
    func_info->setFunction((XilIOFunctionPtr)XilDeviceIOcg6::setValueDisplay,
                           "display_SUNWcg6(set_value;8))");
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "display_SUNWcg6");
    func_info->describeOp(XIL_STEP, 1, "translate_nearest;8");
    func_info->setFunction((XilIOFunctionPtr)XilDeviceIOcg6::copyDisplay,
                           "display_SUNWcg6(translate_nearest;8))");
    addFunction(func_info);
    func_info->destroy();

    func_info = XilFunctionInfo::create();
    func_info->describeOp(XIL_STEP, 1, "display_SUNWcg6");
    func_info->describeOp(XIL_STEP, 1, "cast;16->8");
    func_info->describeOp(XIL_STEP, 1, "threshold;16");
    func_info->describeOp(XIL_STEP, 1, "threshold;16");
    func_info->describeOp(XIL_STEP, 1, "rescale;16");
    func_info->setFunction((XilIOFunctionPtr)XilDeviceIOcg6::winlevDisplay,
                           "display_SUNWcg6(cast;16->8(threshold;16(threshold;16(rescale;16()))))");
    func_info->setPreprocessFunction((XilIOPreprocessFunctionPtr)
                                     XilDeviceIOcg6::winlevDisplayPre);
    func_info->setPostprocessFunction((XilIOPostprocessFunctionPtr)
                                      XilDeviceIOcg6::winlevDisplayPost);
    addFunction(func_info);
    func_info->destroy();

    return XIL_SUCCESS;
}

//
//  The routine that creates and manages the CGDescription list with a
//    single node entry per CG6 device on the system.
//
CG6Description*
XilDeviceManagerIOcg6::getCG6Description(char*           name,
					 XilSystemState* state)
{
    //
    // Make sure only one thread opens the device and
    // does the mapping.
    //
    mutex.lock();
    
    //
    //  Look through the list to determine if the device has already
    //  been opened.
    //
    CG6Description* tmp = baseCG6Description;
    while(tmp) {
	if(strcmp(name, tmp->name) == 0) {
	    mutex.unlock();
	    return tmp;
	}
	tmp = tmp->next;
    }

    //
    //  Well, this device hasn't opened yet so we'll go ahead and
    //  create a new description.
    //
    CG6Description* description= new CG6Description;
    if(!description) {
	XIL_ERROR(state,XIL_ERROR_SYSTEM,"di-1",TRUE);
	mutex.unlock();
	return NULL;
    }
    strcpy(description->name,name);

    //
    //  Now we actually open the device and get its attributes.
    //
    description->fd = open(description->name, O_RDWR);
    if(description->fd < 0) {
	XIL_ERROR(state,XIL_ERROR_RESOURCE,"di-212",TRUE);
	delete description;
	mutex.unlock();
	return NULL;
    }
    
    //
    //  Get the device attributes.
    //
    struct fbgattr attr;
    if(ioctl(description->fd, FBIOGATTR, &attr) < 0) {
	XIL_ERROR(state,XIL_ERROR_SYSTEM,"di-213",TRUE);
	close(description->fd);
	delete description;
	mutex.unlock();
	return NULL;
    }

    //
    //  Be certain the device is really a CG6.
    //
    if(attr.real_type != FBTYPE_SUNFAST_COLOR) {
	//
	//  Somehow we were called on a non-CG6 framebuffer.
	//    Definite error.
	//
	XIL_ERROR(state,XIL_ERROR_SYSTEM,"di-214",TRUE);
	close(description->fd);
	delete description;
	mutex.unlock();
	return NULL;
    }

    //
    //  Get the information describing the CG6 from the FBIOGXINFO
    //  ioctl call.
    //
    cg6_info cg6_information;
    if(ioctl(description->fd, FBIOGXINFO, &cg6_information) < 0) {
	XIL_ERROR(state,XIL_ERROR_SYSTEM,"di-213",TRUE);
	close(description->fd);
	delete description;
	mutex.unlock();
	return NULL;
    }

    //
    //  Fill our description of the CG6.
    //
    description->fb_width  = cg6_information.accessible_width;
    description->fb_height = cg6_information.accessible_height;
    description->fb_size   = cg6_information.vmsize*1024*1024;

    //
    //  Get the register mappings.
    //
    description->fb_fbc = (fbc*)
	mmap(NULL, CG6_FBCTEC_SZ, PROT_READ|PROT_WRITE, MAP_PRIVATE,
	     description->fd, CG6_VADDR_FBCTEC);
    if(description->fb_fbc == (fbc*) -1) {
	XIL_ERROR(state,XIL_ERROR_SYSTEM,"di-215",TRUE);
	close(description->fd);
	delete description;
	mutex.unlock();
	return NULL;
    }

    //
    //  Map the framebuffer itself
    //
    description->fb_mem = (Xil_unsigned8*)
	mmap(NULL, description->fb_size, PROT_READ|PROT_WRITE, MAP_SHARED,
	     description->fd, CG6_VADDR_COLOR);
    if(description->fb_mem == (Xil_unsigned8*) -1) {
	XIL_ERROR(state,XIL_ERROR_SYSTEM,"di-215",TRUE);
	munmap((caddr_t)description->fb_fbc, CG6_FBCTEC_SZ);
	close(description->fd);
	delete description;
	mutex.unlock();
	return NULL;
    }
   
    //
    //  Wait to ensure the GX is idle and ready for us to set some
    //  registers.
    //
    while(description->fb_fbc->l_fbc_status & L_FBC_BUSY);

    //
    //  Initialize the registers to what we want done.
    //
    description->fb_fbc->l_fbc_misc.l_fbc_misc_blit=L_FBC_MISC_BLIT_NOSRC;
    description->fb_fbc->l_fbc_misc.l_fbc_misc_data=L_FBC_MISC_DATA_COLOR8;
    description->fb_fbc->l_fbc_misc.l_fbc_misc_draw=L_FBC_MISC_DRAW_RENDER;
    description->fb_fbc->l_fbc_misc.l_fbc_misc_bwrite0=L_FBC_MISC_BWRITE0_ENABLE;
    description->fb_fbc->l_fbc_misc.l_fbc_misc_bwrite1=L_FBC_MISC_BWRITE1_DISABLE;
    description->fb_fbc->l_fbc_misc.l_fbc_misc_bread=L_FBC_MISC_BREAD_0;
    description->fb_fbc->l_fbc_planemask= 0xff;
    description->fb_fbc->l_fbc_pixelmask= 0xffffffff;
    description->fb_fbc->l_fbc_clipcheck= 0;
    description->fb_fbc->l_fbc_rasteroffx= 0;
    description->fb_fbc->l_fbc_rasteroffy=0;
    description->fb_fbc->l_fbc_autoincx= 0;
    description->fb_fbc->l_fbc_autoincy= 0;

    //
    //  Add the newly created description to our description list and
    //    return the new description to the caller.
    //
    description->next= baseCG6Description;
    baseCG6Description= description;

    //
    // Everything mapped, unlock the mutex
    //
    mutex.unlock();
    return description;
}

//////////////////////////////////////////////////////////////////////////
//
//  Rescale table aquisition
//
//////////////////////////////////////////////////////////////////////////
int
XilDeviceManagerIOcg6::getRescaleTable(XilSystemState* state,
                                       float           mult_const,
                                       float           add_const)
{
    //
    //  The table information for each band.
    //
    int table = -1;
    
    //
    //  Check each of our caches to see if one of them contains the same
    //  inforamtion as our operation.
    //
    rescaleCacheMutex.lock();

    int         empty_table = -1;
    Xil_boolean table_match = FALSE;

    for(int i=0; i<_XILI_NUM_RESCALE_TABLES; i++) {
        //
        //  Is there data in this table?  If not, consider it empty and keep
        //  looking...  
        //
        if(rescaleCache[i] == NULL) {
            if(empty_table == -1 || rescaleCache[empty_table] != NULL) {
                empty_table = i;
            }

            continue;
        }

        //
        //  Assume this is a table we can use...
        //
        table_match = TRUE;
        if((rescaleCacheInfo[i].multConst != mult_const) ||
           (rescaleCacheInfo[i].addConst  != add_const)) {
            table_match = FALSE;
        }

        //
        //  Any luck?
        //
        if(table_match == TRUE) {
            //
            //  Ok, we've found a match for this band.
            //
            rescaleRefCnts[i]++;
            table = i;
            break;
        }

        //
        //  Is nobody else using it (i.e. can we replace it later?)
        //
        if(rescaleRefCnts[i] == 0 && empty_table == -1) {
            empty_table = i;
        }
    }

    //
    //  Did we find an available table band at all?
    //
    if(empty_table == -1 && table_match == FALSE) {
        if(table != -1) {
            rescaleRefCnts[table]--;
        }

        rescaleCacheMutex.unlock();
        return -1;
    } else if(table_match == TRUE) {
        //
        //  Found one -- return it.
        //
        rescaleRefCnts[table]++;
        rescaleCacheMutex.unlock();

        return table;
    }

    //
    //  Fill in the table...
    //
    int et = empty_table;

    if(rescaleCache[et] == NULL) {
        rescaleCache[et] = new Xil_unsigned8[256];

        if(rescaleCache[et] == NULL) {
            if(table != -1) {
                rescaleRefCnts[table]--;
            }

            XIL_ERROR(state, XIL_ERROR_RESOURCE, "di-1", TRUE);
            rescaleCacheMutex.unlock();
            return -1;
        }
    }

    Xil_unsigned8* cdata = rescaleCache[et];
    float tv;
    for(int j=0; j<256; j++) {
        tv       = (mult_const * (float)j) + add_const;
        cdata[j] = _XILI_ROUND_U8(tv);
    }

    rescaleCacheInfo[et].multConst = mult_const;
    rescaleCacheInfo[et].addConst  = add_const;

    //
    //  Bump the reference count for the table, indicate we found a table
    //  for this band and then move onto the next band.
    //
    rescaleRefCnts[et]++;
    table = et;

    rescaleCacheMutex.unlock();

    return table;
}

void
XilDeviceManagerIOcg6::releaseRescaleTable(int table)
{
    rescaleCacheMutex.lock();

    rescaleRefCnts[table]--;

    rescaleCacheMutex.unlock();
}

//////////////////////////////////////////////////////////////////////////
//
//  Lookup table aquisition
//
//////////////////////////////////////////////////////////////////////////
int
XilDeviceManagerIOcg6::getLookupTable(XilSystemState*  state,
                                      XilLookupSingle* lookup)
{
    //
    //  The table information for each band.
    //
    int table = -1;
    
    //
    //  Check each of our caches to see if one of them contains the same
    //  inforamtion as our operation.
    //
    lookupCacheMutex.lock();

    int         empty_table = -1;
    Xil_boolean table_match = FALSE;

    for(int i=0; i<_XILI_NUM_LOOKUP_TABLES; i++) {
        //
        //  Is there data in this table?  If not, consider it empty and keep
        //  looking...  
        //
        if(lookupCache[i] == NULL) {
            if(empty_table == -1 || lookupCache[empty_table] != NULL) {
                empty_table = i;
            }

            continue;
        }

        //
        //  Assume this is a table we cannot use...
        //
        if(lookup->isSameAs(&lookupCacheInfo[i])) {
            //
            //  Ok, we've found a match for this lookup.
            //
            lookupRefCnts[i]++;
            table = i;
            table_match = TRUE;
            break;
        }

        //
        //  Is nobody else using it (i.e. can we replace it later?)
        //
        if(lookupRefCnts[i] == 0 && empty_table == -1) {
            empty_table = i;
        }
    }

    //
    //  Did we find an available table band at all?
    //
    if(empty_table == -1 && table_match == FALSE) {
        if(table != -1) {
            lookupRefCnts[table]--;
        }

        lookupCacheMutex.unlock();
        return -1;
    } else if(table_match == TRUE) {
        //
        //  Found one -- return it.
        //
        lookupRefCnts[table]++;
        lookupCacheMutex.unlock();

        return table;
    }

    //
    //  Fill in the table...
    //
    int et = empty_table;

    if(lookupCache[et] == NULL) {
        lookupCache[et] = new Xil_unsigned8[256];

        if(lookupCache[et] == NULL) {
            if(table != -1) {
                lookupRefCnts[table]--;
            }

            XIL_ERROR(state, XIL_ERROR_RESOURCE, "di-1", TRUE);
            lookupCacheMutex.unlock();
            return NULL;
        }
    }

    Xil_unsigned8* cdata       = lookupCache[et];
    Xil_unsigned8* ldata       = (Xil_unsigned8*)lookup->getData();
    int            offset      = lookup->getOffset();
    unsigned int   num_entries = lookup->getNumEntries();

    if(offset) {
        xili_memset(cdata, ldata[0], offset);
    }

    xili_memcpy(cdata + offset, ldata, num_entries);

    if(offset + num_entries < 256) {
        xili_memset(cdata + offset + num_entries,
                    ldata[num_entries - 1],
                    256 - num_entries - offset);
    }

    //
    //  Store this lookup's version information.
    //
    lookup->getVersion(&lookupCacheInfo[et]);

    //
    //  Bump the reference count for the table, indicate we found a table
    //  for this band and then move onto the next band.
    //
    lookupRefCnts[et]++;
    table = et;

    lookupCacheMutex.unlock();

    return table;
}

void
XilDeviceManagerIOcg6::releaseLookupTable(int table)
{
    lookupCacheMutex.lock();

    lookupRefCnts[table]--;

    lookupCacheMutex.unlock();
}

//////////////////////////////////////////////////////////////////////////
//
//  Window Level table aquisition
//
//////////////////////////////////////////////////////////////////////////
int
XilDeviceManagerIOcg6::getWinlevTable(XilSystemState* state,
                                      float           mult_const,
                                      float           add_const,
                                      Xil_signed16    low1,
                                      Xil_signed16    high1,
                                      Xil_signed16    map1,
                                      Xil_signed16    low2,
                                      Xil_signed16    high2,
                                      Xil_signed16    map2)
{
    //
    //  The table information for each band.
    //
    int table = -1;
    
    //
    //  Check each of our caches to see if one of them contains the same
    //  inforamtion as our operation.
    //
    winlevCacheMutex.lock();

    int         empty_table = -1;
    Xil_boolean table_match = FALSE;

    for(int i=0; i<_XILI_NUM_WINLEV_TABLES; i++) {
        //
        //  Is there data in this table?  If not, consider it empty and keep
        //  looking...  
        //
        if(winlevCache[i] == NULL) {
            if(empty_table == -1 || winlevCache[empty_table] != NULL) {
                empty_table = i;
            }

            continue;
        }

        //
        //  Assume this is a table we can use...
        //
        table_match = TRUE;
        if((winlevCacheInfo[i].multConst != mult_const) ||
           (winlevCacheInfo[i].addConst  != add_const)  ||
           (winlevCacheInfo[i].low1      != low1)       ||
           (winlevCacheInfo[i].high1     != high1)      ||
           (winlevCacheInfo[i].map1      != map1)       ||
           (winlevCacheInfo[i].low2      != low2)       ||
           (winlevCacheInfo[i].high2     != high2)      ||
           (winlevCacheInfo[i].map2      != map2)) {
            table_match = FALSE;
        }

        //
        //  Any luck?
        //
        if(table_match == TRUE) {
            //
            //  Ok, we've found a match for this band.
            //
            winlevRefCnts[i]++;
            table = i;
            break;
        }

        //
        //  Is nobody else using it (i.e. can we replace it later?)
        //
        if(winlevRefCnts[i] == 0 && empty_table == -1) {
            empty_table = i;
        }
    }

    //
    //  Did we find an available table band at all?
    //
    if(empty_table == -1 && table_match == FALSE) {
        if(table != -1) {
            winlevRefCnts[table]--;
        }

        winlevCacheMutex.unlock();
        return -1;
    } else if(table_match == TRUE) {
        //
        //  Found one -- return it.
        //
        winlevRefCnts[table]++;
        winlevCacheMutex.unlock();

        return table;
    }

    //
    //  Fill in the table...
    //
    int et = empty_table;

    if(winlevCache[et] == NULL) {
        winlevCache[et] = new Xil_unsigned8[65536];

        if(winlevCache[et] == NULL) {
            if(table != -1) {
                winlevRefCnts[table]--;
            }

            XIL_ERROR(state, XIL_ERROR_RESOURCE, "di-1", TRUE);
            winlevCacheMutex.unlock();
            return -1;
        }

        winlevCache[et] = &winlevCache[et][32768];
    }

    Xil_unsigned8* cdata = winlevCache[et];

    //
    //  Rebuild the table.
    //
    int high[2];
    high[0] = _XILI_ROUND_S16(((float)(high1 - add_const)) / mult_const);
    high[1] = _XILI_ROUND_S16(((float)(high2 - add_const)) / mult_const);
    int highest;
    int not_highest;
    if(high[0] > high[1]) {
        highest     = 0;
        not_highest = 1;
    } else {
        highest     = 1;
        not_highest = 0;
    }

    int low[2];
    low[0] = _XILI_ROUND_S16(((float)(low1 - add_const)) / mult_const);
    low[1] = _XILI_ROUND_S16(((float)(low2 - add_const)) / mult_const);
    int lowest;
    int not_lowest;
    if(low[0] < low[1]) {
        lowest      = 0;
        not_lowest  = 1;
    } else {
        lowest      = 1;
        not_lowest  = 0;
    }

    int map[2];
    map[0] = map1;
    map[1] = map2;
    if(highest == lowest) {
        //
        //  Range consumed by 1 threshold -- do threshold, rescale elsewhere
        //
        for(i=low[lowest]; i<=high[highest]; i++) {
            cdata[i] = map[lowest];
        }

        for(i=-32768; i<low[lowest]; i++) {
            cdata[i] = (Xil_unsigned8)
                _XILI_ROUND_S16(((float)i) * mult_const + add_const);
        }

        for(i=high[highest] + 1; i<32767; i++) {
            cdata[i] = (Xil_unsigned8)
                _XILI_ROUND_S16(((float)i) * mult_const + add_const);
        }
    } else if(high[not_highest] > low[not_lowest]) {
        //
        //  Ranges overlap -- do two thresholds, rescale elsewhere
        //
        for(i=low[1]; i<=high[1]; i++) {
            cdata[i] = map[1];
        }

        for(i=low[0]; i<=high[0]; i++) {
            cdata[i] = map[0];
        }

        for(i=-32768; i<low[lowest]; i++) {
            cdata[i] = (Xil_unsigned8)
                _XILI_ROUND_S16(((float)i) * mult_const + add_const);
        }

        for(i=high[highest] + 1; i<32767; i++) {
            cdata[i] = (Xil_unsigned8)
                _XILI_ROUND_S16(((float)i) * mult_const + add_const);
        }
    } else if(low[not_lowest] > high[not_highest]) {
        //
        //  Disjoint ranges
        //
        for(i=low[1]; i<=high[1]; i++) {
            cdata[i] = map[1];
        }

        for(i=low[0]; i<=high[0]; i++) {
            cdata[i] = map[0];
        }

        for(i=high[not_highest] + 1; i<low[not_lowest]; i++) {
            cdata[i] = (Xil_unsigned8)
                _XILI_ROUND_S16(((float)i) * mult_const + add_const);
        }

        for(i=-32768; i<low[lowest]; i++) {
            cdata[i] = (Xil_unsigned8)
                _XILI_ROUND_S16(((float)i) * mult_const + add_const);
        }

        for(i=high[highest] + 1; i<32767; i++) {
            cdata[i] = (Xil_unsigned8)
                _XILI_ROUND_S16(((float)i) * mult_const + add_const);
        }
    } else {
        winlevCacheMutex.unlock();
        return -1;
    }

    //
    //  Store cache parameters...
    //
    winlevCacheInfo[et].multConst = mult_const;
    winlevCacheInfo[et].addConst  = add_const;
    winlevCacheInfo[et].low1      = low1;
    winlevCacheInfo[et].high1     = high1;
    winlevCacheInfo[et].map1      = map1;
    winlevCacheInfo[et].low2      = low2;
    winlevCacheInfo[et].high2     = high2;
    winlevCacheInfo[et].map2      = map2;

    //
    //  Bump the reference count for the table, indicate we found a table
    //  for this band and then move onto the next band.
    //
    winlevRefCnts[et]++;
    table = et;

    winlevCacheMutex.unlock();

    return table;
}

void
XilDeviceManagerIOcg6::releaseWinlevTable(int table)
{
    winlevCacheMutex.lock();

    winlevRefCnts[table]--;

    winlevCacheMutex.unlock();
}

