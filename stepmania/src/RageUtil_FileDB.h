#ifndef RAGE_UTIL_FILEDB
#define RAGE_UTIL_FILEDB 1

bool DoesFileExist( const CString &sPath );
bool IsAFile( const CString &sPath );
bool IsADirectory( const CString &sPath );
bool ResolvePath(CString &path);
unsigned GetFileSizeInBytes( const CString &sFilePath );
int GetFileModTime( const CString &sPath );

void FlushDirCache();

#endif
