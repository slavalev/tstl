#################################################################################
#
#  Module Name:		Makefile
#
#  Author:			Slava I. Levtchenko <slavalev@gmail.com>
#
#  Abstract:		common Makefile - nmSYM command line options
#
#  Revision History:	29.07.2002	  started
#						26.09.2002    released (beta)
#
#################################################################################

NMSOPT = /TRANSLATE:SOURCE,PACKAGE,ALWAYS /SOURCE:$(MROOT);$(CI);$(LIBS);
NMSOPTSYS = $(NMSOPT)$(COM_DRV);$(COM_SYS_DRV);
NMSOPTVXD = $(NMSOPT)$(COM_DRV);$(COM_VXD_DRV);
