ifeq ($(RULES_MAKE), )
include $(PDK_INSTALL_PATH)/ti/build/Rules.make
else
include $(RULES_MAKE)
endif

COMP = external_drivers_comp

lib_$(COMP)_BUILD_DEPENDENCY = soc

$(COMP)_DOXYGEN_SUPPORT = no

include $(PDK_INSTALL_PATH)/ti/build/comp_top.mk
