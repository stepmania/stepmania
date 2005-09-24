; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=ChangeGameSettings
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "smpackage.h"

ClassCount=12
Class1=CSmpackageApp
Class2=CSmpackageDlg

ResourceCount=13
Resource1=IDR_MAINFRAME
Resource2=IDD_ENTER_STRING
Class3=CSMPackageInstallDlg
Class4=CSmpackageExportDlg
Resource3=IDD_INSTALL
Class5=EnterName
Resource4=IDD_CONVERT_THEME
Class6=EditInsallations
Resource5=IDD_SHOW_COMMENT
Class7=MainMenuDlg
Resource6=IDD_UNINSTALL_OLD_PACKAGES
Class8=ConvertThemeDlg
Resource7=IDD_ENTER_COMMENT
Class9=EditMetricsDlg
Resource8=IDD_EDIT_INSTALLATIONS
Resource9=IDD_EXPORTER
Class10=EnterComment
Resource10=IDD_EDIT_METRICS
Class11=ShowComment
Resource11=IDD_DIALOG_NAME
Resource12=IDD_MENU
Class12=ChangeGameSettings
Resource13=IDD_CHANGE_GAME_SETTINGS

[CLS:CSmpackageApp]
Type=0
HeaderFile=smpackage.h
ImplementationFile=smpackage.cpp
Filter=N
LastObject=CSmpackageApp
BaseClass=CWinApp
VirtualFilter=AC

[CLS:CSmpackageDlg]
Type=0
HeaderFile=smpackageDlg.h
ImplementationFile=smpackageDlg.cpp
Filter=D
LastObject=CSmpackageDlg
BaseClass=CDialog
VirtualFilter=dWC



[DLG:IDD_INSTALL]
Type=1
Class=CSMPackageInstallDlg
ControlCount=10
Control1=IDC_EDIT_MESSAGE1,edit,1342179460
Control2=IDOK,button,1342242817
Control3=IDCANCEL,button,1342242816
Control4=IDC_BUTTON_BACK,button,1476460544
Control5=IDC_STATIC,static,1342177294
Control6=IDC_EDIT_MESSAGE3,edit,1342179460
Control7=IDC_EDIT_MESSAGE2,edit,1352665220
Control8=IDC_COMBO_DIR,combobox,1344339971
Control9=IDC_BUTTON_EDIT,button,1342242816
Control10=IDC_PROGRESS1,msctls_progress32,1082130432

[CLS:CSMPackageInstallDlg]
Type=0
HeaderFile=SMPackageInstallDlg.h
ImplementationFile=SMPackageInstallDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=CSMPackageInstallDlg
VirtualFilter=dWC

[DLG:IDD_EXPORTER]
Type=1
Class=CSmpackageExportDlg
ControlCount=10
Control1=IDOK,button,1342242817
Control2=IDC_BUTTON_PLAY,button,1342242816
Control3=IDC_BUTTON_EXPORT_AS_ONE,button,1342242816
Control4=IDC_STATIC,static,1342177294
Control5=IDC_BUTTON_EXPORT_AS_INDIVIDUAL,button,1342242816
Control6=IDC_TREE,SysTreeView32,1350631687
Control7=IDC_COMBO_DIR,combobox,1344339971
Control8=IDC_BUTTON_EDIT,button,1342242816
Control9=IDC_STATIC,static,1342308352
Control10=IDC_BUTTON_OPEN,button,1342242816

[CLS:CSmpackageExportDlg]
Type=0
HeaderFile=SmpackageExportDlg.h
ImplementationFile=SmpackageExportDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=CSmpackageExportDlg
VirtualFilter=dWC

[CLS:EnterName]
Type=0
HeaderFile=EnterName.h
ImplementationFile=EnterName.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=IDC_EDIT

[DLG:IDD_EDIT_INSTALLATIONS]
Type=1
Class=EditInsallations
ControlCount=10
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_LIST,listbox,1352728833
Control4=IDC_EDIT,edit,1350631552
Control5=IDC_BUTTON_ADD,button,1342242816
Control6=IDC_BUTTON_REMOVE,button,1342242816
Control7=IDC_BUTTON_MAKE_DEFAULT,button,1342242816
Control8=IDC_STATIC,button,1342177287
Control9=IDC_STATIC,button,1342177287
Control10=IDC_STATIC,static,1342308352

[CLS:EditInsallations]
Type=0
HeaderFile=EditInsallations.h
ImplementationFile=EditInsallations.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=EditInsallations

[DLG:IDD_MENU]
Type=1
Class=MainMenuDlg
ControlCount=14
Control1=IDOK,button,1342242817
Control2=IDC_STATIC,static,1342177294
Control3=IDC_EXPORT_PACKAGES,button,1342242816
Control4=IDC_STATIC,static,1342308352
Control5=IDC_STATIC,button,1342177287
Control6=IDC_STATIC,button,1342177287
Control7=IDC_ANALYZE_ELEMENTS,button,1342242816
Control8=IDC_EDIT_INSTALLATIONS,button,1342242816
Control9=IDC_STATIC,static,1342308352
Control10=IDC_STATIC,button,1342177287
Control11=IDC_STATIC,static,1342308352
Control12=IDC_STATIC,button,1342177287
Control13=IDC_CHANGE_API,button,1342242816
Control14=IDC_STATIC,static,1342308352

