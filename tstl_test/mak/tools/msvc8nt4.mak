#################################################################################
#
#  Module Name:		Makefile
#
#  Author:			Slava I. Levtchenko <slavalev@gmail.com>
#
#  Abstract:		common Makefile - MS VC v.8.0 compiler variables
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
PATH   =$(MSVC8)bin\x86\x86;$(MSVC8)bin\x86;$(PATH)
INCLUDE=$(MSVC8)inc\api;$(INCLUDE)
INCLUDE=$(MSVC8)inc\crt;$(INCLUDE)

LIB=$(MSVC6)lib;$(LIB)
LIB=$(MSVC8)lib\w2k\i386;$(LIB)

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
