#
# @(#)Makefile	1.8 94/05/15 
#
# Templates/Makefile.lib
#       Template library Makefile
#

LIBNAME=libxil

LIB=${ARCH}/${LIBNAME}$(SHAREDLIB_EXT)$(XIL_API_VERS)
SLIB=${ARCH}/${LIBNAME}$(STATICLIB_EXT)
XIL_INST_LOC=$(XIL_LIB_SUBDIR)

include $(XIL_CUSTOM)
include $(PROJECT_ROOT)/make/lib.cfg

# add variant suffix (_g or _p) as appropriate
LIBNAME_V=$(LIBNAME)$(LIB_VARIANT)$(SHAREDLIB_EXT)
DYNAMIC_VERSION_LIBNAME=${LIBNAME_V}$(XIL_API_VERS)

# C and C++ source names, if any, go here -- minus the .c or .cc
C_PIECES=
C_FILES=$(C_PIECES:%=%.c)
C_O_FILES=$(C_PIECES:%=${ARCH}/%.o)

CC_PIECES=  \
	    XilBox \
	    XilBoxList \
            XilCis \
            XilCisBuffer \
            XilCisBufferManager \
            XilColorspace \
            XilColorspaceList \
	    XilCondVar \
	    XilConvexRegionList \
	    XilDeferrableObject \
	    XilDevice \
            XilDeviceCompression \
	    XilDeviceIO \
	    XilDeviceManager \
            XilDeviceManagerCompression \
	    XilDeviceManagerCompute \
	    XilDeviceManagerStorage \
	    XilDeviceManagerIO \
	    XilDeviceStorage \
	    XilDitherMask \
	    XilError \
	    XilFunctionInfo \
	    XilGlobalState \
            XilHistogram \
	    XilImage \
	    XilImageFormat \
	    XilInterpolationTable \
	    XilKernel \
            XilLookup \
            XilLookupSingle \
            XilLookupCombined \
            XilLookupColorcube \
	    XilMutex \
	    XilNonDeferrableObject \
	    XilObject \
	    XilOp \
	    XilOpPrivate \
	    XilRoi \
	    XilRectList \
	    XilScanlineList \
	    XilSel \
	    XilStorage \
	    XilStorageAPI \
	    XilSystemState \
	    XilTile \
	    XilTileList \
	    c_bindings \
	    \
	    XiliBag \
	    XiliBoxListEntry \
            XiliCharHashTable \
            XiliCisBufferLList \
	    XiliConvexRegion \
	    XiliDagManager \
            XiliFrameInfoAList \
	    XiliGetPath \
            XiliObjectHashTable \
	    XiliOpen \
	    XiliOpCache \
	    XiliOpQueueEntry \
	    XiliOpTreeNode \
	    XiliProcessEnv \
	    XiliRect \
	    XiliRoiBitmask \
	    XiliRoiConvexRegion \
	    XiliRoiRect \
	    XiliScheduler \
	    XiliScanlineListInt \
	    XiliThread \
	$(SPECIAL_CCPIECES)
CC_FILES=$(CC_PIECES:%=%.cc)
CC_O_FILES=$(CC_PIECES:%=${ARCH}/%.o)

H_FILES=mapfile

# Assembly source names, if any, go here -- minus the .s
S_PIECES= \
	$(SPECIAL_$(TARCH)_SPIECES)
S_FILES=$(S_PIECES:%=%.S)
S_O_FILES=$(S_FILES:%.S=${ARCH}/%.o)

SRCS=$(C_FILES) $(CC_FILES) $(H_FILES) $(S_FILES)
OBJS=$(C_O_FILES) $(CC_O_FILES) $(S_O_FILES)

#
# Add local stuff here using +=
#

DEFINES  += -D_XIL_LIBXIL_PRIVATE #-DDAG_DEBUG
CPPFLAGS += -I./../include
CFLAGS   += $(CFLAGS_DYNAMIC_V)

LD_PATHS += ${OPENWINHOME}/lib #for OWCONFIG_LIB
LDFLAGS  += $(LDFLAGS_DYNAMIC_V) $(LDFLAGS_DYNAMIC_LIBNAME_V) 
LD_LIBS  += $(LINK_threads) \
            $(OWCONFIG_LIB) \
            $(LINK_X11) \
            $(LINK_Xext) \
            $(LINK_dl) \
            $(LINK_m) \
            $(LINK_intl) \
            $(LINK_c)

ifeq ($(shell uname -s),SunOS)
    LD_LIBS  += -Xlinker -R -Xlinker ${XIL_HOME}/utils
endif
ifeq ($(shell uname -s),Linux)
    LD_LIBS  += -Xlinker -rpath -Xlinker ${XIL_HOME}/utils
endif


#
# Add your list of files to delete here.  The config files
#  already know how to delete some stuff, so you may want
#  to just run 'make clean' first to see what gets missed.
#  'make clobber' already includes 'make clean'
#
CLEAN_ADDITIONS += 
CLOBBER_ADDITIONS +=

#
# Windows need the .lib file ( apart from .dll ) for linking
#
SPECIAL_COMMON_INSTALL = \
	$(INSTALL_VARIANT) -m 555 ${LIB} ${PROJECT_RELEASE}/${XIL_INST_LOC}; \
	cd ${PROJECT_RELEASE}/lib; $(RM) $(LIBNAME_V); $(LN) -s ${LIBNAME_V}${XIL_API_VERS} ${LIBNAME_V}

SPECIAL_sparc_INSTALL= ${SPECIAL_COMMON_INSTALL}
SPECIAL_i386_INSTALL= ${SPECIAL_COMMON_INSTALL}
SPECIAL_ppc_INSTALL= ${SPECIAL_COMMON_INSTALL}
SPECIAL_linux_INSTALL= ${SPECIAL_COMMON_INSTALL}
SPECIAL_HP-UX_INSTALL= $(INSTALL_VARIANT) -m 555 ${LIB} ${PROJECT_RELEASE}/${XIL_INST_LOC}
SPECIAL_IRIX_INSTALL= ${SPECIAL_COMMON_INSTALL}
SPECIAL_windows_INSTALL= \
	$(INSTALL_VARIANT) ${LIB} ${PROJECT_RELEASE}/${XIL_INST_LOC}; \
	$(INSTALL_VARIANT) ${SLIB} ${PROJECT_RELEASE}/${XIL_INST_LOC}

all:	${ARCH} $(WAIT) $(LIB)

# Install the library, appending _g or _p as appropriate.
# for include files, just use $(INSTALL)
install:  all
#	$(INSTALL_VARIANT) -m 555 ${LIB} ${PROJECT_RELEASE}/${XIL_INST_LOC}
	$(SPECIAL_$(TARCH)_INSTALL)

linux/XilSystemState.o: XilSystemState.cc
	${COMPILE.c} -fimplicit-templates -o $@ $<

include $(PROJECT_ROOT)/make/rules/lib.cfg
