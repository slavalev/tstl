#################################################################################
#
#  Module Name:		Makefile
#
#  Author:			Slava I. Levtchenko <slavalev@gmail.com>
#
#  Abstract:		common Makefile - MS VC v.6.0 compiler variables
#
#  Revision History:	29.07.2002	  started
#						26.09.2002    released (beta)
#
#################################################################################

CC = cl.exe
CXX = $(CC)
RSC = rc.exe
MTL = midl.exe
LINK = link.exe
LIBTOOL = link.exe -lib

## ================== Compiler specific ==================

PATH   =$(MSVC6)bin;$(MSVC6)..\common\MSDev98\bin;$(PATH)
INCLUDE=$(MSVC6)atl\include;
INCLUDE=$(MSVC6)include;$(INCLUDE)

LIB=$(MSVC6)mfc\lib;$(LIB)
LIB=$(MSVC6)lib;$(LIB)

!if "$(DEBUG)" == "1"
DEBUGSUFFIX =d
!else
DEBUGSUFFIX =
!endif

!if "$(LIBTYPE)" == "STATIC"
LIBTYPEFLG = T$(DEBUGSUFFIX)
LIBTYPELIB = libcmt$(DEBUGSUFFIX).lib libcpmt$(DEBUGSUFFIX).lib
!elseif "$(LIBTYPE)" == "DYNAMIC"
LIBTYPEFLG = D$(DEBUGSUFFIX) -D_DLL
LIBTYPELIB = msvcrt$(DEBUGSUFFIX).lib msvcprt$(DEBUGSUFFIX).lib
!endif
