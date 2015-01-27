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
//  File:	XiliHashTable.hh
//  Project:	XIL
//  Revision:	1.21
//  Last Mod:	10:20:53, 03/10/00
//
//  Description:
//	
//	A simple hash table for quick storage and retrieval.
//
//      It stores references to void* and uses a char* as a key
//      for determining table location and matching on retrieval.
//
//      The size of the hash table is variable and can be specified to
//      something other than _XILI_DEFAULT_HASH_TABLE_SIZE at
//      constuction.
//
//      By default, the hash table destructor will delete the values
//      put into the table.  If this is not the desired effect, it can
//      be changed by the delete_values_flag at construction time.
//
//  MT Level:   Un-Safe
//
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XiliHashTable.hh	1.21\t00/03/10  "

#ifndef _XILI_HASH_TABLE_HH
#define _XILI_HASH_TABLE_HH

#include "_XilDefines.h"
#include "XiliString.hh"

//
//  Default HashTable Size.
//
//  It should be a prime number for the hashing to work best.
//
#define _XILI_DEFAULT_HASH_TABLE_SIZE  23

template<class Type>
class XiliHashTable {
public:
    //
    //  Associate the given value with the specified key.
    //
    XilStatus        insert(const char*  key,
                            const Type&  value);

    XilStatus        insert(XiliString&  key,
                            const Type&  value);

    XilStatus        lookup(const char*  key,
                            Type&        value);
    
    XilStatus        lookup(XiliString&  key,
                            Type&        value);

    //
    //  Remove AND delete (if delete_values_flag is TRUE) the entry associated
    //  with the given KEY.
    //
    XilStatus        remove(const char*  key);
    XilStatus        remove(XiliString&  key);

    //
    //  Remove and return the entry associated with the given KEY.  Obviously,
    //  the value is not deleted because it's being returned.
    //
    XilStatus        remove(XiliString&  key,
                            Type&        value);
    XilStatus        remove(const char*  key,
                            Type&        value);

    Xil_boolean      isOK();

                     XiliHashTable(unsigned int    hash_table_size    =
                                   _XILI_DEFAULT_HASH_TABLE_SIZE,
                                   XilSystemState* sys_state          = NULL,
                                   Xil_boolean     delete_values_flag = TRUE);
                     ~XiliHashTable();
    
    //
    //  XiliHashRecord is a list of key / value pairs
    //
    class XiliHashRecord {
    public:
        XiliHashRecord* next;
        
        char*           key;
        Type            value;

        XiliHashRecord(char*           init_key,
                       const Type&     init_value,
                       XiliHashRecord* init_next  = NULL) :
            key(init_key),
            value(init_value),
            next(init_next)
        {
        }
    };

    //
    //  Whether the object was constructed ok.
    //
    Xil_boolean         isOKFlag;

    //
    //  The hash table.
    //
    XiliHashRecord**    table;

    //
    //  Initialized at construction.
    //
    const unsigned int  tableSize;
    const Xil_boolean   deleteValues;

    //
    //  The XilSystemState for reporting errors.
    //
    XilSystemState*     systemState;

};

#endif // _XILI_HASH_TABLE_HH
