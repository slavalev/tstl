#################################################################################
#
#  Module Name: 	Makefile
#
#  Author:		Slava I. Levtchenko <slavalev@gmail.com>
#
#  Abstract:    	common Makefile - common variables declaration
#
#  Revision History:	29.07.2002	  started
#						26.09.2002    released (beta)
#
#################################################################################

BIN = .\bin\\
BIN = $(BIN:\\=\)

!include setenv.mak
!include mak\dirs.mak
!include mak\tools\numega.mak

## =============================== C defines ==================================
!if     "$(COMPILER)" == "DJGCC"
!include mak\tools\djgcc.mak
!elseif "$(COMPILER)" == "MSVC6"
!include mak\tools\msvc6.mak
!elseif "$(COMPILER)" == "MSVC7"
!include mak\tools\msvc7.mak
!elseif "$(COMPILER)" == "MSVC8"
!include mak\tools\msvc8.mak
!elseif "$(COMPILER)" == "MSVC8NT4"
!include mak\tools\msvc8nt4.mak
!elseif "$(COMPILER)" == "MSVC8X64"
!include mak\tools\msvc8x64.mak
!else
!include mak\tools\msvc6.mak
!endif

#.SUFFIXES: .asm .as .s $(SUFFIXES)

## ============================== RC defines ==================================
!if     "$(LANG)" == "en"
RCLANG = 0x409
!elseif "$(LANG)" == "ru"
RCLANG = 0x419
!elseif "$(LANG)" == "de"
RCLANG = 0x407
!elseif "$(LANG)" == "kr"
RCLANG = 0x412
!else
RCLANG = 0x409
!endif

OURCFLAGS = -D$(TARGET_OS_VER) -DPRJ_$(PRJ_NAME) -DLANG_$(LANG)

!if "$(DEBUG)" == "1"
OURCFLAGS =$(OURCFLAGS) -DDEBUG
!else
OURCFLAGS =$(OURCFLAGS) -DRELEASE
!endif

DEFLINKFLG = -nologo -incremental:no -nodefaultlib -map
DEFLIBFLG  = -nologo -subsystem:console

!if "$(COMPILER)" != "MSVC8X64"
DEFLINKFLG = -machine:i386 $(DEFLINKFLG)
DEFLIBFLG  = -machine:i386 $(DEFLIBFLG)
!else
DEFLINKFLG = -machine:amd64 $(DEFLINKFLG)
DEFLIBFLG  = -machine:amd64 $(DEFLIBFLG)
!endif

LIBFLG     = $(DEFLIBFLG)

## =========================== LE drivers defines =============================
INCVXD = -I$(MROOT) -I$(CI) -I$(COM_DRV) -I$(COM_VXD_DRV)

!if "$(TARGET_OS_VER)" == "W95"
INCVXD = $(INCVXD) -I$(9XDDK)\inc32 -I$(9XDDK)\inc16
LIBVXD = $(9XDDK)\lib
!elseif "$(TARGET_OS_VER)" == "WME"
INCVXD = $(INCVXD) -I$(9XDDK)\inc\win_me -I$(9XDDK)\inc\win_me\inc16
LIBVXD = $(9XDDK)\lib\i386\free
!else
INCVXD = $(INCVXD) -I$(9XDDK)\inc\win98 -I$(9XDDK)\inc\win98\inc16
LIBVXD = $(9XDDK)\lib\i386\free
!endif

DEFVXD = -DVXD -DBLD_COFF -DIS_32
LINKFLGVXD = $(DEFLINKFLG) -libpath:$(LIBVXD) -vxd -ignore:4078

# -Gs8172 -Gd
CFLGVXD = $(OURCFLAGS) $(DEFVXD) $(INCVXD) -nologo -c -Gs -Gz -Zp1 -W3

MASMFLGVXD = $(OURCFLAGS) $(DEFVXD) $(INCVXD) -DMASM6 -c -nologo -Cx -coff -W3 -WX
RESFLGVXD  = -r -x -DPRJ_$(PRJ_NAME) -DLANG_$(LANG) -i$(CI) -i$(COM_VXD_DRV) -i$(9XDDK)\inc\win98\inc16

## =========================== PE drivers defines =============================
INCSYS = -I$(MROOT) -I$(CI) -I$(COM_DRV) -I$(COM_SYS_DRV)

!if "$(COMPILER)" == "MSVC6"

