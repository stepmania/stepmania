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

//
// File parser for SSG/PLIB
// Written by Dave McClurg (dpm@efn.org) in Feb-2000
// extended by Wolfram Kuss (w_kuss@rz-online.de) in Nov-2000

// This is mainly an lexical analyzer that extracts tokens from ascii-files

// Be sure to read the ssg-documentation, especially the chapter
// on loaders/writers


#define AM_IN_SSGPARSER_CXX 1

#include "ssgLocal.h"
#include "ssgParser.h"


static _ssgParserSpec default_spec =
{
   "\r\n\t ",  // delim_chars_skipable
   0,          // delim_chars_non_skipable
   0,          // open_brace_chars
   0,          // close_brace_chars
   '"',        // quote_char
   0,          // comment_char
	 "//"        // comment_string
} ;


// Output an error
void _ssgParser::error( const char *format, ... )
{
  char msgbuff[ 255 ];
  va_list argp;

  char* msgptr = msgbuff;
  if (linenum)
  {
    msgptr += sprintf ( msgptr,"%s, line %d: ",
      path, linenum );
  }

  va_start( argp, format );
  vsprintf( msgptr, format, argp );
  va_end( argp );

  ulSetError ( UL_WARNING, "%s", msgbuff ) ;
}


// Output a message
void _ssgParser::message( const char *format, ... )
{
  char msgbuff[ 255 ];
  va_list argp;

  char* msgptr = msgbuff;
  if (linenum)
  {
    msgptr += sprintf ( msgptr,"%s, line %d: ",
      path, linenum );
  }

  va_start( argp, format );
  vsprintf( msgptr, format, argp );
  va_end( argp );

  ulSetError ( UL_DEBUG, "%s", msgbuff ) ;
}

// Opens the file and does a few internal calculations based on the spec.
int _ssgParser::openFile( const char* fname, const _ssgParserSpec* _spec )
// returns TRUE on success
{
  if ( !_spec ) _spec = &default_spec ;

	if ( _spec->comment_string != NULL )
	{ assert ( _spec->comment_string [0] != 0 );
	}

  memset(this,0,sizeof(_ssgParser));
  memcpy( &spec, _spec, sizeof(spec) );
  ssgGetCurrentOptions () -> makeModelPath ( path, fname ) ;
  fileptr = fopen( path, "rb" );
  if ( ! fileptr )
	{
    error("cannot open file: %s",path);
		return FALSE;
	}
	eof = FALSE;
	// Calculate anyDelimiter and return.
	anyDelimiter[0] = 0;
	int length = 0;
	if ( spec.delim_chars_skipable != NULL )
	{ length +=strlen ( spec.delim_chars_skipable);
	  strcat(anyDelimiter, spec.delim_chars_skipable);
	}
	if ( spec.delim_chars_non_skipable  != NULL )
	{ length += strlen ( spec.delim_chars_non_skipable ) ;
	  strcat ( anyDelimiter, spec.delim_chars_non_skipable ) ;
	}
	if ( spec.open_brace_chars  != NULL )
	{ length +=strlen ( spec.open_brace_chars );
	  strcat ( anyDelimiter, spec.open_brace_chars );
	}
	if ( spec.close_brace_chars  != NULL )
	{ length +=strlen ( spec.close_brace_chars ) ;
	  strcat ( anyDelimiter, spec.close_brace_chars ) ;
	}
	assert ( length < MAX_DELIMITER_CHARS );
	return TRUE;
}


void _ssgParser::closeFile()
{
  fclose( fileptr ) ;
  fileptr = 0 ;
}

static char *EOF_string = "EOF reached";
static char *EOL_string = "EOL reached";

char* _ssgParser::getNextToken( const char* name )
// Fetches next token, even if it has to read over some empty or commant-only lines to get to it.
// Never returns NULL. Returns EOF_string on EOF.
{
	while(!( curtok < numtok ))
	{	//int startLevel = level;
	  //ulSetError(UL_DEBUG, "Forcing!");
		if(getLine( -999 ) == NULL) // -999
		{	if ( name )
				error("missing %s",name) ;
			return EOF_string;
		}
		assert(curtok==1);
		curtok=0; // redo the get one token that getLine does
	}
  char* token = 0 ;
  assert ( curtok < numtok );
  token = tokptr [ curtok++ ] ;
	return(token) ;
}

char *_ssgParser::peekAtNextToken( const char* name )
// Like getNextToken, but doesnÄt remove the token from the input stream
{
	while(!( curtok < numtok ))
	{	//int startLevel = level;
	  //ulSetError(UL_DEBUG, "Forcing!");
		if(getLine( -999 ) == NULL) // -999
		{	if ( name )
				error("missing %s",name) ;
			return EOF_string;
		}
		assert(curtok==1);
		curtok=0; // redo the get one token that getLine does
	}
  char* token = 0 ;
  assert ( curtok < numtok );
  token = tokptr [ curtok ] ;
	return(token) ;
}

  

