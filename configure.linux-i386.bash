#! /bin/csh

# Provide the path to the base directory of the tree
export XIL_ROOT=`pwd`

# Provide the paths to the C and C++ compilers
export XIL_C_PATH=/usr/bin/gcc
export XIL_CC_PATH=/usr/bin/gcc

# Provide the path to a static libC.a for resolving C++ dependencies
export XIL_LIBCAPC_PATH=/usr/lib/gcc-lib/i386-redhat-linux/3.2.2/libgcc.a

# Provide the path to the reference and test data directory
# This is only necessary for running Xilch tests
export XIL_DATA_DIR=${XIL_ROOT}/data

#
# The rest of the environment variables are automatically set 
# relative to XIL_ROOT
#
export XIL_CUSTOM=${XIL_ROOT}/make/custom/linux.cfg
export XILCHROOT=${XIL_ROOT}/src/xilch
export XILHOME=${XIL_ROOT}/linux
export XILCHHOME=${XIL_ROOT}/linux/xilch
export LD_LIBRARY_PATH=${XILCHHOME}/lib:${XILHOME}/utils:${XILHOME}/lib

PATH=$PATH:./tools/bin
