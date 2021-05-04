#################################################################################
#
#  Module Name: 	Makefile
#
#  Author:		Slava I. Levtchenko <slavalev@gmail.com>
#
#  Abstract:    	common Makefile - build rules selector
#
#  Revision History:	29.07.2002	  started
#						26.09.2002    released (beta)
#
#################################################################################

!if     "$(TARGET_TYPE)" == "dos_application"
!include mak\rules\le_app.mak
!elseif "$(TARGET_TYPE)" == "win9x_driver"
!include mak\rules\le_drv.mak
!elseif "$(TARGET_TYPE)" == "winnt_driver"
!include mak\rules\pe_drv.mak
!elseif "$(TARGET_TYPE)" == "win_gui_app"
!include mak\rules\pe_gui.mak
!elseif "$(TARGET_TYPE)" == "win_cli_app"
!include mak\rules\pe_srv.mak
!else
!error Please set TARGET_TYPE = <win_cli_app | win_gui_app | winnt_driver | win9x_driver | dos_application>.
!endif
