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
//  File:	_XilSystemState.hh
//  Project:	XIL
//  Revision:	1.43
//  Last Mod:	10:21:04, 03/10/00
//
//  MT Level:   SAFE
//
//  Description:
//	
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)_XilSystemState.hh	1.43\t00/03/10  "

#ifndef _XIL_SYSTEM_STATE_HH
#define _XIL_SYSTEM_STATE_HH

//
//  System Includes
//
#ifdef   _XIL_HAS_X11WINDOWS
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif //_XIL_HAS_X11WINDOWS

//
//  C++ Includes
//
#include "_XilDefines.h"
#include "_XilMutex.hh"

//
//  Private Includes
//
#ifdef  _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_INCLUDES

#include "XilSystemStatePrivate.hh"

#undef  _XIL_PRIVATE_INCLUDES
#endif

class  XilSystemState {
private:
    int no_cpus;
                                                                                
public:
    //
    //  Aquire an object by name and type.
    //
    //  This returns a reference to the requested object.  It should not be
    //  destroyed unless the caller is the owner of the object. 
    //
    int                 get_no_cpus() { return this->no_cpus; };

    XilObject*          getObjectByName(const char*   name,
                                        XilObjectType type);


    void                set_unsharp_mode(XilUnsharpMasking mode);
                                                                                                                                                                 
    XilUnsharpMasking   get_unsharp_mode();
                                                                                
    //
    //  Notify the application of an error which has occured, by code, on this
    //  system state.  Although, this can be called directly, it's recommended
    //  that the macros defined at the bottom of this file be used to call
    //  this function. 
    //
    void                notifyError(XilErrorCategory category,
                                    const char*      error_id,
                                    Xil_boolean      primary,
                                    unsigned int     line_number,
                                    const char*      filename,
                                    XilObject*       xil_object,
                                    void*            arg);

    //
    //  Like notifyError() except it generates a warning to the application
    //  (which, by default is not presented to the user via stderr).  Error
    //  handlers can decide how they want to report warnings to the user.
    //
    void                notifyWarning(XilErrorCategory category,
                                      const char*      error_id,
                                      Xil_boolean      primary,
                                      unsigned int     line_number,
                                      const char*      filename,
                                      XilObject*       xil_object,
                                      void*            arg);

    //
    //  Get references to the interpolation tables that are currently valid on
    //  this system state.
    //
    XilStatus           getInterpolationTables(XilInterpolationTable** horiz_table,
                                               XilInterpolationTable** vertical_table);

    //
    //  Get the flag indicating whether show_action is active for this
    //  system state.  Either the user can set it to TRUE or the XIL_DEBUG
    //  environment variable could have been set which makes this always
    //  return TRUE.
    //
    Xil_boolean         getShowActionFlag();
    
    //--------------------------------------------------------------------------
    //
    //  Object Creation Methods
    //

    //
    //  Creation of the XilCis Object
    //
    XilCis*             createXilCis(const char* compressor_name);

    //
    //  Creation of the XilDevice Object
    //
    XilDevice*          createXilDevice(const char* device_name);

    //
    //  Creation of the XilImage Object
    //
    XilImage*           createXilImage(XilImageFormat* image_format);

    XilImage*           createXilImage(unsigned int width,
                                       unsigned int height,
                                       unsigned int nbands,
                                       XilDataType  datatype);

    XilImage*           createXilImage(const char* device_name,
                                       XilDevice*  device);

#if defined(_XIL_HAS_X11WINDOWS) || defined(_WINDOWS)
    //
    //  Creation from an X Window.
    //
    //  The flag indicates whether XIL should create a double buffered XIL
    //  image if the Window will support it.
    //
    XilImage*           createXilImage(Display*    display,
                                       Window      window,
                                       Xil_boolean double_buffering = FALSE);

    //
    // Create an enhanced capability window. This was added
    // to support stereo, but is now the preferred way to
    // access stereo, DBE, and future capabilities. 
    // Multiple capabilities can be OR'ed.
    //
    XilImage*           createXilImage(Display*      display,
                                       Window        window,
                                       XilWindowCaps wincaps);

#endif // _XIL_HAS_X11WINDOWS || _WINDOWS

    //
    //  Creation of the XilImageFormat Object
    //
    XilImageFormat*     createXilImageFormat(unsigned int width,
                                             unsigned int height,
                                             unsigned int nbands,
                                             XilDataType  datatype);
    
    //
    //  Creation of the XilInterpolationTable Object
    //
    XilInterpolationTable*
                        createXilInterpolationTable(unsigned int    kernel_size,
                                                    unsigned int    num_subsamples,
                                                    float*          init_data);
    
    //
    //  Creation of the XilLookup Object
    //
    XilLookupColorcube* createXilLookupColorcube(XilDataType   input_type,
                                                 XilDataType   output_type,
                                                 unsigned int  nbands,
                                                 short         offset,
                                                 int*          multipliers,
                                                 unsigned int* dimensions);

