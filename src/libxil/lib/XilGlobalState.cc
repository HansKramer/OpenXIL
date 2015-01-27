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
//  File:	XilGlobalState.cc
//  Project:	XIL
//  Revision:	1.96
//  Last Mod:	10:08:01, 03/10/00
//
//  Description:
//	
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilGlobalState.cc	1.96\t00/03/10  "

//
//  System Includes
//
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WINDOWS
#include <unistd.h>
#ifdef HPUX
#include <dl.h>
#else
#include <dlfcn.h>
#endif
#endif

#ifdef SOLARIS
#include <X11/Sunowconfig.h>
#endif

//
//  C Includes
//
#include "_XilDefines.h"

//
//  C++ Includes
//
#include "XiliUtils.hh"
#include "_XilGlobalState.hh"
#include "_XilMutex.hh"
#include "_XilSystemState.hh"
#include "_XilDeviceManagerCompute.hh"

//
//  libxil Internal Includes
//
#include "XiliOpTreeNode.hh"
#include "XiliProcessEnv.hh"
#include "XiliDagManager.hh"
#include "XiliScheduler.hh"

// TEMPORARY!
// Put in some definitions to keep things going ...
#ifdef HPUX
#define _SC_PHYS_PAGES -1
#define RTLD_LAZY      -1
#endif

//
//  Class-specific static data
//
XilGlobalState* XilGlobalState::theXGS;
XilOpNumber     XilGlobalState::maxOpNum;
XilMutex        XilGlobalState::maxOpNumMutex;

//
//  The XilGlobalState contructor...
//
//     NOT MT-safe because only one thread can create the global
//     state.
//
//  The computeOpNameTable is large because we are expecting a lot of
//  compute operations during normal operation.  I've chosen 1597 because it's
//  prime and it's larger than the expected number of entries expected in the
//  hash table. 
//
XilGlobalState::XilGlobalState() :
    computePrefix("Compute"),
    storagePrefix("Storage"),
    inputOutputPrefix("IO"),
    compressionPrefix("Compress"),
    opPrefix("Op"),
    computeOpNameTable(1597)
{
    isOKFlag = FALSE;

    //
    //  Set theXGS to "this" immediately because some of the constructors may
    //  attempt to access the global state.
    //
    theXGS = this;

    //
    //  Set the non-Solaris variable configFILE to NULL just in case we fail
    //  so we don't SEGV in the destructor.
    //
#ifndef SOLARIS
    configFILE = NULL;
#endif
    
    //
    //  Read the XIL_RELEASE_DIR environment variable.
    //
#ifndef _XIL_RELEASE_BUILD
    envReleaseDir = getenv("XILHOME");
    if(envReleaseDir != NULL) {
        envReleaseDir = strdup(envReleaseDir);
        if(envReleaseDir == NULL) {
            return;
        }
    }
#endif

    //
    //  Parse the XIL_DEBUG variable to get the assorted flag values by using
    //  XiliProcessEnv. 
    //
    XiliString xil_debug("XIL_DEBUG");
    if(XiliProcessEnv::isSet(xil_debug)) {
        XiliString link("LINK");
        librarySuffix = (char*)XiliProcessEnv::getValue(xil_debug, link);
        if(librarySuffix == NULL) {
            librarySuffix = "";
        }

        XiliString set_synchronize("set_synchronize");
        envSetSynchronize  = XiliProcessEnv::isSet(xil_debug, set_synchronize);

        XiliString show_action("show_action");
        envShowAction      = XiliProcessEnv::isSet(xil_debug, show_action);

        XiliString provide_warnings("provide_warnings");
        envProvideWarnings = XiliProcessEnv::isSet(xil_debug, provide_warnings);

        XiliString tiling_mode("tiling_mode");
        char* tmode = (char*)XiliProcessEnv::getValue(xil_debug, tiling_mode);
        if(tmode != NULL) {
            if(strncmp(tmode, "tile", 4) == 0) {
                envTilingMode = XIL_TILING;
            } else if(strncmp(tmode, "strip", 5) == 0) {
                envTilingMode = XIL_STRIPPING;
            } else {
                envTilingMode = XIL_WHOLE_IMAGE;
            }
        } else {
            envTilingMode = XIL_WHOLE_IMAGE;
        }

        XiliString txsize("txsize");
        char* env_txsize = (char*)XiliProcessEnv::getValue(xil_debug, txsize);
        if(env_txsize != NULL) {
            envTileSizeX = atoi(env_txsize);
        } else {
            envTileSizeX = 0;
        }

        XiliString tysize("tysize");
        char* env_tysize = (char*)XiliProcessEnv::getValue(xil_debug, tysize);
        if(env_tysize != NULL) {
            envTileSizeY = atoi(env_tysize);
        } else {
            envTileSizeY = 0;
        }

        XiliString threads("threads");
        char* env_threads = (char*)XiliProcessEnv::getValue(xil_debug, threads);
        if(env_threads != NULL) {
            envNumThreads = atoi(env_threads);
        } else {
            envNumThreads = 0;
        }

        XiliString split_threshold("split_threshold");
        char* env_sthresh = (char*)XiliProcessEnv::getValue(xil_debug, split_threshold);
        if(env_sthresh != NULL) {
            envSplitThreshold = atoi(env_sthresh);
        } else {
            envSplitThreshold = 0;
        }

        //
        //  Since setting XIL_DEBUG is a special debugging condition, we
        //  output the status of the XIL_DEBUG flags to assist knowing what
        //  has been set and what has not been set.
        //
        fprintf(stderr, "\n---- XIL_DEBUG Status ----\n");
        fprintf(stderr, "  libSuffix:          %s\n", librarySuffix);
        fprintf(stderr, "  show_action:        %s\n",
                envShowAction ? "TRUE" : "FALSE");
        fprintf(stderr, "  set_synchronize:    %s\n",
                envSetSynchronize ? "TRUE" : "FALSE");
        fprintf(stderr, "  provide_warnings:   %s\n",
                envProvideWarnings ? "TRUE" : "FALSE");
        fprintf(stderr, "  tiling_mode:        %s\n",
                envTilingMode == XIL_TILING ? "XIL_TILING" :
                (envTilingMode == XIL_STRIPPING ? "XIL_STRIPPING" : "XIL_WHOLE_IMAGE"));
        fprintf(stderr, "  txsize:             %d\n", envTileSizeX);
        fprintf(stderr, "  tysize:             %d\n", envTileSizeY);
        fprintf(stderr, "  threads:            %d\n", envNumThreads);
        fprintf(stderr, "  split_threshold:    %d\n", envSplitThreshold);
        fprintf(stderr, "\n");
    } else {
        librarySuffix      = "";
        envSetSynchronize  = FALSE;
        envShowAction      = FALSE;
        envProvideWarnings = FALSE;
        envTilingMode      = XIL_WHOLE_IMAGE;
        envTileSizeX       = 0;
        envTileSizeY       = 0;
        envNumThreads      = 0;
        envSplitThreshold  = 0;
    }

    //
    //  The XiliOpTree
    //
    opTreeBase      = new XiliOpTreeNode;
    if(opTreeBase == NULL) {
        return;
    }
    
    //
    //  The XiliDagManager
    //
    dagManager      = new XiliDagManager;
    if(dagManager == NULL) {
        return;
    }
    
    //
    //  The Operation Scheduler
    //
    opScheduler     = new XiliScheduler;
    if(opScheduler == NULL) {
        return;
    }
    
    //
    //  Maximum Operation Number in Op Tree
    //
    maxOpNum = 0;
    
    //
    //  Check the creation of our symbol tables.
    //
    if(symtabCompute.isOK()      == FALSE) return;
    if(symtabStorage.isOK()      == FALSE) return;
    if(symtabInputOutput.isOK()  == FALSE) return;
    if(symtabCompression.isOK()  == FALSE) return;
    if(symtabOp.isOK()           == FALSE) return;
    if(computeOpNameTable.isOK() == FALSE) return;

    //
    //  Create array to hold the operation names...
    //
    numOpNames = 64;
    computeOpNames = new char*[numOpNames];
    if(computeOpNames == NULL) {
        return;
    }
    for(unsigned int i=0; i<numOpNames; i++) {
        computeOpNames[i] = NULL;
    }

    //
    //  Calculate the bytesPerTile value based on the size of system memory.
    //
    bytesPerTile = computeBytesPerTile();

    isOKFlag = TRUE;
}

