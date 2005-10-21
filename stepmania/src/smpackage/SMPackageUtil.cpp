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

		CString sProgramDir = sPath+"\\Program";
		if( !DoesFileExist(sProgramDir) )
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

void SetDefaultInstallDir( int iInstallDirIndex )
{
	// move the specified index to the top of the list
	CStringArray asInstallDirs;
	GetStepManiaInstallDirs( asInstallDirs );
	ASSERT( iInstallDirIndex > 0  &&  iInstallDirIndex < asInstallDirs.size() );
	CString sDefaultInstallDir = asInstallDirs[iInstallDirIndex];
	asInstallDirs.erase( asInstallDirs.begin()+iInstallDirIndex );
	asInstallDirs.insert( asInstallDirs.begin(), sDefaultInstallDir );
	WriteStepManiaInstallDirs( asInstallDirs );
}

void SetDefaultInstallDir( CString sInstallDir )
{
	CStringArray asInstallDirs;
	GetStepManiaInstallDirs( asInstallDirs );

	bool bAlreadyInList = false;
	for( unsigned i=0; i<asInstallDirs.size(); i++ )
	{
		if( asInstallDirs[i].CompareNoCase(sInstallDir) == 0 )
		{
			SetDefaultInstallDir( i );
			break;
		}
	}
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

/* Get a package directory.  For most paths, this is the first two components.  For
 * songs and note skins, this is the first three. */
CString GetPackageDirectory(CString path)
{
	if( path.Find("CVS") != -1 )
		return "";	// skip

	CStringArray Parts;
	split( path, "\\", Parts );

	unsigned NumParts = 2;
	if( !Parts[0].CompareNoCase("Songs") || !Parts[0].CompareNoCase("NoteSkins") )
		NumParts = 3;
	if( Parts.size() < NumParts )
		return "";

	Parts.erase(Parts.begin() + NumParts, Parts.end());

	CString ret = join( "\\", Parts );
	if( !IsADirectory(ret) )
		return "";
	return ret;
}


bool IsValidPackageDirectory(CString path)
{
	/* Make sure the path contains only second-level directories, and doesn't
	 * contain any ".", "..", "...", etc. dirs. */
	CStringArray Parts;
	split( path, "\\", Parts, true );
	if( Parts.size() == 0 )
		return false;

	/* Make sure we're not going to "uninstall" an entire Songs subfolder. */
	unsigned NumParts = 2;
	if( !Parts[0].CompareNoCase("songs") )
		NumParts = 3;
	if( Parts.size() < NumParts )
		return false;

	/* Make sure the path doesn't contain any ".", "..", "...", etc. dirs. */
	for( unsigned i = 0; i < Parts.size(); ++i )
		if( Parts[i][0] == '.' )
			return false;

	return true;
}

