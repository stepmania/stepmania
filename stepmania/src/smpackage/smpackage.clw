; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CSmpackageDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "smpackage.h"

ClassCount=3
Class1=CSmpackageApp
Class2=CSmpackageDlg

ResourceCount=3
Resource1=IDR_MAINFRAME
Resource2=IDD_MANAGER
Class3=CSMPackageInstallDlg
Resource3=IDD_INSTALL

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



[DLG:IDD_MANAGER]
Type=1
Class=CSmpackageDlg
ControlCount=6
Control1=IDOK,button,1342242817
Control2=IDC_LIST,listbox,1352728843
Control3=IDC_BUTTON_PLAY,button,1342242816
Control4=IDC_BUTTON_EXPORT_AS_ONE,button,1476460544
Control5=IDC_STATIC,static,1342177294
Control6=IDC_BUTTON_EXPORT_AS_INDIVIDUAL,button,1476460544

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

