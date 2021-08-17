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

# Source files that need to be handled (e.g. foo.cc)
SRCS = $(wildcard *.cc)

# Name of the static library (e.g. libfoo.a)
PROG_LIB =

# Name of the binary in distribution (e.g. foo)
PROG_BIN = sdf3convert-sdf-sadf
PROG_REL_BIN = sdf3convert-sdf-sadf

# Link with external libraries
USR_LIB_EXT  = $(LIB)sadf.$(LIBSO) $(LIB)fsmsadf.$(LIBSO) $(LIB)csdf.$(LIBSO) $(LIB)sdf.$(LIBSO) $(LIB)base.$(LIBSO)

#-------------------------------------------------------------------------------
# Rules
#-------------------------------------------------------------------------------

include $(SDF3ROOT)/etc/Makefile.rules

#-------------------------------------------------------------------------------
# Additional rules
#-------------------------------------------------------------------------------
