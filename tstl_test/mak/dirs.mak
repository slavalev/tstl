#################################################################################
#
#  Module Name:		Makefile
#
#  Author:			Slava I. Levtchenko <slavalev@gmail.com>
#
#  Abstract:		common Makefile - Locations
#
#  Revision History:	29.07.2002	  started
#						26.09.2002    released (beta)
#
#################################################################################

### common dirs ###
OUTROOT = .\out\\
OUTROOT = $(OUTROOT:\\=\)

!if "$(DEBUG)" == "1"
OUT  = $(OUTROOT)exe.deb\\
OBJS = $(OUTROOT)obj.deb\\
!else
OUT  = $(OUTROOT)exe.rel\\
OBJS = $(OUTROOT)obj.rel\\
!endif

OUT  = $(OUT:\\=\)
OBJS = $(OBJS:\\=\)

OUTDOX = $(OUTROOT)doxygen\\
OUTDOX = $(OUTDOX:\\=\)

APP  = .\app\\
CI   = .\inc\\
CFGS = .\cfg\\
DOC  = .\doc\\
DRV  = .\drv\\
LIBS = .\lib\\
SCR  = .\scr\\
TEST = .\tst\\

APP  = $(APP:\\=\)
CI   = $(CI:\\=\)
CFGS = $(CFGS:\\=\)
DOC  = $(DOC:\\=\)
DRV  = $(DRV:\\=\)
LIBS = $(LIBS:\\=\)
SCR  = $(SCR:\\=\)
TEST = $(TEST:\\=\)

### common applets ###
DOS_APP = $(APP)dos\\
WIN_APP = $(APP)win32\\

DOS_APP = $(DOS_APP:\\=\)
WIN_APP = $(WIN_APP:\\=\)

CLI  = $(WIN_APP)cli\\
GUI  = $(WIN_APP)gui\\
SRV  = $(WIN_APP)srv\\

CLI  = $(CLI:\\=\)
GUI  = $(GUI:\\=\)
SRV  = $(SRV:\\=\)

### common drivers ###
VXD_DRV = $(DRV)9x\\
SYS_DRV = $(DRV)nt\\
DOS_DRV = $(DRV)dos\\
COM_DRV = $(DRV)common\\

VXD_DRV = $(VXD_DRV:\\=\)
SYS_DRV = $(SYS_DRV:\\=\)
DOS_DRV = $(DOS_DRV:\\=\)
COM_DRV = $(COM_DRV:\\=\)

COM_VXD_DRV = $(VXD_DRV)common\\
COM_SYS_DRV = $(SYS_DRV)common\\

COM_VXD_DRV = $(COM_VXD_DRV:\\=\)
COM_SYS_DRV = $(COM_SYS_DRV:\\=\)

### specific paths ###
