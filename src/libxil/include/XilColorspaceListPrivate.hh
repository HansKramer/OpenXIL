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
//  File:	XilColorspaceListPrivate.hh
//  Project:	XIL
//  Revision:	1.6
//  Last Mod:	10:22:11, 03/10/00
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
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------

#ifdef _XIL_PRIVATE_INCLUDES

#include "_XilColorspace.hh"
#include "XiliSLList.hh"

#endif // _XIL_PRIVATE_INCLUDES

#ifdef _XIL_PRIVATE_DATA

public:
    //
    // Constructor
    //
                  XilColorspaceList(XilSystemState* system_state,
                                    XilColorspace** colorspace_array,
                                    unsigned int    num_colorspaces) :
                      XilNonDeferrableObject(system_state,
                                             XIL_COLORSPACE_LIST)
    {
        isOKFlag = FALSE;

        for(unsigned int i=0; i<num_colorspaces; i++) {
            if(list.append(colorspace_array[i]) ==
               _XILI_SLLIST_INVALID_POSITION) {
                return;
            }
        }

        nextQuery       = 0;
        nextPosition    = list.head();

        cachedData      = NULL;
        doneWithDataPtr = NULL;
        
        isOKFlag = TRUE;
    }

    XilObject*    createCopy();

protected:
                  ~XilColorspaceList()
    {
        if(doneWithDataPtr != NULL) {
            (*doneWithDataPtr) (cachedData);
        }
    }

private:
    XiliSLList<XilColorspace*>    list;

    unsigned int                  nextQuery;
    XiliSLListPosition            nextPosition;

    void*                         cachedData;
    XIL_FUNCPTR_DONE_WITH_DATA    doneWithDataPtr;

#endif // _XIL_PRIVATE_DATA
