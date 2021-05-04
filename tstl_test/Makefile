#################################################################################
#
#  Module Name:		Makefile
#
#  Author:			Slava I. Levtchenko <slavalev@gmail.com>
#
#  Abstract:		common Makefile - building projects starts here
#
#  Revision History: 29.07.2002	  started
#					 26.09.2002   released (beta)
#
#################################################################################

!include begin.mak

# Main targets

# all: output vcs clean_exe drv dll app installer finish
all: clean_exe build_app finish

build: output clean_exe drv dll app installer finish

simplebuild: output clean_exe drv dll app finish

LE_EXE =

PE_EXE = $(OUT)$(PRJ_NAME).exe $(OUT)$(PRJ_NAME)64.exe

NSIS_DLL = 

EXE = $(PE_EXE)

DLL64 = 

DLL = $(NSIS_DLL) $(DLL64)

SYS = 

VXD = 

ZIP = ..\$(PRJ_NAME)_src.zip ..\$(PRJ_NAME)_utils.zip ..\$(PRJ_NAME)_bin.zip \
	..\$(PRJ_NAME)_sym.zip

RAR = ..\$(PRJ_NAME)_src.rar ..\$(PRJ_NAME)_utils.rar

NSIS_MODS =

INF = 

LIB_MODS  = 

app: output $(EXE)

dll: output $(DLL) upload_debug

dll64: output $(DLL64) upload_debug

src_zip: clean_zip $(ZIP) upload_zip

src_rar: clean_rar $(RAR) upload_rar

!if "$(TARGET_OS_VER)" == "NT4" || "$(TARGET_OS_VER)" == "NT2K"
# $(VXD) clean_nms 
drv: output $(SYS)
!else
# clean_nms $(VXD)
drv: output $(SYS) 
!endif

# Exotic targets

bpe_drv: output $(SYS) finish upload_debug
ble_drv: output $(VXD) finish upload_debug
pe_drv:  output clean_nms $(SYS) finish upload_debug
le_drv:  output clean_nms $(VXD) finish upload_debug
pe_app:  output $(PE_EXE) finish
le_app:  output $(LE_EXE) finish

console: output clean_exe $(OUT)$(PRJ_NAME).exe $(OUT)$(PRJ_NAME)64.exe finish
libs:    output $(LIB_MODS) finish
update_history: output $(DOC)History.txt
tstl:    clean_zip ..\TSTL.zip

# exe_pack
installer: drv $(EXE) $(DLL) $(NSIS_MODS) export_to_nsi
	$(NSIS)\makensis.exe $(SCR)install.nsi >$(OUT)nsis.log
	if exist ..\signer.bat ..\signer.bat ..\$(PRJ_NAME)Installer.exe "$(PRJ_NAME) Installer package" $(DEBUG) $(WDKROOT:\=\\) $(PRJ_NAME)Installer exe $(OUT:\=\\)

dox doxygen: doxoutput $(OUTDOX)doxygen.cfg
	doxygen.exe $(CFGS)doxyprj.cfg >$(OUTDOX)doxygen.log

# Clean section
clean: clean_all

clean_nms:
	@echo Clean public symbols...
	@if exist $(OUT)$(PRJ_NAME).pdb del $(OUT)$(PRJ_NAME).pdb
	@if exist $(OUT)$(PRJ_NAME).nms del $(OUT)$(PRJ_NAME).nms

clean_lib:
	@echo Clean libraries...
	@if exist $(OUT)*.lib del $(OUT)*.lib

clean_exe:
	@echo Clean binaries...
        @if exist $(OUT)*.ax  del $(OUT)*.ax
	@if exist $(OUT)*.exe del $(OUT)*.exe
	@if exist $(OUT)*.dll del $(OUT)*.dll
	@if exist $(OUT)*.sys del $(OUT)*.sys
	@if exist $(OUT)*.vxd del $(OUT)*.vxd
	@if exist $(OUT)*.pdb del $(OUT)*.pdb
	@if exist $(OUT)*.nms del $(OUT)*.nms

clean_zip:
	if exist ..\TSTL.zip             del ..\TSTL.zip
	if exist ..\$(PRJ_NAME)_src.zip   del ..\$(PRJ_NAME)_src.zip
	if exist ..\$(PRJ_NAME)_utils.zip del ..\$(PRJ_NAME)_utils.zip
	if exist ..\$(PRJ_NAME)_bin.zip   del ..\$(PRJ_NAME)_bin.zip
	if exist ..\$(PRJ_NAME)_sym.zip   del ..\$(PRJ_NAME)_sym.zip
	if exist ..\$(PRJ_NAME)_sdk.zip   del ..\$(PRJ_NAME)_sdk.zip

