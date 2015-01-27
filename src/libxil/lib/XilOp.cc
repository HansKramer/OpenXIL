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
//  File:	XilOp.cc
//  Project:	XIL
//  Revision:	1.161
//  Last Mod:	10:08:09, 03/10/00
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
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilOp.cc	1.161\t00/03/10  "

//
//  System Includes
//
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifdef OP_DEBUG
#include "XiliThread.hh"
#endif

//
//  XIL Includes
//
#include "_XilDefines.h"
#include "_XilGlobalState.hh"
#include "_XilSystemState.hh"
#include "_XilBoxList.hh"
#include "_XilImage.hh"
#include "_XilCis.hh"
#include "_XilOp.hh"
#include "_XilDeviceManagerCompute.hh"
#include "_XilBox.hh"
#include "_XilDeviceIO.hh"

#include "XiliDag.hh"
#include "XiliUtils.hh"
#include "XiliRect.hh"

//--------------------------------------------------------------
//
//  XilOp Protected GPI Routines
//
//--------------------------------------------------------------
XilOp::XilOp(XilOpNumber op_num)
{
    //
    //  Initialize Src/Dst/Parameter Information
    //
    //  None of the object pointers are initialized in the constructor because
    //  we use numSrcs, numDsts and numParams exclusively.  So, we don't need
    //  to initialize the arrays unecessarily.
    //
    //  We just initialize our counters.
    //
    numParams          = 0;
    numSrcs            = 0;
    numDsts            = 0;

    //
    //  We do expect the srcOp[] array entries to be initialized to NULL.
    //
    for(int i=0; i<XIL_MAX_SRC_DEFOBJS; i++) {
        srcOp[i]        = NULL;
        srcIsCapture[i] = FALSE;
    }

    //
    //  Intersected ROI object space flag
    //
    intersectedRoiIsInObjectSpace = FALSE;

    //
    //  The operation number.
    //
    opNumber           = op_num;

    //
    //  The function (i.e. op tree node) we execute and current maximum chain length.
    //
    funcDef            = NULL;
    funcLength         = UINT_MAX;

    //
    //  Always start deferred.
    //
    opStatus           = XILI_DEFERRED;

    //
    //  We belong to no DAG until we're inserted via execute().
    //
    dagRef             = NULL;

    //
    //  Whether we should delete ourself when we're done being flushed.  By
    //  default, it's TRUE, execute() may change it to FALSE for certain
    //  operations.
    //
    deleteMe           = TRUE;

    //
    //  The list of functions which have provided data for us to hold.
    //
    funcDataList       = NULL;

    //
    //  The list of operations which make up a molecule ending at us.
    //
    opList             = NULL;
    moleculeBottom     = NULL;

    //
    //  Nobody references us yet.
    //
    refCount           = 0;

    //
    //  We're not forward mapping until we've got a source operation that
    //  requires forward mapping behavior.
    //
    forwardMappingFlag = FALSE;
    forwardStartOp     = NULL;

    //
    //  Initialize the op behavior flags.
    //
    reordersTiles      = FALSE;

    //
    //  Initialize the effective system state to NULL.
    //
    systemState        = NULL;

    TNF_PROBE_1(xilop_construct, "xilop", "xilop_construct",
                tnf_opaque, "this", this);
}

XilOp::~XilOp()
{
#ifdef OP_DEBUG
    fprintf(stderr, "%02d: %p destroy %15s\n", XiliThread::self(),
            this,
            opNumber == -1 ? "-1" : XilGlobalState::theXGS->lookupOpName(opNumber));
#endif

    TNF_PROBE_1(xilop_destructor_begin, "xilop", "xilop_destructor_begin",
                tnf_opaque, "this", this);

    if(funcDataList != NULL) {
        callPostprocessRoutines();
    }

    //
    //  Throw away the parameters
    //
    if(numParams != 0) {
        for(unsigned int i=0; i<numParams; i++) {
            if(paramDestroyMethod[i] == XIL_DELETE) {
                delete params[i].p;
            } else if(paramDestroyMethod[i] == XIL_DESTROY) {
                params[i].o->destroy();
            } else if(paramDestroyMethod[i] == XIL_RELEASE_REF) {
                params[i].o->releaseDefRef(this);
            }
        }
    }

    //
    //  Effectively toss all of the source operations...
    //
    for(unsigned int i=0; i<numSrcs; i++) {
        //
        //  Remove all references to this operation from the source
        //  objects.   This can cause the object to go away and subsequent
        //  references to be removed so we must test for src[i] being NULL.
        //
        if(src[i] != NULL) {
            src[i]->cleanup(srcQueuePos[i], this);
        }

        if(srcIsCapture[i] && srcOp[i] != NULL) {
            srcOp[i]->destroy();
        }
    }

    //
    //  We only need to call creatorCleanup on dst[0] because its the only one
    //  with op information.
    //
    if(numDsts != 0) {
        if(dst[0] != NULL) {
            dst[0]->creatorCleanup(dstQueuePos[0], this);
        }
    }

    TNF_PROBE_1(xilop_destructor_cleanup_done, "xilop", "xilop_destructor_cleanup_done",
                tnf_opaque, "this", this);
    
    //
    //  Delete the opList.
    //
    if(opList != NULL) {
        delete [] opList;
    }

    //
    //  Release our reference to our DAG
    //
    dagRef->release();

#ifdef OP_DEBUG
    fprintf(stderr, "%02d: %p completed destroy %15s\n", XiliThread::self(),
            this,
            opNumber == -1 ? "-1" : XilGlobalState::theXGS->lookupOpName(opNumber));
#endif

    TNF_PROBE_1(xilop_destructor_end, "xilop", "xilop_destructor_end",
                tnf_opaque, "this", this);
}