[CLS:MainMenuDlg]
Type=0
HeaderFile=MainMenuDlg.h
ImplementationFile=MainMenuDlg.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=IDC_ANALYZE_ELEMENTS

[DLG:IDD_CONVERT_THEME]
Type=1
Class=ConvertThemeDlg
ControlCount=13
Control1=IDOK,button,1342242817
Control2=IDC_STATIC,static,1342177294
Control3=IDC_LIST_THEMES,listbox,1352728835
Control4=IDC_BUTTON_CONVERT,button,1476460544
Control5=IDC_STATIC,static,1342308352
Control6=IDC_STATIC,button,1342177287
Control7=IDC_STATIC,static,1342308352
Control8=IDC_BUTTON_ANALYZE,button,1476460544
Control9=IDC_STATIC,button,1342177287
Control10=IDC_STATIC,static,1342308352
Control11=IDC_BUTTON_EDIT_METRICS,button,1476460544
Control12=IDC_STATIC,static,1342308352
Control13=IDC_BUTTON_ANALYZE_METRICS,button,1476460544

[CLS:ConvertThemeDlg]
Type=0
HeaderFile=onvertThemeDlg.h
ImplementationFile=onvertThemeDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=IDC_LIST_THEMES
VirtualFilter=dWC

[DLG:IDD_EDIT_METRICS]
Type=1
Class=EditMetricsDlg
ControlCount=11
Control1=IDC_BUTTON_CLOSE,button,1342242816
Control2=IDC_STATIC,static,1342177294
Control3=IDC_EDIT_VALUE,edit,1484849220
Control4=IDC_STATIC,static,1342308352
Control5=IDC_EDIT_DEFAULT,edit,1350633540
Control6=IDC_STATIC,static,1342308352
Control7=IDC_BUTTON_OVERRIDE,button,1476460544
Control8=IDC_BUTTON_REMOVE,button,1476460544
Control9=IDC_TREE,SysTreeView32,1350631461
Control10=IDC_BUTTON_SAVE,button,1342242816
Control11=IDC_BUTTON_HELP,button,1342242816

[CLS:EditMetricsDlg]
Type=0
HeaderFile=EditMetricsDlg.h
ImplementationFile=EditMetricsDlg.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=EditMetricsDlg

[DLG:IDD_ENTER_STRING]
Type=1
Class=EnterName
ControlCount=4
Control1=IDC_STATIC,static,1342308352
Control2=IDC_EDIT,edit,1350631552
Control3=IDOK,button,1342242817
Control4=IDCANCEL,button,1342242816

[DLG:IDD_ENTER_COMMENT]
Type=1
Class=EnterComment
ControlCount=5
Control1=65535,static,1342308352
Control2=IDC_EDIT,edit,1350631556
Control3=IDOK,button,1342242817
Control4=IDCANCEL,button,1342242816
Control5=IDC_DONTASK,button,1342242819

[DLG:IDD_DIALOG_NAME]
Type=1
Class=?
ControlCount=5
Control1=IDC_STATIC,static,1342308352
Control2=IDC_EDIT,edit,1350631552
Control3=IDOK,button,1342242817
Control4=IDCANCEL,button,1342242816
Control5=IDC_STATIC,static,1342308352

[CLS:EnterComment]
Type=0
HeaderFile=EnterComment.h
ImplementationFile=EnterComment.cpp
BaseClass=CDialog
Filter=D
LastObject=IDC_EDIT
VirtualFilter=dWC

[DLG:IDD_SHOW_COMMENT]
Type=1
Class=ShowComment
ControlCount=4
Control1=IDC_EDIT,edit,1350633604
Control2=IDOK,button,1342242817
Control3=IDCANCEL,button,1342242816
Control4=IDC_DONTSHOW,button,1342242819

[CLS:ShowComment]
Type=0
HeaderFile=ShowComment.h
ImplementationFile=ShowComment.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=IDC_DONTSHOW

[DLG:IDD_UNINSTALL_OLD_PACKAGES]
Type=1
Class=?
ControlCount=6
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,static,1342308352
Control4=IDC_PACKAGES,edit,1342244992
Control5=IDC_BUTTON1,button,1342242816
Control6=IDC_STATIC,static,1342308352

[DLG:IDD_CHANGE_GAME_SETTINGS]
Type=1
Class=ChangeGameSettings
ControlCount=7
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,button,1342177287
Control4=IDC_RADIO_DEFAULT,button,1342177289
Control5=IDC_RADIO_OPENGL,button,1342177289
Control6=IDC_RADIO_DIRECT3D,button,1342177289
Control7=IDC_STATIC,static,1342308352

[CLS:ChangeGameSettings]
Type=0
HeaderFile=ChangeGameSettings.h
ImplementationFile=ChangeGameSettings.cpp
BaseClass=CDialog
Filter=D
LastObject=IDCANCEL
VirtualFilter=dWC

