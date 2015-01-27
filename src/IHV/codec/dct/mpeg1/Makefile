#
# @(#)Makefile	1.4 97/05/22 
#

include $(XIL_CUSTOM)
include $(PROJECT_ROOT)/make/directory.cfg

# SPECIAL directories are those which are different between architectures.
SPECIAL_sparc_DIRS=
SPECIAL_i386_DIRS=
SPECIAL_ppc_DIRS=

SUB_DIRS= memory \
	  $(SPECIAL_$(TARCH)_DIRS)

include $(PROJECT_ROOT)/make/rules/directory.cfg
