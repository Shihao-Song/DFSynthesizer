#
#   TU Eindhoven
#   Eindhoven, The Netherlands
#
#   Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
#
#   Date            :   March 29, 2002
#   

include $(SDF3ROOT)/etc/Makefile.inc

#-------------------------------------------------------------------------------
# Variables
#-------------------------------------------------------------------------------

COMPONENTS      = base analysis generate output resource_allocation transform

# Link with external libraries

USR_LIB_EXT     = $(LIB)base.$(LIBSO) $(LIB)sdf.$(LIBSO)

MODULE_SH_LIB   = $(LIB)$(MODULE).$(LIBSO)
TOOLS           = tools

#-------------------------------------------------------------------------------
# Rules
#-------------------------------------------------------------------------------

include $(SDF3ROOT)/etc/Makefile-modules.rules

#-------------------------------------------------------------------------------
# Additional rules
#-------------------------------------------------------------------------------
