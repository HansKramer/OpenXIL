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
//  File:	XilGlobalStatePrivate.hh
//  Project:	XIL
//  Revision:	1.43
//  Last Mod:	10:20:47, 03/10/00
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

#ifdef _XIL_PRIVATE_INCLUDES

#include "_XilOp.hh"
#include "_XilDeviceManager.hh"

#include "XiliHashTable.hh"
#include "XiliList.hh"
#include "XiliReadWrite.hh"

//
//  Forward class delcarations
//
class XiliOpTreeNode;
class XiliScheduler;
class XiliDagManager;

//
//  The HashRecord we use for keeping track of the compute pipeline op
//  functions.
//
class XiliOpNameRecord {
public:
    XiliOpNameRecord() :
        opNumber(-1)
    {
    }

    XilOpNumber  opNumber;
};

//
//  The object we use for keeping track of the devices
//
class XiliDeviceRecord {
public:
    XiliDeviceRecord(XilDeviceManager* init_dc) :
        deviceManager(init_dc)
    {
    }

    ~XiliDeviceRecord()
    {
        deviceManager->destroy();
    }

    XilDeviceManager*  deviceManager;
};

//
//  The HashRecord we use for keeping track of the op create functions.
//
class XiliOpRecord {
public:
    XiliOpRecord(XilOpCreateFunctionPtr init_ptr) :
        fptr(init_ptr)
    {
    }

    XilOpCreateFunctionPtr  fptr;
};

//
//  The HashRecord we use for keeping track of the op pipelines
//
//  The createFuncs table here is large because of the expected large
//  number of operations to be placed in the table.  I've chosen 877.
//
class XiliOpOpenRecord {
public:
    XiliOpOpenRecord() :
        createFuncs(877), foundPipelineFlag(FALSE)
    {
    }

    Xil_boolean                  foundPipelineFlag;
    XilMutex                     createFuncsMutex;
    XiliHashTable<XiliOpRecord*> createFuncs;
};

#else

