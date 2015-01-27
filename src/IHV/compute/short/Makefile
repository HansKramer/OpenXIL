#
# @(#)Makefile	1.46 99/01/20 
#
# Templates/Makefile.lib
#       Template library Makefile
#

LIBNAME=xilCompute_memorySHORT

LIB_EXT=$(SHAREDLIB_EXT)$(XIL_GPI_VERS)
LIB=${ARCH}/${LIBNAME}${LIB_EXT}
XIL_INST_LOC=${XIL_DEVHANDLERS_SUBDIR}

include $(XIL_CUSTOM)
include $(PROJECT_ROOT)/make/lib.cfg

# add variant suffix (_g or _p) as appropriate
LIBNAME_V=$(LIBNAME)$(LIB_VARIANT)$(SHAREDLIB_EXT)
DYNAMIC_VERSION_LIBNAME=${LIBNAME_V}$(XIL_GPI_VERS)

# C and C++ source names, if any, go here -- minus the .c or .cc
C_PIECES=
C_FILES=$(C_PIECES:%=%.c)
C_O_FILES=$(C_PIECES:%=${ARCH}/%.o)

CC_PIECES_TO_FILE= $\$(CC_PIECES:%=%.cc)
CC_PIECES_TO_O=    $\$(CC_PIECES:%=${ARCH}/%.o)

CC_PIECES= \
	Absolute \
	Add \
	AddConst \
	AffineNearest \
	AffineBilinear \
	AffineBicubic  \
	AffineGeneral \
	And \
	AndConst \
	BandCombine \
	BlackGeneration \
	Blend \
	Cast \
	Convolve \
	Convolve_Separable \
	Cmy \
	Cmyk \
	ColorConvert \
	Copy \
	CopyWithPlanemask \
	Dilate \
	Divide \
	DivideByConst \
	DivideIntoConst \
	EdgeDetection \
	Erode \
	ErrorDiffusion \
	Extrema \
	Fill \
	Histogram \
	Lookup \
	LookupCombined \
	Max \
	Min \
	Multiply \
	MultiplyConst \
	NearestColor \
	Not \
	Or \
	OrConst \
	OrderedDither \
	Paint \
	Photoycc \
	Rescale \
	Rgb709 \
	Rgblinear \
	RotateNearest \
	RotateBilinear \
	RotateBicubic \
	RotateGeneral \
	ScaleNearest \
	ScaleBilinear \
	ScaleBicubic \
	ScaleGeneral \
	SetValue \
	SoftFill \
	SqueezeRange \
	SubsampleAdaptive \
	Subtract \
	SubtractConst \
	SubtractFromConst \
	TablewarpNearest \
	TablewarpBilinear \
	TablewarpBicubic \
	TablewarpGeneral \
	TablewarpHorizontalNearest \
	TablewarpHorizontalBilinear \
	TablewarpHorizontalBicubic \
	TablewarpHorizontalGeneral \
	TablewarpVerticalNearest \
	TablewarpVerticalBilinear \
	TablewarpVerticalBicubic \
	TablewarpVerticalGeneral \
	Threshold    \
	TranslateBilinear \
	TranslateBicubic \
	TranslateGeneral \
        Transpose \
	Xor \
	XorConst \
	Y601 \
	Y709 \
	Ycc601 \
	Ycc709 \
	Ylinear \
	XilDeviceManagerComputeSHORT
CC_PIECES += Unsharp
CC_PIECES += UnsharpIC

CC_FILES=$(CC_PIECES_TO_FILE)
CC_O_FILES=$(CC_PIECES_TO_O)

H_FILES= XilDeviceManagerComputeSHORT.hh \
	color_convert.hh \
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
CPPFLAGS += -I../computeInfo
CFLAGS   += $(CFLAGS_DYNAMIC_V)

LD_PATHS += 
LD_LIBS  += $(LINK_xiliUtils_ComputeInfo) \
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

##
##  Setup special rule for Cmyk.o because it can't be compiled on i386
##  with inlining turned on. 
##
##  TODO: 6/6/96 jlf  Remove this hack!
##
i386/Cmyk.o: Cmyk.cc
	${COMPILE.c} +d -o $@ Cmyk.cc

include $(PROJECT_ROOT)/make/rules/lib.cfg
