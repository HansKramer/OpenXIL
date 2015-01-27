#
# @(#)Makefile	1.61 99/01/20 
#
# Templates/Makefile.lib
#       Template library Makefile
#

LIBNAME=xilOp_SunSoft-XIL

LIB=${ARCH}/${LIBNAME}$(SHAREDLIB_EXT)$(XIL_GPI_VERS)
XIL_INST_LOC=$(XIL_DEVHANDLERS_SUBDIR)

include $(XIL_CUSTOM)
include $(PROJECT_ROOT)/make/lib.cfg

# add variant suffix (_g or _p) as appropriate
LIBNAME_V=$(LIBNAME)$(LIB_VARIANT)$(SHAREDLIB_EXT)
DYNAMIC_VERSION_LIBNAME=${LIBNAME_V}$(XIL_GPI_VERS)

# C and C++ source names, if any, go here -- minus the .c or .cc
C_PIECES=
C_FILES=$(C_PIECES:%=%.c)
C_O_FILES=$(C_PIECES:%=${ARCH}/%.o)

CC_PIECES=  \
	XiliOpUtils \
	XilOpEntry \
	XilOpArea \
	XilOpAreaKernel \
	XilOpAreaFill \
	XilOpAbsolute \
	XilOpAdd \
	XilOpAddConst \
	XilOpAffine \
	XilOpAnd \
	XilOpAndConst \
	XilOpBandCombine \
	XilOpBlackGeneration \
	XilOpBlend \
	XilOpCapture \
	XilOpCast \
	XilOpChooseColormap \
	XilOpColorConvert \
	XilOpColorCorrect \
	XilOpCompress \
	XilOpConvolve \
	XilOpCopy \
	XilOpCopyPattern \
	XilOpCopyWithPlanemask \
	XilOpDataCollect \
	XilOpDecompress \
	XilOpDilate \
	XilOpDisplay \
	XilOpDivide \
	XilOpDivideByConst \
	XilOpDivideIntoConst \
	XilOpEdgeDetection \
	XilOpErode \
	XilOpErrorDiffusion \
	XilOpExtrema \
	XilOpFill \
	XilOpGeometric \
	XilOpGeometricAffine \
	XilOpGeometricWarp \
	XilOpHistogram \
	XilOpIO \
	XilOpLookup \
	XilOpMax \
	XilOpMin \
	XilOpMultiply \
	XilOpMultiplyConst \
	XilOpNearestColor \
	XilOpNot \
	XilOpOr \
	XilOpOrConst \
	XilOpOrderedDither \
	XilOpPaint \
	XilOpRescale \
	XilOpRotate \
	XilOpScale \
	XilOpSetValue \
	XilOpSoftFill \
	XilOpSqueezeRange \
	XilOpSubsample \
	XilOpSubsampleAdaptive \
	XilOpSubsampleBinaryToGray \
	XilOpSubtract \
	XilOpSubtractConst \
	XilOpSubtractFromConst \
	XilOpTablewarp \
	XilOpTablewarpHorizontal \
	XilOpTablewarpVertical \
	XilOpThreshold \
	XilOpTranslate \
	XilOpTranspose \
	XilOpXor \
	XilOpXorConst
CC_PIECES+= XilOpUnsharp
CC_PIECES+= XilOpUnsharpIC

CC_FILES=$(CC_PIECES:%=%.cc)
CC_O_FILES=$(CC_PIECES:%=${ARCH}/%.o)

H_FILES= \
	XiliOpUtils.hh \
	XilOpArea.hh \
	XilOpAreaKernel.hh \
	XilOpAreaFill.hh \
	XilOpCopy.hh \
	XilOpColorCorrect.hh \
	XilOpColorConvert.hh \
	XilOpDataCollect.hh \
	XilOpGeometric.hh \
	XilOpGeometricAffine.hh \
	XilOpGeometricWarp.hh \
	XilOpIO.hh \
	XilOpOrderedDither.hh \
	XilOpPoint.hh \
	XilOpRotate.hh \
	XilOpScale.hh \
	XilOpSetValue.hh \
	XilOpSubsample.hh \
	XilOpTranslate.hh \
	XilOpTranspose.hh \
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

# 
#  TODO: jlf 2/13/96  Look at this again.
# 
#  This private define allows this class to have access to the internals
#  of the XIL library.  Fooling the compiler with the XilOp derivation 
#  doesn't seem to really work.
#
DEFINES  += -D_XIL_LIBXIL_PRIVATE # -DBOX_DEBUG
CPPFLAGS += -I../include
CFLAGS   += $(CFLAGS_DYNAMIC_V)

LD_PATHS += 
LD_LIBS  += $(LINK_xil) $(LINK_m) $(LINK_c)
LDFLAGS  += $(LDFLAGS_DYNAMIC_V)

#
# Add your list of files to delete here.  The config files
#  already know how to delete some stuff, so you may want
#  to just run 'make clean' first to see what gets missed.
#  'make clobber' already includes 'make clean'
#
CLEAN_ADDITIONS += 
CLOBBER_ADDITIONS +=

all:	${ARCH} $(WAIT) $(LIB)

# Install the library, appending _g or _p as appropriate.
# for include files, just use $(INSTALL)
install:  all
	$(INSTALL_VARIANT) -m 555 ${LIB} $(PROJECT_RELEASE)/${XIL_INST_LOC}

include $(PROJECT_ROOT)/make/rules/lib.cfg
