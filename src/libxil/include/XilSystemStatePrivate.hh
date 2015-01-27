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
//  File:	XilSystemStatePrivate.hh
//  Project:	XIL
//  Revision:	1.36
//  Last Mod:	10:20:49, 03/10/00
//
//  Description:
//	
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilSystemStatePrivate.hh	1.36\t00/03/10  "

#ifdef _XIL_PRIVATE_INCLUDES

//
//  Error class definition.
//
#include "XilError.hh"

//
//  Include hash table for storing named objects.
//
#include "XiliObjectHashTable.hh"

//
//  Include single-linked list for storing deferred objects and error functions.
//
#include "XiliSLList.hh"

//
//  The XIL-Supplied Default Error Handler
//
Xil_boolean    XiliDefaultErrorHandler(XilError* error);

#endif

#ifdef _XIL_PRIVATE_DATA

public:
    //
    //  Constructor
    //
                             XilSystemState();

    //
    //  Methods to lock and unlock the system state.  For some
    //  operations, this is done internally.  For others, the caller
    //  must take care to not have more than one thread call the
    //  system state method.  See the comments for the method to
    //  determine its behavior. 
    //
    void                     lock()
    {
        systemStateMutex.lock();
    }

    void                     unlock()
    {
        systemStateMutex.unlock();
    }

    XilStatus                tryLock()
    {
        return systemStateMutex.tryLock();
    }

    //
    //  Test for successful creation
    //
    Xil_boolean              isOK()
    {
        _XIL_ISOK_TEST()
    }

    //
    //  Creation from an X Window for a specific device.
    //
    XilImage*                createXilImage(Display*    display,
                                            Window      window,
                                            const char* specific_device);

    //
    //  Creation of a temporary XilImage object -- API only
    //
    XilImage*                createXilImageTemp(XilImageFormat* image_format);

    XilImage*                createXilImageTemp(unsigned int width,
                                                unsigned int height,
                                                unsigned int nbands,
                                                XilDataType  datatype);

    //
    //  XIL_PRIVATE constructor for copies of XilLookupColorcube objects.
    //  Enough work goes into the construction that we don't want
    //  to do a complete object creation.
    //
    XilLookupColorcube*       createXilLookupColorcube(XilLookupColorcube*);
    
    //
    //  Complete object creation...similar between all objects
    //
    XilStatus                completeObjectCreation(XilObject* object);

    //
    //  Is the state currently synchronized?
    //
    Xil_boolean              getSynchronized() const;

    //
    //  Set the state to synchronize future operations
    //
    void                     setSynchronized(Xil_boolean onoff);

    //
    //  Default Tile Sizes
    //
    XilStatus                getDefaultTileSize(unsigned int* txsize,
                                                unsigned int* tysize);
    XilStatus                setDefaultTileSize(unsigned int  txsize,
                                                unsigned int  tysize);

    //
    //  Default image allocation mode
    //
    XilTilingMode            getDefaultTilingMode();
    XilStatus                setDefaultTilingMode(XilTilingMode new_tiling_mode);

    //
    //  Used by XilObject::setName() and ~XilObject() to add and remove the
    //  object from our hash table for named objects.
    //
    void                     addNamedObject(const char* name,
                                            XilObject*  object);
    void                     removeNamedObject(const char* name,
                                               XilObject*  object);

    //
    //  Used by XilDeferrableObject() and ~XilDeferrableObject() to add and
    //  remove potentially deferrable objects from our list for defferable
    //  objects.  We must keep a list of these objects in order to sync them
    //  when setSynchronize() is set to TRUE.
    //
    XiliSLListPosition       addDefObject(XilObject* object);
    void                     removeDefObject(XilObject*         object,
                                             XiliSLListPosition list_position);

    //
    //  XIL Error Handling Methods
    //
    XilStatus                installErrorHandler(XilErrorFunc error_function);
    void                     removeErrorHandler(XilErrorFunc error_function);
    Xil_boolean              callNextErrorHandler(XilError* error_obj);

    //
    //  Set references to interpolation tables which now become the tables for
    //  this system state.
    //
    XilStatus                setInterpolationTables(XilInterpolationTable* horiz_table,
                                                    XilInterpolationTable* vertical_table);

    //
    //  Set the flag indicating whether show_action is active for this
    //  system state.  The value contained in the system state can be
    //  overriden by setting show_action in the XIL_DEBUG environment
    //  variable -- which makes it always TRUE.  This can be called with -1
    //  which means read the environment variable or 1 or 0 which turns it on
    //  or off.
    //
    void                     setShowActionFlag(int env_on_off);
    
    //
    //  Set the flag indicating whether provide_warnings is active for this
    //  system state.  The value contained in the system state can be
    //  overriden by setting provide_warnings in the XIL_DEBUG environment
    //  variable -- which makes it always TRUE.
    //
    void                     setProvideWarningsFlag(Xil_boolean on_off);
    Xil_boolean              getProvideWarningsFlag();

                             ~XilSystemState();

#if defined(GCC) || defined(_WINDOWS) || defined(HPUX)
    //
    //  For placating explicit template instantiation.
    //
    int operator == (XilSystemState&) {
        return TRUE;
    }
    XilSystemState& operator = (XilSystemState& rval) {
        *this = rval;
        return *this;
    }
#endif
private:
    //
    //  If we sport a standard object of the given name and type, create it
    //  and return it.
    //
    void                      determine_no_cpus();

    XilObject*                getStandardObject(const char*   name,
                                                XilObjectType type);

    void                      callError(XilError* error);

    XilDeviceManagerIO*       getDeviceManagerForWindow(Display*    display,
                                                        Window      window,
                                                        const char* specific_device);

    XilStatus                 finishIODeviceImage(Display*     display,
                                                  Window       window,
                                                  XilDeviceIO* device,
                                                  XilImage*    image);


    Xil_boolean               isOKFlag;
    Xil_boolean               isSynchronized;

    XilMutex                  systemStateMutex;

    XiliObjectHashTable*      namedObjectTable;
    XiliSLList<XilObject*>    deferredObjectList;
    XiliSLList<XilErrorFunc>  errorFunctionList;

    static XilErrorFunc       defaultErrorHandler;
    static XilMutex           defaultErrorHandlerMutex;

    unsigned int              xDefaultTileSize;
    unsigned int              yDefaultTileSize;

    XilTilingMode             defaultTilingMode;

    XilInterpolationTable*    horizInterpTable;
    XilInterpolationTable*    verticalInterpTable;

    Xil_boolean               showActionFlag;

    Xil_boolean               provideWarningsFlag;

    XilUnsharpMasking         unsharp_mode;
#endif // _XIL_PRIVATE_DATA