//
//  Insert this op into the DAG.
//
XilStatus
XilOp::insert(Xil_boolean lock_dag)
{
#ifdef OP_DEBUG
    fprintf(stderr, "%02d: %p insert %15s:  %p (%p) - %p (%p)\n",
            XiliThread::self(),
            this,
            opNumber == -1 ? "-1" : XilGlobalState::theXGS->lookupOpName(opNumber),
            numSrcs ? src[0] : NULL,
            numSrcs ? ((XilImage*)src[0])->getParent() : NULL,
            numDsts ? dst[0] : NULL,
            numDsts ? ((XilImage*)dst[0])->getParent() : NULL);
#endif            

    unsigned int i;

    //
    //  Determine which DAG we're going into and set the objects.  We do this
    //  first so the DAG is locked while we're making connections.
    //
    XiliDagRef*     dag_ref;

    //
    //  The dag_manager is needed in order to unlock the dag_ref before
    //  returning.  dag_ref->unlockDag() assumes a locked dagManager.
    //
    XiliDagManager* dag_manager = XilGlobalState::theXGS->getDagManager();

    if(lock_dag) {
        dag_ref = setupAndLockDAG();
        if(dag_ref == NULL) {
            XIL_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-404", FALSE);
            return XIL_FAILURE;
        }
    }

    //
    //  Does one of our sources have a device that requires a capture
    //  operation to be inserted?
    //
    for(i=0; i<numSrcs; i++) {
        XilDeviceIO* dev_io = src[i]->getDeviceIO();

        if(dev_io != NULL) {
            XilOp* capture_op = src[i]->createCaptureOp(this, i + 1);

            if(capture_op == NULL) {
                XIL_ERROR(src[i]->getSystemState(), XIL_ERROR_SYSTEM,
                          "di-21", FALSE);
                if(lock_dag) {
                    dag_manager->lock();
                    dag_ref->unlockDag();
                    dag_ref->release();
                    dag_manager->unlock();
                }
                return XIL_FAILURE;
            }

            if(capture_op->execute() == XIL_FAILURE) {
                XIL_ERROR(src[i]->getSystemState(), XIL_ERROR_SYSTEM,
                          "di-400", FALSE);
                if(lock_dag) {
                    dag_manager->lock();
                    dag_ref->unlockDag();
                    dag_ref->release();
                    dag_manager->unlock();
                }
                return XIL_FAILURE;
            }

            //
            //  Indicate this source is a capture operation.
            //
            srcIsCapture[i] = TRUE;
        }
    }
        
    //
    //  Does one of our destinations have a device that requires a display
    //  operation to be inserted?
    //
    //  TODO: 7/25/96 jlf  Think about how this will really work for multiple
    //                     destination operations.  It seems iffy as is.
    //
    for(i=0; i<numDsts; i++) {
        XilDeviceIO* dev_io = dst[i]->getDeviceIO();

        if(dev_io != NULL) {
            //
            //  If the number of depth of the destination is different from
            //  the controlling object (think child) and the controlling
            //  object's storage is not marked as "up-to-date" and the device's
            //  display operation does not support writing sub-portions of a
            //  pixel, then we need to perform a capture into the image prior
            //  to using it to display. 
            //
            //  With deferred execution enabled, this will only cause the
            //  pixels (and tiles) the operation needs to be captured.
            //
            //  TODO: 8/16/96 jlf  Actually it may not do just the required
            //                     tiles because the ROI won't be set properly.
            //
            XilDeferrableObject* controlling_object =
                dev_io->getControllingImage();

            //
            //  TODO: 8/16/96 jlf  When device I/O objects arn't just
            //                     associated with images, remove casts --
            //                     maybe put virtual function on op?
            //
            if(((XilImage*)dst[i])->getNumBands() !=
               ((XilImage*)controlling_object)->getNumBands() &&
               controlling_object->isStorageValid() == FALSE &&
               dev_io->hasSubPixelDisplay() == FALSE) {
                //
                //  Setup a capture into the destination object.
                //
                //  For destination captures, the branch number is above and
                //  beyond the number of sources in the operation.
                //
                XilOp* capture_op = dst[i]->createCaptureOp(this,
                                                            numSrcs + i + 1);

                if(capture_op == NULL) {
                    XIL_ERROR(dst[i]->getSystemState(), XIL_ERROR_SYSTEM,
                              "di-21", FALSE);
                    if(lock_dag) {
                        dag_manager->lock();
                        dag_ref->unlockDag();
                        dag_ref->release();
                        dag_manager->unlock();
                    }
                    return XIL_FAILURE;
                }

                if(capture_op->execute() == XIL_FAILURE) {
                    XIL_ERROR(dst[i]->getSystemState(), XIL_ERROR_SYSTEM,
                              "di-400", FALSE);
                    if(lock_dag) {
                        dag_manager->lock();
                        dag_ref->unlockDag();
                        dag_ref->release();
                        dag_manager->unlock();
                    }
                    return XIL_FAILURE;
                }
            }

            //
            //  Now, go ahead and setup the display operation.
            //
            XilOp* display_op = dst[i]->createDisplayOp(this, i + 1);

            if(display_op == NULL) {
                XIL_ERROR(dst[i]->getSystemState(), XIL_ERROR_SYSTEM,
                          "di-21", FALSE);
                if(lock_dag) {
                    dag_manager->lock();
                    dag_ref->unlockDag();
                    dag_ref->release();
                    dag_manager->unlock();
                }
                return XIL_FAILURE;
            }

            //
            //  Indicate that a potential flush() routine should NOT cleanup
            //  this operation because we still need the op to live until
            //  after we ask the the display op to be executed.
            //
            aquire();

            if(execute() == XIL_FAILURE) {
                XIL_ERROR(dst[i]->getSystemState(), XIL_ERROR_SYSTEM,
                          "di-400", FALSE);
                if(lock_dag) {
                    dag_manager->lock();
                    dag_ref->unlockDag();
                    dag_ref->release();
                    dag_manager->unlock();
                }
                return XIL_FAILURE;
            }

            if(display_op->execute() == XIL_FAILURE) {
                XIL_ERROR(dst[i]->getSystemState(), XIL_ERROR_SYSTEM,
                          "di-400", FALSE);
                if(lock_dag) {
                    dag_manager->lock();
                    dag_ref->unlockDag();
                    dag_ref->release();
                    dag_manager->unlock();
                }
                return XIL_FAILURE;
            }

            this->destroy();

            release();

            if(lock_dag) {
                dag_manager->lock();
                dag_ref->unlockDag();
                dag_ref->release();
                dag_manager->unlock();
            }

#ifdef OP_DEBUG
    fprintf(stderr, "%02d: %p DONE!\n", XiliThread::self(), this);
#endif            

            return XIL_SUCCESS;
        }
    }

    XilStatus status = execute();    

    //
    //  Unlock the DAG we've used in evaluating the operation and release the
    //  temporary reference we aquired.
    //
    if(lock_dag) {
        dag_manager->lock();
        dag_ref->unlockDag();
        dag_ref->release();
        dag_manager->unlock();
    }

#ifdef OP_DEBUG
    fprintf(stderr, "%02d: %p DONE!\n", XiliThread::self(), this);
#endif            

    return status;
}

//
//  Execute this op.  This may defer the op by inserting it into the
//   XilOp DAG depending upon the op's characteristics.
//
XilStatus
XilOp::execute()
{
    unsigned int i;

    //
    //  Start off assuming that this op will not encounter difficulties which
    //  will require it to be flushed immediately -- unless the nature of op
    //  requires that it be flushed on insertion.  This gives derived
    //  implementations to override whether or not the operation is flushed.
    //
    Xil_boolean  must_flush = flushOnInsert();

    //
    //  If there is more than one destination to this operation, then we must
    //  flush it now.  We don't support deferring multiple destination
    //  operations yet.  It should also be noted that deferred execution only
    //  pays attention to the primary destination image.  So, we only setup
    //  dependencies and such on the primary image.
    //
    if(numDsts > 1) {
        must_flush = TRUE;
    }

    //
    //  We will insert all ops into the DAG immediately here except
    //  for some extreme cases.  In this core, we defer the decision
    //  about whether the operations making up a molecule can be
    //  executed until we've located a molecule.
    //
    
    //
    //  Set our source op links to whatever ops that created our source
    //  images.
    //
    for(i=0; i<numSrcs; i++) {
        srcOp[i] = src[i]->getOp();

        //
        //  If one of our sources has a forwardStartOp set, then we need
        //  to propagate the forwardStartOp to us.
        //
        //  TODO: 3/13/96 jlf  Undo the must_flush
        //
        //    For now, I only let this grow to be the single op after the
        //    capture.  So, we mark things as must_flush so this op is flushed
        //    immediately and the results are pushed from the device through
        //    this op and no further.
        //
        if(srcOp[i] != NULL && srcOp[i]->forwardStartOp != NULL) {
            if(forwardStartOp == NULL) {
                forwardStartOp = srcOp[i]->forwardStartOp;
            }
            must_flush = TRUE;
        }
    }

    //
    //  Add this operation as a dependent of our source operations.  We do
    //  this first so that our dependency on the image is stored before
    //  tossIfNoDependents is called before setting up the destination.
    //
    //  Store our position on the op queue because when we refer to
    //  a source object we need specify which instance we're interested in
    //  when flushing.
    //
    //  Unlike previous versions of XIL we do add ourselves as a dependent to
    //  an object which has this op as its creator.  This occurs for in-place
    //  operations.  We use the queue to manage the dependencies from one
    //  in-place operation to the next instead of special casing in-place
    //  operations. 
    //
    for(i=0; i<numSrcs; i++) {
        srcQueuePos[i] = src[i]->addDependent(this, i);

        //
        //  If a source is a temporary image, mark it as "invalid".  Also,
        //  mark it as "destroy when no dependents".
        //
        if(src[i]->isTemp()) {
            src[i]->setValid(FALSE);
            src[i]->setDestroyWhenNoDependents(TRUE);
        }
    }

    //
    //  If an operation has no destination, then we make it an in-place
    //  operation.  This has the effect of creating a seperate op queue entry
    //  for storing the execution status of this operation.  Otherwise, we
    //  would share it with our source operation which doesn't work because
    //  we'll state that we're in the process of being evaluated and our
    //  source operation will look at the same entry and think that another
    //  thread is currently evaluating the source operation and put itself to
    //  sleep waiting for a non-existent thread to complete.
    //
    //  This has no real effect since the compute routine only references the
    //  source image and the operation -- by definition of being a source-only
    //  operation -- gets flushed immediately.
    //
    if(numDsts == 0) {
        setDst(1, src[0]);
    }

    //
    //  Deferred execution only sets up deferral information on the primary
    //  destination image.  pdst is a local copy of our primary image.
    //
    XilDeferrableObject* pdst = dst[0];

    //
    //  Validate all of the destinations since we're going to write into
    //  them now which by definition makes them valid.
    //
    for(i=0; i<numDsts; i++) {
        dst[i]->setValid(TRUE);
    }

    //
    //  Before setting us as the creator op for our destination images, we
    //  check to see if any of the destinations were generated by another
    //  operation.  If this operation overwrites all of the pixels the 
    //  previous operation touched, then we can toss the previous
    //  operation.
    //
    if(thisOpCoversPreviousOp()) {
        while(pdst->tossIfNoDependents()) {
            if(! thisOpCoversPreviousOp()) {
                break;
            }
        }
    }

    //
    //  Store our position on the op queue so when we want to refer
    //  to the destination object, we can specify to which instance we're
    //  refering.
    //
    //  Also update the destination's version number since writing
    //  into an image changes its contents.
    //
    dstQueuePos[0] = pdst->setOp(this);
    pdst->newVersion();

    //
    //  Check to see if the user has indicated they want the operation flushed
    //  immediately.
    //
    if((pdst->getSynchronized() == TRUE) ||
       (pdst->getSystemState()->getSynchronized() == TRUE) ||
       (pdst->getExported() == XIL_EXPORTED)) {
        must_flush = TRUE;
    }

    //
    //  Now, if we've determined that we're supposed to flush the operation
    //  now, go ahead and do so.
    //
    //  Deleting the op will release the DAG reference so we aquire an extra
    //  reference here which we'll use to unlock the dag.
    //
    XilStatus status  = XIL_SUCCESS;
    if(must_flush == TRUE) {
        //
        //  Indicate that the flush() routine should NOT cleanup this
        //  operation because we need the op to live until after
        //  completeResults() has been called by marking deleteMe to FALSE.
        //
        deleteMe = FALSE;

        //
        //  Flush this operation.
        //
        status = flush();

        if(status == XIL_SUCCESS) {
            //
            //  Once the operation is complete, we call completeResults() for
            //  those operations which need to complete the generation results.
            //
            status = completeResults();
        }

        //
        //  The operation has been evaluated since we've called flush() so go
        //  ahead and delete this op because it's no longer needed since
        //  completeResults() has been called.
        //
        deleteMe = TRUE;

        this->destroy();
    }

    return status;
}

