#
# @(#)Makefile	1.11 00/02/02 
#

include $(XIL_CUSTOM)
include $(PROJECT_ROOT)/make/directory.cfg

# SPECIAL directories are those which are different between architectures.
SPECIAL_common_DIRS  = computeInfo byte bit short float

SPECIAL_sparc_DIRS   = ${SPECIAL_common_DIRS} kcms 
SPECIAL_sparc_DIRS   = ${SPECIAL_common_DIRS} 
SPECIAL_i386_DIRS    = ${SPECIAL_common_DIRS} kcms
SPECIAL_linux_DIRS   = ${SPECIAL_common_DIRS} 
SPECIAL_IRIX_DIRS    = ${SPECIAL_common_DIRS} 
SPECIAL_HP-UX_DIRS   = ${SPECIAL_common_DIRS} 
SPECIAL_windows_DIRS = ${SPECIAL_common_DIRS}

SUB_DIRS= $(SPECIAL_$(TARCH)_DIRS)

include $(PROJECT_ROOT)/make/rules/directory.cfg

