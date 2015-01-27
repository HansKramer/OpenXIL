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
//  File:	XilDeviceManager.cc
//  Project:	XIL
//  Revision:	1.6
//  Last Mod:	10:09:01, 03/10/00
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
#pragma ident	"@(#)XilDeviceManager.cc	1.6\t00/03/10  "

#include "_XilDefines.h"
#include "_XilDeviceManager.hh"
#include "_XilGlobalState.hh"

XilDeviceManager::XilDeviceManager()
{
    //
    //  Stash a reference to the global state and the base of the op tree for
    //    future use.  Both are guarenteed not to move for the duration of this
    //    instance of XIL.  We can only use our opTreeBase member when the
    //    tree has been locked via another call to getOpTreeBase().  The
    //    locking is done by the core when describeMembers is called so we can
    //    safely use the cached pointer in addFunction().
    //
    globalState = XilGlobalState::getXilGlobalState();
}

XilDeviceManager::~XilDeviceManager()
{
}

void
XilDeviceManager::destroy()
{
    delete this;
}

XilStatus
XilDeviceManager::addFunction(XilFunctionInfo* func_info)
{
    XiliOpTreeNode*  op_tree_base = globalState->getOpTreeBase(TRUE);

    //
    //  While we still have entries available to us on the list, process each
    //    of the directions.
    //
    XiliList<XiliDirection>* dir_list = func_info->getDirectionList();
    XiliDirection*           dle;
    XiliListPosition         cur_pos  = dir_list->head();
    XiliOpTreeNode*          cur_node = op_tree_base;
    while(cur_pos != _XILI_LIST_INVALID_POSITION) {
        dle = dir_list->reference(cur_pos);
        
        cur_node = cur_node->branch(dle->opNumber);
        cur_pos  = dir_list->next(cur_pos);

        //
        //  If we're at the end of our list, it's time to add the function.
        //    An XIL_STEP will always be the last item because if it's an
        //    XIL_PUSH then we will be visiting the operation again.
        //    And, we have made the decision to shorten the list by
        //    pointing out that there is no need to add XIL_POP directions.
        //
        if(cur_pos == _XILI_LIST_INVALID_POSITION) {
            cur_node->addFunction(this, func_info);
        }
    }

    globalState->releaseOpTreeBase();
    
    return XIL_SUCCESS;
}
