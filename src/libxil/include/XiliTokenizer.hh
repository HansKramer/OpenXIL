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
//  File:	XiliTokenizer.hh
//  Project:	XIL
//  Revision:	1.10
//  Last Mod:	10:20:54, 03/10/00
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
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XiliTokenizer.hh	1.10\t00/03/10  "

#ifndef _XILI_TOKENIZER_HH
#define _XILI_TOKENIZER_HH

#include "_XilSystemState.hh"
#include "_XilMutex.hh"

class	XiliTokenizer {
public:
    //
    //  Constructor
    //
          XiliTokenizer(const char* string,
                        const char* delimiters) :
              delims(delimiters)
    {
        last = NULL;
        next = NULL;

        if((parseString = strdup(string)) == NULL) {
            XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
        } else {
#if defined(_XIL_USE_PTHREADS) || defined(_XIL_USE_SOLTHREADS)
            next = strtok_r(parseString, delims, &last);
#else
            lastptr = &parseString[strlen(parseString)];
            if(next = strtok(parseString, delims)) {
                last = &next[strlen(next)];
            }

            if(last && last < lastptr) {
                last++;
            } else {
                last = NULL;
            }
 
#endif
            if(next == NULL) {
                next = "";
            }
        }
    }
    
          ~XiliTokenizer()
    {
        free(parseString);
    }

    //
    //  Get the next token using the characters in delims to delimate tokens.
    //
    char* getNext()
    {
        char* token = NULL;
        char* ret_val = next;

#if defined(_XIL_USE_PTHREADS) || defined(_XIL_USE_SOLTHREADS)
        token = strtok_r(NULL, delims, &last);
#else
        if(last) {
            last = NULL;
            if(token = strtok(last, delims)) {
                last = &token[strlen(token)];
            }
        }

        if(last && last < lastptr) {
            last++;
        } else {
            last = NULL;
        }
#endif
        if(token == NULL) {
            next = "";
        } else {
            next = token;
        }
        
        return ret_val;
    }

private:
    Xil_boolean isOKFlag;
    char*       parseString;
    char*       last;
    const char* delims;
    char*       next;
    char*       lastptr;
};

#endif // _XILI_TOKENIZER_HH
