#define CO_EXIST_WITH_MFC
#include "global.h"
#include "stdafx.h"
#include "SMPackageUtil.h"
#include "archutils/Win32/RegistryAccess.h"
#include "ProductInfo.h"	
#include "RageUtil.h"	

void SMPackageUtil::WriteStepManiaInstallDirs( const vector<RString>& asInstallDirsToWrite )
{
	RString sKey = "HKEY_LOCAL_MACHINE\\Software\\StepMania\\smpackage\\Installations";

	unsigned i;

	for( i=0; i<100; i++ )
	{
		RString sName = ssprintf("%d",i);
//		Reg.DeleteKey( sName );	// delete key is broken in this library, so just write over it with ""
		RegistryAccess::SetRegValue( sKey, sName, RString() );
	}

	for( i=0; i<asInstallDirsToWrite.size(); i++ )
	{
		RString sName = ssprintf("%d",i);
		RegistryAccess::SetRegValue( sKey, sName, asInstallDirsToWrite[i] );
	}

}

void SMPackageUtil::GetStepManiaInstallDirs( vector<RString>& asInstallDirsOut )
{
	asInstallDirsOut.clear();

	RString sKey = "HKEY_LOCAL_MACHINE\\Software\\StepMania\\smpackage\\Installations";

	for( int i=0; i<100; i++ )
	{
		RString sName = ssprintf("%d",i);

		RString sPath;
		if( !RegistryAccess::GetRegValue(sKey, sName, sPath) )
			continue;

		if( sPath == "" )	// read failed
			continue;	// skip

		RString sProgramDir = sPath+"\\Program";
		if( !DoesFileExist(sProgramDir) )
			continue;	// skip

		asInstallDirsOut.push_back( sPath );
	} 

	// while we're at it, write to clean up stale entries
	WriteStepManiaInstallDirs( asInstallDirsOut );
}

void SMPackageUtil::AddStepManiaInstallDir( RString sNewInstallDir )
{
	vector<RString> asInstallDirs;
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

void SMPackageUtil::SetDefaultInstallDir( int iInstallDirIndex )
{
	// move the specified index to the top of the list
	vector<RString> asInstallDirs;
	GetStepManiaInstallDirs( asInstallDirs );
	ASSERT( iInstallDirIndex >= 0  &&  iInstallDirIndex < (int)asInstallDirs.size() );
	RString sDefaultInstallDir = asInstallDirs[iInstallDirIndex];
	asInstallDirs.erase( asInstallDirs.begin()+iInstallDirIndex );
	asInstallDirs.insert( asInstallDirs.begin(), sDefaultInstallDir );
	WriteStepManiaInstallDirs( asInstallDirs );
}

void SMPackageUtil::SetDefaultInstallDir( RString sInstallDir )
{
	vector<RString> asInstallDirs;
	GetStepManiaInstallDirs( asInstallDirs );

	for( unsigned i=0; i<asInstallDirs.size(); i++ )
	{
		if( asInstallDirs[i].CompareNoCase(sInstallDir) == 0 )
		{
			SetDefaultInstallDir( i );
			break;
		}
	}
}

bool SMPackageUtil::GetPref( RString name, bool &val )
{
	return RegistryAccess::GetRegValue( "HKEY_LOCAL_MACHINE\\Software\\StepMania\\smpackage", name, val );
}

bool SMPackageUtil::SetPref( RString name, bool val )
{
	return RegistryAccess::SetRegValue( "HKEY_LOCAL_MACHINE\\Software\\StepMania\\smpackage", name, val );
}

/* Get a package directory.  For most paths, this is the first two components.  For
 * songs and note skins, this is the first three. */
RString SMPackageUtil::GetPackageDirectory(RString path)
{
	if( path.Find("CVS") != -1 )
		return "";	// skip

	vector<RString> Parts;
	split( path, "\\", Parts );

	unsigned NumParts = 2;
	if( !Parts[0].CompareNoCase("Songs") || !Parts[0].CompareNoCase("NoteSkins") )
		NumParts = 3;
	if( Parts.size() < NumParts )
		return "";

	Parts.erase(Parts.begin() + NumParts, Parts.end());

	RString ret = join( "\\", Parts );
	if( !IsADirectory(ret) )
		return "";
	return ret;
}

bool SMPackageUtil::IsValidPackageDirectory( RString path )
{
	/* Make sure the path contains only second-level directories, and doesn't
	 * contain any ".", "..", "...", etc. dirs. */
	vector<RString> Parts;
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

bool SMPackageUtil::LaunchGame()
{
	PROCESS_INFORMATION pi;
	STARTUPINFO	si;
	ZeroMemory( &si, sizeof(si) );

	RString sFile = PRODUCT_NAME ".exe";
	if( !DoesFileExist(sFile) )
	{
		sFile = "Program\\" + sFile;
		if( !DoesFileExist(sFile) )
		{
			MessageBox( NULL, "Could not find " PRODUCT_NAME ".exe", "Error", MB_ICONEXCLAMATION );
			return false;
		}
	}

	CreateProcess(
		sFile,	// pointer to name of executable module
		NULL,	// pointer to command line string
		NULL,  // process security attributes
		NULL,   // thread security attributes
		false,  // handle inheritance flag
		0, // creation flags
		NULL,  // pointer to new environment block
		NULL,   // pointer to current directory name
		&si,  // pointer to STARTUPINFO
		&pi  // pointer to PROCESS_INFORMATION
	);
	return true;
}
