#################################################################################
#
#  Module Name:		Makefile
#
#  Author:			Slava I. Levtchenko <slavalev@gmail.com>
#
#  Abstract:		common Makefile - module dependencies
#
#  Revision History: 29.07.2002	  started
#					 26.09.2002   released (beta)
#
#################################################################################

CONSOLE_MODULES = $(APP)$(PRJ_NAME).cpp $(APP)sysioctl.cpp

SRV_INCLUDES = /I$(TSTL)

# /T
MKDEP_FLAGS = /W /V /E

$(OUT)Makefile.dep dep:
	if exist $(OUT)Makefile.dep attrib -R $(OUT)Makefile.dep
	$(BIN)mkdep.exe $(MKDEP_FLAGS) /O$(OUT)tmp.dep /P$(OBJS)$(PRJ_NAME)\ $(SRV_INCLUDES) $(CONSOLE_MODULES)
	type $(OUT)tmp.dep  >$(OUT)Makefile.dep
	type MkDep.log >$(OUT)MakeDep.log
	del $(OUT)tmp.dep
	del MkDep.log
