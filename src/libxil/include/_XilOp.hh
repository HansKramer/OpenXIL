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
//  File:	_XilOp.hh
//  Project:	XIL
//  Revision:	1.82
//  Last Mod:	10:21:20, 03/10/00
//
//  Description:
//	
//	
//	
//	
//	
//	
//  MT-Level:  SAFE
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)_XilOp.hh	1.82\t00/03/10  "


#include "_XilGPIDefines.hh"
#include "_XilObject.hh"
#include "_XilStorage.hh"

#ifdef  _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_INCLUDES

#include "XilOpPrivate.hh"

#undef  _XIL_PRIVATE_INCLUDES
#endif

#ifndef _XIL_OP_HH

class XilOp {
public:
    //
    //  Get Parameter Methods
    //
    //  Status indicates whether the parameter has successfully been
    //     set by the getParam routine.
    //
    //  NOTE: These parameter routines require the parameters be
    //        obtained in the same format in which they were stored.
    //        See the XIL Device Extensibility and Porting Guide for
    //        the parameters for a particular operation.
    //
    void                 getParam(unsigned int  n,
                                  int*          param);
    void                 getParam(unsigned int  n,
                                  unsigned int* param);
    void                 getParam(unsigned int  n,
                                  XilLongLong*  param);
    void                 getParam(unsigned int  n,
                                  float*        param);
    void                 getParam(unsigned int  n,
                                  double*       param);
    void                 getParam(unsigned int  n,
                                  void**        param);
    void                 getParam(unsigned int  n,
                                  XilObject**   param);

    //
    //  These are called by the compute routine to aquire the operation's
    //  image and/or CISs
    //
    XilImage*            getSrcImage(unsigned int n=1) const;
    XilImage*            getDstImage(unsigned int n=1) const;
    
    XilCis*              getSrcCis(unsigned int n=1) const;
    XilCis*              getDstCis(unsigned int n=1) const;

    //
    //  Returns an operations number which uniquely identifies this type of
    //  operation.  Each string name (i.e. copy;8()) is assigned a operation
    //  number upon registration with the XIL library.  This provides access
    //  to that op number for this operation.
    //
    unsigned int         getOpNumber() const;

    //
    //  Molecules in XIL 1.3 are given the base op and are then provided a
    //  list of ops in a depth-first ordering contained in the molecule.  The
    //  compute operation aquires this list from the op it is given which is
    //  the op writing into the molecule's destination images.  If there is
    //  only one op in the compute operation (this one), this function will
    //  generate a list with only one op in it.
    //
    XilOp**              getOpList();

    //
    //  Method for compute routines to get the data attached by a
    //  preprocess routine on the op.
    //
    void*                getPreprocessData(XilDeviceManagerCompute* compute_device,
                                           unsigned int             func_ident = 0);
    void*                getPreprocessData(XilDeviceIO* io_device,
                                           unsigned int func_ident = 0);

    void*                getPreprocessData(XilDeviceCompression* codec_device,
                                           unsigned int          func_ident = 0);

    //
    //  Splits the given box list in an operation-specific manner on source
    //  tile boundaries so that the entries in the list will cross tile
    //  boundaries to minimize the amount of storage cobbling required to
    //  complete the operation.  For many operations, no storage cobbling will
    //  be required. 
    //
    XilStatus            splitOnTileBoundaries(XilBoxList* boxlist);

    //
    //  This tells a random access device if it should go out and capture a
    //  new image or return another portion of the currently captured image.
    //
    Xil_boolean          isNewFrame();

    //
    //  Backward map a single point in destination box space to the
    //  corresponding point in source box space.  The last (optional) argument
    //  indicates which source to backward map into.  Rects and convex regions
    //  can be backward mapped by mapping each of the points.
    //
    XilStatus             backwardMap(XilBox*       dst_box,
				      double        dx,
				      double        dy,
				      XilBox*       src_box,
				      double*       sx,
				      double*       sy,
				      unsigned int  src_number = 1);


    //
    //  Data collection operator methods to report intermediate
    //  results and final results. These are callled by the compute
    //  routine.
    //
    XilStatus              reportResults(void* results[]);

private:
#ifdef _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_DATA
    
#include "XilOpPrivate.hh"
    
#undef  _XIL_PRIVATE_DATA
#else
    //
    //  Make it clear to the compiler that a destructor exists.  This is done
    //  so GPI users will get a compile-time error if they attempt to delete
    //  this class.
    //
                           ~XilOp();
#endif // _XIL_LIBXIL_PRIVATE
};

#ifdef  _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_POST_INCLUDES

#include "XilOpPrivate.hh"

#undef  _XIL_PRIVATE_POST_INCLUDES
#endif

#define _XIL_OP_HH
#endif // _XIL_OP_HH
