#ifndef RAGE_FILE_DRIVER_DIRECT_HELPERS_H
#define RAGE_FILE_DRIVER_DIRECT_HELPERS_H

#include <fcntl.h>

#if defined(XBOX)
int DoMkdir( const CString &sPath, int perm );
int DoOpen( const CString &sPath, int flags, int perm );
int DoStat( const CString &sPath, struct stat *st );
int DoRemove( const CString &sPath );
int DoRmdir( const CString &sPath );
HANDLE DoFindFirstFile( const CString &sPath, WIN32_FIND_DATA *fd );
#else
#define DoOpen open
#define DoStat stat
#define DoMkdir mkdir
#define DoFindFirstFile FindFirstFile
#define DoRemove remove
#define DoRmdir rmdir
#endif

#if defined(WIN32)
int WinMoveFile( CString sOldPath, CString sNewPath );
#endif

#if !defined(O_BINARY)
#define O_BINARY 0
#endif

bool CreateDirectories( CString Path );
bool PathReady( CString path );

#endif
