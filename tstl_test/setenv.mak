#################################################################################
#
#  Module Name:		Makefile
#
#  Author:		Slava I. Levtchenko <slavalev@gmail.com>
#
#  Abstract:		common Makefile - main environment settings
#
#  Revision History: 29.07.2002	  started
#					 26.09.2002   released (beta)
#
#################################################################################

PRJ_NAME = tstl_test

!ifndef TOOLKIT_VOL
# home = E:/
TOOLKIT_VOL = D:/
!endif

TOOLKIT_VOL=$(TOOLKIT_VOL:/=\)

# protect for redefine
!ifndef DEBUG
# Debug flag [0|1]
DEBUG = 0
!endif

!ifndef LANG
# en, ru, de, fr, ""
LANG = en
!endif

# select NT DDK
!ifndef TARGET_OS_VER
# <NT4>, NT2K, XP, W95, <W98>, WME. NT4 is multiplatform building configuration key
TARGET_OS_VER = NT4
!endif

## ==================== DDK variables ====================
!ifndef WDKROOT
#WDKROOT=$(TOOLKIT_VOL)WinDDK\6001
WDKROOT=$(TOOLKIT_VOL)WinDDK\7000
!endif

# must be to point on NT2K DDK root
!ifndef DDKROOT
DDKROOT=$(TOOLKIT_VOL)NTDDK
!endif

!ifndef BASEDIR
# must be to point on NT4 DDK root
BASEDIR=$(TOOLKIT_VOL)NT4DDK
#BASEDIR=$(LIBS)BaseDDK
!endif

!if "$(TARGET_OS_VER)" == "NT4"
DDKROOT=$(BASEDIR)
!endif

# must be to point on W95 DDK root
!ifndef 9XDDK

!if "$(TARGET_OS_VER)" == "W95"
9XDDK = $(TOOLKIT_VOL)95DDK
!else
9XDDK = $(TOOLKIT_VOL)NTDDK
!endif

!endif

!ifndef BUILD_ALT_DIR
BUILD_ALT_DIR = fre
!endif

DDK_INC_DEST=$(DDKROOT)\inc
DDK_INC_PATH=$(DDK_INC_DEST)\ddk
WDM_INC_PATH=$(DDK_INC_PATH)\wdm
DDK_LIB_DEST=$(DDKROOT)\lib$(BUILD_ALT_DIR)

## =================== DDK specific ======================
!include mak\ddk.mak

## ==================== SDK defines ======================
## short names only!!!
!ifndef INCSDK
#INCSDK=$(TOOLKIT_VOL)PROGRA~1\MICROS~4\INCLUDE
INCSDK="C:\Program Files\Microsoft Visual Studio .NET 2003\Vc7\PlatformSDK\Include"
!endif

!ifndef LIBSDK
LIBSDK=$(TOOLKIT_VOL)PROGRA~1\MICROS~4\LIB
!endif

## =================== DX SDK defines ====================
DXSDKROOT = $(TOOLKIT_VOL)DXSDK\\

!ifndef INCDXSDK
INCDXSDK=$(DXSDKROOT)include;$(DXSDKROOT)samples\Multimedia\DirectShow\BaseClasses;
!endif

!ifndef LIBDXSDK
LIBDXSDK=$(DXSDKROOT)lib
!endif

## =================== DJ GPP defines ====================
!ifndef DJGPP
DJGPP=$(TOOLKIT_VOL)DJGPP/
DJGPP=$(DJGPP:\=/)
!endif

## ============== Null Soft Install System ===============
!ifndef NSIS
NSIS=C:\PROGRA~1\NSIS\\
NSIS=$(NSIS:\\=\)
!endif

## ================== SoftIce location ===================
!ifndef NMS
NMS = $(TOOLKIT_VOL)NMS4.2.7\nmsym.exe
NMP = "G:\Program Files\Compuware\DriverStudio\SoftICE\icepack.exe"
!endif

## ============= MS VC 6.X root install path =============
!ifndef MSVC6
MSVC6=C:\Program Files\Microsoft Visual Studio\VC98\\
MSVC6=$(MSVC6:\\=\)
!endif

## ============= MS VC 7.X root install path =============
!ifndef MSVC7
MSVC7=C:\Program Files\Microsoft Visual Studio .NET 2003\Vc7\\
MSVC7=$(MSVC7:\\=\)
!endif

## ============= MS VC 8.X root install path =============
!ifndef MSVC8
MSVC8=$(WDKROOT)\\
MSVC8=$(MSVC8:\\=\)
!endif

## == Xoreax IncrediBuild distributed C compiling system =
!ifndef IBC
IBC=""
#IBC="C:\Program Files\Xoreax\IncrediBuild\BuildConsole.exe"
!endif

## ====================== TSTL ===========================
!ifndef TSTL
TSTL=..\inc\\
TSTL=$(TSTL:\\=\)
!endif

# STATIC (linked), DYNAMIC (dlls)
LIBTYPE = STATIC

EDITOR = notepad.exe

# Version control system
# VSS, ST, SVN
VCS = SVN
