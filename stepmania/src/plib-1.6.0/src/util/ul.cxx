/*
     PLIB - A Suite of Portable Game Libraries
     Copyright (C) 1998,2002  Steve Baker
 
     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Library General Public
     License as published by the Free Software Foundation; either
     version 2 of the License, or (at your option) any later version.
 
     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Library General Public License for more details.
 
     You should have received a copy of the GNU Library General Public
     License along with this library; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 
     For further information visit http://plib.sourceforge.net

     $Id$
*/


#include "ul.h"

#include <sys/types.h>
#include <sys/stat.h>

#if defined(__CYGWIN__) || !defined(WIN32)
#include <dirent.h>
#elif defined(WIN32)
#include <direct.h>
#endif


#if defined(WIN32) && !defined(__CYGWIN__)
#define SLASH "\\"
#else
#define SLASH "/"
#endif


struct _ulDir
{
  char dirname [ UL_NAME_MAX+1 ];
  ulDirEnt curr ;

#if defined(WIN32) && !defined(__CYGWIN__)
  WIN32_FIND_DATA data ;
  HANDLE hFind ;
  bool first ;
  bool done ;
#else
  DIR* dirp ;
#endif
} ;


void ulInit ()
{
}


ulDir* ulOpenDir ( const char* dirname )
{
  ulDir* dir = new ulDir;
  if ( dir != NULL )
  {
    strcpy( dir->dirname, dirname ) ;
#if defined(WIN32) && !defined(__CYGWIN__)
    char search[_MAX_PATH];
    strcpy(search,dirname);
    
    //munch the directory seperator
    int len = strlen(search);
    if ( len>0 && strchr("/\\",search[len-1]) )
      search[len-1] = 0;
    
    //add the wildcard
    strcat(search,"/*.*");
    
    dir->first = true;
    dir->done = false;
    dir->hFind = FindFirstFile(search, &dir->data);
    if (dir->hFind == INVALID_HANDLE_VALUE)
    {
      delete dir;
      dir = NULL;
    }
#else
    dir->dirp = opendir(dirname) ;

    if ( dir->dirp == NULL )
    {
      delete dir ;
      dir = NULL ;
    }
#endif
  }
  return dir;
}


ulDirEnt* ulReadDir ( ulDir* dir )
{
  //read the next entry from the directory
#if defined(WIN32) && !defined(__CYGWIN__)
  //update state
  if ( dir->first )
    dir->first = false ;
  else if ( !dir->done && !FindNextFile(dir->hFind,&dir->data) )
    dir->done = true ;
  if ( dir->done )
    return NULL ;

  strcpy( dir->curr.d_name, dir->data.cFileName ) ;
#else
  struct dirent* direntp = readdir( dir->dirp );
  if ( !direntp )
    return NULL ;

  strcpy( dir->curr.d_name, direntp->d_name );
#endif

  char path[ 1000 + UL_NAME_MAX+1 ];
  sprintf( path, "%s/%s", dir->dirname, dir->curr.d_name );

  //determine if this entry is a directory
#if defined(WIN32) && !defined(__CYGWIN__)
  struct _stat buf;
  int result = _stat(path,&buf);
  if ( result == 0 )
    dir->curr.d_isdir = (buf.st_mode & _S_IFDIR) != 0 ;
  else
    dir->curr.d_isdir = false ;
#else
  struct stat buf ;
  if ( stat(path,&buf) == 0 )
    dir->curr.d_isdir = (buf.st_mode & S_IFDIR) != 0 ;
  else
    dir->curr.d_isdir = false ;
#endif

  return( &dir->curr ) ;
}



void ulCloseDir ( ulDir* dir )
{
  if ( dir != NULL )
  {
#if defined(WIN32) && !defined(__CYGWIN__)
    FindClose(dir->hFind);
#else
    closedir ( dir->dirp ) ;
#endif
    delete dir ;
  }
}

bool ulFileExists ( const char *fileName )
{
  struct stat buf ;

  if ( stat ( fileName, &buf ) < 0 )
    return false ;

// wk: _MSC_VER is predefined by Microsoft (Visual) C++
// MSVC doesnt know S_ISREG
#ifdef _MSC_VER
  return ((S_IFREG & buf.st_mode ) !=0) ;
#else
  return ((S_ISREG ( buf.st_mode )) != 0) ;
#endif
}

char* ulMakePath( char* path, const char* dir, const char* fname )
{
  if ( fname )
  {
    if ( fname [ 0 ] != '\0' && fname [ 0 ] != '/' &&
       dir != NULL && dir[0] != '\0' )
    {
      strcpy ( path, dir ) ;
      strcat ( path, SLASH ) ;
      strcat ( path, fname ) ;
    }
    else
      strcpy ( path, fname ) ;
  }
  else
     path [0] = 0 ;
  return( path );
}