clean_rar:
	if exist ..\TSTL.rar             del ..\TSTL.rar
	if exist ..\$(PRJ_NAME)_src.rar   del ..\$(PRJ_NAME)_src.rar
	if exist ..\$(PRJ_NAME)_utils.rar del ..\$(PRJ_NAME)_utils.rar

clean_all:
	-if exist $(OUTROOT) deltree /Y $(OUTROOT) 2>nul
	-if exist $(OUTROOT) rmdir /S /Q $(OUTROOT)
	if exist $(OUT)Makefile.dep attrib -R $(OUT)Makefile.dep
	if exist $(OUT)Makefile.dep del $(OUT)Makefile.dep

# Auxalary procedures section
output:
	if not exist $(OUTROOT). mkdir $(OUTROOT).
	if not exist $(OUT).     mkdir $(OUT).
	if not exist $(OBJS).    mkdir $(OBJS).
	@echo Start recompile at: >$(OUT)time.log
	@echo. |time >>$(OUT)time.log

finish:
	@echo Finish recompile at: >>$(OUT)time.log
	@echo. |time >>$(OUT)time.log

upload_zip:
	cd ..
	if exist Upload.bat call Upload.bat $(PRJ_NAME) $(DEBUG) zip
	cd $(PRJ_NAME)

upload_rar:
	cd ..
	if exist Upload.bat call Upload.bat $(PRJ_NAME) $(DEBUG) rar
	cd $(PRJ_NAME)

upload: upload_zip upload_rar

upload_release: $(DOC)History.txt src_zip
	cd ..
!if "$(DEBUG)"=="0"
	if exist UploadRelease.bat call UploadRelease.bat $(PRJ_NAME) $(DEBUG) zip $(OUT:\=\\)
	-if exist ExportVersion2SourceControl.bat call ExportVersion2SourceControl.bat $(PRJ_NAME) $(VCS)
!endif
	cd $(PRJ_NAME)

upload_release_simple: src_zip
	cd ..
!if "$(DEBUG)"=="0"
	if exist UploadRelease.bat call UploadRelease.bat $(PRJ_NAME) $(DEBUG) zip $(OUT:\=\\)
	-if exist ExportVersion2SourceControl.bat call ExportVersion2SourceControl.bat $(PRJ_NAME) $(VCS)
!endif
	cd $(PRJ_NAME)

upload_test:
	cd ..
	if exist UploadTest.bat call UploadTest.bat $(PRJ_NAME)
	cd $(PRJ_NAME)

$(DOC)History.txt: $(DOC)Hist_top.txt $(DOC)History.src
	copy /B /Y $(DOC)Hist_top.txt + $(DOC)History.src $@

$(DOC)History.src: $(OUT)Message.txt
	copy /B /Y $(OUT)Message.txt + $@ $(OUT)Message.txt
	move /Y    $(OUT)Message.txt $@

$(OUT)Message.txt:
	$(BIN)vcs.exe -s $(CI)prj_ver.h @$(DOC)History.vcs >$@
	-$(EDITOR) $@
	copy /B /Y $@ $(DOC)LastMessage.txt

upload_debug:
!if "$(DEBUG)" == "1"
	cd ..
	if exist UploadDebug.bat call UploadDebug.bat $(PRJ_NAME) $(OUT:.\=\) x64
	cd $(PRJ_NAME)
!endif

#	$(BIN)vcs.exe -p $(CI)prj_ver.h $(COM_DRV)global.inc
vcs:
!if "$(DEBUG)" == "0"
	if exist $(CI)prj_ver.h attrib -R $(CI)prj_ver.h
	$(BIN)vcs.exe -o $(CI)prj_ver.h
!else
	$(BIN)vcs.exe -v $(CI)prj_ver.h
!endif

export_to_nsi: $(CI)prj_ver.h
	echo !define OUT   "..\$(OUT)" >$(SCR)makedefs.nsi
	echo !define BIN  "..\$(BIN)" >>$(SCR)makedefs.nsi
	echo !define LANG  "$(LANG)"  >>$(SCR)makedefs.nsi
	echo !define DEBUG "$(DEBUG)" >>$(SCR)makedefs.nsi
	echo !define DJGPP "$(DJGPP)" >>$(SCR)makedefs.nsi
	echo !define PRJ_NAME "$(PRJ_NAME)" >>$(SCR)makedefs.nsi
	echo !define EDK   "$(EDK)"   >>$(SCR)makedefs.nsi
	$(BIN)sed -f $(SCR)cpp2nsi.sed $** >$(OUT)version.nsi

