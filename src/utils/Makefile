#
# @(#)Makefile	1.45 99/01/20 
#
# Templates/Makefile.leaf
# 	Template leaf node Makefile
#

LIBNAME=xiliUtils

SLIB=${ARCH}/${LIBNAME}$(STATICLIB_EXT)
LIB=${ARCH}/${LIBNAME}$(SHAREDLIB_EXT)$(XIL_GPI_VERS)

XIL_INST_LOC=${XIL_UTILS_SUBDIR}

include $(XIL_CUSTOM)
include $(PROJECT_ROOT)/make/lib.cfg

# add variant suffix (_g or _p) as appropriate
LIBNAME_V=$(LIBNAME)$(LIB_VARIANT)$(SHAREDLIB_EXT)
DYNAMIC_VERSION_LIBNAME=${LIBNAME_V}$(XIL_GPI_VERS)

# SPECIAL rules for memcpy which differs between architectures
SPECIAL_sparc_XILMEMCPY= xili_memcpy.S
SPECIAL_i386_XILMEMCPY= xili_memcpy.cc
SPECIAL_ppc_XILMEMCPY= xili_memcpy.cc
SPECIAL_linux_XILMEMCPY= xili_memcpy.cc
SPECIAL_windows_XILMEMCPY= xili_memcpy.cc
SPECIAL_HP-UX_XILMEMCPY= xili_memcpy.cc
SPECIAL_IRIX_XILMEMCPY= xili_memcpy.cc
 
SPECIAL_sparc_XILMEMCPYRULE= ${COMPILE.S} -o $@ $(SPECIAL_sparc_XILMEMCPY)
SPECIAL_i386_XILMEMCPYRULE=  ${COMPILE.c} -o $@ $(SPECIAL_i386_XILMEMCPY)
SPECIAL_ppc_XILMEMCPYRULE=   ${COMPILE.c} -o $@ $(SPECIAL_ppc_XILMEMCPY)
SPECIAL_linux_XILMEMCPYRULE=   ${COMPILE.c} -o $@ $(SPECIAL_linux_XILMEMCPY)
SPECIAL_windows_XILMEMCPYRULE= ${COMPILE.cc} $(SPECIAL_windows_XILMEMCPY) /Fo$@
SPECIAL_HP-UX_XILMEMCPYRULE=   ${COMPILE.c} -o $@ $(SPECIAL_HP-UX_XILMEMCPY)
SPECIAL_IRIX_XILMEMCPYRULE=   ${COMPILE.c} -o $@ $(SPECIAL_IRIX_XILMEMCPY)
 
# SPECIAL rules for memset which differs between architectures
SPECIAL_sparc_XILMEMSET= xili_memset.S
SPECIAL_i386_XILMEMSET= xili_memset.cc
SPECIAL_ppc_XILMEMSET= xili_memset.cc
SPECIAL_linux_XILMEMSET= xili_memset.cc
SPECIAL_windows_XILMEMSET= xili_memset.cc
SPECIAL_HP-UX_XILMEMSET= xili_memset.cc
SPECIAL_IRIX_XILMEMSET= xili_memset.cc
 
SPECIAL_sparc_XILMEMSETRULE= ${COMPILE.S} -o $@ $(SPECIAL_sparc_XILMEMSET)
SPECIAL_i386_XILMEMSETRULE=  ${COMPILE.c} -o $@ $(SPECIAL_i386_XILMEMSET)
SPECIAL_ppc_XILMEMSETRULE=   ${COMPILE.c} -o $@ $(SPECIAL_ppc_XILMEMSET)
SPECIAL_linux_XILMEMSETRULE=   ${COMPILE.c} -o $@ $(SPECIAL_linux_XILMEMSET)
SPECIAL_windows_XILMEMSETRULE= ${COMPILE.cc} $(SPECIAL_windows_XILMEMSET) /Fo$@
SPECIAL_HP-UX_XILMEMSETRULE=   ${COMPILE.c} -o $@ $(SPECIAL_HP-UX_XILMEMSET)
SPECIAL_IRIX_XILMEMSETRULE=   ${COMPILE.c} -o $@ $(SPECIAL_IRIX_XILMEMSET)
 
