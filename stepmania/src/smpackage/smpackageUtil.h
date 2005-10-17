#ifndef SMPACKAGE_UTIL_H
#define SMPACKAGE_UTIL_H

#include "RageUtil.h"

void WriteStepManiaInstallDirs( const CStringArray& asInstallDirsToWrite );
void GetStepManiaInstallDirs( CStringArray& asInstallDirsOut );
void AddStepManiaInstallDir( CString sNewInstallDir );
void SetDefaultInstallDir( int iInstallDirIndex );

bool GetPref( CString name, bool &val );
bool SetPref( CString name, bool val );

CString GetPackageDirectory(CString path);
bool IsValidPackageDirectory(CString path);

#endif
