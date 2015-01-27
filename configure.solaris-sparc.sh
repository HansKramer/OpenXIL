#!/bin/sh
#
HOSTARCH=`uname -p`
#
# Provide the path to the base directory of the tree
XIL_ROOT=`pwd`; export XIL_ROOT
#
# Provide the paths to the C and C++ compilers
XIL_C_PATH=/usr/sfw/bin/gcc; export XIL_C_PATH
XIL_CC_PATH=/usr/sfw/bin/gcc; export XIL_CC_PATH
#
# Provide the path to a static libC.a for resolving C++ dependencies
XIL_LIBCAPC_PATH=$XIL_ROOT/tools/lib/${HOSTARCH}/xil_libC.a; export XIL_LIBCAPC_PATH
#
# Provide the path to the reference and test data directory
# This is only necessary for running Xilch tests
XIL_DATA_DIR=/shared/xil/data; export XIL_DATA_DIR
#
#
# The rest of the environment variables are automatically set 
# relative to XIL_ROOT
#
XIL_CUSTOM=${XIL_ROOT}/make/custom/gnu.cfg; export XIL_CUSTOM
XILCHROOT=${XIL_ROOT}/src/xilch; export XILCHROOT
XILHOME=${XIL_ROOT}/${HOSTARCH}; export XILHOME
XILCHHOME=${XIL_ROOT}/${HOSTARCH}/xilch; export XILCHHOME
LD_LIBRARY_PATH=${XILCHHOME}/lib:${XILHOME}/utils:${XILHOME}/lib; export LD_LIBRARY_PATH 
 

