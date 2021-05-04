#################################################################################
#
#  Module Name:		Makefile
#
#  Author:			Slava I. Levtchenko <slavalev@gmail.com>
#
#  Abstract:		common Makefile - implicit building rules for LE drivers
#
#  Revision History:	29.07.2002	  started
#						26.09.2002    released (beta)
#
#################################################################################

{$(MROOT)}.c{$(OBJS)}.obj:
	@if not exist $(OBJS). mkdir $(OBJS).
	@set INCLUDE=$(INCLUDE)
	$(CC) $(CFLGVXD) $< -Fo$@

{$(MROOT)}.cpp{$(OBJS)}.obj:
	@if not exist $(OBJS). mkdir $(OBJS).
	@set INCLUDE=$(INCLUDE)
	$(CC) $(CFLGVXD) $< -Fo$@

{$(MROOT)}.rc{$(OBJS)}.res:
	@if not exist $(OBJS). mkdir $(OBJS).
	@set INCLUDE=$(INCLUDE)
	$(RSC16) $(RESFLGVXD) $<
	move $(MROOT)\*.res $@

{$(MROOT)}.asm{$(OBJS)}.obj:
	@if not exist $(OBJS). mkdir $(OBJS).
	$(MASM) $(MASMFLGVXD) -Fo$@ $<

$(OUT)$(MODULE).lib: $(OBJECTS)
	$(LIBTOOL) $(LIBFLG) -out:$@ $**

$(OUT)$(MODULE).vxd: $(OBJECTS) $(LIBLIST)
	$(LINK) $(LINKFLGVXD) -out:$@ $** $(OSLIBLIST)
	$(ADDRES2VXD) $@ $(RES)
!if "$(DEBUG)" == "1"
	-if exist $(NMS) $(NMS) $(NMSOPTVXD) $@
	if exist $(OUT)$(MODULE).pdb del $(OUT)$(MODULE).pdb
!endif

!include $(OUT)Makefile.dep