    XilLookupSingle*    createXilLookupSingle(XilDataType  input_type,
                                              XilDataType  output_type,
                                              unsigned int nbands,
                                              unsigned int num_entries,
                                              short        offset,
                                              void*        data);

    XilLookupCombined*  createXilLookupCombined(XilLookupSingle* lookup_list[],
                                                unsigned int     num_lookups);

    //
    //  Creation of the XilHistogram Object
    //
    XilHistogram*       createXilHistogram(unsigned int  nbands,
                                           unsigned int* nbins,
                                           float*        low_value,
                                           float*        high_value);

    //
    //  Creation of the XilKernel Object
    //
    XilKernel*	        createXilKernel(unsigned int width,
                                        unsigned int height,
                                        int          key_x,
                                        int          key_y,
                                        float*       input_data);

    XilKernel*	        createXilKernel(unsigned int width,
                                        unsigned int height,
                                        int          key_x,
                                        int          key_y,
                                        float*       x_data,
                                        float*       y_data);

    //
    //  Creation of the XilDitherMask Object
    //
    XilDitherMask*      createXilDitherMask(unsigned int xsize,
                                            unsigned int ysize,
                                            unsigned int num_bands,
                                            float*       input_data);

    //
    //  Creation of the XilSel Object
    //
    XilSel*	        createXilSel(unsigned int  xsize,
                                     unsigned int  ysize,
                                     int           key_x,
                                     int           key_y,
                                     unsigned int* input_data);

    //
    //  Creation of the XilStorageAPI Object
    //
    XilStorageAPI*      createXilStorageAPI(XilImage* image);

    //
    //  Creation of the XilRoi Object
    //
    XilRoi*             createXilRoi();

    //
    //  Creation of the XilColorspace Object
    //
    XilColorspace*      createXilColorspace(char*               name,
                                            XilColorspaceOpCode op,
                                            unsigned int        bands);

    XilColorspace*      createXilColorspace(XilColorspaceType type,
                                            void*             data,
                                            Xil_boolean       copy_data = FALSE);

    XilColorspace*      createXilColorspace(Display* display,
					    int      screen,
    					    Visual*  visual);
    

    //
    //  Creation of the XilColorspaceList Object
    //
    XilColorspaceList*  createXilColorspaceList(XilColorspace* colorspace_array[],
                                                unsigned int   num_colorspaces);

    //
    //--------------------------------------------------------------------------

private:
#ifdef  _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_DATA

#include "XilSystemStatePrivate.hh"

#undef  _XIL_PRIVATE_DATA
#else
    //
    //  Make it clear to the compiler that a destructor exists.  This is done
    //  so GPI users will get a compile-time error if they attempt to delete
    //  this class.
    //
                        ~XilSystemState();
#endif
};

//
//  Macros for the different ways to report errors back to the application.
//
//  We create an _state instead of calling sys_state directly because we want
//  to support calling the function on a NULL pointer. 
//
#define XIL_ERROR(sys_state, category, id, primary) \
{ \
    XilSystemState* _state = (sys_state); \
    _state->notifyError((category), (id), (primary), __LINE__, __FILE__, (XilObject*)NULL, NULL); \
}

#define XIL_OBJ_ERROR(sys_state, category, id, primary, object) \
{ \
    XilSystemState* _state = (sys_state); \
    _state->notifyError((category), (id), (primary), __LINE__, __FILE__, (object), NULL); \
}

#define XIL_ERROR_WITH_ARG(sys_state, category, id, primary, arg) \
{ \
    XilSystemState* _state = (sys_state); \
    _state->notifyError((category), (id), (primary), __LINE__, __FILE__,(XilObject*)NULL, arg); \
}

#define XIL_WARNING(sys_state, category, id, primary) \
{ \
    XilSystemState* _state = (sys_state); \
    _state->notifyWarning((category), (id), (primary), __LINE__, __FILE__,(XilObject*)NULL, NULL); \
}

#define XIL_OBJ_WARNING(sys_state, category, id, primary, object) \
{ \
    XilSystemState* _state = (sys_state); \
    _state->notifyWarning((category), (id), (primary), __LINE__, __FILE__, (object), NULL); \
}

#define XIL_WARNING_WITH_ARG(sys_state, category, id, primary, arg) \
{ \
    XilSystemState* _state = (sys_state); \
    _state->notifyWarning((category), (id), (primary), __LINE__, __FILE__,(XilObject*)NULL, (arg)); \
}

#define XIL_OBJ_STR_ERROR(sys_state, category, id, primary, object, str) \
{ \
    XilSystemState* _state = (sys_state); \
    object->setErrorString((str));  \
    _state->notifyError((category), (id), (primary), __LINE__, __FILE__, (object), NULL); \
    object->setErrorString(NULL); \
}

#endif // _XIL_SYSTEM_STATE_HH
