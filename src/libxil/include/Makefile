#
# @(#)Makefile	1.98 00/01/31 
#
# Templates/Makefile.leaf
# 	Template leaf node Makefile
#

include $(XIL_CUSTOM)
include $(PROJECT_ROOT)/make/leaf.cfg

XIL_INST_LOC=$(PROJECT_RELEASE)/include/xil

XIL_OFFICIAL_HEADERFILES =\
	xilGPI.hh \
	xil.h

##
##  Per Minutes of LSARC Commitment Review for 1995/117 and 1993/641 
##  on 3/18/96:
##
##    "It might be a good idea to give this an underscore lead in, to warn
##     people about them.  The team will install them with an underscore.
##     These will be considered Private."
##
XIL_PRIVATE_HEADERFILES=\
	_XilBox.hh \
	_XilBoxList.hh \
	_XilCis.hh \
        _XilCisBuffer.hh \
        _XilCisBufferManager.hh \
	_XilClasses.hh \
        _XilColorspace.hh \
        _XilColorspaceList.hh \
	_XilFunctionInfo.hh \
	_XilCondVar.hh \
	_XilConvexRegionList.hh \
	_XilDeferrableObject.hh \
	_XilDevice.hh \
	_XilDeviceStorage.hh \
	_XilDeviceCompression.hh \
	_XilDeviceIO.hh \
	_XilDeviceManager.hh \
	_XilDeviceManagerCompute.hh \
	_XilDeviceManagerCompression.hh \
	_XilDeviceManagerStorage.hh \
	_XilDeviceManagerIO.hh \
	_XilDitherMask.hh \
	_XilGlobalState.hh \
	_XilGPIDefines.hh \
	_XilHistogram.hh \
	_XilImage.hh \
	_XilImageFormat.hh \
	_XilInterpolationTable.hh \
	_XilKernel.hh \
	_XilLookup.hh \
	_XilLookupSingle.hh \
	_XilLookupColorcube.hh \
	_XilLookupCombined.hh \
	_XilMutex.hh \
	_XilNonDeferrableObject.hh \
	_XilObject.hh \
	_XilOp.hh \
	_XilRectList.hh \
	_XilRoi.hh \
	_XilScanlineList.hh \
	_XilSel.hh \
	_XilStorage.hh \
	_XilSystemState.hh \
	_XilTile.hh \
	_XilTileList.hh \
	_XilDefines.h \
	_c_bindings.h \
	version.h

SPECIAL_i386_CONFIG=config.1
SPECIAL_ppc_CONFIG=config.1
SPECIAL_sparc_CONFIG=config.1

SPECIAL_linux_CONFIG=config.2
SPECIAL_windows_CONFIG=config.2
SPECIAL_IRIX_CONFIG=config.2
SPECIAL_HP-UX_CONFIG=config.2

OTHER_INSTALL_FILES= \
	${SPECIAL_$(TARCH)_CONFIG}

PRIVATE_HH_FILES=\
	XilBoxPrivate.hh \
	XilBoxListPrivate.hh \
	XilCisPrivate.hh \
	XilCisBufferPrivate.hh \
	XilCisBufferManagerPrivate.hh \
	XilClassesPrivate.hh \
        XilColorspacePrivate.hh \
        XilColorspaceListPrivate.hh \
	XilCondVarPrivate.hh \
	XilConvexRegionListPrivate.hh \
	XilDeferrableObjectPrivate.hh \
	XilDefinesPrivate.hh \
	XilDevicePrivate.hh \
	XilDeviceIOPrivate.hh \
	XilDeviceManagerPrivate.hh \
	XilDeviceManagerComputePrivate.hh \
	XilDeviceManagerCompressionPrivate.hh \
	XilDitherMaskPrivate.hh \
	XilError.hh \
	XilErrorPrivate.hh \
	XilFunctionInfoPrivate.hh \
	XilGlobalStatePrivate.hh \
        XilHistogramPrivate.hh \
	XilImagePrivate.hh \
	XilImageFormatPrivate.hh \
	XilInterpolationTablePrivate.hh \
	XilKernelPrivate.hh \
        XilLookupBasePrivate.hh \
        XilLookupSinglePrivate.hh \
        XilLookupColorcubePrivate.hh \
        XilLookupCombinedPrivate.hh \
	XilMutexPrivate.hh \
        XilNonDeferrableObjectPrivate.hh \
	XilObjectPrivate.hh \
	XilOpPrivate.hh \
	XilRectListPrivate.hh \
	XilRoiPrivate.hh \
	XilScanlineListPrivate.hh \
	XilSelPrivate.hh \
	XilStorageAPI.hh \
	XilStoragePrivate.hh \
	XilSystemStatePrivate.hh \
	XilTilePrivate.hh \
	XilTileListPrivate.hh \
	XiliBag.hh \
	XiliBitField.hh \
	XiliBoxListEntry.hh \
        XiliCharHashTable.hh \
        XiliCisBufferLList.hh \
	XiliConvexRegion.hh \
	XiliDagManager.hh \
	XiliDag.hh \
        XiliFrameInfoAList.hh \
        XiliFrameInfo.hh \
	XiliHashTable.hh \
	XiliHashTable.cc \
	XiliList.hh \
	XiliList.cc \
	XiliMarker.hh \
	XiliMemoryDefines.hh \
        XiliObjectHashTable.hh \
	XiliOpCache.hh \
	XiliOpQueueEntry.hh \
	XiliOpTreeNode.hh \
	XiliProcessEnv.hh \
	XiliReadWrite.hh \
	XiliRect.hh \
	XiliScanlineListInt.hh \
	XiliScheduler.hh \
	XiliSLList.hh \
	XiliSLList.cc \
	XiliStack.hh \
	XiliStorageRecord.hh \
	XiliString.hh \
	XiliThread.hh \
	XiliTokenizer.hh \
        XiliXPanoramix.hh

        SHELL = bash

all:	$(PRIVATE_HH_FILES) \
	$(XIL_OFFICIAL_HEADERFILES) \
	$(XIL_PRIVATE_HEADERFILES) \
	$(OTHER_INSTALL_FILES) 

##
##  Install the header files
##
install:  all
	$(INSTALL) -m 644 ${SPECIAL_$(TARCH)_CONFIG} ${PROJECT_RELEASE}/config
	$(INSTALL) -m 444 $(XIL_OFFICIAL_HEADERFILES) ${XIL_INST_LOC}
	$(INSTALL) -m 444 $(XIL_PRIVATE_HEADERFILES) ${XIL_INST_LOC}

include $(PROJECT_ROOT)/make/rules/leaf.cfg

