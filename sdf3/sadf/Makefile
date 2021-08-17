#
#   TU Eindhoven
#   Eindhoven, The Netherlands
#
#   Author          :   Bart Theelen (B.D.Theelen@tue.nl)
#
#   Date            :   29 August 2006

include $(SDF3ROOT)/etc/Makefile.inc

#-------------------------------------------------------------------------------
# Variables
#-------------------------------------------------------------------------------

COMPONENTS      = analysis base print simulation transformation verification

# Link with libraries
USR_LIB_EXT  	= $(LIB)fsmsadf.$(LIBSO) $(LIB)csdf.$(LIBSO) $(LIB)sdf.$(LIBSO) $(LIB)base.$(LIBSO) 
MODULE_SH_LIB   = $(LIB)$(MODULE).$(LIBSO)
TOOLS           = tools

#-------------------------------------------------------------------------------
# Rules
#-------------------------------------------------------------------------------

include $(SDF3ROOT)/etc/Makefile-modules.rules

#-------------------------------------------------------------------------------
# Additional rules
#-------------------------------------------------------------------------------