public:
    //
    //  Create the specified XilOp
    //
    //  MT-safe
    //
    XilOp*                       createXilOp(const char* operation_name,
                                             void*       args[],
                                             int         arg_count);

    XilOpCreateFunctionPtr       getXilOpCreateFunc(const char* operation_name);
    
    //
    //  Aquire the base of the Op Tree
    //
    //
    //  It may be possible that one thread is executing something off the
    //  tree at the same time another thread is inserting a new compute
    //  device into the tree.
    //
    //  It is not possible for two threads to be inserting new compute
    //  devices into the tree at the same time.  Currently, the function
    //  XilGlobalState::getDeviceComputeManager() guarentees this from
    //  happening.
    //
    //  The read-write lock guarentees that only one thread will update the tree
    //  even though many threads may be reading the tree at one time.
    //
    XiliOpTreeNode*              getOpTreeBase(Xil_boolean for_writing = FALSE)
    {
        if(for_writing == TRUE) {
            opTreeBaseLock.writeLock();
        } else {
            opTreeBaseLock.readLock();
        }

        return opTreeBase;
    }

    //
    //  Release lock on opTreeBase.
    //
    void                         releaseOpTreeBase()
    {
        opTreeBaseLock.unlock();
    }

    //
    //  Create or Remove a System State
    //
    //  MT-safe
    //
    XilSystemState*              createSystemState();
    XilSystemState*              getFirstSystemState();
    void                         destroySystemState(XilSystemState* state);

    //
    //  Generic get function for XilDeviceManager by NAME
    //
    //  MT-safe
    //
    XilDeviceManager*            genericGetDeviceManager(const char*    name,
                                                         const char*    prefix,
                                                         const char*    entry_symbol,
                                                         XiliHashTable<XiliDeviceRecord*>& symtab,
                                                         Xil_boolean    gen_error = TRUE);

    //
    //  Get an XilDeviceManagerCompute by NAME
    //
    //  MT-safe
    //
    XilDeviceManagerCompute*     getDeviceManagerCompute(const char*  name,
                                                         unsigned int priority);

    //
    //  Get an XilDeviceManagerStorage by NAME
    //
    //  MT-safe
    //
    XilDeviceManagerStorage*     getDeviceManagerStorage(const char* name);

    //
    //  Get an XilDeviceManagerIO by NAME
    //
    //  MT-safe
    //
    XilDeviceManagerIO*          getDeviceManagerIO(const char* name,
                                                    Xil_boolean generate_errors);

    //
    //  Get an XilDeviceManagerCompression by NAME
    //
    //  MT-safe
    //
    XilDeviceManagerCompression* getDeviceManagerCompression(const char* name);

    //
    //  Load the dependency information from the configuration files and
    //  any non-dependent compute devices.
    //
    //  MT-safe
    //
    XilStatus                    loadDependents(Xil_boolean do_full_init = FALSE);

    //
    //  Lookup/Insert a compute operation and assign an op number if it
    //  doesn't already have one assigned. 
    //
    XilOpNumber                  insertComputeOp(const char* operation_name);

    //
    //  Return the function name of the given operation number.
    //
    char*                        lookupOpName(XilOpNumber op_number);

    //
    //  Keeps track of highest numbered op in the entire tree.
    //
    XilOpNumber                  getMaxOpNumber();
    XilOpNumber                  incrementMaxOpNumber();

    //
    //  Get the DAG Manager which maintains information on all of the
    //  independent DAGs currently in use.  There is only one of these tables
    //  created per-process. 
    //
    XiliDagManager*              getDagManager()
    {
        return dagManager;
    }

    //
    //  Get the Operation Scheduler.
    //
    XiliScheduler*               getOpScheduler()
    {
        return opScheduler;
    }

    //
    //  For XIL 1.3, we support XIL_DEBUG in a different fashion.  We store
    //  the environment flag information at start-up and then make it
    //  available to the rest of the library through the global state.
    //
    //  The available flags on XIL_DEBUG are:
    //
    //    link_?            (where ? is a character to append to pipeline names)
    //    show_action       (output the device and operation being executed)
    //    set_synchronize   (execute all operations immediately)
    //    provide_warnings  (have default error handler also output warnings)
    //    tiling_mode=*     (set the tiling mode -- either 'tile' or 'strip')
    //    txsize=*          (set the default X tile size)
    //    tysize=*          (set the default Y tile size)
    //    threads=*         (override the number of threads to create for this machine)
    //    split_threshold=* (override the minimum ysize for thread splitting)
    //
    //  The XILHOME variable was highly misused by customers in earlier
    //  releases and is no longer going to be used for determining pipeline
    //  locations.  The final product has no mechanism for specifying a
    //  different path/directory than the compiled in and documented locations
    //  For our development, a new environment variable, "XILRELEASEDIR" will
    //  permit specifying a new release directory to read pipelines and all
    //  other information from -- just like XILHOME used to...when building
    //  the final version of the product, its capabilities will be compiled
    //  out...
    //
#ifndef _XIL_RELEASE_BUILD
    const char*                  getReleaseDirectory()
    {
        return envReleaseDir;
    }
#endif

    Xil_boolean                  getShowAction()
    {
        return envShowAction;
    }

    Xil_boolean                  getSetSynchronize()
    {
        return envSetSynchronize;
    }

    Xil_boolean                  getProvideWarnings()
    {
        return envProvideWarnings;
    }

    XilTilingMode                getTilingMode()
    {
        return envTilingMode;
    }

    unsigned int                 getTileSizeX()
    {
        return envTileSizeX;
    }

    unsigned int                 getTileSizeY()
    {
        return envTileSizeY;
    }

    unsigned int                 getNumThreads()
    {
        return envNumThreads;
    }

    unsigned int                 getSplitThreshold()
    {
        return envSplitThreshold;
    }

    //
    //  Returns the variable holding the target number of bytes for each tile
    //  as computed based on the system memory during construction of the
    //  global state. 
    //  The XilImage uses this value to compute the tile size for a given
    //  image based on the number of bands and the data type.
    //
    unsigned int                 getBytesPerTile()
    {
        return bytesPerTile;
    }

    //
    //  The constructor. It is only called once per application.
    //
                                 XilGlobalState();
                                 ~XilGlobalState();

    //
    //  Test for successful creation
    //
    //  MT-safe
    //
    Xil_boolean isOK();

    //
    //  Make public so internal routines can access the pointer directly.
    //
    static XilGlobalState*    theXGS;

