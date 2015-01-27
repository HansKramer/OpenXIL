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
//  File:	XiliString.hh
//  Project:	XIL
//  Revision:	1.9
//  Last Mod:	10:20:52, 03/10/00
//
//  Description:
//	
//	This class keeps a character string and hashes that string.  It
//      keeps a cache of the hash value so the string is hashed only 
//      if it is changed.
//
//      NOTE:  Currently, there is no mechanism for resetting the string
//             so the string is only hashed the first time it is used.
//
//      NOTE:  This class does NOT copy the string it's given.  It will
//             only store a reference to the string.
//
//  MT-level:  UNsafe
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XiliString.hh	1.9\t00/03/10  "

#ifndef _XILI_STRING_HH
#define _XILI_STRING_HH

//
//  System Includes
//
#include <string.h>

//
//  XIL Includes
//
#include "_XilDefines.h"
#include "XiliUtils.hh"

class XiliString {
public:
    //
    //  Function to get a fairly random hash of the string.
    //
    unsigned int  hash()
    {
        if(cacheValidFlag == FALSE) {
            cacheHashValue = xili_hash_string(theString);
            cacheValidFlag = TRUE;
        }

        return cacheHashValue;
    }

    //
    //  Return the length of the string, -1 if the string is NULL.
    //
    int          length()
    {
        if(strLength == -1 && theString != NULL) {
            strLength = strlen(theString);
        }
        return strLength;
    }
    
    //
    //  Operator to cast this to a const char* or char*.
    //
                  operator const char*()
    {
        return (const char*)theString;
    }


    //
    //  Constructors
    //
                    XiliString(const char* init_ptr) :
                        theString(init_ptr)
    {
        strLength = -1;
        cacheValidFlag  = FALSE;
    }
    
                    XiliString(const XiliString& init_string) :
                        theString(init_string.theString)
    {
        strLength      = init_string.strLength;
        cacheValidFlag = init_string.cacheValidFlag;
        cacheHashValue = init_string.cacheHashValue;
    }
    
                  XiliString(const XiliString* init_string) :
                        theString(init_string->theString)
    {
        strLength      = init_string->strLength;
        cacheValidFlag = init_string->cacheValidFlag;
        cacheHashValue = init_string->cacheHashValue;
    }
    

    //
    //  Destructor
    //
                  ~XiliString()
    {
    }
    

protected:
    const char*   theString;
    int           strLength;
    unsigned int  cacheHashValue;
    Xil_boolean   cacheValidFlag;
};

#endif // _XILI_STRING_HH
