#
# @(#)Makefile	1.10 94/06/20 Sun Microsystems North Carolina Development Center
#
# make/Makefile
#
# Not strictly necessary to pull in this stuff.  But it helps with
#  working with the templates and poking around.
#

include $(XIL_CUSTOM)
include $(PROJECT_ROOT)/make/leaf.cfg

CLEAN_ADDITIONS += 
CLOBBER_ADDITIONS +=

all:

# Use -k on Templates so the SCCSID will be a useful template
# We bend over backwards to avoid problems here, since
#  people don't want to see errors (even ignored ones)
#  when getting the templates.
#
# Get a fresh copy of the template if the template is not
# checked out by *anyone*

get_templates: FORCEIT
	@-cd Templates; \
	for s in SCCS/s.*; do \
		b=`$(ECHO) $$s | $(SED) 's?^SCCS/s.??'`; \
		if [ -f SCCS/s.$$b ] && [ ! -f SCCS/p.$$b ]; \
		then \
			$(RM) $$b; \
			$(SCCS_GET) -k $$b; \
			$(CHMOD) -w $$b; \
		fi; \
	done

retrieve: get_templates
	-[ -d SCCS/. ] && $(SCCS_GET) SCCS
	-cd os;        [ -d SCCS/. ] && $(SCCS_GET) SCCS
	-cd compilers; [ -d SCCS/. ] && $(SCCS_GET) SCCS

install:  retrieve

get_DEPENDS=retrieve

include $(PROJECT_ROOT)/make/rules/leaf.cfg
