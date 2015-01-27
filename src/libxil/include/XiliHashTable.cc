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
//  File:	XiliHashTable.cc
//  Project:	XIL
//  Revision:	1.19
//  Last Mod:	10:07:56, 03/10/00
//
//  Description:
//	More implementation of XiliHashTable template.
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XiliHashTable.cc	1.19\t00/03/10  "

//
//  System Includes
//
#include <string.h>

//
//  C++ Includes
//
#include "_XilDefines.h"

#include "XiliUtils.hh"
#include "XiliHashTable.hh"

template <class Type>
XiliHashTable<Type>::XiliHashTable(unsigned int    hash_table_size,
                                   XilSystemState* sys_state,
                                   Xil_boolean     delete_values_flag)
    : tableSize(hash_table_size),
      systemState(sys_state),
      deleteValues(delete_values_flag)
{
    isOKFlag = FALSE;

    table     = new XiliHashRecord*[tableSize];

    if(table == NULL) {
        XIL_ERROR(systemState, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return;
    }

    xili_memset(table, 0, sizeof(XiliHashRecord*)*tableSize);

    isOKFlag = TRUE;
}

template <class Type>
XiliHashTable<Type>::~XiliHashTable()
{
    if(table) {
        for(unsigned int i=0; i<tableSize; i++) {
            XiliHashRecord* tmp = table[i];
            while(tmp) {
                XiliHashRecord* next = tmp->next;

                //
                //  Always allocated using strdup().
                //
                free(tmp->key);
                
                if(deleteValues) {
                    delete tmp->value;
                }
                
                delete tmp;

                tmp = next;
            }
        }
        delete table;
    }
}

template <class Type>
Xil_boolean
XiliHashTable<Type>::isOK()
{
    _XIL_ISOK_TEST();
}

//
//  Insert a value into the hash table.
//
template <class Type>
XilStatus
XiliHashTable<Type>::insert(const char* key,
                            const Type& value)
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
    XiliHashRecord* record =
        new XiliHashRecord(strdup(key), value, table[location]);

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
template <class Type>
XilStatus
XiliHashTable<Type>::lookup(const char* key,
                            Type&       value)
{
    unsigned int location = xili_hash_string(key) % tableSize;
    
    XiliHashRecord* head = table[location];

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
template <class Type>
XilStatus
XiliHashTable<Type>::remove(const char* key)
{
    Type value;
    if(remove(key, value) == XIL_SUCCESS) {
        if(deleteValues) {
            delete value;
        }
    }

    return XIL_FAILURE;
}

//
//  Remove an entry from the hash table.
//
template <class Type>
XilStatus
XiliHashTable<Type>::remove(const char* key,
                            Type&       value)
{
    unsigned int location = xili_hash_string(key) % tableSize;

    XiliHashRecord* head = table[location];
    XiliHashRecord* prev = NULL;

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
template <class Type>
XilStatus
XiliHashTable<Type>::insert(XiliString& key,
                            const Type& value)
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
    XiliHashRecord* record =
        new XiliHashRecord(strdup(key), value, table[location]);

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
template <class Type>
XilStatus
XiliHashTable<Type>::lookup(XiliString& key,
                            Type&       value)
{
    unsigned int location = key.hash() % tableSize;
    
    XiliHashRecord* head = table[location];

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

//
//  Remove an entry from the hash table.
//
template <class Type>
XilStatus
XiliHashTable<Type>::remove(XiliString& key)
{
    Type value;
    if(remove(key, value) == XIL_SUCCESS) {
        if(deleteValues) {
            delete value;
        }
    }

    return XIL_FAILURE;
}

template <class Type>
XilStatus
XiliHashTable<Type>::remove(XiliString& key,
                            Type&       value)
{
    unsigned int location = key.hash() % tableSize;

    XiliHashRecord* head = table[location];
    XiliHashRecord* prev = NULL;

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