//
//  The XilGlobalState destructor
//
//     NOT MT-safe because only one thread can destroy the global
//     state.
//
XilGlobalState::~XilGlobalState()
{
#ifndef _XIL_RELEASE_BUILD
    //
    //  Created via strdup().
    //
    free(envReleaseDir);
#endif

    delete opTreeBase;
    delete dagManager;
    delete opScheduler;

    //
    //  Delete all of the entries in the table of compute op names.
    //  We start at 1 instead of 0 because there is no such thing as op #0.
    //
    for(unsigned int i=1; i<numOpNames; i++) {
        //
        //  computeOpNames[i] allocated via strdup() (i.e. malloc())
        //
        free(computeOpNames[i]);
    }
    delete [] computeOpNames;

    //
    //  Cleanup the XiliProcessEnv class
    //
    XiliProcessEnv::cleanup();

#ifdef SOLARIS
    OWconfigClose(NULL);
#else
    fclose(configFILE);
#endif

    //
    //  We must set 'theXGS' to NULL in case the application attempts to
    //  reopen XIL.  Unless theXGS is NULL, the library will not construct a
    //  new global state.
    //
    theXGS = NULL;

    //
    //  The destructors for the hash tables will delete all of their values
    //  (XiliDeviceRecord*) whose destructor calls destroy() on the device
    //  manager it contains.  Thus, all of the device managers are destroyed
    //  without requiring explicit code here.
    //
}

//
//  Verification the global state was created correctly.
//
Xil_boolean
XilGlobalState::isOK() {
    _XIL_ISOK_TEST();
}

//
//  Get the instantiation of this class.
//
//     Creation is NOT MT-safe because XilOpen ensures only one thread
//     can be requesting creation of the global state.
//
XilGlobalState*
XilGlobalState::getXilGlobalState()
{
    if(theXGS == NULL) {
        theXGS = new XilGlobalState();
        
        if(theXGS == NULL || theXGS->isOK() == FALSE) {
            XIL_ERROR(NULL, XIL_ERROR_SYSTEM, "di-246", FALSE);
            delete theXGS;
            theXGS = NULL;
        }
    }
    
    return theXGS;
}


//
//  Create a new system state.
//
//    Fully MT-safe since many threads can create system states at the
//    same time.
//
XilSystemState*
XilGlobalState::createSystemState()
{
    XilSystemState* new_system_state = new XilSystemState();
    if(new_system_state == NULL) {
        XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
    }

    if(new_system_state->isOK() == FALSE) {
        XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-244", TRUE);
        return NULL;
    }

    //
    //  Keep a linked list of system states...
    //
    systemStateListMutex.lock();

    systemStateList.append(new_system_state);
    
    systemStateListMutex.unlock();

    return new_system_state;
}   

//
//  Return the first system state in the list.
//
XilSystemState*
XilGlobalState::getFirstSystemState()
{
    XilSystemState* sys_state;

    systemStateListMutex.lock();

    XiliListPosition pos = systemStateList.head();

    if(pos == _XILI_LIST_INVALID_POSITION) {
        sys_state = NULL;
    } else {
        sys_state = systemStateList.reference(pos);
    }
    
    systemStateListMutex.unlock();

    return sys_state;
}

//
//  Destroy a system state
//
void
XilGlobalState::destroySystemState(XilSystemState* state)
{
    _XIL_TEST_FOR_NULL_PTR_VOID("di-260", state);

    //
    //  Start looking through our list of system states.
    //
    systemStateListMutex.lock();

    XiliListIterator<XilSystemState>  li(&systemStateList);
    XilSystemState*                   tmpstate;
    
    while(tmpstate = li.getNext()) {
        if(tmpstate == state) {
            systemStateList.remove(li.getCurrentPosition());
            
            delete state;

            break;
        }
    }

    if(systemStateList.isEmpty()) {
        systemStateListMutex.unlock();

        //
        //  If it's the last system state, then we have no need for the global
        //  state because all xil_open() calls have been matched with
        //  xil_close() calls and the appilcation's use of the library is
        //  complete.
        //
        delete this;
    } else {
        systemStateListMutex.unlock();
    }
}

