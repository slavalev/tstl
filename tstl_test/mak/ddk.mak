#################################################################################
#
#  Module Name:		Makefile
#
#  Author:			Slava I. Levtchenko <slavalev@gmail.com>
#
#  Abstract:		common Makefile - DDK specific settings
#
#  Revision History:	29.07.2002	  started
#						26.09.2002    released (beta)
#
#################################################################################

!if "$(TARGET_OS_VER)" == "NT2K"

MASM = "$(DDKROOT)\bin\ml.exe"
RSC16 = "$(9XDDK)\bin\win98\bin16\rc.exe"
ADDRES2VXD = "$(9XDDK)\bin\win98\adrc2vxd.exe"

!elseif "$(TARGET_OS_VER)" == "NT4"

MASM = "$(BASEDIR)\bin\ml.exe"
RSC16 = "$(9XDDK)\bin\win98\bin16\rc.exe"
ADDRES2VXD = "$(9XDDK)\bin\win98\adrc2vxd.exe"

!elseif "$(TARGET_OS_VER)" == "W95"

MASM  = "$(9XDDK)\MASM611C\ml.exe"
RSC16 = $(BIN)rc16.exe
ADDRES2VXD = "$(9XDDK)\bin\adrc2vxd.exe"

!elseif "$(TARGET_OS_VER)" == "W98"

MASM  = "$(9XDDK)\bin\win98\ml.exe"
RSC16 = "$(9XDDK)\bin\win98\bin16\rc.exe"
ADDRES2VXD = "$(9XDDK)\bin\win98\adrc2vxd.exe"

!elseif "$(TARGET_OS_VER)" == "WME"

MASM  = "$(9XDDK)\bin\win_me\bin\ml.exe"
RSC16 = "$(9XDDK)\bin\win_me\bin16\rc.exe"
ADDRES2VXD = "$(9XDDK)\bin\win_me\bin\adrc2vxd.exe"

!else

MASM  = "$(9XDDK)\MASM611C\ml.exe"
RSC16 = $(BIN)rc16.exe
ADDRES2VXD = "$(9XDDK)\bin\adrc2vxd.exe"

!endif