doxoutput:
	if not exist $(OUTROOT). mkdir $(OUTROOT).
	-if exist $(OUTDOX) deltree /Y $(OUTDOX) 2>nul
	-if exist $(OUTDOX) rmdir /S /Q $(OUTDOX)
	if not exist $(OUTDOX) mkdir $(OUTDOX)

$(OUTDOX)doxygen.cfg: doxoutput
	$(BIN)vcs.exe -s $(CI)prj_ver.h @$(CFGS)doxygen.vcs >$@
	echo PROJECT_NAME=$(PRJ_NAME) >>$@
	echo CHM_FILE=..\$(PRJ_NAME).chm >>$@

exe_pack: $(EXE) $(GUI_DLL)
	-$(BIN)upx.exe $**
	-$(BIN)Patcher.exe $** n UPX! "UPX|"

# Testing section
test:   install
	-sync
	stop
!if "$(DEBUG)" == "1"
	if exist $(OUT)$(PRJ_NAME).nms $(NMS) /SYM:$(OUT)$(PRJ_NAME).nms;
!endif
	-net start $(PRJ_NAME)

stop:
	-net stop  $(PRJ_NAME)

test_installer:
	..\$(PRJ_NAME)Installer.exe

install:
	if exist $(SCR)$(PRJ_NAME).inf copy $(SCR)$(PRJ_NAME).inf $(OUT)$(PRJ_NAME).inf /Y
	cd $(OUT)
	rem $(WINDIR)\system32\rundll32.exe setupapi,InstallHinfSection DefaultInstall 132 $(PRJ_NAME).inf
	cd ..\..
!if "$(TARGET_OS_VER)" == "W98"
	rem if exist $(OUT)$(PRJ_NAME).vxd copy $(OUT)$(PRJ_NAME).vxd $(WINDIR)\system\IOSUBSYS\$(PRJ_NAME).vxd  /Y
!endif
!if "$(TARGET_OS_VER)" == "NT4"
	if exist $(OUT)$(PRJ_NAME).sys copy $(OUT)$(PRJ_NAME).sys $(WINDIR)\system32\drivers\$(PRJ_NAME).sys /Y
	if exist $(SCR)$(PRJ_NAME).reg $(WINDIR)\regedit.exe /s $(SCR)$(PRJ_NAME).reg
!if "$(DEBUG)" == "1"
	$(NMP)
!endif
!endif

uninstall:
	net stop $(PRJ_NAME)
	if exist $(WINDIR)\system\IOSUBSYS\$(PRJ_NAME).vxd  del $(WINDIR)\system\IOSUBSYS\$(PRJ_NAME).vxd
	if exist $(WINDIR)\system32\drivers\$(PRJ_NAME).sys del $(WINDIR)\system32\drivers\$(PRJ_NAME).sys

loadnms:
!if "$(DEBUG)" == "1"
	if exist $(OUT)$(PRJ_NAME).nms $(NMS) /SYM:$(OUT)$(PRJ_NAME).nms;
!endif

build_app:
	cd $(TSTL)impl
	$(MAKE) -nologo
	cd ..\..\..
	cd $(APP)
	$(MAKE) -nologo
	cd ..

# Module list root rules 
$(OUT)$(PRJ_NAME).exe: $(OUT)Makefile.dep
	$(MAKE) -nologo -f $(APP)Makefile.32 $@

$(OUT)$(PRJ_NAME)64.exe: $(OUT)Makefile.dep
	$(MAKE) -nologo -f $(APP)Makefile.64 $@

EXCLUDE = *.ax *.aps *.bak *.cab *.dll *.dep *.deb* *.err *.exe *.exp \
	  *.log *.map *.ncb *.ncp *.nms *.obj *.ocx *.opt *.pdb *.pe \
	  *.rel* *.res *.scc .svn *.sys *.tlb *.tmp *.zip *.vxd $(PRJ_NAME)\$(OUT)*.inf

..\TSTL.zip:
	if exist ..\TSTL.zip attrib -R ..\TSTL.zip
	cd lib
	..\$(BIN)zip.exe -r9 TSTL.zip TSTL -x *.scc
	move /Y TSTL.zip ..\..
	-sync
	cd ..

..\$(PRJ_NAME)_src.zip:
	cd ..
	if exist $(PRJ_NAME)_src.zip attrib -R $(PRJ_NAME)_src.zip
	if exist $(PRJ_NAME)_src.zip del       $(PRJ_NAME)_src.zip
	$(PRJ_NAME)\$(BIN)zip.exe -r9 $(PRJ_NAME)_src.zip $(PRJ_NAME) -x $(EXCLUDE)
	-sync
	cd $(PRJ_NAME)

