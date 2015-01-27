#
# @(#)Makefile	1.6 98/01/15 
#

include $(XIL_CUSTOM)
include $(PROJECT_ROOT)/make/directory.cfg

SUB_DIRS=include lib ops

GET_PO_FILE= \
	-[ -d SCCS/. ] && $(SCCS_GET) xil.po

INSTALL_PO_FILE= $(INSTALL) -m 444 xil.po  \
        $(PROJECT_ROOT)/$(HOST_ARCH_NAME)/locale/C/LC_MESSAGES

all_PRE_EXTRA= 	   $(GET_PO_FILE)
install_PRE_EXTRA= $(GET_PO_FILE); $(INSTALL_PO_FILE)
release_install_PRE_EXTRA= $(install_PRE_EXTRA)

include $(PROJECT_ROOT)/make/rules/directory.cfg