static int recursiveFindFileInSubDirs ( char * filenameOutput, 
												const char * tPath,  const char * tfnameInput ) 
// recursively calls itself
// returns true on success
{ int bFound = FALSE;
  char tempString [ 1024 ];

	ulMakePath ( filenameOutput, tPath, tfnameInput ) ;
	if ( ulFileExists ( filenameOutput ))
		return TRUE; // success

	ulDir* dirp = ulOpenDir(tPath);
	if ( dirp != NULL )
	{
		ulDirEnt* dp;
		while ( ! bFound && ((dp = ulReadDir(dirp)) != NULL ) )
		{
			if ( dp->d_isdir )
			  // I am doing recursive ulOpenDir/ulReadDirs here.
				// I know this works under Windo$.
				if ( ( 0 != strcmp( dp->d_name, ".")) && ( 0 != strcmp( dp->d_name, "..")) )
				{
					
					ulMakePath ( tempString, tPath, dp->d_name) ;
				  bFound = recursiveFindFileInSubDirs ( filenameOutput, tempString, tfnameInput );
				}
			
		}
		ulCloseDir(dirp);
	}
	return bFound;
}
    


void ulFindFile( char *filenameOutput, const char *path, 
											  const char * tfnameInput, const char *sAPOM ) 
// adds tfnameInput to the path and puts this into the buffer filenameOutput.
// sAPOM is used iff path contains "$(APOM)"

// handles special chars in path:
// ";;" is replaced by ";"
// "$$" is replaced by "$"
// "$(APOM)" is replaced by sAPOM
// If there are ";" in path, the path-variable is interpreted as several paths "segments",
// delimited by ";". The first file found by this function is returned.
// It looks from left to right.
// A segment may end in $(...). ulFindFile will then look in in this path and recursively in all the sub-paths
//
// Some examples:
//
// for loading *.MDl-models, it is very nice to set the texture path to
// $(APOM);$(APOM)/texture;$(APOM)/../texture
// This consits of three segments and tells ulFindFile to look in the 
// path of the model, in a subpath texture and in a path texture "besides" the path of the model
// Some *.mdl-models are shipped in a directory which conatins a "texture"-directory, a 
// "Model"-directory and others. In this case you find the texture in $(APOM)/../texture
//
// Another example: You have all your textures in a directory-structure under /roomplan.
// For example brick is under /roomplan/bricks, wood is under /roomplan/wood, oak is 
// under /roomplan/wood/oak. Then you should use the following texture path:
// "/roomplan/$(...)"
//
// If you dont want all of the bells and whistles, just call:
// _ssgMakePath ( filenameOutput, path, tfnameInput ) ;
	 


{
  
	char temp_texture_path[1024], *s_ptr, *s_ptr1, *current_path;
	
	strncpy(temp_texture_path, path, 1024);
	current_path = temp_texture_path;
	s_ptr = temp_texture_path;
	while ( *s_ptr != 0 )
	{ if ( *s_ptr == ';' )
		{ if ( s_ptr [ 1 ] == ';' )
			{ // replace ";;" with ";"
		    s_ptr1 = ++s_ptr; // both pointers on second ";"
				while ( *s_ptr1 != 0)
				{	s_ptr1 [ 0 ] = s_ptr1 [ 1 ];
				  s_ptr1++;
				}
			}
			else
			{ // found a single ';'. This delimits paths
				*s_ptr++ = 0;
				ulMakePath ( filenameOutput, current_path, tfnameInput ) ;
				if ( ulFileExists ( filenameOutput ) )
					return; // success!
				// this path doesnt hold the texture. Try next one
				current_path = s_ptr;
			}
		}
		else if ( *s_ptr == '$' )
		{ 
			if ( s_ptr [ 1 ] == '$' )
			{ // replace "$$" with "$"
		    s_ptr1 = ++s_ptr; // both pointers on second "$"
				while ( *s_ptr1 != 0)
				{	s_ptr1 [ 0 ] = s_ptr1 [ 1 ];
				  s_ptr1++;
				}
			}
			else if ( 0 == strncmp( s_ptr, "$(APOM)", strlen("$(APOM)" ) ) )
			{ // replace "$(APOM)" by sAPOM
				char temp_buffer[1024];
				* s_ptr = 0;
				s_ptr += strlen ( "$(APOM)" );
				strcpy ( temp_buffer, s_ptr );
				strcat ( current_path, sAPOM);
				s_ptr = & current_path [ strlen(current_path) ] ; // onto the 0
				strcat ( current_path, temp_buffer );
			}
			else if ( 0 == strncmp( s_ptr, "$(...)", strlen("$(...)" ) ) )
			{	
				//strcpy(temp_texture_path_for_recursion, current_path);

				char * nextPath=s_ptr;
				nextPath += strlen("$(...)" );

				while (*nextPath != 0 )
				{ if ( *nextPath == ';' )
					{ if ( nextPath[1] == ';' )
							nextPath++; // so its "add 2 " togehter with the ++ further down
						else
						{
							*nextPath = 0;
							break; // breaks the while
						}
					}
					nextPath++;
				}
				// This segment of the path ends with a 0 now
				// *****
				char tPath [ 1024 ];
				strcpy ( tPath, current_path ) ;
				tPath [ (long) (s_ptr - current_path) ] = 0;
				
				// So, lets recurse into the sub-dirs:
				// Here I just assume that the "$(...)" is the last thing in this segment
				if ( recursiveFindFileInSubDirs ( filenameOutput, tPath,  tfnameInput ) )
					return ; // success

				// *****
				// we handled the path-segment current_path containing the $(...) and 
				// didnt find the file, so go on to the nect segment
				current_path = nextPath; // points to a 0 if this was the last segment
				s_ptr = current_path;
			}	
			else
				s_ptr++;
		}
		else // neither ';' nor '$'
			s_ptr++;
	}
  ulMakePath ( filenameOutput, current_path, tfnameInput ) ; // pfusch? kludge?

}

