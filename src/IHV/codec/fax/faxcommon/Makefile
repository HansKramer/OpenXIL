#
# @(#)Makefile	1.14 97/05/27 
#
# Templates/Makefile.lib
#       Template library Makefile
#
LIBNAME=xiliUtils_FaxCommon

LIB=${ARCH}/${LIBNAME}$(SHAREDLIB_EXT)$(XIL_GPI_VERS)
SLIB=${ARCH}/${LIBNAME}$(STATICLIB_EXT)$(XIL_GPI_VERS)
XIL_INST_LOC=$(XIL_UTILS_SUBDIR)

include $(XIL_CUSTOM)
include $(PROJECT_ROOT)/make/lib.cfg

# add variant suffix (_g or _p) as appropriate
LIBNAME_V=xiliUtils_FaxCommon$(LIB_VARIANT)$(SHAREDLIB_EXT)
DYNAMIC_VERSION_LIBNAME=${LIBNAME_V}$(XIL_GPI_VERS)

# C and C++ source names, if any, go here -- minus the .c or .cc
C_PIECES=
C_FILES=$(C_PIECES:%=%.c)
C_O_FILES=$(C_PIECES:%=${ARCH}/%.o)

CC_PIECES_TO_FILE= $\$(CC_PIECES2:%=%.cc)
CC_PIECES_TO_O=    $\$(CC_PIECES:%=${ARCH}/%.o)

CC_PIECES = \
    XilDeviceCompressionFax  \
    fax_tables


CC_FILES=$(CC_PIECES_TO_FILE)
CC_O_FILES=$(CC_PIECES_TO_O)

H_FILES= \
	mapfile \
	XilDeviceCompressionFax.hh

# Assembly source names, if any, go here -- minus the .s
S_PIECES=
S_FILES=$(S_PIECES:%=%.S)
S_O_FILES=$(S_FILES:%.S=${ARCH}/%.o)

SRCS=$(C_FILES) $(CC_FILES) $(H_FILES) $(S_FILES)
OBJS=$(C_O_FILES) $(CC_O_FILES) $(S_O_FILES)

#
# Add local stuff here using +=
#
DEFINES   += 
CPPFLAGS  += 
CFLAGS    += $(CFLAGS_DYNAMIC_V)

LD_PATHS  += 
LD_LIBS   += $(LINK_xil) $(LINK_c)
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
        $(INSTALL_VARIANT) -m 555 ${LIB} ${PROJECT_RELEASE}/${XIL_INST_LOC};

SPECIAL_sparc_INSTALL= ${SPECIAL_COMMON_INSTALL}
SPECIAL_i386_INSTALL= ${SPECIAL_COMMON_INSTALL}
SPECIAL_ppc_INSTALL= ${SPECIAL_COMMON_INSTALL}
SPECIAL_linux_INSTALL= ${SPECIAL_COMMON_INSTALL}
SPECIAL_IRIX_INSTALL= ${SPECIAL_COMMON_INSTALL}
SPECIAL_HP-UX_INSTALL= ${SPECIAL_COMMON_INSTALL}
SPECIAL_windows_INSTALL= \
        $(INSTALL_VARIANT) ${LIB} ${PROJECT_RELEASE}/${XIL_INST_LOC}; \
        $(INSTALL_VARIANT) ${SLIB} ${PROJECT_RELEASE}/${XIL_INST_LOC}

all:	${ARCH} $(WAIT) $(LIB)

# Install the program(s), appending _g or _p as appropriate.
# for include files, just use $(INSTALL)
install:  all
	$(SPECIAL_$(TARCH)_INSTALL)

include $(PROJECT_ROOT)/make/rules/lib.cfg
