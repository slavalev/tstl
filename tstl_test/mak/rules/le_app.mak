#################################################################################
#
#  Module Name:		Makefile
#
#  Author:			Slava I. Levtchenko <slavalev@gmail.com>
#
#  Abstract:		common Makefile - implicit building rules for LE applications
#
#  Revision History:	29.07.2002	  started
#						26.09.2002    released (beta)
#
#################################################################################

{$(MROOT)}.c{$(OBJS)}.obj:
	@set DJGPP=$(DJGPP)DJGPP.ENV
	@set PATH=$(DJGPP)BIN;$(PATH)
	@set C_INCLUDE_PATH=$(C_INCLUDE_PATH)
	$(CC) $(CFLGLE) $< -o $@

{$(MROOT)}.cpp{$(OBJS)}.obj:
	@set DJGPP=$(DJGPP)DJGPP.ENV
	@set PATH=$(DJGPP)BIN;$(PATH)
	@set CPLUS_INCLUDE_PATH=$(CPLUS_INCLUDE_PATH)
	$(CC) $(CFLGLE) $< -o $@

{$(MROOT)}.as{$(OBJS)}.obj:
	@set DJGPP=$(DJGPP)DJGPP.ENV
	@set PATH=$(DJGPP)BIN;$(PATH)
	$(GASM) $(GASMFLG) -o $@ $**

$(OBJS)link.tmp: $(OBJECTS) $(LIBLIST)
	echo LOAD $(OBJECTS:.obj =.obj, ) > $(OBJS)link.tmp
	if not "$(LIBLIST)" == "" echo LOAD $(LIBLIST:.a =.a, ) >> $(OBJS)link.tmp

$(OBJS)lib.tmp: $(OBJECTS)
	echo LOAD $(OBJECTS:.obj =.obj, ) > $(OBJS)lib.tmp

$(OUT)$(MODULE).a: $(OBJS)lib.tmp $(OBJECTS)
	$(LIBTOOL) $(LIBFLGLE) -M $(OBJS)lib.tmp -o $@
	del $(OBJECTS)

$(OUT)$(MODULE).exe: $(OBJS)link.tmp $(OBJECTS)
	$(LINK) -Wl$(LINKFLGLE: =,),-c$(OBJS)link.tmp -o $@ $(OSLIBLIST)
	del $(OBJECTS)

!include $(OUT)Makefile.dep