!if "$(TARGET_OS_VER)" == "NT2K"
INCSYS = $(INCSYS) -I$(DDK_INC_PATH) -I$(DDK_INC_DEST) -I$(WDM_INC_PATH)
DEFSYS = -D_WIN32_WINNT=0x0500 -DWINVER=0x0500 -D_WIN32_IE=0x0500 -DMSC -DPNP_POWER
LINKFLGSYS = -subsystem:native,5.00 -version:5.00 -osversion:5.00 -driver:wdm
!elseif "$(TARGET_OS_VER)" == "NT4"
INCSYS = $(INCSYS) -I$(BASEDIR)\inc
DEFSYS = -D_WIN32_WINNT=0x0400 -DWINVER=0x0400
LINKFLGSYS = -subsystem:native,4.00 -version:4.00 -osversion:4.00 
!endif

# -D_DLL=1 -DCONDITION_HANDLING=1
DEFSYS = $(DEFSYS) -D_X86_=1 -Di386=1 -D_IDWBUILD -DDEVL=1 -DFPO=1 \
	 -DWIN32=100 -DWINNT=1 -DNT_UP=1 -DNT_INST=0 -D_NT1X_=100 \
	 -DWIN32_LEAN_AND_MEAN=1 -DSTD_CALL -DIS_32 -D_ABS_MALLOC

# -TP -EHa
CFLGSYS = $(OURCFLAGS) $(INCSYS) $(DEFSYS) -nologo -c -cbstring \
	  -Gi- -Gm- -Gs8172 -Gy -Gz -GF -GR -GX- -Zel -Zp8 -QIfdiv- -QI0f -W3 -WX

# -opt:ref -opt:icf - templates buggy linking
# -force:multiple -ignore:4001,4037,4039,4044,4049,4065,4070,4078,4087,4089,4096,4198,4210,4217
LINKFLGSYS = $(LINKFLGSYS) $(DEFLINKFLG) -driver -base:0x10000 -entry:DriverEntry@8 \
	 -align:0x20 -fullbuild -optidata -stack:262144,4096 -section:init,d \
	 -merge:_PAGE=PAGE -merge:_TEXT=.text -merge:.rdata=.text

MASMFLGSYS= $(OURCFLAGS) $(INCSYS) -c -nologo -Cx -coff -W3 -WX -DBLD_COFF -DIS_32 -DMASM6

!if "$(TARGET_OS_VER)" == "NT2K"
LIBSYS = $(DDK_LIB_DEST)\i386\\
!elseif "$(TARGET_OS_VER)" == "NT4"
LIBSYS = $(BASEDIR)\lib\i386\free\\
!endif

!else if "$(COMPILER)" == "MSVC8NT4"

!if "$(TARGET_OS_VER)" == "NT2K"
INCSYS = $(INCSYS) -I$(DDK_INC_PATH) -I$(DDK_INC_DEST) -I$(WDM_INC_PATH)
DEFSYS = -D_WIN32_WINNT=0x0500 -DWINVER=0x0500 -D_WIN32_IE=0x0500 -DMSC -DPNP_POWER
LINKFLGSYS = -subsystem:native,5.00 -version:5.00 -osversion:5.00 -driver:wdm
!elseif "$(TARGET_OS_VER)" == "NT4"
INCSYS = $(INCSYS) -I$(BASEDIR)\inc
DEFSYS = -D_WIN32_WINNT=0x0400 -DWINVER=0x0400
LINKFLGSYS = -subsystem:native,4.00 -version:4.00 -osversion:4.00 
!endif

# -D_DLL=1 -DCONDITION_HANDLING=1
DEFSYS = $(DEFSYS) -D_X86_=1 -Di386=1 -D_IDWBUILD -DDEVL=1 -DFPO=1 \
	 -DWIN32=100 -DWINNT=1 -DNT_UP=1 -DNT_INST=0 -D_NT1X_=100 \
	 -DWIN32_LEAN_AND_MEAN=1 -DSTD_CALL -DIS_32 -D_ABS_MALLOC

# -TP -EHa
CFLGSYS = $(OURCFLAGS) $(INCSYS) $(DEFSYS) -nologo -c -cbstring \
	 -EHs-c- -Gm- -Gy -Gz -Gs8172 -Zp8 -GF -GR- -GS- -W3 -WX

# -section:init,d -force:multiple -ignore:4001,4037,4039,4044,4049,4065,4070,4078,4087,4089,4096,4198,4210,4217
LINKFLGSYS = $(LINKFLGSYS) $(DEFLINKFLG) -driver -base:0x10000 \
	 -entry:DriverEntry@8 -align:0x20 -stack:262144,4096 -fullbuild \
	 -merge:_PAGE=PAGE -merge:_TEXT=.text -merge:.rdata=.text

MASMFLGSYS= $(OURCFLAGS) $(INCSYS) -c -nologo -Cx -coff -W3 -WX -DBLD_COFF -DIS_32 -DMASM6

