#include "stdafx.h"
#include "SMPackageUtil.h"
#include "Registry.h"

void WriteStepManiaInstallDirs( const CStringArray& asInstallDirsToWrite )
{
	CRegistry Reg;
	Reg.SetRootKey(HKEY_LOCAL_MACHINE);
	Reg.SetKey("Software\\StepMania\\smpackage\\Installations", TRUE);	// create if not already present

	int i;

	for( i=0; i<100; i++ )
	{
		CString sName = ssprintf("%d",i);
//		Reg.DeleteKey( sName );	// delete key is broken in this library, so just write over it with ""
		Reg.WriteString( sName, "" );
	}

	for( i=0; i<asInstallDirsToWrite.GetSize(); i++ )
	{
		CString sName = ssprintf("%d",i);
		Reg.WriteString( sName, asInstallDirsToWrite[i] );
	}

}

void GetStepManiaInstallDirs( CStringArray& asInstallDirsOut )
{
	asInstallDirsOut.RemoveAll();

	CRegistry Reg;
	Reg.SetRootKey(HKEY_LOCAL_MACHINE);
	Reg.SetKey("Software\\StepMania\\smpackage\\Installations", TRUE);	// create if not already present

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

void AddStepManiaInstallDir( CString sNewInstallDir )
{
	CStringArray asInstallDirs;
	GetStepManiaInstallDirs( asInstallDirs );

	bool bAlreadyInList = false;
	for( int i=0; i<asInstallDirs.GetSize(); i++ )
	{
		if( asInstallDirs[i].CompareNoCase(sNewInstallDir) == 0 )
		{
			bAlreadyInList = true;
			break;
		}
	}

	if( !bAlreadyInList )
		asInstallDirs.Add( sNewInstallDir );

	WriteStepManiaInstallDirs( asInstallDirs );
}


