#################################################################################
#
#  Module Name:		Makefile
#
#  Author:			Slava I. Levtchenko <slavalev@gmail.com>
#
#  Abstract:		common Makefile - MS VC v.7.0 compiler variables
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
PATH   =$(MSVC7)bin;$(MSVC7)..\Common7\IDE;$(PATH)
INCLUDE=$(MSVC7)PlatformSDK\include;
INCLUDE=$(MSVC7)atlmfc\include;$(INCLUDE)
INCLUDE=$(MSVC7)include;$(INCLUDE)

LIB=$(MSVC7)PlatformSDK\lib;$(LIB)
LIB=$(MSVC7)atlmfc\lib;$(LIB)
LIB=$(MSVC7)lib;$(LIB)

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
