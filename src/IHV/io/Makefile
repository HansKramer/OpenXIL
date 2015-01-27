#
# @(#)Makefile	1.18 00/01/31 
#

include $(XIL_CUSTOM)
include $(PROJECT_ROOT)/make/directory.cfg

# SPECIAL directories are those which are different between architectures.
SPECIAL_common_DIRS  = xlib xshm

SPECIAL_sparc_DIRS   = ${SPECIAL_common_DIRS}
SPECIAL_i386_DIRS    = ${SPECIAL_common_DIRS} 
SPECIAL_ppc_DIRS     = ${SPECIAL_common_DIRS} 
SPECIAL_linux_DIRS   = ${SPECIAL_common_DIRS} 
SPECIAL_IRIX_DIRS    = ${SPECIAL_common_DIRS} 
SPECIAL_HP-UX_DIRS   = ${SPECIAL_common_DIRS} 

SPECIAL_windows_DIRS = win32 

SUB_DIRS= $(SPECIAL_$(TARCH)_DIRS)

include $(PROJECT_ROOT)/make/rules/directory.cfg
