#
# @(#)Makefile	1.11 00/03/14 
#
# Templates/Makefile.leaf
# 	Template leaf node Makefile
#

include $(XIL_CUSTOM)
include $(PROJECT_ROOT)/make/leaf.cfg
include $(CONFIG.$(HOST_ARCH_NAME).C)

# C and C++ source names, if any, go here -- minus the .c or .cc
C_PIECES=fileio window example1
C_FILES=$(C_PIECES:%=%.c)
C_O_FILES=$(C_PIECES:%=${ARCH}/%.o)

H_FILES= Makefile.ship

# Assembly source names, if any, go here -- minus the .s
S_PIECES=
S_FILES=$(S_PIECES:%=%.S)
S_O_FILES=$(S_FILES:%.S=${ARCH}/%.o)

SRCS=$(C_FILES) $(CC_FILES) $(H_FILES) $(S_FILES)
OBJS=$(C_O_FILES) $(CC_O_FILES) $(S_O_FILES)

PGMS=${ARCH}/example1

#
# (OPTIONAL) Add local stuff here using +=
#

DEFINES  +=
CPPFLAGS += 
CFLAGS   +=

LD_PATHS  += 
LD_LIBS   += -lxil$(LIB_VARIANT) -lX11
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

${ARCH}/example1: ${OBJS}
	${LINK.c} -o $@ ${OBJS} ${LD_LIBS}

# Install the program(s), appending _g or _p as appropriate.
# for include files, just use $(INSTALL)

install:  all
	$(INSTALL_VARIANT) -m 555 ${PGMS} ${PROJECT_RELEASE}/bin

# Include the general rules for building targets
include $(PROJECT_ROOT)/make/rules/leaf.cfg