//
//  Calculate a target number of bytes for a single tile within XIL.  The
//  XilImage uses this value combined with the TILING_MODE, the number of
//  bands in an image and the datatype of an image to give the image a tile
//  size.
//
unsigned int
XilGlobalState::computeBytesPerTile()
{
    //
    //  The calculation determines the amount of physical memory on the
    //  machine, assumes the tiles can use 75% of the memory without major
    //  paging problems and then computes what size the tiles would need to be
    //  in order to fit 16 of them into that quantity of memory.  Finally,
    //  this number is clamped to a maximum of 4Mb because once tiles get any
    //  bigger, they can't be moved in and out of memory very efficiently.
    //
    float physical_mem_bytes;
    physical_mem_bytes  = (float) xili_sysconf(_SC_PHYS_PAGES);
    physical_mem_bytes *= (float) xili_sysconf(_SC_PAGESIZE);

    //
    //  Calculate what 75% would be.
    //
    physical_mem_bytes *= 0.75F;

    //
    //  Calculate what it takes to get 8 tiles into that space.
    //
    physical_mem_bytes /= 8.0F;

    //
    //  Verify it's no bigger than 4Mb -- if so, clamp it to 4Mb
    //
    if(physical_mem_bytes > 4.0F*1024.0F*1024.0F) {
        physical_mem_bytes = 4.0F*1024.0F*1024.0F;
    }

    return (unsigned int)physical_mem_bytes;
}

//
//  Return the highest numbered compute op in the entire tree.
//
XilOpNumber
XilGlobalState::getMaxOpNumber()
{
    XilOpNumber tmp;
    
    maxOpNumMutex.lock();
    tmp = maxOpNum;
    maxOpNumMutex.unlock();

    return tmp;
}

//
//  Increment and return the highest numbered compute op in
//    the entire tree.
//
XilOpNumber
XilGlobalState::incrementMaxOpNumber()
{
    XilOpNumber tmp;
    
    maxOpNumMutex.lock();
    tmp = ++maxOpNum;
    maxOpNumMutex.unlock();

    return tmp;
}

XilOpNumber
XilGlobalState::lookupOpNumber(const char* operation_name)
{
    
    computeOpNameTableMutex.lock();
    
    XiliOpNameRecord* rec;
    XilOpNumber       op_number;
    if(computeOpNameTable.lookup(operation_name, rec) == XIL_FAILURE) {
        op_number = -1;
    } else {
        op_number = rec->opNumber;
    }

    computeOpNameTableMutex.unlock();
    
    return op_number;
}

XilOpNumber
XilGlobalState::insertComputeOp(const char* operation_name)
{
    computeOpNameTableMutex.lock();
    

    XiliOpNameRecord* rec;
    if(computeOpNameTable.lookup(operation_name, rec) == XIL_FAILURE) {
        //
        //  A new operation name.  Get a new op number for it and insert the
        //  number into our table associated with the given string. 
        //
        rec           = new XiliOpNameRecord();
        rec->opNumber = incrementMaxOpNumber();
        
        computeOpNameTable.insert(operation_name, rec);

        //
        //  This stores the op name with the number so given the op number we
        //  get the operation name back.
        //
        storeOpName(rec->opNumber, operation_name);
    } else if(rec->opNumber < 0) {
        //
        //  We've found this name but we don't have a number for it.  So,
        //  insert a number for it.
        //
        rec->opNumber = incrementMaxOpNumber();
    }

    XilOpNumber op_number = rec->opNumber;
    
    computeOpNameTableMutex.unlock();
    
    return op_number;
}

//
//  To be called only while computeOpNameTableMutex is held.
//
void
XilGlobalState::storeOpName(XilOpNumber op_number,
                            const char* op_name)
{
    //
    //  Store the string around so we can convert the op number into a
    //  character string.
    //
    if(op_number >= (int)numOpNames) {
        //
        //  We need to grow the table.
        //
        char** tmpstrs = new char*[numOpNames<<1];

        xili_memcpy(tmpstrs, computeOpNames, numOpNames*sizeof(char*));
        xili_memset(tmpstrs + numOpNames, 0, numOpNames*sizeof(char*));

        delete [] computeOpNames;

        computeOpNames = tmpstrs;

        numOpNames = numOpNames<<1;
    }

    computeOpNames[op_number] = strdup(op_name);
}

char*
XilGlobalState::lookupOpName(XilOpNumber op_number)
{
    char* retval;
    
    computeOpNameTableMutex.lock();

    retval = computeOpNames[op_number];
    
    computeOpNameTableMutex.unlock();

    return retval;
}


