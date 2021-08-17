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

COMPONENTS      = base 

# hack to make things work. This should be fixed by moving the 'base' stuff out
# of tools.
USR_LIB_EXT  = $(LIB)sdf.$(LIBSO) $(LIB)base.$(LIBSO)
MODULE_SH_LIB   = $(LIB)$(MODULE).$(LIBSO)

TOOLS           = sdf3analyze sdf3flow sdf3generate sdf3transform \
                  sdf3flow-mnemee sdf3inputconvert sdf3outputconvert

#-------------------------------------------------------------------------------
# Rules
#-------------------------------------------------------------------------------

include $(SDF3ROOT)/etc/Makefile-modules.rules

#-------------------------------------------------------------------------------
# Additional rules
#-------------------------------------------------------------------------------
