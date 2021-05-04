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
INCLUDE=$(MSVC8)inc\inc16;
INCLUDE=$(MSVC8)inc\atl30;$(INCLUDE)
INCLUDE=$(MSVC8)inc\api;$(INCLUDE)
INCLUDE=$(MSVC8)inc\crt;$(INCLUDE)

LIB=$(MSVC8)lib\lib16;$(LIB)
LIB=$(MSVC8)lib\atl\i386;$(LIB)
LIB=$(MSVC8)lib\wnet\i386;$(LIB)
LIB=$(MSVC8)lib\crt\i386;$(LIB)

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
