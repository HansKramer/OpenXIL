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
//  File:	XiliObjectHashTable.cc
//  Project:	XIL
//  Revision:	1.3
//  Last Mod:	10:09:07, 03/10/00
//
//  Description:
//	
//	A hash table specialized for XilObject.
//
//      It stores references to XilObject and uses a char* as a key
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
#pragma ident	"@(#)XiliObjectHashTable.cc	1.3\t00/03/10  "

#include "XiliObjectHashTable.hh"
#include "_XilSystemState.hh"
#include "_XilObject.hh"

XiliObjectHashTable::XiliObjectHashTable(unsigned int    hash_table_size,
                                   XilSystemState* sys_state,
                                   Xil_boolean     delete_values_flag)
    : tableSize(hash_table_size),
      systemState(sys_state),
      deleteValues(delete_values_flag)
{
    isOKFlag = FALSE;

    table     = new XiliObjectHashRecord*[tableSize];

    if(table == NULL) {
        XIL_ERROR(systemState, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return;
    }

    memset(table, 0, sizeof(XiliObjectHashRecord*)*tableSize);

    isOKFlag = TRUE;
}

//
// Need to use destroy() rather than delete for XilObject
//
XiliObjectHashTable::~XiliObjectHashTable()
{
    if(table) {
        for(unsigned int i=0; i<tableSize; i++) {
            XiliObjectHashRecord* tmp = table[i];
            while(tmp) {
                XiliObjectHashRecord* next = tmp->next;

                //
                //  Destroying the object has the effect of it being removed
                //  from the table so we just destroy it and move onto the
                //  next entry.
                //
                tmp->value->destroy();

                tmp = next;
            }
        }
        delete table;
    }
}

XilStatus
XiliObjectHashTable::remove(const char* key)
{
    XilObject* value = NULL;

    if(remove(key, value) == XIL_SUCCESS) {
        value->destroy();
    }

    return XIL_FAILURE;
}

XilStatus 
XiliObjectHashTable::remove(XiliString& key)
{
    XilObject* value = NULL;
    if(remove(key, value) == XIL_SUCCESS) {
        value->destroy();
    }

    return XIL_FAILURE;
}

Xil_boolean
XiliObjectHashTable::isOK()
{
    _XIL_ISOK_TEST();
}

//
//  Insert a value into the hash table.
//
XilStatus
XiliObjectHashTable::insert(const char* key,
                            XilObjectPtr& value)
{
    //
    //  Hash the string using the utility hash function to generate a
    //  relatively sparse location.  Mod that with the tableSize to determine
    //  where to put the key/value.
    //
    //  NOTE:  The XiliString class uses this same hash function.  In order to
    //         support this insertion method, this must remain true.
    //
    unsigned int location = xili_hash_string(key) % tableSize;

    //
    //  Create and fill-in a new record to hold the information.
    //
    XiliObjectHashRecord* record =
        new XiliObjectHashRecord(strdup(key), value, table[location]);

    if(record == NULL || record->key == NULL) {
        XIL_ERROR(systemState, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }

    table[location] = record;

    return XIL_SUCCESS;
}

//
//  Lookup a value from the hash table.
//
XilStatus
XiliObjectHashTable::lookup(const char* key,
                            XilObjectPtr&       value)
{
    unsigned int location = xili_hash_string(key) % tableSize;
    
    XiliObjectHashRecord* head = table[location];

    while(head) {
        if(!strcmp(head->key, key)) {
            value = head->value;

            return XIL_SUCCESS;
        } else {
            head    = head->next;
        }
    }

    return XIL_FAILURE;
}

//
//  Remove an entry from the hash table.
//
XilStatus
XiliObjectHashTable::remove(const char* key,
                            XilObjectPtr&       value)
{
    unsigned int location = xili_hash_string(key) % tableSize;

    XiliObjectHashRecord* head = table[location];
    XiliObjectHashRecord* prev = NULL;

    while(head) {
        if(!strcmp(head->key, key)) {
            if(prev != NULL) {
                prev->next      = head->next;
            } else {
                table[location] = NULL;
            }

            //
            //  Always allocated using strdup().
            //
            free(head->key);

            value = head->value;

            delete head;

            return XIL_SUCCESS;
        } else {
            if(prev != NULL) {
                prev = prev->next;
            } else {
                prev = head;
            }

            head    = head->next;
        }
    }

    return XIL_FAILURE;
}

//
//  Insert a value into the hash table.
//
XilStatus
XiliObjectHashTable::insert(XiliString& key,
                            XilObjectPtr& value)
{
    //
    //  Hash the string using the XiliString hash function to generate a
    //  relatively sparse location.  Mod that with the tableSize to determine
    //  where to put the key/value.
    //
    unsigned int location = key.hash() % tableSize;
    
    //
    //  Create and fill-in a new record to hold the information.
    //
    XiliObjectHashRecord* record =
        new XiliObjectHashRecord(strdup(key), value, table[location]);

    if(record == NULL || record->key == NULL) {
        XIL_ERROR(systemState, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }
    
    table[location] = record;

    return XIL_SUCCESS;
}

//
//  Lookup a value from the hash table.
//
XilStatus
XiliObjectHashTable::lookup(XiliString& key,
                            XilObjectPtr&       value)
{
    unsigned int location = key.hash() % tableSize;
    
    XiliObjectHashRecord* head = table[location];

    //
    //  Run through the linked list found at that location and search for the
    //  key.
    //
    while(head) {
        if(!strcmp(head->key, key)) {
            value = head->value;

            return XIL_SUCCESS;
        } else {
            head    = head->next;
        }
    }

    return XIL_FAILURE;
}

XilStatus
XiliObjectHashTable::remove(XiliString& key,
                            XilObjectPtr&       value)
{
    unsigned int location = key.hash() % tableSize;

    XiliObjectHashRecord* head = table[location];
    XiliObjectHashRecord* prev = NULL;

    while(head) {
        if(!strcmp(head->key, key)) {
            if(prev != NULL) {
                prev->next      = head->next;
            } else {
                table[location] = NULL;
            }

            //
            //  Always allocated using strdup().
            //
            free(head->key);

            value = head->value;

            delete head;
    
            return XIL_SUCCESS;
        } else {
            if(prev != NULL) {
                prev = prev->next;
            } else {
                prev = head;
            }

            head    = head->next;
        }
    }

    return XIL_FAILURE;
}