//
//  Process dependency information from configuration file.
//
//  TODO: There should probably be some distinction between dependents in
//    the OWconfig file.  Compression and I/O should be indicated as
//    such in the file.
//
//  NOTE: loadDependents() manipulates the I/O, Compute and Compression symbol
//        tables.  Thus, the symtab mutexes must not be held by the caller.
//
//  Cannot be inlined because XiliOpen calls this function.
//
XilStatus
XilGlobalState::loadDependents(Xil_boolean do_full_init)
{
    //
    //  Define the priority values for loading compute devices.
    //
    //  TODO: 4/18/96 jlf  What if no priority is specified?
    //
    const int XILI_PRIORITY_MIN     =    0;
    const int XILI_PRIORITY_MAX     = 1000;
    const int XILI_PRIORITY_SKIP    =   -1;

    //
    //  On Solaris, we use the OWconfig interface to access our config file.
    //  On other platforms, we define our own file format akin to the
    //  xil.compute format from earlier versions of the library.
    //
    static XilMutex    load_dependents_mutex;

#ifdef SOLARIS
    const int XILI_PRIORITY_DEFAULT =  100;

    load_dependents_mutex.lock();
    if(do_full_init) {
        char* xil_config_file = XiliGetPath("config");

        int status;
        if(xil_config_file == NULL) {
            status = OWconfigInit("/usr/openwin/lib/xil/config",
                                  NULL, 0, NULL, NULL);
        } else {
            status = OWconfigInit(xil_config_file, NULL, 0, NULL, NULL);

            free(xil_config_file); 
        }

	if(status != OWCFG_OK &&
           status != OWCFG_OPEN1FAIL) {
	    XIL_ERROR(NULL, XIL_ERROR_CONFIGURATION, "di-364", TRUE);
	    load_dependents_mutex.unlock();
	    return XIL_FAILURE;
	}
    }

    //
    // If we get here then status is OWCFG_OK, OWCFG_OPEN1FAIL, or
    // OWCFG_OPEN2FAIL.
    //

    //
    //  Get list of compute devices.
    //
    char** compute_names = OWconfigGetClassNames("XIL_COMPUTE");
    if(compute_names == NULL) {
	load_dependents_mutex.unlock();
        return XIL_FAILURE;
    }

    XilStatus ret_val = XIL_FAILURE;
    for(int i = 0; compute_names[i] != NULL; i++) {
	XiliDeviceRecord* record;
        //
        // If the compute driver is already loaded then don't bother
        //
        symtabComputeMutex.lock();
        if(symtabCompute.lookup(compute_names[i], record) == XIL_SUCCESS) {
            symtabComputeMutex.unlock();
            ret_val = XIL_SUCCESS;
            continue;
        }
        symtabComputeMutex.unlock();

        //
        //  Get the priority attribute and convert to an int
        //
        char* priority = OWconfigGetAttribute("XIL_COMPUTE",
                                              compute_names[i],
                                              "priority");
        int priority_int = XILI_PRIORITY_DEFAULT;
        if(priority != NULL) {
            priority_int = atoi(priority);
            OWconfigFreeAttribute(priority);
        }

        if(priority_int == XILI_PRIORITY_SKIP) {
            continue;
        }

        //
        //  Clip the priority value to the allowable range
        //
        if(priority_int > XILI_PRIORITY_MAX) {
            priority_int = XILI_PRIORITY_MAX;
        } else if(priority_int < XILI_PRIORITY_MIN) {
            priority_int = XILI_PRIORITY_MIN;
        }

        //
        //  Get any dependencies on the compute handler.
        //
	char* dependencies = OWconfigGetAttribute("XIL_COMPUTE",
						  compute_names[i],
						  "dependencies");

        int  count;			// number of dependencies
	char depend[5][30];		// separated dependencies
	if(dependencies == NULL) {
	    count = 0;
	} else {
	    //
	    //  TODO:  These are hard-limits that need to become documented or
	    //  soft.
	    //
	    count = sscanf(dependencies, "%29s%29s%29s%29s%29s",
			   depend[0], depend[1], depend[2], depend[3],
			   depend[4]);

            //
	    // If there is a "dependencies" attribute but its value is
	    // blank and thus sscanf() returns EOF, then act like there
	    // are no dependencies specified.
            //
	    if(count == EOF) {
		count = 0;
	    }

            OWconfigFreeAttribute(dependencies);
	}

	//
        // See if all of the dependancies are met
        //
        for(int j=0; j<count; j++) {
            record = NULL;

            symtabCompressionMutex.lock();
            symtabCompression.lookup(depend[j], record);
            symtabCompressionMutex.unlock();
            if(record != NULL) {
                if(record->deviceManager == NULL) {
                    break;
                }
            } else {
                symtabInputOutputMutex.lock();
                symtabInputOutput.lookup(depend[j], record);
                symtabInputOutputMutex.unlock();
                if(record != NULL) {
                    if(record->deviceManager == NULL) {
                        break;
                    }
                } else {
                    break;
                }
            }
        }
        
        //
        //  If we made it all of the way through then load the driver
        //
        if(j == count) {
            //
            //  Get/Open/Insert the compute device into the library.
            //
            if(getDeviceManagerCompute(compute_names[i],
                                       priority_int) != NULL) {
                ret_val = XIL_SUCCESS;
            }
        }
    }

    OWconfigFreeClassNames(compute_names);

    load_dependents_mutex.unlock();

    return ret_val;
#else
    //
    //  On non-Solaris platforms, we use our own file format for loading
    //  compute device information and dependencies.
    //
    load_dependents_mutex.lock();
    if(do_full_init) {
        char* xil_config_file = XiliGetPath("config");

        if(xil_config_file == NULL) {
            xil_config_file = strdup("/usr/lib/xil/config");
            if(xil_config_file == NULL) {
                XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
                load_dependents_mutex.unlock();
                return XIL_FAILURE;
            }
        }

        //
        //  Open the configuration file.
        //
        configFILE = fopen(xil_config_file, "r");
        if(configFILE == NULL) {
            XIL_ERROR(NULL, XIL_ERROR_CONFIGURATION, "di-364", TRUE);
            load_dependents_mutex.unlock();
            return XIL_FAILURE;
        }

        free(xil_config_file); 
    }

    //
    //  Start at the beginning of the file.
    //
    fseek(configFILE, 0L, 0);

    XilStatus    ret_val = XIL_FAILURE;
    for (;;) {

        int  priority;
        char buffer[2048];
        char depend[5][64];
        XiliDeviceRecord* record;
        char compute_handler[256];

        memset(buffer, '\0', 2048);
        if(fgets(buffer, 2048, configFILE) == NULL) {
            break;
        }

        if (buffer[strlen(buffer) - 1] == '\n') {
            buffer[strlen(buffer) - 1] = '\0';
        }

        if(!buffer[0] || buffer[0] == '#') {
            continue;
        }

        priority  = 0;

        *depend[0] = '-';
        *depend[1] = '-';
        *depend[2] = '-';
        *depend[3] = '-';
        *depend[4] = '-';

        if(sscanf(buffer, "%255s %d %63s %63s %63s %63s %63s\n",
                 compute_handler, &priority,
                 depend[0], depend[1], depend[2], depend[3], depend[4]) != 7) {
            continue;
        }


        unsigned int num_dependents = 0;
        if(*depend[0] != '-') num_dependents++;
        if(*depend[1] != '-') num_dependents++;
        if(*depend[2] != '-') num_dependents++;
        if(*depend[3] != '-') num_dependents++;
        if(*depend[4] != '-') num_dependents++;
        
        //
        //  If the compute driver is already loaded then move on...
        //
        symtabComputeMutex.lock();
        if(symtabCompute.lookup(compute_handler, record) == XIL_SUCCESS) {
            symtabComputeMutex.unlock();
            ret_val = XIL_SUCCESS;
            continue;
        }
        symtabComputeMutex.unlock();

        //
        //  Check priority...
        //
        if(priority == XILI_PRIORITY_SKIP) {
            continue;
        }

        //
        //  Clip the priority value to the allowable range
        //
        if(priority > XILI_PRIORITY_MAX) {
            priority = XILI_PRIORITY_MAX;
        } else if(priority < XILI_PRIORITY_MIN) {
            priority = XILI_PRIORITY_MIN;
        }

        //
        //  Check dependents...
        //
        unsigned int i;
        for(i=0; i<num_dependents; i++) {
            XiliDeviceRecord* record;

            //
            //  Is it a CODEC or IO dependency?
            //
            if(strncmp(depend[i], "CODEC_", 6) == 0) {
                symtabCompressionMutex.lock();
                XilStatus status =
                    symtabCompression.lookup(depend[i], record);
                symtabCompressionMutex.unlock();

                //
                //  Not loaded?
                //
                if(status == XIL_FAILURE) {
                    break;
                }

                //
                //  Loaded unsuccessfully?
                //
                if(record->deviceManager == NULL) {
                    break;
                }
            } else if(strncmp(depend[i], "IO_", 3) == 0) {
                symtabInputOutputMutex.lock();
                XilStatus status =
                    symtabInputOutput.lookup(depend[i], record);
                symtabInputOutputMutex.unlock();

                //
                //  Not loaded?
                //
                if(status == XIL_FAILURE) {
                    break;
                }

                //
                //  Loaded unsuccessfully?
                //
                if(record->deviceManager == NULL) {
                    break;
                }
            } else {
                //
                //  Not a valid dependency...
                //
                break;
            }
        }

        //
        //  Are all of them loaded?
        //
        if(i == num_dependents) {
            //
            //  Get/Open/Insert the compute device into the library.
            //
            if(getDeviceManagerCompute(compute_handler, priority) != NULL) {
                ret_val = XIL_SUCCESS;
            }
        }
    } // for loop

    load_dependents_mutex.unlock();
    return ret_val;
#endif // SOLARIS
}

