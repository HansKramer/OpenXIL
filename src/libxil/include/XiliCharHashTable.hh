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
//  File:	XiliCharHashTable.hh
//  Project:	XIL
//  Revision:	1.2
//  Last Mod:	10:22:18, 03/10/00
//
//  Description:
//	
//	A hash table specialized for strings (char*)
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
#pragma ident	"@(#)XiliCharHashTable.hh	1.2\t00/03/10  "

#ifndef _XILI_CHAR_HASH_TABLE_HH
#define _XILI_CHAR_HASH_TABLE_HH

#include "_XilDefines.h"
#include "XiliString.hh"

//
//  Default HashTable Size.
//
//  It should be a prime number for the hashing to work best.
//
#define _XILI_DEFAULT_HASH_TABLE_SIZE  23


typedef char* CharPtr;

class XiliCharHashTable {
public:

    //
    //  Associate the given value with the specified key.
    //
    XilStatus        insert(const char*  key,
                            CharPtr&  value);

    XilStatus        insert(XiliString&  key,
                            CharPtr&  value);

    XilStatus        lookup(const char*  key,
                            CharPtr&        value);
    
    XilStatus        lookup(XiliString&  key,
                            CharPtr&        value);

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
                            CharPtr&        value);
    XilStatus        remove(const char*  key,
                            CharPtr&        value);

    Xil_boolean      isOK();

                     XiliCharHashTable(unsigned int    hash_table_size    =
                                   _XILI_DEFAULT_HASH_TABLE_SIZE,
                                   XilSystemState* sys_state          = NULL,
                                   Xil_boolean     delete_values_flag = TRUE);
                     ~XiliCharHashTable();
    

    //
    //  XiliCharHashRecord is a list of key / value pairs
    //
    class XiliCharHashRecord {
    public:
        XiliCharHashRecord* next;
        
        char*           key;
        CharPtr           value;

        XiliCharHashRecord(char*           init_key,
                           CharPtr&     init_value,
                           XiliCharHashRecord* init_next  = NULL) :
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
    XiliCharHashRecord**    table;

    //
    //  Initialized at construction.
    //
    const unsigned int  tableSize;
    const Xil_boolean   deleteValues;

    //
    //  The XilSystemState for reporting errors.
    //
    XilSystemState*     systemState;

public:
    //
    // Accessor functions to be inlined
    //
    const unsigned int     getTableSize()           { return tableSize; }
    XiliCharHashRecord*    getTable(unsigned int i) { return table[i]; }

};

#endif // _XILI_CHAR_HASH_TABLE_HH
