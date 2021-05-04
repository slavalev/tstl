#################################################################################
#
#  Module Name:		Makefile
#
#  Author:			Slava I. Levtchenko <slavalev@gmail.com>
#
#  Abstract:		common Makefile - implicit building rules for GUI applications
#
#  Revision History:	29.07.2002	  started
#						26.09.2002    released (beta)
#
#################################################################################

# Xoreax IncrediBuild distributed C compiling system
!if "$(SOURCES)" != ""

$(OBJECTS): $(SOURCES)
	@if not exist $(OBJS). mkdir $(OBJS).
	echo $(SOURCES:\=/)>$(OBJS)source.files
	$(BIN)gawk -f $(SCR)ibc\ibc.awk $(OBJS)source.files >$(OBJS)files.dsp
	$(BIN)sed -e "s/ibc/$(MODULE)/g" $(SCR)ibc\ibc_head.dsp >$(MODULE).dsp
	type $(OBJS)files.dsp >>$(MODULE).dsp
	set OBJS=$(OBJS)
	$(IBC) $(MODULE).dsp /CL_ADD="$(CFLGGUI)" /NOLOGO /NOLINK /USEENV /SHOWCMD /SHOWTIME /SHOWAGENT /BEEP /TITLE="$(MODULE) building..."
!else
{$(MROOT)}.c{$(OBJS)}.obj:
	@if not exist $(OBJS). mkdir $(OBJS).
	@set INCLUDE=$(INCLUDE)
	$(CC) $(CFLGGUI) $< -Fo$@

{$(MROOT)}.cpp{$(OBJS)}.obj:
	@if not exist $(OBJS). mkdir $(OBJS).
	@set INCLUDE=$(INCLUDE)
	$(CC) $(CFLGGUI) $< -Fo$@
!endif

{$(MROOT)}.rc{$(OBJS)}.res:
	@if not exist $(OBJS). mkdir $(OBJS).
	@set INCLUDE=$(INCLUDE)
	$(RSC) $(RESFLGGUI) $<
	move $(MROOT)\*.res $@

{$(MROOT)}.odl{$(OBJS)}.tlb:
	@if not exist $(OBJS). mkdir $(OBJS).
	$(MTL) $(TLBFLG) -tlb $@ $<

$(OUT)$(MODULE).lib: $(OBJECTS)
	$(LIBTOOL) $(LIBFLG) -out:$@ $**

$(OUT)$(MODULE).dll: $(OBJECTS) $(RES) $(LIBLIST)
	@set LIB=$(LIB)
	$(LINK) $(LINKFLGDLL) -out:$@ $** $(OSLIBLIST)

#!if "$(DEBUG)" == "1"
#	-if exist $(NMS) $(NMS) $(NMSOPT) $@
#!endif

$(OUT)$(MODULE).ocx: $(OBJECTS) $(RES) $(LIBLIST)
	@set LIB=$(LIB)
	$(LINK) $(LINKFLGDLL) -out:$@ $** $(OSLIBLIST)

#!if "$(DEBUG)" == "1"
#	-if exist $(NMS) $(NMS) $(NMSOPT) $@
#!endif

$(OUT)$(MODULE).exe: $(OBJECTS) $(RES) $(LIBLIST) 
	@set LIB=$(LIB)
	$(LINK) $(LINKFLGGUI) -out:$@ $** $(OSLIBLIST)

#!if "$(DEBUG)" == "1"
#	-if exist $(NMS) $(NMS) $(NMSOPT) $@
#!endif

!include $(OUT)Makefile.dep
