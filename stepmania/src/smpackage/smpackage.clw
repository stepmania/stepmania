; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CSmpackageExportDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "smpackage.h"

ClassCount=6
Class1=CSmpackageApp
Class2=CSmpackageDlg

ResourceCount=5
Resource1=IDR_MAINFRAME
Resource2=IDD_DIALOG_NAME
Class3=CSMPackageInstallDlg
Class4=CSmpackageExportDlg
Resource3=IDD_EXPORTER
Class5=EnterName
Resource4=IDD_INSTALL
Class6=EditInsallations
Resource5=IDD_EDIT_INSTALLATIONS

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
LastObject=IDC_BUTTON_PLAY
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

