#
# @(#)Makefile	1.9 98/02/09 
#
############
#
include $(XIL_CUSTOM)
include $(PROJECT_ROOT)/make/directory.cfg

SPECIAL_unix_DIRS=example1 example2 display movie_maker_example movie_player_example

SPECIAL_sparc_DIRS=$(SPECIAL_unix_DIRS)
SPECIAL_i386_DIRS=$(SPECIAL_unix_DIRS)
SPECIAL_ppc_DIRS=$(SPECIAL_unix_DIRS)

SPECIAL_HP-UX_DIRS=$(SPECIAL_unix_DIRS)
SPECIAL_IRIX_DIRS=$(SPECIAL_unix_DIRS)
SPECIAL_linux_DIRS=$(SPECIAL_unix_DIRS)

SPECIAL_windows_DIRS=win32

SUB_DIRS=\
          $(SPECIAL_$(TARCH)_DIRS)

XIL_TO_XGL=if [ "$(XGLHOME)" ]; then echo "libxil_to_xgl"; fi

SUB_DIRS += $(XIL_TO_XGL:sh)

# "get" the XGL files even if not building. They're needed for SUNWxilmn
GET_XGL_FILES= \
	-cd libxil_to_xgl;        [ -d SCCS/. ] && $(SCCS_GET) SCCS

GET_XGL_FILES_sparc=$(GET_XGL_FILES)
GET_XGL_FILES_i386=$(GET_XGL_FILES)
GET_XGL_FILES_ppc=$(GET_XGL_FILES)

GET_XGL_FILES_HP-UX=
GET_XGL_FILES_IRIX=
GET_XGL_FILES_linux=

GET_XGL_FILES_windows=

all_PRE_EXTRA= 	   $(GET_XGL_FILES_$(TARCH))
install_PRE_EXTRA= $(all_PRE_EXTRA)
release_install_PRE_EXTRA= $(install_PRE_EXTRA)

include $(PROJECT_ROOT)/make/rules/directory.cfg
