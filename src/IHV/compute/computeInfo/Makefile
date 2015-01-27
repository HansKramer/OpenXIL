#
# @(#)Makefile	1.30 99/01/20 
#
# Templates/Makefile.lib
# 	Template leaf node Makefile
#
LIBNAME=xiliUtils_ComputeInfo
LIB=${ARCH}/${LIBNAME}$(SHAREDLIB_EXT)$(XIL_GPI_VERS)
SLIB=${ARCH}/${LIBNAME}$(STATICLIB_EXT)$(XIL_GPI_VERS)
XIL_INST_LOC=${XIL_UTILS_SUBDIR}

include $(XIL_CUSTOM)
include $(PROJECT_ROOT)/make/lib.cfg

# add variant suffix (_g or _p) as appropriate
LIBNAME_V=xiliUtils_ComputeInfo$(LIB_VARIANT)$(SHAREDLIB_EXT)
DYNAMIC_VERSION_LIBNAME=${LIBNAME_V}$(XIL_GPI_VERS)

# C and C++ source names, if any, go here -- minus the .c or .cc
C_PIECES=
C_FILES=$(C_PIECES:%=%.c)
C_O_FILES=$(C_PIECES:%=${ARCH}/%.o)

CC_PIECES=\
	XiliNearestIndexSelector \
	XiliDiffusionUtils \
	XiliOrderedDitherLut \
	ComputeInfo       \
	ComputeInfoBIT    \
	ComputeInfoBYTE   \
	ComputeInfoSHORT  \
	ComputeInfoFLOAT  \
	ComputeInfoGENERAL

CC_FILES=$(CC_PIECES:%=%.cc)
CC_O_FILES=$(CC_PIECES:%=${ARCH}/%.o)

H_FILES=\
	mapfile \
	XiliNearestIndexSelector.hh \
	XiliDiffusionUtils.hh \
	XiliOrderedDitherLut.hh \
	ComputeInfo.hh

# Assembly source names, if any, go here -- minus the .s
S_PIECES=
S_FILES=$(S_PIECES:%=%.S)
S_O_FILES=$(S_FILES:%.S=${ARCH}/%.o)

SRCS=$(C_FILES) $(CC_FILES) $(H_FILES) $(S_FILES)
OBJS=$(C_O_FILES) $(CC_O_FILES) $(S_O_FILES)


#
# (OPTIONAL) Add local stuff here using +=
#
DEFINES   += 
CPPFLAGS  += 
CFLAGS    += $(CFLAGS_DYNAMIC_V)

LD_PATHS  += 
LD_LIBS   += $(LINK_xil) $(LINK_m) $(LINK_c)
LDFLAGS   += $(LDFLAGS_DYNAMIC_V) $(LDFLAGS_DYNAMIC_LIBNAME_V)

#
# Add your list of files to delete here.  The config files
#  already know how to delete some stuff, so you may want
#  to just run 'make clean' first to see what gets missed.
#  'make clobber' already includes 'make clean'
#

CLEAN_ADDITIONS += $(LIB)
CLOBBER_ADDITIONS +=

SPECIAL_COMMON_INSTALL = \
        $(INSTALL_VARIANT) -m 555 ${LIB} ${PROJECT_RELEASE}/${XIL_INST_LOC}; \
        cd ${PROJECT_RELEASE}/${XIL_INST_LOC}; $(RM) $(LIBNAME_V); \
        $(LN) -s ${LIBNAME_V}$(XIL_GPI_VERS) ${LIBNAME_V}

SPECIAL_sparc_INSTALL= ${SPECIAL_COMMON_INSTALL}
SPECIAL_i386_INSTALL= ${SPECIAL_COMMON_INSTALL}
SPECIAL_ppc_INSTALL= ${SPECIAL_COMMON_INSTALL}
SPECIAL_linux_INSTALL= ${SPECIAL_COMMON_INSTALL}
SPECIAL_IRIX_INSTALL= ${SPECIAL_COMMON_INSTALL}
SPECIAL_HP-UX_INSTALL=  \
        $(INSTALL_VARIANT) -m 555 ${LIB} ${PROJECT_RELEASE}/${XIL_INST_LOC};
SPECIAL_windows_INSTALL= \
        $(INSTALL_VARIANT) ${LIB} ${PROJECT_RELEASE}/${XIL_INST_LOC}; \
        $(INSTALL_VARIANT) ${SLIB} ${PROJECT_RELEASE}/${XIL_INST_LOC}



all:	${ARCH} $(SRCS) $(WAIT) $(LIB)

# Install the program(s), appending _g or _p as appropriate.
# for include files, just use $(INSTALL)
install:  all
	$(SPECIAL_$(TARCH)_INSTALL)

# Include the general rules for building targets
include $(PROJECT_ROOT)/make/rules/lib.cfg
