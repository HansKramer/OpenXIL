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
//  File:	XilBoxListPrivate.hh
//  Project:	XIL
//  Revision:	1.22
//  Last Mod:	10:21:59, 03/10/00
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
#pragma ident	"@(#)XilBoxListPrivate.hh	1.22\t00/03/10  "

//
//  INCLUDE Portion of private header file
//
#ifdef _XIL_PRIVATE_INCLUDES

#include "XiliSLList.hh"
#include "XiliBoxListEntry.hh"

#endif // _XIL_PRIVATE_INCLUDES

//
//  DATA Portion of private header file
//
#ifdef _XIL_PRIVATE_DATA
public:
#ifdef DEBUG
//
//  Print the contents of the box list to stderr.
//  A new incarnation of dump() that GCC won't have trouble with
//
    void                        dump()
    {
    fprintf(stderr, "XilBoxList dump of %p:\n", this);
    XiliSLList<XiliBoxListEntry*>*        blist = this->getList();
    XiliSLListIterator<XiliBoxListEntry*> bl_iterator(blist);
    XiliBoxListEntry*                     ble;
        while(bl_iterator.getNext(ble) == XIL_SUCCESS) {

            for(int i=0; i<(numSrcs+numDsts); i++) {
                ble->boxes[i].dump();
            }
        fprintf(stderr,"-------------next entry------------\n");
        }
    }
#endif

    //
    //  Adds a box to the list...
    //
    XilStatus                      addEntry(XiliBoxListEntry* new_ble);

    //
    //  Other classes that manipulat this list (XilOp for example) needs to be
    //  able to access at the list.
    //
    XiliSLList<XiliBoxListEntry*>* getList()
    {
        return &list;
    }

    //
    //  Set the number of active sources and number of destinations...
    //
    void                           setNumSrcs(unsigned int num_srcs)
    {
        numSrcs = num_srcs;
    }

    void                           setNumDsts(unsigned int num_dsts)
    {
        numDsts = num_dsts;
    }

    //
    //  Get the number of active sources and number of destinations...
    //
    unsigned int                   getNumSrcs()
    {
        return numSrcs;
    }

    unsigned int                   getNumDsts()
    {
        return numDsts;
    }

    //
    //  Clear all of the boxes from this box list.
    //
    void                           reset(XilSystemState* system_state,
                                         unsigned int    num_srcs,
                                         unsigned int    num_dsts)
    {
		this->destructorVars();
		this->constructorVars(system_state, num_srcs, num_dsts);
    }

    //
    //  Get the number of boxes on the failed list.
    //
    unsigned int                   getNumFailed()
    {
        return failedList.length();
    }

    //
    //  Reset the box list to begin again with the failed boxes as well.
    //
    XilStatus                      resetFromFailed();

    void                           setSystemState(XilSystemState* system_state)
    {
        systemState = system_state;
    }

    //
    //  Constructors and destructor...
    //
                                   XilBoxList(XilSystemState* system_state,
                                              unsigned int    num_srcs,
                                              unsigned int    num_dsts);

                                   ~XilBoxList();

    _XIL_NEW_DELETE_OVERLOAD_PUBLIC(XilBoxList)

private:
    //
    //  Construction is a two step process.  First, this constructor is called
    //  if the overload creates a new object for the list.  Then, the real
    //  constructor above is called.
    //
                                   XilBoxList();

    _XIL_NEW_DELETE_OVERLOAD_PRIVATE(XilBoxList)

    //
    //  Setup the next entry on the list -- used by getNext() routines.
    //
    void                           setupNextBox();

    unsigned int                   numSrcs;
    unsigned int                   numDsts;

    XiliSLList<XiliBoxListEntry*>  list;

    //
    //  When a compute routine marks a box as "failed", we move it to this
    //  list.  Then, upon a resetToFailed() causes us to prepend the contents
    //  of the failedList into the primary list for reprocessing.
    //
    XiliSLList<XiliBoxListEntry*>  failedList;

    XiliBoxListEntry*              outstandingBoxListEntry;

    XiliSLListPosition             currentPos;

    XilSystemState*                systemState;

	// Here are the working parts of the constructor and destructor;
	// they are used both in those and in the reset method.
	void							constructorVars(XilSystemState* system_state,
													unsigned int    num_srcs,
													unsigned int    num_dsts)
    {
		numSrcs                 = num_srcs;
		numDsts                 = num_dsts;
		currentPos              = _XILI_SLLIST_INVALID_POSITION;
		outstandingBoxListEntry = NULL;
		systemState             = system_state;
	}

	void							destructorVars()
	{
		//
		//  Delete any outstanding boxes in the case we didn't iterate to 
		//  the end of the list.
		//
		if(outstandingBoxListEntry != NULL) {
			delete outstandingBoxListEntry;
		}

		//
		//  Similarly, delete any entries that remain on the list.
		//
		XiliBoxListEntry*  ble;
		while((list.remove(list.head(), ble)) == XIL_SUCCESS) {
			delete ble;
		}
	}
#endif  // _XIL_PRIVATE_DATA
