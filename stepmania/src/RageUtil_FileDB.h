#ifndef RAGE_UTIL_FILEDB
#define RAGE_UTIL_FILEDB 1

bool DoesFileExist( const CString &sPath );
bool IsAFile( const CString &sPath );
bool IsADirectory( const CString &sPath );
unsigned GetFileSizeInBytes( const CString &sFilePath );
bool DoStat(CString sPath, struct stat *st);

void FlushDirCache();

#endif
