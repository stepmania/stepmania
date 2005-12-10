#ifndef SMPackageUtil_H
#define SMPackageUtil_H

namespace SMPackageUtil
{
	void WriteStepManiaInstallDirs( const vector<RString>& asInstallDirsToWrite );
	void GetStepManiaInstallDirs( vector<RString>& asInstallDirsOut );
	void AddStepManiaInstallDir( RString sNewInstallDir );
	void SetDefaultInstallDir( int iInstallDirIndex );
	void SetDefaultInstallDir( RString sInstallDir );

	bool GetPref( RString name, bool &val );
	bool SetPref( RString name, bool val );

	RString GetPackageDirectory(RString path);
	bool IsValidPackageDirectory(RString path);

	void LaunchGame();
}

#endif