void
XilOp::setOpNumber(XilOpNumber op_num)
{
    //
    //  Reset the operation number.
    //
    opNumber = op_num;
}

//
//   Source and Destination Initialization Functions
//
void
XilOp::setSrc(unsigned int         n,
              XilDeferrableObject* init_src)
{
    src[n-1] = init_src;

    if(n > numSrcs) {
        numSrcs = n;
    }
}

void
XilOp::setDst(unsigned int         n,
              XilDeferrableObject* init_dst)
{
    dst[n-1] = init_dst;

    if(n > numDsts) {
        numDsts = n;
    }
}

//
//  Op Parameter Set Methods
//
void
XilOp::setParam(unsigned int n,
                int          param)
{
    if(n > numParams) {
        numParams = n;
    }

    n--;

    params[n].i = param;
    paramType[n]= XILTYPE_INT;
    paramDestroyMethod[n] = XIL_DONT_DELETE;
}

void
XilOp::setParam(unsigned int n,
                unsigned int param)
{
    if(n > numParams) {
        numParams = n;
    }

    n--;

    params[n].ui = param;
    paramType[n]= XILTYPE_UINT;
    paramDestroyMethod[n] = XIL_DONT_DELETE;
}

void
XilOp::setParam(unsigned int n, 
                XilLongLong  param)
{
    if(n > numParams) {
        numParams = n;
    }

    n--;

    params[n].ll = param;
    paramType[n] = XILTYPE_LONGLONG;
    paramDestroyMethod[n] = XIL_DONT_DELETE;
}

void
XilOp::setParam(unsigned int n,
                float        param)
{
    if(n > numParams) {
        numParams = n;
    }

    n--;

    params[n].f = param;
    paramType[n]= XILTYPE_FLOAT;
    paramDestroyMethod[n] = XIL_DONT_DELETE;
}

void
XilOp::setParam(unsigned int n,
                double       param)
{
    if(n > numParams) {
        numParams = n;
    }

    n--;

    params[n].d  = param;
    paramType[n] = XILTYPE_DOUBLE;
    paramDestroyMethod[n] = XIL_DONT_DELETE;
}

void
XilOp::setParam(unsigned int     n,
                void*            param,
                XilDestroyMethod destroy)
{
    if(n > numParams) {
        numParams = n;
    }

    n--;

    params[n].p = param;
    paramType[n]= XILTYPE_PTR;
    paramDestroyMethod[n] = destroy;
}

void
XilOp::setParam(unsigned int            n,
                XilNonDeferrableObject* param,
                XilDestroyMethod        destroy)
{
    if(n > numParams) {
        numParams = n;
    }

    n--;

    params[n].o           = param;
    paramType[n]          = XILTYPE_OBJECT;
    paramDestroyMethod[n] = destroy;
}

XilRoi*
XilOp::getIntersectedRoi()
{
    if(intersectedRoi.getSystemState() == NULL) {
        intersectedRoi.setSystemState(getSystemState());
    }

    return &intersectedRoi;
}


//--------------------------------------------------------------
//
//  XilOp Public GPI Supported Routines
//
//--------------------------------------------------------------

//
//  As Image
//
XilImage*
XilOp::getSrcImage(unsigned int n) const
{
    return n > numSrcs ? NULL : (XilImage*)src[n-1];
}

XilImage*
XilOp::getDstImage(unsigned int n) const
{
    return n > numDsts ? NULL : (XilImage*)dst[n-1];
}

//
//  As CIS
//
XilCis*
XilOp::getSrcCis(unsigned int n) const
{
    return n > numSrcs ? NULL : (XilCis*)src[n-1];
}

XilCis*
XilOp::getDstCis(unsigned int n) const
{
    return n > numDsts ? NULL : (XilCis*)dst[n-1];
}