!if "$(TARGET_OS_VER)" == "NT2K"
LIBSYS = $(DDK_LIB_DEST)\i386\\
!elseif "$(TARGET_OS_VER)" == "NT4"
LIBSYS = $(BASEDIR)\lib\i386\free\\
!endif

!else if "$(COMPILER)" == "MSVC8"

INCSYS = -I$(WDKROOT)\inc\ddk $(INCSYS)

DEFSYS = -Di386=1 -D_IDWBUILD -DDEVL=1 -DFPO=1 \
	 -D_WIN32_WINNT=0x0501 -DWINVER=0x0501 -D_WIN32_IE=0x0501 \
	 -DWIN32=100 -DWINNT=1 -DNT_UP=1 -DNT_INST=0 -D_NT1X_=100 \
	 -DWIN32_LEAN_AND_MEAN=1 -DNTDDI_VERSION=0x05010200 \
	 -D_X86_=1 -D_M_IX86=1

CFLGSYS = $(OURCFLAGS) $(INCSYS) $(DEFSYS) -nologo -c -cbstring \
	 -GF -GR- -Gm- -Gs8172 -Gy -Gz -Zp8 -W3 -WX -EHs-c- -GS-

# -section:init,d -force:multiple -ignore:4001,4037,4039,4065,4070,4078,4087,4089,4096
LINKFLGSYS = $(DEFLINKFLG) -driver -base:0x10000 -entry:DriverEntry -align:0x80 \
	 -fullbuild -subsystem:native,5.10 -version:5.10 -osversion:5.10 \
	 -merge:_PAGE=PAGE -merge:_TEXT=.text -merge:.rdata=.text
	 

MASMFLGSYS= $(OURCFLAGS) $(INCSYS) -c -nologo -Cx -W3 -WX -DBLD_COFF -DMASM6 -DIS_32 -D_X86_=1

LIBSYS    = $(WDKROOT)\lib\wxp\i386\\
MASM      = "$(WDKROOT)\bin\x86\ml64.exe"

!else if "$(COMPILER)" == "MSVC8X64"

INCSYS = $(INCSYS) -I$(WDKROOT)\inc\ddk

DEFSYS = -Di386=1 -D_IDWBUILD -DDEVL=1 -DFPO=1 \
	 -D_WIN32_WINNT=0x0501 -DWINVER=0x0501 -D_WIN32_IE=0x0501 \
	 -DWIN32=100 -DWINNT=1 -DNT_UP=1 -DNT_INST=0 -D_NT1X_=100 \
	 -DWIN32_LEAN_AND_MEAN=1 -DNTDDI_VERSION=0x05010200 \
	 -D_AMD64_=1 -D_M_AMD64 -DAMD64 -D_WIN64 -DWIN64

CFLGSYS = $(OURCFLAGS) $(INCSYS) $(DEFSYS) -nologo -c -cbstring \
	 -GF -GR- -Gm- -Gs8172 -Gy -Gz -Zp8 -W3 -WX -EHs-c- -GS-

# -section:init,d -force:multiple -ignore:4001,4037,4039,4065,4070,4078,4087,4089,4096
LINKFLGSYS = $(DEFLINKFLG) -driver -base:0x10000 -entry:DriverEntry -align:0x80 \
	 -fullbuild -subsystem:native,5.10 -version:5.10 -osversion:5.10 \
	 -merge:_PAGE=PAGE -merge:_TEXT=.text -merge:.rdata=.text

MASMFLGSYS= $(OURCFLAGS) $(INCSYS) -c -nologo -Cx -W3 -WX -DBLD_COFF -DMASM6 \
	    -DIS_64 -D_AMD64_=1 -D_M_AMD64 -DAMD64 -D_WIN64 -DWIN64

LIBSYS    = $(WDKROOT)\lib\wnet\amd64\\
MASM      = "$(WDKROOT)\bin\x86\amd64\ml64.exe"
!else
!endif

LIBFLGSYS = $(DEFLIBFLG) -ignore:4001,4037,4039,4065,4070,4078,4087,4089,4096
RESFLGSYS = -l $(RCLANG) -r $(OURCFLAGS) $(DEFSYS) $(INCSYS)

## ============================= LE defines ===================================
INCLE = -I$(MROOT) -I$(CI)

CFLGLE = -DPRJ_$(PRJ_NAME) -DLANG_$(LANG) $(INCLE) -c
LINKFLGLE =
LIBFLGLE = crs

## ============================= DLL defines ==================================
INCDLL = -I$(MROOT) -I$(CI) 

