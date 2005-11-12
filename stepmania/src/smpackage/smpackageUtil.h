#ifndef SMPACKAGE_UTIL_H
#define SMPACKAGE_UTIL_H

#include "RageUtil.h"
#include "../ProductInfo.h"

static const CString PREFERENCES_INI = "Save\\Preferences.ini";
static const CString KEYMAPS_INI = "Save\\Keymaps.ini";

void WriteStepManiaInstallDirs( const CStringArray& asInstallDirsToWrite );
void GetStepManiaInstallDirs( CStringArray& asInstallDirsOut );
void AddStepManiaInstallDir( CString sNewInstallDir );
void SetDefaultInstallDir( int iInstallDirIndex );
void SetDefaultInstallDir( CString sInstallDir );

bool GetPref( CString name, bool &val );
bool SetPref( CString name, bool val );

CString GetPackageDirectory(CString path);
bool IsValidPackageDirectory(CString path);

#endif