//
//  Miscellaneous Op Aquisition Functions
//
XilOp**
XilOp::getOpList()
{
    if(opList == NULL) {
        opList = new XilOp*;

        if(opList == NULL) {
            XIL_ERROR(getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
            return NULL;
        }

        opList[0] = this;
    }

    return opList;
}

unsigned int
XilOp::getOpNumber() const
{
    return opNumber;
}

void*
XilOp::getPreprocessData(XilDeviceManagerCompute* compute_device,
                         unsigned int             func_ident)
{
    funcDataListMutex.lock();
    
    XiliFuncData* tmpxfd = funcDataList;

    while(tmpxfd != NULL) {
        //
        //  Search for the given compute device...
        //
        if(tmpxfd->funcDef->deviceManager == compute_device &&
           tmpxfd->funcId                 == func_ident) {

	    funcDataListMutex.unlock();
	    
            return tmpxfd->funcInfo;
        }

        tmpxfd = tmpxfd->next;
    }
    
    funcDataListMutex.unlock();
	    
    return NULL;
}

void*
XilOp::getPreprocessData(XilDeviceIO* io_device,
                         unsigned int func_ident)
{
    funcDataListMutex.lock();
    
    XiliFuncData* tmpxfd = funcDataList;

    while(tmpxfd != NULL) {
        //
        //  Search for the given compute device...
        //
        if(tmpxfd->ioDevice == io_device &&
           tmpxfd->funcId   == func_ident) {

	    funcDataListMutex.unlock();
	    
            return tmpxfd->funcInfo;
        }

        tmpxfd = tmpxfd->next;
    }
    
    funcDataListMutex.unlock();
	    
    return NULL;
}

void*
XilOp::getPreprocessData(XilDeviceCompression* codec_device,
                         unsigned int          func_ident)
{
    funcDataListMutex.lock();
    
    XiliFuncData* tmpxfd = funcDataList;

    while(tmpxfd != NULL) {
        //
        //  Search for the given compute device...
        //
        if(tmpxfd->codecDevice == codec_device &&
           tmpxfd->funcId      == func_ident) {

	    funcDataListMutex.unlock();
	    
            return tmpxfd->funcInfo;
        }

        tmpxfd = tmpxfd->next;
    }
    
    funcDataListMutex.unlock();
	    
    return NULL;
}

void
XilOp::callPostprocessRoutines()
{
    //
    //  If a compute routine had a preprocess function, then now that we know
    //  this operation is complete, we may need to call a postprocess function.
    //
    while(funcDataList != NULL) {
        XiliFunctionDef* fdef = funcDataList->funcDef;

        //
        //  Call the appropriate function based on the type...
        //
        switch(fdef->functionType) {
          case XILI_COMPUTE_FUNC:
            {
                XilComputePostprocessFunctionPtr devMem = 
                    fdef->funcInfo.getComputePostFunction();
                if(devMem != NULL) {
                    TNF_PROBE_1(call_postprocess, 
                                "pre_post_process", 
                                "call_postprocess",
                                tnf_opaque, 
                                "op", 
                                this);

                    // Porting note:  HP CC found original, with "devMem" 
                    // expanded in place, too difficult (HP CC warn 2046
                    // and 2047).  Same comment in the other uses of devMem
                    // and ioDev below
                    if((((XilDeviceManagerCompute*)fdef->deviceManager)->*(devMem))(this, funcDataList->funcInfo) == XIL_FAILURE) {
                        //
                        //  TODO:  1/27/96 jlf  Do we really want to generate an error?
                        //
                    }
                }
            }
          break;

          case XILI_IO_FUNC:
            {
                XilIOPostprocessFunctionPtr devMem = 
                            fdef->funcInfo.getIOPostFunction();
                if(devMem != NULL) {
                    TNF_PROBE_1(call_io_postprocess, 
                                "pre_post_process", 
                                "call_io_postprocess",
                                tnf_opaque, 
                                "op", 
                                this);

                    if(src[0] != NULL && src[0]->getDeviceIO() != NULL) {
                        XilDeviceIO* ioDev = src[0]->getDeviceIO();
                        ((ioDev)->*(devMem))(this, funcDataList->funcInfo);
                    } else if(dst[0] != NULL && dst[0]->getDeviceIO() != NULL) {
                        XilDeviceIO* ioDev = dst[0]->getDeviceIO();
                        ((ioDev)->*(devMem))(this, funcDataList->funcInfo);
                    }
                }
            }
          break;

          case XILI_CODEC_FUNC:
            {
                XilCodecPostprocessFunctionPtr devMem = 
                                fdef->funcInfo.getCodecPostFunction();
                if(devMem != NULL) {
                    TNF_PROBE_1(call_codec_postprocess, 
                                "pre_post_process", 
                                "call_codec_postprocess",
                                tnf_opaque, 
                                "op",
                                this);

                    if(src[0] != NULL &&
                           ((XilCis*)src[0])->getDeviceCompression() != NULL) {
                        XilDeviceCompression* comDev = 
                            ((XilCis*)src[0])->getDeviceCompression();
                        ((comDev)->*(devMem))(this, funcDataList->funcInfo);
                    } else if(dst[0] != NULL &&
                          ((XilCis*)dst[0])->getDeviceCompression() != NULL) {
                        XilDeviceCompression* comDev =
                            ((XilCis*)dst[0])->getDeviceCompression();
                        ((comDev)->*(devMem))(this, funcDataList->funcInfo);
                    }
                }
            }
          break;
        }
            
        //
        //  Clean up the list.
        //
        XiliFuncData* tmp = funcDataList->next;
        delete funcDataList;
        funcDataList = tmp;
    }
}

//
//   Op Parameter Aquisition Functions
//
void
XilOp::getParam(unsigned int n,
                int*        param)
{
    *param = params[n-1].i;
}

void
XilOp::getParam(unsigned int  n,
                unsigned int* param)
{
    *param = params[n-1].ui;
}

void
XilOp::getParam(unsigned int n, 
                XilLongLong* param)
{
    *param = params[n-1].ll;
}

void
XilOp::getParam(unsigned int n,
                float*       param)
{
    *param = params[n-1].f;
}

void
XilOp::getParam(unsigned int n,
                double*      param)
{
    *param = params[n-1].d;
}

void
XilOp::getParam(unsigned int n,
                void**       param)
{
    *param = params[n-1].p;
}

void
XilOp::getParam(unsigned int n,
                XilObject**  param)
{
    *param = params[n-1].o;
}
    
//--------------------------------------------------------------
//
//  XilOp DERIVED Routines
//
//--------------------------------------------------------------

Xil_boolean
XilOp::isInPlace() const
{
    int src_count = numSrcs;
    int dst_count = numDsts;

    while(--dst_count >= 0) {
        while(--src_count >= 0) {
            if(src[src_count] == dst[dst_count]) {
                return TRUE;
            }
        }
    }

    return FALSE;
}

Xil_boolean
XilOp::isCommutative() const
{
    return FALSE;
}

Xil_boolean
XilOp::isIOOperation(XiliOpIOType)
{
    return FALSE;
}

Xil_boolean
XilOp::canBeSplit()
{
    return TRUE;
}

Xil_boolean
XilOp::flushOnInsert() const
{
    return FALSE;
}

//------------------------------------------------------------------------
//
//  Function:	getSrcGlobalSpaceRoi()/getDstGlobalSpaceRoi()
//
//  Description:
//	Gets the global space ROI for the specified source or
//      destination.  It may not be what is on the deferrable object 
//	when molecules are concered.  It may be the intersected ROI
//      from another operation.
//	
//------------------------------------------------------------------------
XilRoi*
XilOp::getSrcGlobalSpaceRoi(unsigned int src_num)
{
    //
    //  If we're not part of a molecule, and we don't have an operation from
    //  the requested source writing into us, and the molecule it's a part of
    //  isn't the same as the one we're a part of, then we use what's in the
    //  deferrable object.  Otherwise, we get the ROI from the operation above
    //  us.
    //
    if(moleculeBottom == NULL ||
       srcOp[src_num] == NULL ||
       moleculeBottom != srcOp[src_num]->moleculeBottom) {
        return src[src_num]->getGlobalSpaceRoi();
    } else {
        return &srcOp[src_num]->intersectedRoi;
    }
}

XilRoi*
XilOp::getDstGlobalSpaceRoi(unsigned int dst_num)
{
    //
    //  For now, they're the same.  We always respond with the ROI of the
    //  primary desination image.
    //
    return dst[dst_num]->getGlobalSpaceRoi();
}

XilStatus
XilOp::generateIntersectedRoi()
{
    if(numDsts == 0) {
        //
        //  Intersect the sources instead - just one src and put it in
        //  intersectedRoi 
        //
        if(getIntersectedRoi() == NULL) {
            return XIL_FAILURE;
        }

        XilRoi* src_roi = getSrcGlobalSpaceRoi(0);
        if(src_roi == NULL) {
            XIL_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-401", FALSE);
            return XIL_FAILURE;
        }

        //
        //  Copy the 0th source ROI into the intersectedRoi.
        //
        intersectedRoi = *src_roi;

        //
        //  Loop through additional srcs, intersecting i_roi and src, put
        //  results in i_roi.
        //
        for(unsigned int i=1; i<numSrcs; i++) {
            src_roi = getSrcGlobalSpaceRoi(i);

            if(src_roi == NULL) {
                XIL_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-401", FALSE);
                return XIL_FAILURE;
            }

            if(src_roi->intersect_inplace(&intersectedRoi) == XIL_FAILURE) {
                return XIL_FAILURE;
            }
        }
    } else {
        if(getIntersectedRoi() == NULL) {
            return XIL_FAILURE;
        }

        XilRoi* dst_roi = getDstGlobalSpaceRoi();
        if(dst_roi == NULL) {
            XIL_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-402", FALSE);
            return XIL_FAILURE;
        }

        if(numSrcs == 0) {
            //
            //  Set the intersected roi to that of the dst globalSpaceRoi
            //
            intersectedRoi = *(dst_roi);
        } else {
            XilRoi* src_roi = getSrcGlobalSpaceRoi(0);
            if(src_roi == NULL) {
                XIL_ERROR(getSystemState(), XIL_ERROR_INTERNAL,
                          "di-401", FALSE);
                return XIL_FAILURE;
            }

            //
            //  Intersect the dest and src[0] image and place results in
            //  i_roi. 
            //
            dst_roi->intersect(src_roi, &intersectedRoi);

            //
            // Loop through additional srcs, intersecting i_roi and src, put
            // results in i_roi 
            //
            for(unsigned int i=1; i<numSrcs; i++) {
                src_roi = getSrcGlobalSpaceRoi(i);
                if(src_roi == NULL) {
                    XIL_ERROR(getSystemState(), XIL_ERROR_INTERNAL,
                              "di-401", FALSE);
                    return XIL_FAILURE;
                }
                    
                src_roi->intersect_inplace(&intersectedRoi);
            }
        }
    }

    //
    //  The newly created intersectedRoi is in global space and
    //  must be tagged as such.  The intersectedRoi can be created
    //  more than once within the life of the op, so setting the 
    //  flag in the XilOp constructor is not sufficient.
    //
    intersectedRoiIsInObjectSpace = FALSE;
    return XIL_SUCCESS;
}

//------------------------------------------------------------------------
//
//  Function:	generateBoxList()
//
//  Description:
//	Take a box that is valid in the destination space (usurally the
//	bbox of the intersectedRoi for the destination image).  It's a
//	box in global space.
//	
//	In the case of a single operation (no molecule), we just backward
//	map the box into each of the op's source images.
//
//	When there is a molecule, we backward map up the chain to the 
//	first operation and then backward map into each source.
//
//------------------------------------------------------------------------
XilStatus
XilOp::generateBoxList(XilBoxList* box_list,
                       XiliRect*   dst_rect)
{
    //
    //  Get the box list entry we'll fill.
    //
    XiliBoxListEntry* ble = new XiliBoxListEntry;
    if(ble == NULL) {
        XIL_ERROR(getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }
    XilBox* b = ble->boxes;

    //
    //  Our working rect.
    //
    XiliRect* src_rect = dst_rect->constructNew();
    if(src_rect == NULL) {
        delete ble;
        XIL_ERROR(getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }

    unsigned int i;
    unsigned int j = 0;
    if(opList == NULL) {
        //
        //  Single-operation case.  Backward map into each of this op's source
        //  images. 
        //
        for(i=0; i<numSrcs; i++) {
            if(gsBackwardMap(dst_rect, src_rect, i) == XIL_FAILURE) {
                //
                //  A failure here means the destination rect backward mapped
                //  to something outside the source and clips to nothing when
                //  moved into object space.  So, we return XIL_SUCCESS but we
                //  set the given box to an empty box.  The caller can test
                //  the box being empty to detect this case.  By doing this it
                //  means one can detect the difference between an
                //  unrecoverable failure (returning XIL_FAILURE) and the
                //  operation not needing to be executed.
                //
                dst_rect->set(0, 0, -1, -1);
                src_rect->destroy();
                delete ble;

                return XIL_SUCCESS;
            }

            if((moveIntoObjectSpace(src_rect, src[i])  == XIL_FAILURE) ||
               (setBoxStorage(src_rect, src[i], &b[j]) == XIL_FAILURE)) {
                //
                //  A failure here means the box clips to nothing when moved
                //  into object space.  So, we return XIL_SUCCESS but we set
                //  the given box to an empty box.  The caller can test the
                //  box being empty to detect this case.  By doing this it
                //  means one can detect the difference between an
                //  unrecoverable failure (returning XIL_FAILURE) and the
                //  operation not needing to be executed.
                //
                dst_rect->set(0, 0, -1, -1);
                src_rect->destroy();
                delete ble;

                return XIL_SUCCESS;
            }

            j++;
        }

        //
        //  Finalize with setting each destination's box.
        //
        for(i=0; i<numDsts; i++) {
            if((moveIntoObjectSpace(dst_rect, dst[i])  == XIL_FAILURE) ||
               (setBoxStorage(dst_rect, dst[i], &b[j]) == XIL_FAILURE)) {
                //
                //  A failure here means the box clips to nothing when moved
                //  into object space.  So, we return XIL_SUCCESS but we set
                //  the given box to an empty box.  The caller can test the
                //  box being empty to detect this case.  By doing this it
                //  means one can detect the difference between an
                //  unrecoverable failure (returning XIL_FAILURE) and the
                //  operation not needing to be executed.
                //
                dst_rect->set(0, 0, -1, -1);
                src_rect->destroy();
                delete ble;

                return XIL_SUCCESS;
            }

            j++;
        }
    } else {
        //
        //  TODO: 5/1/96 jlf  Deal with operations that have no destinations.
        //
        
        //
        //  It's a molecule.  Traverse up the chain of operations and backward
        //  map from destination to the appropriate source.  If a backward map
        //  causes the storage area to grow, then we reset the pixel
        //  coordinates of box that we're backward mapping into so the source
        //  storage box becomes the next op's destination box.
        //

        //
        //  Our working rect.
        //  This must be an XiliRect* instead of an XiliRectInt on the stack because
        //  if a geometric operation is part of the molecule chain, the rect may have
        //  to hold a fractional part after the backward-mapping. This fractional part
        //  will need to be maintained until after the Op specific setBoxStorage call.
        //
        XiliRect* base_rect = dst_rect->createCopy();
        if(base_rect == NULL) {
            delete ble;
            XIL_ERROR(getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
            return XIL_FAILURE;
        }


        XilBox       tmp_box;
        unsigned int top_op_index = funcLength - 1;
        for(i=0; i<top_op_index; i++) {
            //
            //  When generating the box list, skip I/O operations because the
            //  image that we're really interested in is on the operation
            //  immediately before or after the I/O operation.  By starting
            //  with the I/O operation's box, we'll get the incorrect results
            //  because it will be the controlling image.
            //
            if(opList[i]->isIOOperation()) {
                continue;
            }

            //
            //  Backward map the rect for each op until we're at the
            //  destination image of the top op (which is the source of the
            //  n-1 op).
            //
            if(opList[i]->gsBackwardMap(base_rect, base_rect, 1) == XIL_FAILURE) {
                //
                //  A failure here means the destination rect backward mapped
                //  to something outside the source and clips to nothing when
                //  moved into object space.  So, we return XIL_SUCCESS but we
                //  set the given box to an empty box.  The caller can test
                //  the box being empty to detect this case.  By doing this it
                //  means one can detect the difference between an
                //  unrecoverable failure (returning XIL_FAILURE) and the
                //  operation not needing to be executed.
                //
                dst_rect->set(0, 0, -1, -1);
                src_rect->destroy();
                base_rect->destroy();
                delete ble;

                return XIL_SUCCESS;
            }

            //
            //  Move the base_rect into object space for setting storage.
            //
            if(opList[i]->moveIntoObjectSpace(base_rect,
                                              opList[i]->src[0]) == XIL_FAILURE) {
                //
                //  A failure here means the box clips to nothing when moved
                //  into object space.  So, we return XIL_SUCCESS but we set
                //  the given box to an empty box.  The caller can test the
                //  box being empty to detect this case.  By doing this it
                //  means one can detect the difference between an
                //  unrecoverable failure (returning XIL_FAILURE) and the
                //  operation not needing to be executed.
                //
                dst_rect->set(0, 0, -1, -1);
                src_rect->destroy();
                base_rect->destroy();
                delete ble;

                return XIL_SUCCESS;
            }

            //
            //  Have the op set the storage for the source box and use the
            //  result as the starting point for the next operation.
            //
            if(opList[i]->setBoxStorage(base_rect,
                                        opList[i]->src[0],
                                        &tmp_box) == XIL_FAILURE) {
                //
                //  A failure here means the box clips to nothing when moved
                //  into object space.  So, we return XIL_SUCCESS but we set
                //  the given box to an empty box.  The caller can test the
                //  box being empty to detect this case.  By doing this it
                //  means one can detect the difference between an
                //  unrecoverable failure (returning XIL_FAILURE) and the
                //  operation not needing to be executed.
                //
                dst_rect->set(0, 0, -1, -1);
                src_rect->destroy();
                base_rect->destroy();
                delete ble;

                return XIL_SUCCESS;
            }

            //
            //  If specific storage information has been set (i.e. the storage
            //  information is different than the front box), then we update
            //  the rect to represent the storage area.  We only want this if
            //  the storage was modified for a parent image -- otherwise,
            //  we're taking child offsets into account multiple times.
            //
            //  TODO: 11/12/96 jlf  Remove cast to XilImage* somehow...
            //
            if(tmp_box.isStorageSet() &&
               ((XilImage*)opList[i]->src[0])->getParent() == NULL) {
                int          x;
                int          y;
                unsigned int xsize;
                unsigned int ysize;
                int          band;

                tmp_box.getStorageLocation(&x, &y, &xsize, &ysize, &band);

                base_rect->set(x, y, (x + xsize - 1), (y + ysize - 1));
            }

            //
            //  Move the base_rect back into global space for remaining
            //  mappings and setting up box list. 
            //
            if(opList[i]->moveIntoGlobalSpace(base_rect,
                                              opList[i]->src[0]) == XIL_FAILURE) {
                //
                //  A failure here means the rect clips to nothing when moved
                //  into global space somehow.  So, we return XIL_SUCCESS but
                //  we set the given box to an empty box.  The caller can test
                //  the box being empty to detect this case.  By doing this it
                //  means one can detect the difference between an
                //  unrecoverable failure (returning XIL_FAILURE) and the
                //  operation not needing to be executed.
                //
                dst_rect->set(0, 0, -1, -1);
                src_rect->destroy();
                base_rect->destroy();
                delete ble;

                return XIL_SUCCESS;
            }
        }
        XilOp* top_op = opList[top_op_index];

        //
        //  Now, backward map the base_rect from the destination to each of
        //  the sources in the top op -- much like single op case.
        //
        for(i=0; i<top_op->numSrcs; i++) {
            if(top_op->gsBackwardMap(base_rect, src_rect, i) == XIL_FAILURE) {
                //
                //  A failure here means the destination rect backward mapped
                //  to something outside the source and clips to nothing when
                //  moved into object space.  So, we return XIL_SUCCESS but we
                //  set the given box to an empty box.  The caller can test
                //  the box being empty to detect this case.  By doing this it
                //  means one can detect the difference between an
                //  unrecoverable failure (returning XIL_FAILURE) and the
                //  operation not needing to be executed.
                //
                dst_rect->set(0, 0, -1, -1);
                src_rect->destroy();
                base_rect->destroy();
                delete ble;

                return XIL_SUCCESS;
            }

            if((top_op->moveIntoObjectSpace(src_rect, top_op->src[i])  == XIL_FAILURE) ||
               (top_op->setBoxStorage(src_rect, top_op->src[i], &b[j]) == XIL_FAILURE)) {
                //
                //  A failure here means the box clips to nothing when moved
                //  into object space.  So, we return XIL_SUCCESS but we set
                //  the given box to an empty box.  The caller can test the
                //  box being empty to detect this case.  By doing this it
                //  means one can detect the difference between an
                //  unrecoverable failure (returning XIL_FAILURE) and the
                //  operation not needing to be executed.
                //
                dst_rect->set(0, 0, -1, -1);
                src_rect->destroy();
                base_rect->destroy();
                delete ble;

                return XIL_SUCCESS;
            }

            j++;
        }

        //
        //  Reset the number of sources on the box list to be equal to the
        //  number of sources on the top operation.
        //
        box_list->setNumSrcs(top_op->numSrcs);

        //
        //  Finalize with setting each destination's box.
        //
        for(i=0; i<numDsts; i++) {
            if((moveIntoObjectSpace(dst_rect, dst[i])  == XIL_FAILURE) ||
               (setBoxStorage(dst_rect, dst[i], &b[j]) == XIL_FAILURE)) {
                //
                //  A failure here means the box clips to nothing when moved
                //  into object space.  So, we return XIL_SUCCESS but we set
                //  the given box to an empty box.  The caller can test the
                //  box being empty to detect this case.  By doing this it
                //  means one can detect the difference between an
                //  unrecoverable failure (returning XIL_FAILURE) and the
                //  operation not needing to be executed.
                //
                dst_rect->set(0, 0, -1, -1);
                src_rect->destroy();
                base_rect->destroy();
                delete ble;

                return XIL_SUCCESS;
            }

            j++;
        }
        
        //
        //  Clean up the created working rect
        //
        base_rect->destroy();
    }

    //
    //  TODO: 4/18/96 jlf  Handle > 4 boxes
    //
    //    For multi branch molecules, we'll need to handle more than 4 boxes.
    //    In addition, > 4 boxes is needed if we go beyond 3 sources and 1
    //    destination.
    //
    if(box_list->addEntry(ble) == XIL_FAILURE) {
        src_rect->destroy();
        delete ble;
        XIL_ERROR(getSystemState(), XIL_ERROR_INTERNAL, "di-403", FALSE);

        return XIL_FAILURE;
    }

    src_rect->destroy();

    return XIL_SUCCESS;
}

//------------------------------------------------------------------------
//
//  Function:	moveIntoGlobalSpace()
//
//  Description:
//	Moves the given box or ROI into global space.  By default this 
//      means asking the object to convert from object space to global
//	space.
//
//------------------------------------------------------------------------
XilStatus
XilOp::moveIntoGlobalSpace(XiliRect*            rect,
                           XilDeferrableObject* object)
{
    return object->convertToGlobalSpace(rect);
}

XilStatus
XilOp::moveIntoGlobalSpace(XilRoi*              roi,
                           XilDeferrableObject* object)
{
    return object->convertToGlobalSpace(roi);
}

//------------------------------------------------------------------------
//
//  Function:	moveIntoObjectSpace()
//
//  Description:
//	Moves the given box or ROI into object space.  By default this 
//      means asking the object to convert from global space to object
//	space.
//
//------------------------------------------------------------------------
XilStatus
XilOp::moveIntoObjectSpace(XiliRect*            rect,
                           XilDeferrableObject* object)
{
    return object->convertToObjectSpace(rect);
}

XilStatus
XilOp::moveIntoObjectSpace(XilRoi*              roi,
                           XilDeferrableObject* object)
{
    return object->convertToObjectSpace(roi);
}

//------------------------------------------------------------------------
//
//  Function:	setBoxStorage()
//
//  Description:
//      Sets the storage information on a box for the given rect and
//      object.
//
//      By default, we copy the coordinates from the rect into the box
//      and then request the object to update the box's storage
//      information.
//
//------------------------------------------------------------------------
XilStatus
XilOp::setBoxStorage(XiliRect*            rect,
                     XilDeferrableObject* object,
                     XilBox*              box)
{
    *box = *rect;

    if(object->setBoxStorage(box) == XIL_FAILURE) {
        return XIL_FAILURE;
    }

    return XIL_SUCCESS;
}

//------------------------------------------------------------------------
//
//  Function:	clipToTile()
//
//  Description:
//      Clips the given rect (which is expected to be valid in the given
//      object's image space) to tile number in the specified object.
//	
//------------------------------------------------------------------------
XilStatus
XilOp::clipToTile(XilDeferrableObject* defobj,
                  XilTileNumber        tile_num,
                  XiliRect*            rect)
{
    if ((defobj->clipToTile(tile_num, rect)) == XIL_SUCCESS) {
       return XIL_SUCCESS;
    } else {
       return XIL_FAILURE;
    }
}

//------------------------------------------------------------------------
//
//  Function:	readjustBoxStorage()
//
//  Description:
//      Used exclusively by I/O operations to adjust the given rect
//      back to their source or destination operation's space.
//	
//------------------------------------------------------------------------
XilStatus
XilOp::readjustBoxStorage(XiliRect*)
{
    return XIL_SUCCESS;
}

//------------------------------------------------------------------------
//
//  Function:	splitOnTileBoundaries()
//
//  Description:
//	Split the given box list on source tile boundaries.
//	
//  MT-level:  safe
//	
//------------------------------------------------------------------------
XilStatus
XilOp::splitOnTileBoundaries(XilBoxList* boxlist)
{
    if(opList != NULL) {
        //
        //  Setup variables that point at the op at the "top" of the molecule
        //  (the source operation).
        //
        unsigned int top_op_index = funcLength - 1;
        XilOp*       top_op       = opList[top_op_index];
        unsigned int i;

        //
        //  Quickly check to see whether any of the source images to the
        //  molecule has tiles.  If not, return immediately.
        //
        {
            Xil_boolean has_tiles = FALSE;
            for(i=0; i<top_op->numSrcs; i++) {
                if(top_op->src[i]->getNumTiles() != 1) {
                    has_tiles = TRUE;
                    break;
                }
            }

            if(! has_tiles) {
                return XIL_SUCCESS;
            }
        }

        //
        //  TODO: 5/1/96 jlf  Deal with operations that have no destinations.
        //
        
        //
        //  It's a molecule.  Traverse up the chain of operations and backward
        //  map from destination to the appropriate source.  If a backward map
        //  causes the storage area to grow, then we reset the pixel
        //  coordinates of box that we're backward mapping into so the source
        //  storage box becomes the next op's destination box.
        //

        //
        //  Check to see that there is only one entry on the list.
        //
        XiliSLList<XiliBoxListEntry*>* list = boxlist->getList();
        if(list->length() != 1) {
            return XIL_FAILURE;
        }

        //
        //  Get the destination box that we'll backward map.
        //
        XiliBoxListEntry* blentry = list->reference(list->head());
        if(blentry == NULL) {
            return XIL_FAILURE;
        }

        //
        //  Construct a rect that represents the box.
        //
        XiliRectInt  base_rect(&blentry->boxes[boxlist->getNumSrcs()]);

        //
        //  Now, move the box into global space for backward mapping.
        //
        if(this->moveIntoGlobalSpace(&base_rect, dst[0]) == XIL_FAILURE) {
            return XIL_FAILURE;
        }

        XilBox tmp_box;
        for(i=0; i<top_op_index; i++) {
            //
            //  When moving back to the source op, skip I/O operations because
            //  the image that we're really interested in is on the operation
            //  immediately before or after the I/O operation.  By starting
            //  with the I/O operation's box, we'll get the incorrect results
            //  because it will be the controlling image.
            //
            if(opList[i]->isIOOperation()) {
                //
                //  Here, we call an I/O operation specific call to
                //  potentially undo adjustments that were made due to moving
                //  the operation's storage box with respect to the
                //  controlling image versus the preceeding operation's
                //  image.
                //
                if(opList[i]->readjustBoxStorage(&base_rect) == XIL_FAILURE) {
                    return XIL_FAILURE;
                }

                continue;
            }

            //
            //  Backward map the rect for each op until we're at the
            //  destination image of the top op (which is the source of the
            //  n-1 op).
            //
            if(opList[i]->gsBackwardMap(&base_rect,
                                        &base_rect, 1) == XIL_FAILURE) {
                return XIL_FAILURE;
            }

            //
            //  Move the base_rect into object space for setting storage.
            //
            if(opList[i]->moveIntoObjectSpace(&base_rect,
                                              opList[i]->src[0]) == XIL_FAILURE) {
                return XIL_FAILURE;
            }

            //
            //  Have the op set the storage for the source box and use the
            //  result as the starting point for the next operation.
            //
            if(opList[i]->setBoxStorage(&base_rect,
                                        opList[i]->src[0],
                                        &tmp_box) == XIL_FAILURE) {
                return XIL_FAILURE;
            }

            //
            //  If specific storage information has been set (i.e. the storage
            //  information is different than the front box), then we update
            //  the rect to represent the storage area.  We only want this if
            //  the storage was modified for a parent image -- otherwise,
            //  we're taking child offsets into account multiple times.
            //
            //  TODO: 11/12/96 jlf  Remove cast to XilImage* somehow...
            //
            if(tmp_box.isStorageSet() &&
               ((XilImage*)opList[i]->src[0])->getParent() == NULL) {
                int          x;
                int          y;
                unsigned int xsize;
                unsigned int ysize;
                int          band;

                tmp_box.getStorageLocation(&x, &y, &xsize, &ysize, &band);

                base_rect.set(x, y, (x + xsize - 1), (y + ysize - 1));
            }

            //
            //  Move the base_rect back into global space for remaining
            //  mappings and setting up box list. 
            //
            if(opList[i]->moveIntoGlobalSpace(&base_rect,
                                              opList[i]->src[0]) == XIL_FAILURE) {
                return XIL_FAILURE;
            }
        }

        //
        //  Now, base_rect is the global-space representation of the area in
        //  the top op's destination that needs to be processed.  We'll call
        //  vSplitOnTileBoundaries() on the top op with a modified box list to
        //  get a true split and then forward map all of the boxes back into
        //  the final destination. 
        //
        if(top_op->isIOOperation()) {
            //
            //  Here, we call an I/O operation specific call to
            //  potentially undo adjustments that were made due to moving
            //  the operation's storage box with respect to the
            //  controlling image versus the preceeding operation's
            //  image.
            //
            if(top_op->readjustBoxStorage(&base_rect) == XIL_FAILURE) {
                return XIL_FAILURE;
            }
        }

        //
        //  First, move the base_rect into object space.
        //
        if(top_op->moveIntoObjectSpace(&base_rect,
                                       top_op->dst[0]) == XIL_FAILURE) {
            return XIL_FAILURE;
        }

        //
        //  Set the storage information on the top op's destination.
        //
        if(top_op->setBoxStorage(&base_rect,
                                 top_op->dst[0],
                                 &tmp_box) == XIL_FAILURE) {
            return XIL_FAILURE;
        }

        //
        //  Set the destination box on the box list to be the tmp_box (the
        //  box backward mapped to the top op's destination).
        //
        blentry->boxes[top_op->numSrcs] = tmp_box;

        //
        //  Now, call the virtual splitOnTileBoundaries() on the source op to
        //  split the box list.
        //
        if(top_op->vSplitOnTileBoundaries(boxlist) == XIL_FAILURE) {
            return XIL_FAILURE;
        }

        //
        //  Finalize by forward mapping all of the destination boxes (that
        //  have been split) into the final destination.
        //
        XiliSLListIterator<XiliBoxListEntry*> bl_iterator(list);
        XiliBoxListEntry*                     ble;
        unsigned int                          dst_index = top_op->numSrcs;
        while(bl_iterator.getNext(ble) == XIL_SUCCESS) {
            //
            //  Construct a rect to represent the box for forward mapping.
            //
            XiliRectInt tmp_rect(&ble->boxes[dst_index]);

            //
            //  Move the rect into global space.
            //
            if(top_op->moveIntoGlobalSpace(&tmp_rect,
                                           top_op->dst[0]) == XIL_FAILURE) {
                return XIL_FAILURE;
            }

            //
            //  Forward map the rect for each op until we're back at the
            //  bottom op (this one).
            //
            for(i=top_op_index - 1; i!=0; i--) {
                opList[i]->gsForwardMap(&tmp_rect, 1, &tmp_rect);
            }

            //
            //  Move the resultant rect into destination object space.
            //
            if(this->moveIntoObjectSpace(&tmp_rect, dst[0]) == XIL_FAILURE) {
                return XIL_FAILURE;
            }

            //
            //  Set the storage information for the destination image.
            //
            if(this->setBoxStorage(&tmp_rect,
                                   dst[0],
                                   &ble->boxes[dst_index]) == XIL_FAILURE) {
                return XIL_FAILURE;
            }
        }

        return XIL_SUCCESS;
    } else {
        return vSplitOnTileBoundaries(boxlist);
    }
}

//
//  This is the default implementation of divideBoxList
//  it is special cased for things like geometric operations
//  where the division needs to take care of backward and
//  forward mapping of source boxes.
//  The default implementation assumes that the same split
//  in source is suitable for splitting in the dest.
//
Xil_boolean
XilOp::divideBoxList(XilBoxList*   boxlist,
		     unsigned int  box_number,
		     unsigned int  tile_xdelta,
		     unsigned int  tile_ydelta)
{
    //
    //  "real" corners for box list entry.
    //
    int box_x1, box_x2, box_y1, box_y2;

    //
    //  storage corners for box list entry.
    //
    int box_x1s, box_x2s, box_y1s, box_y2s, box_band;

    //
    //  "real" corners for active box.
    //
    int abox_x1, abox_x2, abox_y1, abox_y2;

    //
    //  storage corners for active box.
    //
    int abox_x1s, abox_x2s, abox_y1s, abox_y2s, abox_band;

    //
    //  TODO: 12/18/95 maynard  if(box_number > ble->boxCount) return FALSE;
    //
    //        Not checking initial argument, because I don't want to do
    //        it in the loop.
    //
    XiliSLList<XiliBoxListEntry*>*        list = boxlist->getList();
    XiliSLListIterator<XiliBoxListEntry*> bl_iterator(list);
    XiliBoxListEntry*                     ble;
    while(bl_iterator.getNext(ble) == XIL_SUCCESS) {
        //
        //  Get the corresponding box from the entry we're going to split.
        //
        XilBox* active_box = &ble->boxes[box_number];
        
        //
        //  Get both the "real" corners and the storage corners.  We're doing
        //  the splitting based on the storage corners since the tile boundaries
        //  are with respect to the storage. 
        //
        active_box->getAsCorners(&abox_x1, &abox_y1,
                                 &abox_x2, &abox_y2);
        active_box->getStorageAsCorners(&abox_x1s, &abox_y1s,
                                        &abox_x2s, &abox_y2s, &abox_band);

        //
        //  Does a tile boundary split this box in X?
        //
        unsigned int tile_count     = abox_x1s/tile_xdelta;
        unsigned int tile_xboundary = (tile_xdelta*(tile_count+1) - 1);

        if((unsigned int)abox_x2s > tile_xboundary) {
            //
            //  Split on X-boundary
            //
            unsigned int      delta_x = tile_xboundary - abox_x1s;

            XiliBoxListEntry* new_ble = new XiliBoxListEntry;

            new_ble->boxCount = ble->boxCount;

            for(unsigned int i=0; i<ble->boxCount; i++) {
                ble->boxes[i].getAsCorners(&box_x1, &box_y1, &box_x2, &box_y2);
                ble->boxes[i].getStorageAsCorners(&box_x1s, &box_y1s,
                                                  &box_x2s, &box_y2s,
                                                  &box_band);

                new_ble->boxes[i].setAsCorners(box_x1 + delta_x + 1,
                                               box_y1,
                                               box_x2,
                                               box_y2);
                new_ble->boxes[i].setStorageAsCorners(box_x1s + delta_x + 1,
                                                      box_y1s,
                                                      box_x2s,
                                                      box_y2s,
                                                      box_band);

                ble->boxes[i].setAsCorners(box_x1,
                                           box_y1,
                                           box_x1 + delta_x,
                                           box_y2);
                ble->boxes[i].setStorageAsCorners(box_x1s,
                                                  box_y1s,
                                                  box_x1s + delta_x,
                                                  box_y2s,
                                                  box_band);
            }

            if(list->insertAfter(new_ble,
                                 bl_iterator.getCurrentPosition()) == _XILI_SLLIST_INVALID_POSITION) {
                //
                //  Oddly enough, insertion failed.
                //
                //  TODO: 2/26/96 jlf  Generate secondary failure?
                //
                return FALSE;
            }
        }

        //
        //  Does a tile boundary split this box in Y?
        //
        unsigned int tile_yboundary;

        tile_count     = abox_y1s/tile_ydelta;
        tile_yboundary = (tile_ydelta*(tile_count+1) - 1);

        while((unsigned int)abox_y2s > tile_yboundary) {
            //
            //  Split the created box along possible y's
            //
            unsigned int      delta_y = tile_yboundary - abox_y1s;
          
            XiliBoxListEntry* new_ble = new XiliBoxListEntry;
          
            new_ble->boxCount = ble->boxCount;

            for(unsigned int i=0; i<ble->boxCount; i++) {
                ble->boxes[i].getAsCorners(&box_x1,&box_y1,&box_x2,&box_y2);
                ble->boxes[i].getStorageAsCorners(&box_x1s, &box_y1s,
                                                  &box_x2s, &box_y2s,
                                                  &box_band);

                new_ble->boxes[i].setAsCorners(box_x1,
                                               box_y1 + delta_y + 1,
                                               box_x2,
                                               box_y2);
                new_ble->boxes[i].setStorageAsCorners(box_x1s,
                                                      box_y1s + delta_y + 1, 
                                                      box_x2s,
                                                      box_y2s,
                                                      box_band);

                ble->boxes[i].setAsCorners(box_x1,
                                           box_y1,
                                           box_x2,
                                           box_y1 + delta_y);
                ble->boxes[i].setStorageAsCorners(box_x1s,
                                                  box_y1s,
                                                  box_x2s,
                                                  box_y1s + delta_y,
                                                  box_band);
            }

            if(list->insertAfter(new_ble,
                                bl_iterator.getCurrentPosition()) == _XILI_SLLIST_INVALID_POSITION) {
                //
                //  Oddly enough, insertion failed.
                //
                //  TODO: 2/26/96 jlf  Generate secondary failure?
                //
                return FALSE;
            }

            //
            //  Move to the next tile boundary in y
            //
            tile_yboundary += tile_ydelta;

            //
            //  Move on to the next box which will be the latter (bottom)
            //  portion of our most recent split.  This should always work if
            //  insertAfter() worked.  If it doesn't then we've got a bigger
            //  problem.
            //
            if(bl_iterator.getNext(ble) == XIL_FAILURE) {
                //
                //  TODO: 2/26/96 jlf  Generate secondary failure?
                //
                return FALSE;
            }

            //
            //  Reset our active box to the next one in the list.
            //
            active_box = &ble->boxes[box_number];
            
            active_box->getAsCorners(&abox_x1, &abox_y1, &abox_x2, &abox_y2);
            active_box->getStorageAsCorners(&abox_x1s, &abox_y1s,
                                            &abox_x2s, &abox_y2s,
                                            &abox_band);
        }
    }

    return TRUE;
}

//
//  Backward map a single point from destination to a source.  We call a
//  virtual function so the ops can overload the behavior.
//
XilStatus
XilOp::backwardMap(XilBox*       dst_box,
		   double        dx,
		   double        dy,
		   XilBox*       src_box,
		   double*       sx,
		   double*       sy,
		   unsigned int  src_num)
{
    return vBackwardMap(dst_box, dx, dy,
                        src_box, sx, sy, src_num);
}

Xil_boolean
XilOp::canForwardMap()
{
    return TRUE;
}

void
XilOp::setForwardMapping(Xil_boolean flag)
{
    forwardMappingFlag = flag;
    forwardStartOp     = this;
}

//--------------------------------------------------------------
//
//  switchToAlternateOp() does nothing but returns XIL_FAILURE
//  for the default case.
//
//--------------------------------------------------------------
XilStatus
XilOp::switchToAlternateOp()
{
    return XIL_FAILURE;
}

//--------------------------------------------------------------
//
//  XilOp Data Collection operators
//
//--------------------------------------------------------------
XilStatus
XilOp::reportResults(void* results[])
{
    return vReportResults(results);
}

//--------------------------------------------------------------
//
//  XilOp method used by IO devices to determine whether to
//  to capture a new frame or not.
//
//  When an XIL capture op is created, the isNewFrameFlag is
//  set to TRUE.  Once called, we change it to FALSE so 
//  calls from capture return FALSE.
//
//--------------------------------------------------------------
Xil_boolean
XilOp::isNewFrame()
{
    isNewFrameMutex.lock();

    Xil_boolean flag_val;

    if(isNewFrameFlag) {
        isNewFrameFlag = FALSE;
        flag_val = TRUE;
    } else {
        flag_val = FALSE;
    }

    isNewFrameMutex.unlock();

    return flag_val;
}