CFLGDLL = $(OURCFLAGS) $(INCDLL) -nologo -c -M$(LIBTYPEFLG) -W3 -WX -GR- -EHs-c- \
	  -D"WIN32" -D"_WIN32" -D"_MBCS"

!if "$(COMPILER)" != "MSVC8X64"
CFLGDLL = $(CFLGDLL) -D"_X86_"=1
!else
CFLGDLL = $(CFLGDLL) -D"_AMD64_"=1 -D"_M_AMD64" -D"AMD64" -D"_WIN64" -D"WIN64" -GS-
!endif

LINKFLGDLL = $(DEFLINKFLG) -subsystem:windows -dll
RESFLGDLL  = -l $(RCLANG) -r $(OURCFLAGS) $(INCDLL)

## ========================== CLI and Services ================================
CFLGSRV = $(CFLGDLL) -D"_CONSOLE"
LINKFLGSRV = $(DEFLINKFLG) -subsystem:console
RESFLGSRV  = $(RESFLGDLL)

## ============================= GUI defines ==================================

CFLGGUI = $(CFLGDLL) -D"_WINDOWS"
LINKFLGGUI = $(DEFLINKFLG) -subsystem:windows
RESFLGGUI  = $(RESFLGDLL)
TLBFLG     = -nologo -mktyplib203 -win32

## ======================== Applets debug macros ==============================
!if "$(DEBUG)" == "1"

CFLGDLL = $(CFLGDLL) -D"_DEBUG" -Oid -Z7
CFLGGUI = $(CFLGGUI) -D"_DEBUG" -Oid -Z7
CFLGSRV = $(CFLGSRV) -D"_DEBUG" -Oid -Z7
CFLGSYS = $(CFLGSYS) -D"_DEBUG" -Oid -Z7
CFLGVXD = $(CFLGVXD) -D"_DEBUG" -Oid -Z7
CFLGLE  = $(CFLGLE) -g -DDEBUG

LINKFLGDLL = $(LINKFLGDLL) -debug -debug:full -debugtype:cv
LINKFLGEXE = $(LINKFLGEXE) -debug -debug:full -debugtype:cv
LINKFLGGUI = $(LINKFLGGUI) -debug -debug:full -debugtype:cv
LINKFLGSRV = $(LINKFLGSRV) -debug -debugtype:cv
LINKFLGVXD = $(LINKFLGVXD) -debug -debug:full -debugtype:cv
LINKFLGLE  = $(LINKFLGLE) -g

LIBFLGSYS  = $(LIBFLGSYS)  -debugtype:cv
LIBFLG     = $(LIBFLG)     -debugtype:cv

MASMFLGSYS = $(MASMFLGSYS) -D"_DEBUG" -Zd -Zi
MASMFLGVXD = $(MASMFLGVXD) -D"_DEBUG" -Zd -Zi 

RESFLGDLL  = $(RESFLGDLL)  -D"_DEBUG"
RESFLGGUI  = $(RESFLGGUI)  -D"_DEBUG"
RESFLGSRV  = $(RESFLGSRV)  -D"_DEBUG"
RESFLGSYS  = $(RESFLGSYS)  -D"_DEBUG"
!else
CFLGDLL = $(CFLGDLL) -D"NDEBUG" -Oid
CFLGGUI = $(CFLGGUI) -D"NDEBUG" -O2
CFLGSRV = $(CFLGSRV) -D"NDEBUG" -Oid -Z7
CFLGSYS = $(CFLGSYS) -D"NDEBUG" -Oid -Z7
CFLGVXD = $(CFLGVXD) -D"NDEBUG" -Oid
CFLGLE  = $(CFLGLE) -s -O2

LINKFLGDLL = $(LINKFLGDLL) -release -debug:none
LINKFLGEXE = $(LINKFLGEXE) -release -debug:none
LINKFLGGUI = $(LINKFLGGUI) -release -debug:none
#LINKFLGSRV = $(LINKFLGSRV) -release -debug:none
LINKFLGVXD = $(LINKFLGVXD) -release -debug:none
LINKFLGLE  = $(LINKFLGLE) -s

MASMFLGSYS = $(MASMFLGSYS) -D"NDEBUG"
MASMFLGVXD = $(MASMFLGVXD) -D"NDEBUG"

RESFLGDLL  = $(RESFLGDLL)  -D"NDEBUG"
RESFLGGUI  = $(RESFLGGUI)  -D"NDEBUG"
RESFLGSRV  = $(RESFLGSRV)  -D"NDEBUG"
RESFLGSYS  = $(RESFLGSYS)  -D"NDEBUG"
!endif

LINKFLGSRV = $(LINKFLGSRV) -debug -debugtype:cv
LINKFLGSYS = $(LINKFLGSYS) -debug -debugtype:cv
