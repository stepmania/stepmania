; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=EditMetricsDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "smpackage.h"

ClassCount=9
Class1=CSmpackageApp
Class2=CSmpackageDlg

ResourceCount=8
Resource1=IDR_MAINFRAME
Resource2=IDD_EXPORTER
Class3=CSMPackageInstallDlg
Class4=CSmpackageExportDlg
Resource3=IDD_CONVERT_THEME
Class5=EnterName
Resource4=IDD_DIALOG_NAME
Class6=EditInsallations
Resource5=IDD_INSTALL
Class7=MainMenuDlg
Resource6=IDD_EDIT_INSTALLATIONS
Class8=ConvertThemeDlg
Resource7=IDD_MENU
Class9=EditMetricsDlg
Resource8=IDD_EDIT_METRICS

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
ControlCount=9
Control1=IDC_EDIT_MESSAGE1,edit,1342179460
Control2=IDOK,button,1342242817
Control3=IDCANCEL,button,1342242816
Control4=IDC_BUTTON_BACK,button,1476460544
Control5=IDC_STATIC,static,1342177294
Control6=IDC_EDIT_MESSAGE3,edit,1342179460
Control7=IDC_EDIT_MESSAGE2,edit,1352665220
Control8=IDC_COMBO_DIR,combobox,1344339971
Control9=IDC_BUTTON_EDIT,button,1342242816

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

[DLG:IDD_DIALOG_NAME]
Type=1
Class=EnterName
ControlCount=5
Control1=IDC_STATIC,static,1342308352
Control2=IDC_EDIT,edit,1350631552
Control3=IDOK,button,1342242817
Control4=IDCANCEL,button,1342242816
Control5=IDC_STATIC,static,1342308352

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
ControlCount=11
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

[CLS:MainMenuDlg]
Type=0
HeaderFile=MainMenuDlg.h
ImplementationFile=MainMenuDlg.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=IDC_CONVERT_THEME

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
ControlCount=13
Control1=IDOK,button,1342242817
Control2=IDC_STATIC,static,1342177294
Control3=IDC_EDIT_VALUE,edit,1484849220
Control4=IDC_STATIC,static,1342308352
Control5=IDC_EDIT_DEFAULT,edit,1350633540
Control6=IDC_STATIC,static,1342308352
Control7=IDC_BUTTON_OVERRIDE,button,1476460544
Control8=IDC_BUTTON_REMOVE,button,1476460544
Control9=IDC_TREE,SysTreeView32,1350631461
Control10=IDC_STATIC,static,1342308352
Control11=IDC_STATIC,static,1342308352
Control12=IDC_BUTTON_REFRESH,button,1342242816
Control13=IDC_BUTTON_SAVE,button,1342242816

[CLS:EditMetricsDlg]
Type=0
HeaderFile=EditMetricsDlg.h
ImplementationFile=EditMetricsDlg.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=EditMetricsDlg

