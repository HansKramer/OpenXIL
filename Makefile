#
# @(#)Makefile	1.56 00/02/16 Sun Microsystems North Carolina Development Center
#
# top level directory.
# This Makefile is *not* a good example of a directory Makefile.
#

include $(XIL_CUSTOM)
include $(PROJECT_ROOT)/make/directory.cfg

SUB_DIRS=src

CREATE_DIRS = 	bin \
		lib \
		locale \
		locale/C \
		locale/C/LC_MESSAGES \
		devhandlers \
		utils \
		include \
		include/xil \
		man \
		man/man3 \
		xilch \
		xilch/bin \
		xilch/config \
		xilch/include \
		xilch/include/xilch \
		xilch/lib

#
# For all platforms except Win32 create a symbolic link from $XILCHHOME/data
# to $(XIL_DATA_DIR) the latter of which should be defined in $XIL_CUSTOM.
# Win32 does not support symbolic links so that in this case all test data
# must be explicitely copied under $XILCHHOME/data.
#
SPECIAL_common_LN_DATA = $(LN) -s $(XIL_DATA_DIR) $(PROJECT_RELEASE)/xilch/data

SPECIAL_sparc_LN_DATA = $(SPECIAL_common_LN_DATA)
SPECIAL_i386_LN_DATA  = $(SPECIAL_common_LN_DATA)
SPECIAL_ppv_LN_DATA   = $(SPECIAL_common_LN_DATA)

SPECIAL_linux_LN_DATA = $(SPECIAL_common_LN_DATA)
SPECIAL_IRIX_LN_DATA  = $(SPECIAL_common_LN_DATA)
SPECIAL_HP-UX_LN_DATA = $(SPECIAL_common_LN_DATA)

SPECIAL_windows_LN_DATA =

# build script runs sun4-release for 4.1.x machine
#		    sparc-release for 5.0 machine
#
release sun4-release sparc-release:
	$(MAKE) install

all_PRE_EXTRA =	\
	-$(MKDIR) -p ${CREATE_DIRS:%=$(PROJECT_RELEASE)/%}; \
	chmod +x tools/bin/install-if-change; \
	chmod +x tools/bin/make-release; \
	chmod +x tools/bin/make-package; \
	cd make; $(MAKE) install; \
	$(SPECIAL_$(TARCH)_LN_DATA)
install_PRE_EXTRA = $(all_PRE_EXTRA)
release_install_PRE_EXTRA = $(install_PRE_EXTRA)

clobber_EXTRA = \
	$(RM) -r ${CREATE_DIRS:%=$(PROJECT_RELEASE)/%}; \
	/usr/bin/find $(PROJECT_ROOT) -name .make.\* -exec $(RM) {} \;

include $(PROJECT_ROOT)/make/rules/directory.cfg