int _ssgParser::getNextFloat( SGfloat &retVal, const char* name )
// returns TRUE on success
{
  char *endptr, *token = getNextToken(name);
  retVal = SGfloat(strtod( token, &endptr));
	if ( (endptr == NULL) || (*endptr == 0))
    return TRUE;
	else
	{ error("The field %s should contain a floating point number but contains %s",name, token) ;
		return FALSE;
	}
}

int _ssgParser::getNextInt( int & retVal, const char* name )
// returns TRUE on success
{
  char *endptr, *token = getNextToken(name);
  retVal = int(strtol( token, &endptr, 10));
	if ( (endptr == NULL) || (*endptr == 0))
    return TRUE;
	else
	{ error("The field %s should contain an integer number but contains %s",name, token) ;
		return FALSE;
	}
}

int _ssgParser::getNextString(char *&retVal, const char* name ) // returns TRUE on success
// wk: This is only for strings where we know they are inside spec.quote_chars, correct?
{
   char *token = getNextToken( NULL );
   
   if ( spec.quote_char && *token == spec.quote_char )
     {
	//knock off the quotes
	token++ ;
	int len = strlen( token ) ;
	if (len > 0 && token[len-1] == spec.quote_char)
	  token[len-1] = 0;
     }
   
   if( name != NULL && strcmp( token, name  ) ) 
     {
	error("Expected %s but got %s instead", name, token) ;
	return FALSE;	
     }
      
   retVal = token;
   return TRUE;
}

int _ssgParser::getNextUInt( unsigned int & retVal, const char* name )
// returns TRUE on success
{ char *endptr, *token = getNextToken(name);
  retVal = (unsigned int)(strtol( token, &endptr, 10));
	if ( (endptr == NULL) || (*endptr == 0))
    return TRUE;
	else
	{ error("The field %s should contain an integer number but contains %s",name, token) ;
		return FALSE;
	}
}


void _ssgParser::expectNextToken( const char* name )
// Swallows the next token. If it is not name, then there is an error message
{
  char* token = getNextToken(name);
  if (strcmp(token,name))
    error("missing %s",name) ;
}

// internal function. A token consisting of a single char has been found.
// This is copied to a new buffer, so that I have the space to add the 0.
void _ssgParser::addOneCharToken ( char *ptr ) 
{
	assert( (long)onechartokenbuf_ptr- (long)onechartokenbuf < 4096 ) ; // Buffer overflow
	
	onechartokenbuf_ptr [ 0 ] = *ptr;
	onechartokenbuf_ptr [ 1 ] = 0;
	tokptr [ numtok++ ] = onechartokenbuf_ptr;
	onechartokenbuf_ptr += 2; // prepare for nect onechartoken
}

static char *mystrchr( const char *string, int c )
// like strchr, but string may be NULL
{
	if (string == NULL )
		return NULL;
	else
		return strchr( string, c );
}


// gets the next line (no matter where it is), without tokenizing it
// useful for parsing text-formatted files which are identified by
// comments at the very beginning
char* _ssgParser::getRawLine()
// return NULL on eof
{
   tokbuf[0]=0;
   
   //get the next line with something on it
   if ( fgets ( linebuf, sizeof(linebuf), fileptr ) == NULL )
     { 
	eol = TRUE;
	eof = TRUE;
	return(0) ;
     }
   
   memcpy( tokbuf, linebuf, sizeof(linebuf) ) ;

   return tokbuf;
}

