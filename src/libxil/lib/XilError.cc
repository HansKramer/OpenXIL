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
//  File:	XilError.cc
//  Project:	XIL
//  Revision:	1.20
//  Last Mod:	10:08:42, 03/10/00
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
//  MT-level:  UNSAFE
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilError.cc	1.20\t00/03/10  "

//
//  System Includes
//
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#if !defined(IRIX) && !defined(HPUX) && !defined(_WINDOWS)
#include <libintl.h>
#endif
#include <stdlib.h>

//
//  XIL Includes
//
#include "XiliUtils.hh"
#include "_XilDefines.h"
#include "XiliMemMapFile.hh"
#include "_XilSystemState.hh"

#include "XilError.hh"

//
//  TODO: 1/30/96 jlf  Sync this with the one in the db build program so
//                     it's in one place instead of two.
//
struct XiliErrorDBHeader {
    unsigned int version;
    unsigned int num_entries;

    unsigned int extra_data[32];
};

const char*
XilError::getString()
{
    static XilMutex db_mutex;
    static XiliErrorDBHeader header;
    static unsigned int* offset_table;
    static XiliMemMapFile *mMap = NULL;

    //
    //  Check that the id is one that lives in our table.
    //
    if(strncmp(errorId, "di-", 3)) {
        return errorId;
    }

    db_mutex.lock();

    //
    //  TODO: 1/30/96 jlf  Figure out how to generate errors properly in this
    //                     routine. 
    //
    if(mMap == NULL) {
        char* err_fname = XiliGetPath("utils/xil_errors_db");

        if(err_fname == NULL) {
            db_mutex.unlock();
            return errorId;
        }

        mMap = new XiliMemMapFile(err_fname,O_RDONLY,M_PROT_RD,M_MAP_SHARED);
        free(err_fname);

        if(mMap->memMap() == XIL_FAILURE) {
            db_mutex.unlock();
            delete mMap; mMap=NULL;
            return errorId;
        }

        header = *((XiliErrorDBHeader*)mMap->getMemMap());

        if(header.version != 1) {
	    db_mutex.unlock();
            delete mMap; mMap=NULL;
            return errorId;
        }

        offset_table = (unsigned int*)mMap->getMemMap();
        offset_table = (unsigned int *)
                       (((char*)offset_table)+sizeof(XiliErrorDBHeader));
    }

    db_mutex.unlock();

    unsigned int id_num = atoi(errorId+3);
    if(id_num == 0) {
        delete mMap; mMap=NULL;
        return errorId;
    }

    //
    //  The errors start a 1, our table starts at 0.
    //
    id_num--;  

    if(offset_table[id_num] == -1) {            // no error defined for this
        return errorId;
    }
    
    if(id_num >= header.num_entries) {
        return
            (const char*)xili_dgettext("xil",
                                  "ERROR in string lookup -- id is > table size");
    } else {
        if(errorArg != NULL) {
            //
            //  We need to sprintf into a buffer with the given arguments.
            //
            const char* errstr =
                (const char*)xili_dgettext("xil", 
                               (char*)mMap->getMemMap() + offset_table[id_num]);

            errorString = new char[strlen(errstr) + BUFSIZ];

            if(errorString == NULL) {
                //
                //  Just return it without the args -- more useful than no error
                //  at all....
                //
                return errstr;
            }

            sprintf(errorString, errstr, errorArg);

            return (const char*)errorString;
        } else {
            return (const char*)xili_dgettext("xil",
                               (char*)mMap->getMemMap() + offset_table[id_num]);
        }
    }
}

//
//  Call next handler in the handler hierarchy
//
Xil_boolean
XilError::callNextErrorHandler()
{
    return errorSystemState->callNextErrorHandler(this);
}

