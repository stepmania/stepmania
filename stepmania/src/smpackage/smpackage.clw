; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=EnterName
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "smpackage.h"

ClassCount=5
Class1=CSmpackageApp
Class2=CSmpackageDlg

ResourceCount=4
Resource1=IDR_MAINFRAME
Resource2=IDD_INSTALL
Class3=CSMPackageInstallDlg
Class4=CSmpackageExportDlg
Resource3=IDD_EXPORTER
Class5=EnterName
Resource4=IDD_DIALOG_NAME

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
ControlCount=7
Control1=IDC_EDIT_MESSAGE1,edit,1342179460
Control2=IDOK,button,1342242817
Control3=IDCANCEL,button,1342242816
Control4=IDC_BUTTON_BACK,button,1476460544
Control5=IDC_STATIC,static,1342177294
Control6=IDC_EDIT_MESSAGE3,edit,1342179460
Control7=IDC_EDIT_MESSAGE2,edit,1352665220

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
ControlCount=6
Control1=IDOK,button,1342242817
Control2=IDC_BUTTON_PLAY,button,1342242816
Control3=IDC_BUTTON_EXPORT_AS_ONE,button,1342242816
Control4=IDC_STATIC,static,1342177294
Control5=IDC_BUTTON_EXPORT_AS_INDIVIDUAL,button,1342242816
Control6=IDC_TREE,SysTreeView32,1350631687

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