#ifdef _WINDOWS
#define _XIL_DEVHANDLERS_DL            "devhandlers/xil%s_%s.dll"
#define _XIL_DEVHANDLERS_SFX_DL        "devhandlers/xil%s_%s%s.dll"
#elif defined(HPUX)
#define _XIL_DEVHANDLERS_DL            "devhandlers/xil%s_%s.sl"
#define _XIL_DEVHANDLERS_SFX_DL        "devhandlers/xil%s_%s%s.sl"
#else
#define _XIL_DEVHANDLERS_DL            "devhandlers/xil%s_%s.so.%d"
#define _XIL_DEVHANDLERS_SFX_DL        "devhandlers/xil%s_%s%s.so.%d"
#endif

#if !defined(HPUX)
inline
#endif
void
XilGlobalState::printDLError(const char* pipeline_prefix,
                             const char* pipeline_name,
                             Xil_boolean gen_error)
{
    char error_arg[XILI_PATH_MAX];
    const char* errstr = xili_dlerror();

#ifdef _WINDOWS
    sprintf(error_arg, _XIL_DEVHANDLERS_SFX_DL,
                pipeline_prefix, pipeline_name, librarySuffix);
#else
    sprintf(error_arg, _XIL_DEVHANDLERS_SFX_DL,
                pipeline_prefix, pipeline_name, librarySuffix,
                XIL_PIPELINE_VERSION);
#endif
    strcat(error_arg, " : ");
    strcat(error_arg, errstr);

    if(gen_error) {
        XIL_ERROR_WITH_ARG(NULL, XIL_ERROR_CONFIGURATION, "di-412",
                           TRUE, error_arg);
    } else {
        XIL_WARNING_WITH_ARG(NULL, XIL_ERROR_CONFIGURATION, "di-412",
                             TRUE, error_arg);
    }
}


#if !defined(HPUX) && !defined(IRIX)
inline
#endif
char*
XilGlobalState::generatePipelinePath(const char* pipeline_prefix,
                                     const char* pipeline_name,
                                     Xil_boolean add_suffix)
{
    char pathname[XILI_PATH_MAX];

    if(add_suffix == TRUE) {
#ifdef _WINDOWS
        sprintf(pathname, _XIL_DEVHANDLERS_SFX_DL,
                pipeline_prefix, pipeline_name, librarySuffix);
#else
        sprintf(pathname, _XIL_DEVHANDLERS_SFX_DL,
                pipeline_prefix, pipeline_name, librarySuffix,
                XIL_PIPELINE_VERSION);
#endif
    } else {
        sprintf(pathname, _XIL_DEVHANDLERS_DL,
                pipeline_prefix, pipeline_name, XIL_PIPELINE_VERSION);
    }


    //
    //  TODO:  The access() calls in XiliGetPath may not be necessary.
    //         They're slowing things down and should be replaced by just
    //         letting the actual opens fail.
    //
    return XiliGetPath(pathname);
}

#if !defined(IRIX)
inline
#endif
XilDlHandle
XilGlobalState::dlopenPipeline(const char* pipeline_prefix,
                               const char* pipeline_name)
{
    //
    //  First try dlopening the pipeline with the suffix attached.
    //     This call generates the name and adds the suffix as indicated by
    //     the last argument.
    //
    char* pipeline_path =
        generatePipelinePath(pipeline_prefix, pipeline_name, TRUE);

    //
    //  Try open the pipeline with whatever the suffix may be.
    //
    XilDlHandle dlhandle = NULL;
    if(pipeline_path != NULL) {
        dlhandle = xili_dlopen(pipeline_path);
    }

    //
    //  If the suffix version failed, then try opening without the suffix.
    //
    if((dlhandle == NULL || pipeline_path == NULL) &&
       librarySuffix[0] != '\0') {
        free(pipeline_path);

        pipeline_path = generatePipelinePath(pipeline_prefix,
                                             pipeline_name,
                                             FALSE);
        if(pipeline_path == NULL) {
            dlhandle = NULL;
        } else {
            dlhandle = xili_dlopen(pipeline_path);
        }
    }

    free(pipeline_path);
    
    return dlhandle;
}

