#
# @(#)Makefile	1.2 97/05/06 
#
# Templates/Makefile.leaf
# 	Template leaf node Makefile
#
include $(XIL_CUSTOM)
include $(PROJECT_ROOT)/make/leaf.cfg

all:

# Install the script
install:  all
	$(INSTALL) -m 555 xilcompdesc.pl ${PROJECT_RELEASE}/bin

# Include the general rules for building targets
include $(PROJECT_ROOT)/make/rules/leaf.cfg
