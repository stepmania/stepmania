
#include "Registry.h"
#include "RageUtil.h"


inline void WriteStepManiaInstallDirs( const CStringArray& asInstallDirsToWrite )
{
	CRegistry Reg;
	Reg.SetRootKey(HKEY_LOCAL_MACHINE);
	Reg.SetKey("Software\\StepMania\\smpackage", TRUE);	// create if not already present

	int i;

	for( i=0; i<100; i++ )
	{
		CString sName = ssprintf("%d",i);
		Reg.DeleteKey( sName );
	}

	for( i=0; i<asInstallDirsToWrite.GetSize(); i++ )
	{
		CString sName = ssprintf("%d",i);
		Reg.WriteString( sName, asInstallDirsToWrite[i] );
	}

}

inline void GetStepManiaInstallDirs( CStringArray& asInstallDirsOut )
{
	asInstallDirsOut.RemoveAll();

	CRegistry Reg;
	Reg.SetRootKey(HKEY_LOCAL_MACHINE);
	Reg.SetKey("Software\\StepMania\\smpackage", TRUE);	// create if not already present

	for( int i=0; i<100; i++ )
	{
		CString sName = ssprintf("%d",i);

		CString sPath = Reg.ReadString( sName, "" );

		if( sPath == "" )	// read failed
			continue;	// skip

		asInstallDirsOut.Add( sPath );
	} 

	// while we're at it, write to clean up stale entries
	WriteStepManiaInstallDirs( asInstallDirsOut );
}

inline void AddStepManiaInstallDir( CString sNewInstallDir )
{
	CStringArray asInstallDirs;
	GetStepManiaInstallDirs( asInstallDirs );

	bool bAlreadyInList = false;
	for( int i=0; i<asInstallDirs.GetSize(); i++ )
	{
		if( asInstallDirs[i] == sNewInstallDir )
		{
			bAlreadyInList = true;
			break;
		}
	}

	if( !bAlreadyInList )
		asInstallDirs.Add( sNewInstallDir );

	WriteStepManiaInstallDirs( asInstallDirs );
}
