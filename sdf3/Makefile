#
#   TU Eindhoven
#   Eindhoven, The Netherlands
#
#   Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
#
#   Date            :   March 29, 2002
#
#   History         :
#       13-04-11    :   Add sdf as dependency for fsmsadf (M.L.P.J. Koedam)
#   

MAKE		= make
MAKEFLAGS	= --no-print-directory

SDF3ROOT    = $(PWD)
export SDF3ROOT

MODULES 	= base sdf csdf fsmsadf sadf tools
.PHONY: doc $(MODULES)

# Settings
include $(SDF3ROOT)/etc/Makefile.inc

# Default target (build all)
default: all

# Build all modules
all: $(MODULES)

# Build single module and its dependencies
base:
	@echo ""
	@echo "### Module "$@
	@cd $@ && MODULE=$@ $(MAKE) $(MAKEFLAGS) && cd .. ;

sdf:	base
	@echo ""
	@echo "### Module "$@
	@cd $@ && MODULE=$@ $(MAKE) $(MAKEFLAGS) && cd .. ;

csdf:	sdf fsmsadf
	@echo ""
	@echo "### Module "$@
	@cd $@ && MODULE=$@ $(MAKE) $(MAKEFLAGS) && cd .. ;

sadf:	csdf fsmsadf
	@echo ""
	@echo "### Module "$@
	@cd $@ && MODULE=$@ $(MAKE) $(MAKEFLAGS) && cd .. ;

fsmsadf: sdf 
	@echo ""
	@echo "### Module "$@
	@cd $@ && MODULE=$@ $(MAKE) $(MAKEFLAGS) && cd .. ;

tools: 
	@echo ""
	@echo "### Module "$@
	@cd $@ && MODULE=$@ $(MAKE) $(MAKEFLAGS) && cd .. ;

# Cleanup
clean:	
	@echo Cleaning...
	@$(RM) $(BLD_DIR)/work $(BLD_DIR)/release $(BLD_DIR)/Debug $(BLD_DIR)/Release $(BLD_DIR)/doc $(CLEAN)

# Documentation
doc:
	cd doc && doxygen doxygen.cfg && cd .. ;

