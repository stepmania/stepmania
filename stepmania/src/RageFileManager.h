#ifndef RAGE_FILE_MANAGER_H
#define RAGE_FILE_MANAGER_H

#include "RageFile.h"

extern CString InitialWorkingDirectory;
extern CString DirOfExecutable;

class RageFileObj;
class RageFileManager
{
public:
	RageFileManager( CString argv0 );
	~RageFileManager();
	void MountInitialFilesystems();

	void GetDirListing( const CString &sPath, CStringArray &AddTo, bool bOnlyDirs, bool bReturnPathToo );
	bool Remove( const CString &sPath );
	
	enum FileType { TYPE_FILE, TYPE_DIR, TYPE_NONE };
	FileType GetFileType( const CString &sPath );

	bool IsAFile( const CString &sPath );
	bool IsADirectory( const CString &sPath );
	bool DoesFileExist( const CString &sPath );

	int GetFileSizeInBytes( const CString &sPath );
	int GetFileHash( const CString &sPath );

	void Mount( CString Type, CString RealPath, CString MountPoint );
	void Unmount( CString Type, CString Root, CString MountPoint );
	bool IsMounted( CString MountPoint );
	bool MountpointIsReady( CString MountPoint );

	void FlushDirCache( const CString &sPath );

	/* Used only by RageFile: */
	RageFileObj *Open( const CString &sPath, int mode, RageFile &p, int &err );
	void Close( RageFileObj *obj );
	RageFileObj *CopyFileObj( const RageFileObj *cpy, RageFile &p );

private:
	RageFileObj *OpenForWriting( const CString &sPath, int mode, RageFile &p, int &err );
};

extern RageFileManager *FILEMAN;

#endif
