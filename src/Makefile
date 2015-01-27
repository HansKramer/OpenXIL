#
# @(#)Makefile	1.22 98/02/12 
#
############
#
include $(XIL_CUSTOM)
include $(PROJECT_ROOT)/make/directory.cfg

SPECIAL_sun4_SUB_DIRS  = man

SPECIAL_sparc_SUB_DIRS = $(SPECIAL_sun4_SUB_DIRS)
SPECIAL_i386_SUB_DIRS  = $(SPECIAL_sun4_SUB_DIRS)
SPECIAL_ppc_SUB_DIRS  = $(SPECIAL_sun4_SUB_DIRS)

SPECIAL_HP-UX_SUB_DIRS = man
SPECIAL_IRIX_SUB_DIRS = man
SPECIAL_linux_SUB_DIRS = man

SPECIAL_windows_SUB_DIRS =

SUB_DIRS=libxil/include utils libxil bin IHV examples \
	$(SPECIAL_$(TARCH)_SUB_DIRS) release

XILCH_SUB_DIR=if [ "$(XILCHHOME)" ]; then echo "xilch"; fi

XILCH_sun4_SUB_DIR = $(XILCH_SUB_DIR:sh)

XILCH_sparc_SUB_DIR = $(XILCH_sun4_SUB_DIR)
XILCH_i386_SUB_DIR = $(XILCH_sun4_SUB_DIR)
XILCH_ppc_SUB_DIR = $(XILCH_sun4_SUB_DIR)

# XILCH_SUB_DIR:sh doesn't work on Linux or Win32 Cygnus GNUmake.
XILCH_HP-UX_SUB_DIR = xilch
XILCH_IRIX_SUB_DIR = xilch
XILCH_linux_SUB_DIR = xilch

XILCH_windows_SUB_DIR = xilch

SUB_DIRS += $(XILCH_$(TARCH)_SUB_DIR)

include $(PROJECT_ROOT)/make/rules/directory.cfg
