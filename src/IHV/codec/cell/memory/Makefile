#
# @(#)Makefile	1.12 99/01/20 
#
# Templates/Makefile.lib
#       Template library Makefile
#
LIBNAME=xilCompress_Cell
LIB=${ARCH}/${LIBNAME}$(SHAREDLIB_EXT)$(XIL_GPI_VERS)
XIL_INST_LOC=$(XIL_DEVHANDLERS_SUBDIR)

include $(XIL_CUSTOM)
include $(PROJECT_ROOT)/make/lib.cfg

# C and C++ source names, if any, go here -- minus the .c or .cc
C_PIECES=
C_FILES=$(C_PIECES:%=%.c)
C_O_FILES=$(C_PIECES:%=${ARCH}/%.o)

CC_PIECES_TO_FILE= $\$(CC_PIECES2:%=%.cc)
CC_PIECES_TO_O=    $\$(CC_PIECES:%=${ARCH}/%.o)

CC_PIECES = \
    AdaptiveColormapSelection \
    CellCompressorData \
    CellDecompressorData \
    CellAttribs \
    CellFrame \
    CellOutput \
    CmapTable \
    InFrame \
    adjustStart \
    bitRateControl \
    burnFrames \
    burnLines \
    byteStreamError \
    decompressHeader \
    findNextFrameBoundary \
    flush \
    interEncode \
    seek \
    controlColormapOutput \
    encodeBTC \
    encodeDither \
    intraEncodeImage \
    temporalFilterImages \
    compress \
    decompress \
    decompressNearestColor \
    decompressOrderedDither \
    XilDeviceCompressionCell \
    XilDeviceManagerCompressionCell 


CC_FILES=$(CC_PIECES_TO_FILE)
CC_O_FILES=$(CC_PIECES_TO_O)

H_FILES= \
    AdaptiveColormapSelection.hh \
    CmapTable.hh \
    InFrame.hh \
    CellAttribs.hh \
    CellDefines.hh \
    CellFrame.hh \
    CellOutput.hh \
    CellCompressorData.hh \
    CellDecompressorData.hh \
    XilDeviceCompressionCell.hh \
    XilDeviceManagerCompressionCell.hh \
    mapfile

# Assembly source names, if any, go here -- minus the .s
S_PIECES=
S_FILES=$(S_PIECES:%=%.S)
S_O_FILES=$(S_FILES:%.S=${ARCH}/%.o)

SRCS=$(C_FILES) $(CC_FILES) $(H_FILES) $(S_FILES)
OBJS=$(C_O_FILES) $(CC_O_FILES) $(S_O_FILES)

#
# Add local stuff here using +=
#
DEFINES  +=
CPPFLAGS += -I$(PROJECT_ROOT)/src/IHV/codec/common 
CFLAGS   += $(CFLAGS_DYNAMIC_V)

LD_PATHS += 
LD_LIBS  += $(LINK_xiliUtils_ComputeInfo) \
            $(LINK_xiliUtils_Codec)       \
            $(LINK_xil)                   \
            $(LINK_m)                     \
            $(LINK_c)

LDFLAGS  += $(LDFLAGS_DYNAMIC_V)

#
# Add your list of files to delete here.  The config files
#  already know how to delete some stuff, so you may want
#  to just run 'make clean' first to see what gets missed.
#  'make clobber' already includes 'make clean'
#

CLEAN_ADDITIONS += $(LIB)
CLOBBER_ADDITIONS +=

both: install debug_install
all:	${ARCH} $(WAIT) $(LIB)

# Install the library, appending _g or _p as appropriate.
# for include files, just use $(INSTALL)
install:  all
	$(INSTALL_VARIANT) -m 555 ${LIB} ${PROJECT_RELEASE}/${XIL_INST_LOC}

include $(PROJECT_ROOT)/make/rules/lib.cfg
