#
# @(#)Makefile	1.30 00/01/24 
#
# Templates/Makefile.lib
#       Template library Makefile
#
LIBNAME=xiliUtils_Codec
LIB=${ARCH}/${LIBNAME}$(SHAREDLIB_EXT)$(XIL_GPI_VERS)
SLIB=${ARCH}/${LIBNAME}$(STATICLIB_EXT)$(XIL_GPI_VERS)
XIL_INST_LOC=$(XIL_UTILS_SUBDIR)

include $(XIL_CUSTOM)
include $(PROJECT_ROOT)/make/lib.cfg

# add variant suffix (_g or _p) as appropriate
LIBNAME_V=xiliUtils_Codec$(LIB_VARIANT)$(SHAREDLIB_EXT)
DYNAMIC_VERSION_LIBNAME=${LIBNAME_V}$(XIL_GPI_VERS)

# C and C++ source names, if any, go here -- minus the .c or .cc
C_PIECES=
C_FILES=$(C_PIECES:%=%.c)
C_O_FILES=$(C_PIECES:%=${ARCH}/%.o)

CC_PIECES_TO_FILE= $\$(CC_PIECES2:%=%.cc)
CC_PIECES_TO_O=    $\$(CC_PIECES:%=${ARCH}/%.o)

SPECIAL_CCPIECES=
SPECIAL_sparc_CCPIECES= Dct_S
SPECIAL_sparc_SPIECES= dct fidct
SPECIAL_sun4_CCPIECES= $(SPECIAL_sparc_CCPIECES)
SPECIAL_sun4_SPIECES= $(SPECIAL_sparc_SPIECES)
SPECIAL_i386_CCPIECES= Dct_C Idct
SPECIAL_i386_SPIECES=
SPECIAL_ppc_CCPIECES=  Dct_C Idct
SPECIAL_ppc_SPIECES=
SPECIAL_linux_CCPIECES= Dct_C Idct
SPECIAL_linux_SPIECES=
SPECIAL_HP-UX_CCPIECES= Dct_C Idct
SPECIAL_HP-UX_SPIECES=
SPECIAL_IRIX_CCPIECES= Dct_C Idct
SPECIAL_IRIX_SPIECES=
SPECIAL_windows_CCPIECES= Dct_C Idct
SPECIAL_windows_SPIECES=

CC_PIECES = \
    ColorValue \
    CompressInfo \
    DctBlockUtils \
    DecompressInfo \
    HTable \
    Huffman_Code \
    Huffman_Encoder \
    IdctDecoder \
    IdctParser \
    IdctSplatter \
    JpegByteArray \
    JpegParser \
    JpegHuffmanDecoderBase \
    Jpeg_Huffman_Encoder_Base \
    MacroBlockBlt \
    QTable \
    Quantizer \
    SingleBuffer \
    Ycc2RgbConverter \
    ZigZag \
    xili_codec_utils \
    $(SPECIAL_$(TARCH)_CCPIECES) \
    $(SPECIAL_CCPIECES)

CC_FILES=$(CC_PIECES_TO_FILE)
CC_O_FILES=$(CC_PIECES_TO_O)

H_FILES= \
	mapfile \
	ColorValue.hh \
        CompressInfo.hh \
        DctBlockUtils.hh \
        DecompressInfo.hh \
	Dct.hh \
	HTable.hh \
	Huffman_Code.hh \
	Huffman_Encoder.hh \
	Idct.hh \
	IdctDecoder.hh \
	IdctParser.hh \
	IdctSplatter.hh \
	JpegBandInfo.hh \
	JpegByteArray.hh \
	JpegMacros.hh \
	JpegParser.hh \
	JpegHuffmanDecoderBase.hh \
	Jpeg_Huffman_Encoder_Base.hh \
	MacroBlockBlt.hh \
	QTable.hh \
	Quantizer.hh \
	SingleBuffer.hh \
	Ycc2RgbConverter.hh \
	ZigZag.hh \
        xili_codec_utils.hh


# Assembly source names, if any, go here -- minus the .s
#S_PIECES= dct fdither fidct strip
S_PIECES= $(SPECIAL_$(TARCH)_SPIECES)
S_FILES=$(S_PIECES:%=%.S)
S_O_FILES=$(S_FILES:%.S=${ARCH}/%.o)

SRCS=$(C_FILES) $(CC_FILES) $(H_FILES) $(S_FILES)
OBJS=$(C_O_FILES) $(CC_O_FILES) $(S_O_FILES)
    
#
# Add local stuff here using +=
#
SPECIAL_linux_HEADER_PATH=-I$(PROJECT_ROOT)/src/libxil/include
DEFINES   += 
CPPFLAGS  += -I$(PROJECT_ROOT)/src/IHV/compute/computeInfo \
		$(SPECIAL_$(TARCH)_HEADER_PATH)
CFLAGS    += $(CFLAGS_DYNAMIC_V)

LD_PATHS  += 
LD_LIBS  += $(LINK_xiliUtils_ComputeInfo) \
            $(LINK_xil)                   \
            $(LINK_c)

LDFLAGS   += $(LDFLAGS_DYNAMIC_V) $(LDFLAGS_DYNAMIC_LIBNAME_V)

#
# Add your list of files to delete here.  The config files
#  already know how to delete some stuff, so you may want
#  to just run 'make clean' first to see what gets missed.
#  'make clobber' already includes 'make clean'
#

CLEAN_ADDITIONS += $(LIB)
CLOBBER_ADDITIONS +=

SPECIAL_COMMON_INSTALL = \
        $(INSTALL_VARIANT) -m 555 ${LIB} ${PROJECT_RELEASE}/${XIL_INST_LOC};

SPECIAL_sparc_INSTALL= ${SPECIAL_COMMON_INSTALL}
SPECIAL_i386_INSTALL= ${SPECIAL_COMMON_INSTALL}
SPECIAL_ppc_INSTALL= ${SPECIAL_COMMON_INSTALL}
SPECIAL_linux_INSTALL= ${SPECIAL_COMMON_INSTALL}
SPECIAL_HP-UX_INSTALL= ${SPECIAL_COMMON_INSTALL}
SPECIAL_IRIX_INSTALL= ${SPECIAL_COMMON_INSTALL}
SPECIAL_windows_INSTALL= \
        $(INSTALL_VARIANT) ${LIB} ${PROJECT_RELEASE}/${XIL_INST_LOC}; \
        $(INSTALL_VARIANT) ${SLIB} ${PROJECT_RELEASE}/${XIL_INST_LOC}


all:	${ARCH} $(WAIT) $(LIB)

# Install the library, appending _g or _p as appropriate.
# for include files, just use $(INSTALL)
install:  all
	$(SPECIAL_$(TARCH)_INSTALL)

##
##  Setup special rule for Cmyk.o because it can't be compiled on i386
##  with inlining turned on. 
##
##  TODO: 2/24/97 jlf  Remove this when GCC works!
##
linux/MacroBlockBlt.o: MacroBlockBlt.cc
	${COMPILE.c} -O0 -fstrength-reduce -fthread-jumps -funroll-loops -fcse-follow-jumps -fexpensive-optimizations -fdelayed-branch -fschedule-insns2 -finline-functions -o $@ MacroBlockBlt.cc

include $(PROJECT_ROOT)/make/rules/lib.cfg