..\$(PRJ_NAME)_sdk.zip:
	cd ..
	if exist $(PRJ_NAME)_sdk.zip attrib -R $(PRJ_NAME)_sdk.zip
	if exist $(PRJ_NAME)_sdk.zip del       $(PRJ_NAME)_sdk.zip
	$(PRJ_NAME)\$(BIN)zip.exe -r9 $(PRJ_NAME)_sdk.zip $(PRJ_NAME)\inc $(PRJ_NAME)\app\win32\cli\$(PRJ_NAME) \
	$(PRJ_NAME)\lib\NTDrv $(PRJ_NAME)\lib\dprint.c $(PRJ_NAME)\lib\appioctl.cpp \
	$(PRJ_NAME)\lib\Rules\rules.h  $(PRJ_NAME)\lib\Rules\win32\media.h -x $(EXCLUDE) *Makefile*
	$(PRJ_NAME)\$(BIN)zip.exe -rj9 $(PRJ_NAME)_sdk.zip  $(PRJ_NAME)\$(OUT)Rules.lib $(PRJ_NAME)\$(OUT)Rules64.lib
	-sync
	cd $(PRJ_NAME)

..\$(PRJ_NAME)_utils.zip:
	cd ..
	if exist $(PRJ_NAME)_utils.zip attrib -R $(PRJ_NAME)_utils.zip
	if exist $(PRJ_NAME)_utils.zip del       $(PRJ_NAME)_utils.zip
	$(PRJ_NAME)\$(BIN)zip.exe -r9 $(PRJ_NAME)_utils.zip $(PRJ_NAME)\$(BIN) -x *.cvsignore *.scc *.zip
	-sync
	cd $(PRJ_NAME)

..\$(PRJ_NAME)_bin.zip:
	cd ..
	if exist $(PRJ_NAME)_bin.zip attrib -R $(PRJ_NAME)_bin.zip
	if exist $(PRJ_NAME)_bin.zip del       $(PRJ_NAME)_bin.zip
	-$(PRJ_NAME)\$(BIN)zip.exe -rj9 $(PRJ_NAME)_bin.zip $(PRJ_NAME)\$(OUT) -x *.cdf *.dep *.exp *.lib *.log *.map *.nms *.nsi *.pdb *.tmp
	-sync
	cd $(PRJ_NAME)

..\$(PRJ_NAME)_sym.zip:
	cd ..
	if exist $(PRJ_NAME)_sym.zip attrib -R $(PRJ_NAME)_sym.zip
	if exist $(PRJ_NAME)_sym.zip del       $(PRJ_NAME)_sym.zip
	-$(PRJ_NAME)\$(BIN)zip.exe -rj9 $(PRJ_NAME)_sym.zip $(PRJ_NAME)\$(OUT) -x *.cat *.cdf *.dep *.dll *.exe *.inf *.exp *.lib *.log *.nms *.nsi *.sys *.tmp
	-sync
	cd $(PRJ_NAME)

..\$(PRJ_NAME)_src.rar:
	cd ..
	if exist $(PRJ_NAME)_src.rar attrib -R $(PRJ_NAME)_src.rar
	if exist $(PRJ_NAME)_src.rar del       $(PRJ_NAME)_src.rar
	$(PRJ_NAME)\$(BIN)Rar.exe a -rsm5 $(PRJ_NAME)_src.rar $(PRJ_NAME) -x@.\$(PRJ_NAME)\mak\exclude.lst
	-sync
	cd $(PRJ_NAME)

..\$(PRJ_NAME)_utils.rar:
	cd ..
	if exist $(PRJ_NAME)_utils.rar attrib -R $(PRJ_NAME)_utils.rar
	if exist $(PRJ_NAME)_utils.rar del       $(PRJ_NAME)_utils.rar
	$(PRJ_NAME)\$(BIN)Rar.exe a -rsm5 $(PRJ_NAME)_utils.rar $(PRJ_NAME)\$(BIN) -x*.cvsignore -x*.scc -x*.zip
	-sync
	cd $(PRJ_NAME)

.SUFFIXES: .dll .exe .inf .sys .vxd $(SUFFIXES)

# Copy methods
{$(BIN)}.exe{$(OUT)}.exe:
	@if exist $< copy $< $@

{$(BIN)}.dll{$(OUT)}.dll:
	@if exist $< copy $< $@

{$(SCR)}.inf{$(OUT)}.inf:
	@if exist $< copy $< $@

{$(OUT)}.dll{$(NSIS)Plugins}.dll:
	@if exist $< copy $< $@

{$(BIN)}.dll{$(NSIS)Plugins}.dll:
	@if exist $< copy $< $@

# Create dependenses

!include mak\dep.mak