# C and C++ source names, if any, go here -- minus the .c or .cc
C_PIECES=
C_FILES=$(C_PIECES:%=%.c)
C_O_FILES=$(C_PIECES:%=${ARCH}/%.o)

CC_PIECES=\
	XiliUtilities \
	XiliColormapGenerator \
 	xili_hash_string \
 	xili_bit_utils \
	xili_geom_utils \
 	xili_svd \
        XiliMemMapFile \
        xili_dyn_links

CC_FILES=$(CC_PIECES:%=%.cc)
CC_O_FILES=$(CC_PIECES:%=${ARCH}/%.o)

H_FILES=\
	mapfile \
	XiliUtils.hh \
        XiliColormapGenerator.hh \
	XiliCSop.hh \
        xili_geom_utils.hh \
        xili_interp_utils.hh \
        xili_fill_utils.hh \
        XiliMemMapFile.hh \
	xili_svd.hh


# Assembly source names, if any, go here -- minus the .s
S_PIECES=
S_FILES=$(S_PIECES:%=%.S)
S_O_FILES=$(S_FILES:%.S=${ARCH}/%.o)

SRCS=$(C_FILES) $(CC_FILES) $(H_FILES) $(S_FILES)
OBJS=$(C_O_FILES) $(CC_O_FILES) $(S_O_FILES) \
  	${ARCH}/xili_memcpy.o \
  	${ARCH}/xili_memset.o

#
# (OPTIONAL) Add local stuff here using +=
#

DEFINES  +=
CPPFLAGS +=  -I$(PROJECT_ROOT)/src/libxil/include
CFLAGS   += $(CFLAGS_DYNAMIC_V)

#
#  Clear LD_LIBS because we're the only library that doesn't link 
#  with xiliutils.so
#
LD_LIBS=

#
# -l options replaced with LINK_?? in compilers/?.cfg
#
LD_PATHS += 
LD_LIBS  += $(LINK_threads)  \
			$(CPLUSPLUSLIB_A) \
			$(LINK_dl) \
			$(LINK_m) \
			$(LINK_c) \
			$(LINK_intl)
LDFLAGS  += $(LDFLAGS_DYNAMIC_V) $(LDFLAGS_DYNAMIC_LIBNAME_V)

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
        $(INSTALL_VARIANT) -m 555 ${LIB} ${PROJECT_RELEASE}/${XIL_INST_LOC};

SPECIAL_sparc_INSTALL= ${SPECIAL_COMMON_INSTALL}
SPECIAL_i386_INSTALL= ${SPECIAL_COMMON_INSTALL}
SPECIAL_ppc_INSTALL= ${SPECIAL_COMMON_INSTALL}
SPECIAL_linux_INSTALL= ${SPECIAL_COMMON_INSTALL}
SPECIAL_HP-UX_INSTALL= ${SPECIAL_COMMON_INSTALL}
SPECIAL_IRIX_INSTALL= ${SPECIAL_COMMON_INSTALL}
SPECIAL_windows_INSTALL= \
        $(INSTALL_VARIANT) ${LIB} ${PROJECT_RELEASE}/${XIL_INST_LOC}; \
        $(INSTALL_VARIANT) ${SLIB} ${PROJECT_RELEASE}/${XIL_INST_LOC}



all:	${ARCH} $(WAIT) $(LIB)

# Install the program(s), appending _g or _p as appropriate.
# for include files, just use $(INSTALL)
install:  all
	$(SPECIAL_$(TARCH)_INSTALL)

${ARCH}/xili_memcpy.o:  $(SPECIAL_$(TARCH)_XILMEMCPY)
	$(SPECIAL_$(TARCH)_XILMEMCPYRULE)
 
${ARCH}/xili_memset.o:  $(SPECIAL_$(TARCH)_XILMEMSET)
	$(SPECIAL_$(TARCH)_XILMEMSETRULE)

# Include the general rules for building targets
include $(PROJECT_ROOT)/make/rules/lib.cfg
