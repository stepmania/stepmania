#include "stdafx.h"
#include "SMPackageUtil.h"
#include "Registry.h"

void WriteStepManiaInstallDirs( const CStringArray& asInstallDirsToWrite )
{
	CRegistry Reg;
	Reg.SetRootKey(HKEY_LOCAL_MACHINE);
	Reg.SetKey("Software\\StepMania\\smpackage\\Installations", TRUE);	// create if not already present

	unsigned i;

	for( i=0; i<100; i++ )
	{
		CString sName = ssprintf("%d",i);
//		Reg.DeleteKey( sName );	// delete key is broken in this library, so just write over it with ""
		Reg.WriteString( sName, "" );
	}

	for( i=0; i<asInstallDirsToWrite.size(); i++ )
	{
		CString sName = ssprintf("%d",i);
		Reg.WriteString( sName, asInstallDirsToWrite[i] );
	}

}

void GetStepManiaInstallDirs( CStringArray& asInstallDirsOut )
{
	asInstallDirsOut.clear();

	CRegistry Reg;
	Reg.SetRootKey(HKEY_LOCAL_MACHINE);
	Reg.SetKey("Software\\StepMania\\smpackage\\Installations", TRUE);	// create if not already present

	for( int i=0; i<100; i++ )
	{
		CString sName = ssprintf("%d",i);

		CString sPath = Reg.ReadString( sName, "" );

		if( sPath == "" )	// read failed
			continue;	// skip

		asInstallDirsOut.push_back( sPath );
	} 

	// while we're at it, write to clean up stale entries
	WriteStepManiaInstallDirs( asInstallDirsOut );
}

void AddStepManiaInstallDir( CString sNewInstallDir )
{
	CStringArray asInstallDirs;
	GetStepManiaInstallDirs( asInstallDirs );

	bool bAlreadyInList = false;
	for( unsigned i=0; i<asInstallDirs.size(); i++ )
	{
		if( asInstallDirs[i].CompareNoCase(sNewInstallDir) == 0 )
		{
			bAlreadyInList = true;
			break;
		}
	}

	if( !bAlreadyInList )
		asInstallDirs.push_back( sNewInstallDir );

	WriteStepManiaInstallDirs( asInstallDirs );
}


bool GetPref( CString name, bool &val )
{
	CRegistry Reg;
	Reg.SetRootKey(HKEY_LOCAL_MACHINE);
	Reg.SetKey("Software\\StepMania\\smpackage", FALSE);	// don't create if not already present
	return Reg.Read( name, val );
}

bool SetPref( CString name, bool val )
{
	CRegistry Reg;
	Reg.SetRootKey(HKEY_LOCAL_MACHINE);
	Reg.SetKey("Software\\StepMania\\smpackage", TRUE);	// don't create if not already present
	Reg.WriteBool( name, val );
	return false;

}