//
//  The type for the device manager creation functions
//
typedef XilDeviceManager*            (*DEV_MANAGER_FPTR)(unsigned int,
                                                         unsigned int,
                                                         unsigned int*,
                                                         unsigned int*);

//
//  Too large to be expanded inline by DevPro 4.x compilers
//
XilDeviceManager*
XilGlobalState::genericGetDeviceManager(const char*    pipeline_name,
                                        const char*    pipeline_prefix,
                                        const char*    entry_symbol,
                                        XiliHashTable<XiliDeviceRecord*>& symtab,
                                        Xil_boolean    gen_error)
{
    //
    //  Otherwise, we need to insert this pipeline_name and put it
    //  into the table.
    //
    XiliDeviceRecord* record = new XiliDeviceRecord(NULL);
    if(record == NULL) {
        if(gen_error) {
            XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
            XIL_ERROR_WITH_ARG(NULL, XIL_ERROR_CONFIGURATION, "di-158", FALSE,
                               (void*)pipeline_name);
        } else {
            XIL_WARNING(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
            XIL_WARNING_WITH_ARG(NULL, XIL_ERROR_CONFIGURATION, "di-158", FALSE,
                                 (void*)pipeline_name);
        }

        return NULL;
    }

    XilDlHandle dlhandle = dlopenPipeline(pipeline_prefix, pipeline_name);

    if(dlhandle == NULL) {
        //
        //  Insert a record with a NULL deviceManager so we don't keep trying
        //  to open it. 
        //
        symtab.insert(pipeline_name, record);

        printDLError(pipeline_prefix, pipeline_name, gen_error);

        if(gen_error) {
            XIL_ERROR_WITH_ARG(NULL, XIL_ERROR_CONFIGURATION, "di-158", FALSE,
                               (void*)pipeline_name);
        } else {
            XIL_WARNING_WITH_ARG(NULL, XIL_ERROR_CONFIGURATION, "di-158", FALSE,
                                 (void*)pipeline_name);
        }

        return NULL;
    }

    //
    //  Get the device manager creation function
    //
    DEV_MANAGER_FPTR create_func =
        (DEV_MANAGER_FPTR)xili_dlsym(dlhandle, entry_symbol);

    if(create_func == NULL) {
        //
        //  Insert a record with a NULL deviceManager so we don't keep trying
        //  to open it. 
        //
        symtab.insert(pipeline_name, record);

        xili_dlclose(dlhandle);

        if(gen_error) {
            XIL_ERROR_WITH_ARG(NULL, XIL_ERROR_CONFIGURATION, "di-159", TRUE,
                               (void*)pipeline_name);
        } else {
            XIL_WARNING_WITH_ARG(NULL, XIL_ERROR_CONFIGURATION, "di-159", TRUE,
                                 (void*)pipeline_name);
        }            

        return NULL;
    }

    //
    //  Create the device manager object
    //
    unsigned int devhandler_major_version = 0;
    unsigned int devhandler_minor_version = 0;

    record->deviceManager = (XilDeviceManager*)
        (*create_func)(XIL_GPI_MAJOR_VERSION,
                       XIL_GPI_MINOR_VERSION,
                       &devhandler_major_version,
                       &devhandler_minor_version);
    if(record->deviceManager == NULL) {
        //
        //  Insert a record with a NULL deviceManager so we don't keep trying
        //  to open it. 
        //
        symtab.insert(pipeline_name, record);

        //
        //  No error necessary -- this is recoverable...
        //
        xili_dlclose(dlhandle);

        return NULL;
    }
    
    symtab.insert(pipeline_name, record);
    
    return record->deviceManager;
}

XilDeviceManagerCompute*
XilGlobalState::getDeviceManagerCompute(const char*  pipeline_name,
                                        unsigned int priority)
{
    //
    //  See if we've already loaded the requested pipeline...
    //
    //  I lock the lookup and insertion process since only one thread
    //  should insert the information for a particular device manager at one
    //  time. 
    //
    symtabComputeMutex.lock();

    //
    //  If we found one, we're done.
    //
    XiliDeviceRecord* record;
    if(symtabCompute.lookup(pipeline_name, record) == XIL_SUCCESS) {
        symtabComputeMutex.unlock();
        return (XilDeviceManagerCompute*)record->deviceManager;
    }

    //
    //  Attempt to load the compute device manger.
    //
    XilDeviceManagerCompute* xdmc = (XilDeviceManagerCompute*)
        genericGetDeviceManager(pipeline_name,
                                computePrefix,
                                _XILI_COMPUTE_ENTRY_SYMBOL,
                                symtabCompute,
                                FALSE);

    if(xdmc != NULL) {
        //
        //  Set the priority on the device manager...
        //
        xdmc->setPriority(priority);

        //
        //  Now, lookup to get the record we just inserted...
        //
        //  We had better find it...
        //
        if(symtabCompute.lookup(pipeline_name, record) == XIL_SUCCESS) {
            if(xdmc->describeMembers() == XIL_FAILURE) {
                XIL_WARNING_WITH_ARG(NULL, XIL_ERROR_CONFIGURATION,
                                     "di-387", TRUE, (void*)pipeline_name);
                xdmc = NULL;
            }
        } else {
            xdmc = NULL;
        }
    }

    symtabComputeMutex.unlock();

    return xdmc;
}

XilDeviceManagerStorage*
XilGlobalState::getDeviceManagerStorage(const char* pipeline_name)
{
    //
    //  See if we've already loaded the requested pipeline...
    //
    //  I lock the lookup and insertion process since only one thread
    //  should insert the information for a particular device manager at one
    //  time. 
    //
    symtabStorageMutex.lock();

    //
    //  If we found one, we're done.
    //
    XiliDeviceRecord* record;
    if(symtabStorage.lookup(pipeline_name, record) == XIL_SUCCESS) {
        symtabStorageMutex.unlock();
        return (XilDeviceManagerStorage*)record->deviceManager;
    }

    XilDeviceManagerStorage* xdms = (XilDeviceManagerStorage*)
        genericGetDeviceManager(pipeline_name,
                                storagePrefix,
                                _XILI_STORAGE_ENTRY_SYMBOL,
                                symtabStorage);

    symtabStorageMutex.unlock();

    return xdms;
}

XilDeviceManagerIO*
XilGlobalState::getDeviceManagerIO(const char* pipeline_name,
                                   Xil_boolean generate_errors)
{
    //
    //  See if we've already loaded the requested pipeline...
    //
    //  I lock the lookup and insertion process since only one thread
    //  should insert the information for a particular device manager at one
    //  time. 
    //
    symtabInputOutputMutex.lock();

    //
    //  If we found one, we're done.
    //
    XiliDeviceRecord* record;
    if(symtabInputOutput.lookup(pipeline_name, record) == XIL_SUCCESS) {
        symtabInputOutputMutex.unlock();
        return (XilDeviceManagerIO*)record->deviceManager;
    } else {
        if(strncmp(pipeline_name, "io", 2) == 0) {
            //
            //  Move the pipeline name past the "io"
            //
            pipeline_name += 2;
        }            
        if(symtabInputOutput.lookup(pipeline_name, record) == XIL_SUCCESS) {
            symtabInputOutputMutex.unlock();
            return (XilDeviceManagerIO*)record->deviceManager;
        }
    }

    XilDeviceManagerIO* xdmio = (XilDeviceManagerIO*)
        genericGetDeviceManager(pipeline_name,
                                inputOutputPrefix,
                                _XILI_IO_ENTRY_SYMBOL,
                                symtabInputOutput,
                                FALSE);

    //
    //  Earlier versions of XIL required an "io" prepended to the device name.
    //  Because of this, loading devhandlers with the "io" prepended may cause a
    //  failure due to the removal of this expectation.  So, we check to see
    //  if the first two characters are "io" and if they are, we attempt to
    //  load the pipeline again with them removed. 
    //
    if(xdmio == NULL) {
        if(strncmp(pipeline_name, "io", 2) == 0) {
            //
            //  Move the pipeline name past the "io"
            //
            pipeline_name += 2;

            xdmio = (XilDeviceManagerIO*)
                genericGetDeviceManager(pipeline_name,
                                        inputOutputPrefix,
                                        _XILI_IO_ENTRY_SYMBOL,
                                        symtabInputOutput,
                                        generate_errors);
        } else if(generate_errors == TRUE) {
            //
            //  Even if we didn't update pipeline_name, we now still need to
            //  attempt again in order to get the correct errors generated
            //  generated.
            //
            xdmio = (XilDeviceManagerIO*)
                genericGetDeviceManager(pipeline_name,
                                        inputOutputPrefix,
                                        _XILI_IO_ENTRY_SYMBOL,
                                        symtabInputOutput,
                                        generate_errors);
        }
    }

    if(xdmio != NULL) {
        //
        //  Load any compute devices dependent upon this I/O handler.
        //
        symtabInputOutputMutex.unlock();

        loadDependents();

        symtabInputOutputMutex.lock();

        //
        //  Now, lookup to get the record we just inserted...
        //
        //  We had better find it...
        //
        if(symtabInputOutput.lookup(pipeline_name, record) == XIL_SUCCESS) {
            if(record->deviceManager->describeMembers() == XIL_FAILURE) {
                XIL_WARNING_WITH_ARG(NULL, XIL_ERROR_CONFIGURATION,
                                     "di-387", TRUE, (void*)pipeline_name);
                xdmio = NULL;
            }
        } else {
            xdmio = NULL;
        }
    }

    symtabInputOutputMutex.unlock();

    return xdmio;
}


XilDeviceManagerCompression*
XilGlobalState::getDeviceManagerCompression(const char* pipeline_name)
{
    //
    //  See if we've already loaded the requested pipeline...
    //
    //  I lock the lookup and insertion process since only one thread
    //  should insert the information for a particular device manager at one
    //  time. 
    //
    symtabCompressionMutex.lock();

    //
    //  If we found one, we're done.
    //
    XiliDeviceRecord* record;
    if(symtabCompression.lookup(pipeline_name, record) == XIL_SUCCESS) {
        symtabCompressionMutex.unlock();
        return (XilDeviceManagerCompression*)record->deviceManager;
    }

    XilDeviceManagerCompression* xdmc = (XilDeviceManagerCompression*)
        genericGetDeviceManager(pipeline_name,
                                compressionPrefix,
                                _XILI_COMPRESSION_ENTRY_SYMBOL,
                                symtabCompression);

    if(xdmc != NULL) {
        //
        //  Load any compute devices dependent upon this Compression handler.
        //
        symtabCompressionMutex.unlock();

        loadDependents();

        symtabCompressionMutex.lock();

        //
        //  Now, lookup to get the record we just inserted...
        //
        //  We had better find it...
        //
        if(symtabCompression.lookup(pipeline_name, record) == XIL_SUCCESS) {
            if(record->deviceManager->describeMembers() == XIL_FAILURE) {
                XIL_WARNING_WITH_ARG(NULL, XIL_ERROR_CONFIGURATION,
                                     "di-387", TRUE, (void*)pipeline_name);
                xdmc = NULL;
            }
        } else {
            xdmc = NULL;
        }
    }

    symtabCompressionMutex.unlock();

    return xdmc;
}


#include "_XilOp.hh"

//
//  The type for the Op pipline functions
//
typedef XilStatus  (*OP_OPEN_FPTR)();

XilOp*
XilGlobalState::createXilOp(const char*  operation_name,
                            void*        args[],
                            int          arg_count)
{
    XilOpCreateFunctionPtr tmp =
        getXilOpCreateFunc(operation_name);

    if(tmp == NULL) {
        return NULL;
    } else {
        return (*tmp)(operation_name, args, arg_count);
    }
}

XilOpCreateFunctionPtr
XilGlobalState::getXilOpCreateFunc(const char* operation_name)
{
    //
    //  See if we've already loaded the requested pipeline...
    //
    char      pipeline_name[1024];
    sprintf(pipeline_name, "%s-%s",
            XIL_DEFAULT_COMPANY_NAME, XIL_DEFAULT_CLASSIFICATION);

    //
    //  I lock the lookup and insertion process since only one thread
    //  should insert the information for a particular Compression device
    //  at one time.
    //
    symtabOpMutex.lock();
  
    //
    //  If we found one, we can create an op.
    //
    XiliOpOpenRecord* record;
    if(symtabOp.lookup(pipeline_name, record) == XIL_SUCCESS) {
        symtabOpMutex.unlock();
        
        if(record->foundPipelineFlag == FALSE) {
            return NULL;
        }

        record->createFuncsMutex.lock();        
        XiliOpRecord* oprec = NULL;
        record->createFuncs.lookup(operation_name, oprec);
        record->createFuncsMutex.unlock();

        if(oprec == NULL) {
            return NULL;
        } else {
            if(oprec->fptr == NULL) {
                return NULL;
            } else {
                return oprec->fptr;
            }
        }
    }

    //
    //  Try an open the op pipeline.
    //
    XilDlHandle dlhandle = dlopenPipeline(opPrefix, pipeline_name);

    if(dlhandle == NULL) {
        printDLError(opPrefix, pipeline_name);
        XIL_ERROR_WITH_ARG(NULL, XIL_ERROR_CONFIGURATION, "di-158", FALSE,
                           (void*)pipeline_name);
        symtabOpMutex.unlock();
        return NULL;
    }
    
    //
    //  Now we insert the pipeline_name into the table.
    //
    record = new XiliOpOpenRecord();
    if(record == NULL) {
        XIL_ERROR(NULL, XIL_ERROR_RESOURCE,"di-1", TRUE);
        XIL_ERROR_WITH_ARG(NULL, XIL_ERROR_CONFIGURATION, "di-158", FALSE,
                           (void*)pipeline_name);
        delete record;
        symtabOpMutex.unlock();
        return NULL;
    }

    //
    //  Get the op entry function
    //
    OP_OPEN_FPTR open_func;
    open_func = (OP_OPEN_FPTR) xili_dlsym(dlhandle, _XILI_OP_ENTRY_SYMBOL);

    if(open_func == NULL) {
        XIL_ERROR_WITH_ARG(NULL, XIL_ERROR_CONFIGURATION, "di-159", TRUE,
                           (void*)pipeline_name);
        xili_dlclose(dlhandle);
        delete record;
        symtabOpMutex.unlock();
        return NULL;
    }

    //
    //  Call the op entry.  Store our dlhandle and record ptr tmpData
    //  so describeOpFunction() has access.
    //
    tmpData[0] = dlhandle;
    tmpData[1] = record;
    XilStatus status = (*open_func)();
    if(status == XIL_FAILURE) {
        xili_dlclose(dlhandle);
        delete record;
        symtabOpMutex.unlock();
        return NULL;
    }
    
    //
    //  We can't unlock the symbol table until this point because the
    //  record returned by the symbol table will not be filled
    //  with proper information until this point.
    //
    record->foundPipelineFlag = TRUE;
    symtabOp.insert(pipeline_name, record);

    record->createFuncsMutex.lock();
    XiliOpRecord* oprec = NULL;
    record->createFuncs.lookup(operation_name, oprec);
    record->createFuncsMutex.unlock();
    symtabOpMutex.unlock();
    
    if(oprec == NULL) {
        return NULL;
    } else {
        if(oprec->fptr == NULL) {
            return NULL;
        } else {
            return oprec->fptr;
        }
    }
}

XilStatus
XilGlobalState::describeOpFunction(const char* operation_name,
                                   const char* op_class_name,
                                   void*       )
{
    XilDlHandle       dlhandle = (XilDlHandle)tmpData[0];
    XiliOpOpenRecord* record   = (XiliOpOpenRecord*)tmpData[1];

    char tmpbuf[1024];

    //
    //  The symbol name is based not only on the name of the class,
    //  but the length of the class name.  It's based on the alphabet
    //  where for a length mod 52 equalling 1-26, uppercase A-Z is
    //  used as the length character.  A length 27-52 is a-z.  And,
    //  for lengths > 52, add a digit starting at 0 for each time it's
    //  divisible by 52.
    //
    //  TODO:  There is probably a better way to do this.
    //
    int  op_class_name_len = strlen(op_class_name);
    char length_char;
    if(op_class_name_len % 52 < 26) {
        length_char = (op_class_name_len % 52) + 0x41;
    } else {
        length_char = ((op_class_name_len + 26) % 52) + 0x61;
    }

#if defined(GCC) || defined(HPUX) || defined(IRIX)
    //  TODO:  Probably needs to be fixed for above reason too
    sprintf(tmpbuf, _XILI_OP_FUNC_ENTRY_SYMBOL, 
                                op_class_name_len, op_class_name);
#elif defined(_WINDOWS)
    sprintf(tmpbuf, _XILI_OP_FUNC_ENTRY_SYMBOL, op_class_name);
#else
    int  loop_count        = (op_class_name_len/52) - 1;
    if(loop_count >= 0) {
        sprintf(tmpbuf, _XILI_OP_FUNC_ENTRY_LOOP_SYMBOL,
                                loop_count, length_char, op_class_name);
    } else {
        sprintf(tmpbuf, _XILI_OP_FUNC_ENTRY_SYMBOL,
                                length_char, op_class_name);
    }
#endif
    
    
    //
    //  Get the op entry function
    //
    XilOpCreateFunctionPtr create_func = (XilOpCreateFunctionPtr)
        xili_dlsym(dlhandle, tmpbuf);
    if(create_func == NULL) {
        XIL_ERROR_WITH_ARG(NULL, XIL_ERROR_CONFIGURATION, "di-159", TRUE,
                           NULL);
        return XIL_FAILURE;
    }


    XiliOpRecord* oprec = new XiliOpRecord(create_func);
    
    record->createFuncsMutex.lock();
    record->createFuncs.insert(operation_name, oprec);
    record->createFuncsMutex.unlock();

    return XIL_SUCCESS;
}
