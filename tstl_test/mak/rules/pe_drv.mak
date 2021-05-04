#################################################################################
#
#  Module Name:		Makefile
#
#  Author:			Slava I. Levtchenko <slavalev@gmail.com>
#
#  Abstract:		common Makefile - implicit building rules for PE drivers
#
#  Revision History:	29.07.2002	  started
#						26.09.2002    released (beta)
#
#################################################################################

!if "$(SOURCES)" != ""

$(OBJECTS): $(SOURCES)
	@if not exist $(OBJS). mkdir $(OBJS).
	echo $(SOURCES:\=/)>$(OBJS)source.files
	$(BIN)gawk -f $(SCR)ibc\ibc.awk $(OBJS)source.files >$(OBJS)files.dsp
	$(BIN)sed -e "s/ibc/$(MODULE)/g" $(SCR)ibc\ibc_head.dsp >$(MODULE).dsp
	type $(OBJS)files.dsp >>$(MODULE).dsp
	set OBJS=$(OBJS)
	$(IBC) $(MODULE).dsp /CL_ADD="$(CFLGSYS)" /CL_REM="ML" /NOLOGO /NOLINK /USEENV /SHOWCMD /SHOWTIME /SHOWAGENT /BEEP /TITLE="$(MODULE) building..."
!else
{$(MROOT)}.c{$(OBJS)}.obj:
	@if not exist $(OBJS). mkdir $(OBJS).
	@set INCLUDE=$(INCLUDE)
	$(CC) $(CFLGSYS) $< -Fo$@

{$(MROOT)}.cpp{$(OBJS)}.obj:
	@if not exist $(OBJS). mkdir $(OBJS).
	@set INCLUDE=$(INCLUDE)
	$(CC) $(CFLGSYS) $< -Fo$@
!endif

{$(MROOT)}.rc{$(OBJS)}.res:
	@if not exist $(OBJS). mkdir $(OBJS).
	@set INCLUDE=$(INCLUDE)
	$(RSC) $(RESFLGSYS) $<
	move $(MROOT)\*.res $@

{$(MROOT)}.asm{$(OBJS)}.obj:
	@if not exist $(OBJS). mkdir $(OBJS).
	$(MASM) $(MASMFLGSYS) -Fo$@ $<

$(OUT)$(MODULE).lib: $(OBJECTS)
	$(LIBTOOL) $(LIBFLGSYS) -out:$@ $**

$(OUT)$(MODULE).dll: $(OBJECTS) $(RES) $(LIBLIST)
	$(LINK) $(LINKFLGSYS) $(DEF) -out:$@ $** $(OSLIBLIST)
	if exist ..\signer.bat ..\signer.bat $@ "$(PRJ_NAME): $(MODULE).dll" $(DEBUG) $(WDKROOT:\=\\) $(MODULE) dll $(OUT:\=\\)

#!if "$(DEBUG)" == "1"
#	-if exist $(NMS) $(NMS) $(NMSOPT) $@
#!endif

$(OUT)$(MODULE).sys: $(OBJECTS) $(RES) $(LIBLIST)
	$(LINK) $(LINKFLGSYS) $(DEF) -out:$@ $** $(OSLIBLIST)
	if exist ..\signer.bat ..\signer.bat $@ "$(PRJ_NAME): $(MODULE).sys" $(DEBUG) $(WDKROOT:\=\\) $(MODULE) sys $(OUT:\=\\)

#!if "$(DEBUG)" == "1"
#	-if exist $(NMS) $(NMS) $(NMSOPTSYS) $@
#!endif

!include $(OUT)Makefile.dep