private:
    static XilOpNumber        maxOpNum;
    static XilMutex           maxOpNumMutex;

    Xil_boolean               isOKFlag;
    char*                     librarySuffix;
    
    XiliOpTreeNode*           opTreeBase;
    XiliReadWrite             opTreeBaseLock;

    XiliList<XilSystemState>  systemStateList;
    XilMutex                  systemStateListMutex;

    XiliHashTable<XiliDeviceRecord*>   symtabCompute;
    XilMutex                  symtabComputeMutex;
    
    XiliHashTable<XiliDeviceRecord*>   symtabStorage;
    XilMutex                  symtabStorageMutex;

    XiliHashTable<XiliDeviceRecord*>   symtabCompression;
    XilMutex                  symtabCompressionMutex;
    
    XiliHashTable<XiliDeviceRecord*>   symtabInputOutput;
    XilMutex                  symtabInputOutputMutex;
    
    XiliHashTable<XiliOpOpenRecord*>   symtabOp;
    XilMutex                  symtabOpMutex;
    
    XiliHashTable<XiliOpNameRecord*>   computeOpNameTable;
    XilMutex                  computeOpNameTableMutex;
    char**                    computeOpNames;
    unsigned int              numOpNames;
    
    const char*               computePrefix;
    const char*               storagePrefix;
    const char*               inputOutputPrefix;
    const char*               compressionPrefix;
    const char*               opPrefix;

    XiliDagManager*           dagManager;

    XiliScheduler*            opScheduler;

    void*                     tmpData[3];

    //
    //  Environment variable control flags -- XIL_DEBUG and XIL_RELEASE_DIR
    //
#ifndef _XIL_RELEASE_BUILD
    char*                     envReleaseDir;
#endif
    Xil_boolean               envShowAction;
    Xil_boolean               envSetSynchronize;
    Xil_boolean               envProvideWarnings;
    XilTilingMode             envTilingMode;
    unsigned int              envTileSizeX;
    unsigned int              envTileSizeY;
    unsigned int              envNumThreads;
    unsigned int              envSplitThreshold;

    //
    //  Variable that holds the target number of bytes for each tile.  The
    //  XilImage uses this value to compute the tile size for a given image
    //  based on the number of bands and the data type.
    //
    unsigned int              bytesPerTile;

    //
    //  Method that computes the bytesPerTile value and is called by the
    //  constructor.
    //
    unsigned int              computeBytesPerTile();

#ifndef SOLARIS
    //
    //  Variables used in loading our own file format config file.
    //
    FILE*                     configFILE;
#endif

    //
    //  Used to store an op name in a table for future reference.
    //
    void                      storeOpName(XilOpNumber  op_number,
                                          const char*  op_name);

    //
    //  Dynamically opens a pipeline taking librarySuffix and other
    //  XIL rules into account
    //
    XilDlHandle               dlopenPipeline(const char* pipeline_prefix,
                                             const char* pipeline_name);

    char*                     generatePipelinePath(const char* pipeline_prefix,
                                                   const char* pipeline_name,
                                                   Xil_boolean add_suffix);

    void                      printDLError(const char* pipeline_prefix,
                                           const char* pipeline_name,
                                           Xil_boolean gen_error = TRUE);
#endif
