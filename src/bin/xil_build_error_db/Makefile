#
# @(#)Makefile	1.9 97/05/07 
#
# Templates/Makefile.leaf
# 	Template leaf node Makefile
#

include $(XIL_CUSTOM)
include $(PROJECT_ROOT)/make/leaf.cfg

# C and C++ source names, if any, go here -- minus the .c or .cc
C_PIECES=
C_FILES=$(C_PIECES:%=%.c)
C_O_FILES=$(C_PIECES:%=${ARCH}/%.o)

CC_PIECES=xil_build_error_db
CC_FILES=$(CC_PIECES:%=%.cc)
CC_O_FILES=$(CC_PIECES:%=${ARCH}/%.o)

H_FILES=

# Assembly source names, if any, go here -- minus the .s
S_PIECES=
S_FILES=$(S_PIECES:%=%.S)
S_O_FILES=$(S_FILES:%.S=${ARCH}/%.o)

SRCS=$(C_FILES) $(CC_FILES) $(H_FILES) $(S_FILES)
OBJS=$(C_O_FILES) $(CC_O_FILES) $(S_O_FILES)

PGMS=${ARCH}/xil_build_error_db$(BINARY_EXT)

#
# (OPTIONAL) Add local stuff here using +=
#

DEFINES  +=
CPPFLAGS +=
CFLAGS   +=

#
#  Clear LD_LIBS because we're not expected to link with xiliUtils.
#
LD_LIBS= -lstdc++

LD_PATHS  += 
LD_LIBS   += 
LDFLAGS   += 

#
# Add your list of files to delete here.  The config files
#  already know how to delete some stuff, so you may want
#  to just run 'make clean' first to see what gets missed.
#  'make clobber' already includes 'make clean'
#

CLEAN_ADDITIONS += 
CLOBBER_ADDITIONS +=

all:	${ARCH} $(SRCS) $(WAIT) $(PGMS)

#
#  LD_OUTFILE - defined in compilers/*.cfg
#
${ARCH}/xil_build_error_db$(BINARY_EXT): ${OBJS}
	${LINK.c} $(LD_OUTFILE) ${OBJS} ${LD_LIBS}

# Install the program(s), appending _g or _p as appropriate.
# for include files, just use $(INSTALL)
install:  all
	$(INSTALL_VARIANT) -m 555 ${PGMS} ${PROJECT_RELEASE}/bin
	${RM} ${PROJECT_RELEASE}/${XIL_UTILS_SUBDIR}/xil_errors_db
	${PROJECT_RELEASE}/bin/xil_build_error_db${LIB_VARIANT} ${PROJECT_ROOT}/src/libxil/xil.po ${PROJECT_RELEASE}/${XIL_UTILS_SUBDIR}/xil_errors_db
	${CHMOD} 444 ${PROJECT_RELEASE}/${XIL_UTILS_SUBDIR}/xil_errors_db


# Include the general rules for building targets
include $(PROJECT_ROOT)/make/rules/leaf.cfg
