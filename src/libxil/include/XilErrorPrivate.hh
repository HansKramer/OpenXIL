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
//  File:	XilErrorPrivate.hh
//  Project:	XIL
//  Revision:	1.10
//  Last Mod:	10:21:54, 03/10/00
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
//  MT-level:  UN-SAFE
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilErrorPrivate.hh	1.10\t00/03/10  "

#ifdef _XIL_PRIVATE_INCLUDES

#include <string.h>

#include "XiliList.hh"

#endif

#ifdef _XIL_PRIVATE_DATA
public:
    //
    //  Get the error string for the identifier associated with this object.
    //
    const char*              getString();

    //
    //  Get and Set the Error Identifier string.
    //
    const char*              getId()
    {
        return errorId;
    }

    void                     setId(const char* id_string)
    {
        errorId = id_string;
    }

    //
    //  Get and Set the Line Number.
    //
    unsigned int             getLine()
    {
        return errorLine;
    }

    void                     setLine(unsigned int line_number)
    {
        errorLine = line_number;
    }

    //
    //  Get and Set the Filename.
    //
    const char*              getFilename()
    {
        return errorFilename;
    }

    void                     setFilename(const char* filename)
    {
        errorFilename = filename;
    }

    //
    //  Get and Set the extra variable argument which is passed into sprintf
    //  to generate the error.
    //
    void*                    getArg()
    {
        return errorArg;
    }

    void                     setArg(void* arg)
    {
        errorArg = arg;
    }

    //
    //  Get and Set the Error Cagetory.
    //
    XilErrorCategory         getErrorCategory()
    {
        return errorCategory;
    }

    void                     setErrorCategory(XilErrorCategory category)
    {
        errorCategory = category;
    }

    //
    //  Get the Error Cagetory as a string.
    //
    char*                    getErrorCategoryString()
    {
        switch(errorCategory) {
          case XIL_ERROR_SYSTEM:
            return "XIL_ERROR_SYSTEM";
            
          case XIL_ERROR_RESOURCE:
            return "XIL_ERROR_RESOURCE";

          case XIL_ERROR_ARITHMETIC:
            return "XIL_ERROR_ARITHMETIC";

          case XIL_ERROR_CIS_DATA:
            return "XIL_ERROR_CIS_DATA";

          case XIL_ERROR_USER:
            return "XIL_ERROR_USER";

          case XIL_ERROR_CONFIGURATION:
            return "XIL_ERROR_CONFIGURATION";

          case XIL_ERROR_OTHER:
            return "XIL_ERROR_OTHER";

          case XIL_ERROR_INTERNAL:
          default:
            return "XIL_ERROR_INTERNAL";
        }
    }

    //
    //  Get and Set whether the error is a primary error.
    //
    Xil_boolean              getPrimaryFlag()
    {
        return errorPrimaryFlag;
    }

    void                     setPrimaryFlag(Xil_boolean primary_flag)
    {
        errorPrimaryFlag = primary_flag;
    }

    //
    //  Get and Set whether the error is really just a warning.
    //
    Xil_boolean              isWarning()
    {
        return isWarningFlag;
    }

    void                     setIsWarningFlag(Xil_boolean warning_flag)
    {
        isWarningFlag = warning_flag;
    }

    //
    //  Get and Set the object associated with this error.
    //
    XilObject*               getObject()
    {
        return errorObject;
    }

    const char*              getLocation()
    {
        const char* err_file;
        if(errorFilename == NULL) {
            err_file = "UNKNOWN_FILE";
        } else {
            err_file = errorFilename;
        }

        //
        //  We're pretty certain that the length of the line number when
        //  converted into a string will be within 20 characters.
        //
        errorLocation = new char[strlen(err_file) + 20];

        if(errorLocation != NULL) {
            sprintf(errorLocation, "%s;%d", err_file, getLine());
        }

        return errorLocation;
    }

    void                     setObject(XilObject* object)
    {
        errorObject = object;
    }

    //
    //  Get and Set the system state associated with this error.
    //
    XilSystemState*          getSystemState()
    {
        return errorSystemState;
    }

    void                     setSystemState(XilSystemState* sys_state)
    {
        errorSystemState = sys_state;
    }

    //
    //  Call next handler in the handler hierarchy
    //
    Xil_boolean              callNextErrorHandler();

    //
    //  Get and set the handler position information as the system state keeps
    //  track of which handler to call next.
    //
    XiliListPosition         getHandlerPosition()
    {
        return handlerPosition;
    }

    void                     setHandlerPosition(XiliListPosition position)
    {
        handlerPosition = position;
    }

    //
    //  Constructor/Destructor
    //
                             XilError()
    {
        errorId          = NULL;
        errorLine        = 0;
        errorFilename    = NULL;
        errorCategory    = XIL_ERROR_INTERNAL;
        errorPrimaryFlag = FALSE;
        errorObject      = NULL;
        errorSystemState = NULL;
        errorArg         = NULL;

        errorString      = NULL;
        errorLocation    = NULL;

        isWarningFlag    = FALSE;

        handlerPosition  = _XILI_LIST_INVALID_POSITION;
    }

                             ~XilError()
    {
        delete errorLocation;
        delete errorString;
    }

private:
    const char*              errorId;
    unsigned int             errorLine;
    const char*              errorFilename;
    XilErrorCategory         errorCategory;
    Xil_boolean              errorPrimaryFlag;
    XilObject*               errorObject;
    void*                    errorArg;
    XilSystemState*          errorSystemState;
    Xil_boolean              isWarningFlag;

    //
    //  Location string generated on-demand for this error.
    //
    char*                    errorLocation;

    //
    //  If we needed to create a temporary buffer to hold the final error
    //  string, it will be set on here for us to delete when destructed.
    //
    char*                    errorString;

    XiliListPosition         handlerPosition;
#endif // _XIL_PRIVATE_DATA