// wk: This works and is IMHO robust.
// However, I feel it could be smaller, more elegant and readable.
char* _ssgParser::getLine( int startLevel )
// return NULL on eof or if (level < startLevel)
{
	// throw away old tokens
  tokbuf [ 0 ] = 0 ;
  numtok = 0 ;
  curtok = 0 ;
	eol = FALSE;
	onechartokenbuf_ptr = onechartokenbuf ;
	
  //get the next line with something on it
  char* ptr = tokbuf , *tptr;
  while ( *ptr == 0 )
  {
		linenum++ ;
		if ( fgets ( linebuf, sizeof(linebuf), fileptr ) == NULL )
		{ eol = TRUE;
			eof = TRUE;
			return(0) ;
		}
		memcpy( tokbuf, linebuf, sizeof(linebuf) ) ;
		ptr = tokbuf ;

		// check for comments
		tptr=strchr(tokbuf, spec.comment_char);
		if ( tptr != NULL )
			*tptr = 0;
		if ( spec.comment_string != NULL )
		{
			tptr=strstr(tokbuf, spec.comment_string);
			if ( tptr != NULL )
				*tptr = 0;
		}

		//skip delim_chars
		if ( spec.delim_chars_skipable != NULL )
			while ( *ptr && strchr(spec.delim_chars_skipable,*ptr) )
				ptr++ ;
  }

  //tokenize the line
  numtok = 0 ;
  while ( *ptr )
  {
     //skip delim_chars
		if ( spec.delim_chars_skipable != NULL )
			while ( *ptr && strchr(spec.delim_chars_skipable,*ptr) )
				ptr++ ;

		if ( *ptr == 0 )
			break; // only skipable stuff left, dont create another token.
  
		// now unnessary?:
		if ( *ptr == spec.comment_char )
    {
      *ptr = 0 ;
      break;
    }

    //count the token
    tokptr [ numtok++ ] = ptr ;

    //handle quoted string
    if ( spec.quote_char && *ptr == spec.quote_char )
    {
      ptr++ ;
      while ( *ptr && *ptr != spec.quote_char )
        ptr++ ;
    }

    //adjust level
    if ( spec.open_brace_chars && *ptr && mystrchr(spec.open_brace_chars,*ptr) )
      level++ ;
    else if ( spec.close_brace_chars && *ptr && mystrchr(spec.close_brace_chars,*ptr) )
      level-- ;
		else
			//find end of token
			while ( *ptr && !strchr(anyDelimiter,*ptr) )
				ptr++ ;
		
		if ( *ptr != 0 )
			if ( ptr == tokptr [ numtok-1 ] )
			{ // we dont want tokens of length zero
				assert(NULL==mystrchr(spec.delim_chars_skipable,*ptr));
				// ptr is non-skipable, return it as token of length one
				numtok--;                  // remove zero-length token
				addOneCharToken ( ptr ) ;  // and add new token instead
				*ptr++ = 0;
				continue;
			}

    //mark end of token
		if( *ptr && ( mystrchr(spec.delim_chars_non_skipable,*ptr) 
			        || mystrchr(spec.open_brace_chars,*ptr)
							|| mystrchr(spec.close_brace_chars,*ptr) ) )
		{ 
			// ptr is non-skipable, return it as token of length one
			// additional to the one already in tokptr [ numtok-1 ].
			addOneCharToken ( ptr ) ;
			*ptr++ = 0;
		}
		if ( spec.delim_chars_skipable != NULL )
			while ( *ptr && strchr(spec.delim_chars_skipable,*ptr) )
				*ptr++ = 0 ;
  }
  if (level >= startLevel)
    return parseToken (0) ;
  return 0 ;
}


char* _ssgParser::parseToken( const char* name )
// returns the next token from the current line.
// Never returns NULL, but may return EOL_string
{
  char* token = EOL_string ;
  if ( curtok < numtok )
    token = tokptr [ curtok++ ] ;
  else 
	{ eol = TRUE;
		if ( name )
			error("missing %s",name) ;
	}
  return(token) ;
}


int _ssgParser::parseString(char *&retVal, const char* name ) // returns TRUE on success
// wk: This is only for strings where we know they are inside spec.quote_chars, correct?
{
  char* token = EOL_string ;
	retVal = EOL_string ;

  if ( curtok >= numtok )
  { eol = TRUE;
		if ( name )
	    error("missing %s",name) ;
		return FALSE;
	}
  
	if ( numtok > 0 && spec.quote_char && *tokptr [ curtok ] == spec.quote_char )
  {
    token = tokptr [ curtok++ ] ;

    //knock off the quotes
    token++ ;
    int len = strlen (token) ;
    if (len > 0 && token[len-1] == spec.quote_char)
       token[len-1] = 0 ;
  }
  else 
  { if ( name )
	    error("missing %s",name) ;
		return FALSE;
	}
   retVal = token;
  return TRUE;
}

int _ssgParser::parseDouble( double &retVal, const char* name )
// returns TRUE on success
{
  char *endptr, *token = parseToken(name);
  retVal = strtod( token, &endptr);
	if ( (endptr == NULL) || (*endptr == 0))
    return TRUE;
	else
	{ error("The field %s should contain a floating point number but contains %s",name, token) ;
		return FALSE;
	}
}

int _ssgParser::parseFloat( SGfloat &retVal, const char* name )
// returns TRUE on success
{
  char *endptr, *token = parseToken(name);
  retVal = SGfloat(strtod( token, &endptr));
	if ( (endptr == NULL) || (*endptr == 0))
    return TRUE;
	else
	{ error("The field %s should contain a floating point number but contains %s",name, token) ;
		return FALSE;
	}
}

int _ssgParser::parseInt(int &retVal, const char* name )
// returns TRUE on success
{
  char *endptr, *token = parseToken(name);
  retVal = int(strtol( token, &endptr, 10));
	if ( (endptr == NULL) || (*endptr == 0))
    return TRUE;
	else
	{ error("The field %s should contain an integer number but contains %s",name, token) ;
		return FALSE;
	}
}

int _ssgParser::parseUInt(unsigned int &retVal, const char* name )
// returns TRUE on success
{
  char *endptr, *token = parseToken(name);
	long l = strtol( token, &endptr, 10);
	if (l<0)
		message("The field %s should contain an UNSIGNED integer number but contains %s",name, token) ;
  retVal = (unsigned int)(l);
	if ( (endptr == NULL) || (*endptr == 0))
    return TRUE;
	else
	{ error("The field %s should contain an integer number but contains %s",name, token) ;
		return FALSE;
	}
}



void _ssgParser::expect( const char* name )
// Swallows the next token. If it is not name, then there is an error message
{
  char* token = parseToken(name);
  if (strcmp(token,name))
    error("missing %s",name) ;
}