///////////////////// string handling ///////////////////////////////

/*
  Strdup is *evil* - use this instead...
*/

char *ulStrDup ( const char *s )
{
  int s_sz = strlen ( s ) + 1 ;
  char *ret = new char [ s_sz ] ;

  memcpy ( ret, s, s_sz ) ;
  return ret ;
}



// string comparisons that are *not* case sensitive:

/*
  I'm sick of half the machines on the planet supporting
  strncasecmp and the other half strnicmp - so here is my own
  offering:
*/

int ulStrNEqual ( const char *s1, const char *s2, int len )
{
  int l1 = (s1==NULL) ? 0 : strlen ( s1 ) ;
  int l2 = (s2==NULL) ? 0 : strlen ( s2 ) ;

  if ( l1 > len ) l1 = len ;

  if ( l2 < l1 || l1 < len )
    return FALSE ;

  for ( int i = 0 ; i < l1 ; i++ )
  {
    char c1 = s1[i] ;
    char c2 = s2[i] ;

    if ( c1 == c2 )
     continue ;

    if ( c1 >= 'a' && c1 <= 'z' )
      c1 = c1 - ('a'-'A') ;

    if ( c2 >= 'a' && c2 <= 'z' )
      c2 = c2 - ('a'-'A') ;

    if ( c1 != c2 )
     return FALSE ;
  }

  return TRUE ;
}


/*
  I'm sick of half the machines on the planet supporting
  strcasecmp and the other half stricmp - so here is my own
  offering:
*/

int ulStrEqual ( const char *s1, const char *s2 )
{
  int l1 = (s1==NULL) ? 0 : strlen ( s1 ) ;
  int l2 = (s2==NULL) ? 0 : strlen ( s2 ) ;

  if ( l1 != l2 ) return FALSE ;

  for ( int i = 0 ; i < l1 ; i++ )
  {
    char c1 = s1[i] ;
    char c2 = s2[i] ;

    if ( c1 == c2 )
     continue ;

    if ( c1 >= 'a' && c1 <= 'z' )
      c1 = c1 - ('a'-'A') ;

    if ( c2 >= 'a' && c2 <= 'z' )
      c2 = c2 - ('a'-'A') ;

    if ( c1 != c2 )
     return FALSE ;
  }

  return TRUE ;
}


int ulIsAbsolutePathName ( const char *pathname )
{
  /*
    Is this an absolute pathname or a relative one?
  */
 
	if ( (pathname == NULL) || (pathname [0] == 0) )
		return FALSE;

#if defined(WIN32) && !defined(__CYGWIN__)
 
  /*
    Under WinDOS, it's an absolute path if it starts
    with a slash *or* if it starts with a drive letter,
    a colon and a slash.
  */

  return ( pathname[0] == SLASH[0] || pathname[0] == '/' ) ||
         (
           (
             ( pathname[0] >= 'a' && pathname[0] <= 'z' ) ||
             ( pathname[0] >= 'A' && pathname[0] <= 'Z' )
           ) &&
           pathname[1] == ':' &&
           ( pathname[2] == SLASH[0] || pathname[2] == '/' )
         ) ;
#elif defined(macintosh)
  return (pathname [0] != ':' && strchr( pathname, ':') != NULL );
#else 
  return pathname [0] == SLASH[0] ;
#endif
}


char *ulGetCWD ( char *result, int maxlength )
{
  /*
    Return the current working directory into 'result' - which
    has enough space for 'maxlength-1' characters and a '\0'.
  */

#if defined(_MSC_VER) || defined(__MINGW__)
  return _getcwd ( result, maxlength ) ;
#else
  return getcwd ( result, maxlength ) ;
#endif
}


