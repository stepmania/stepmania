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


#ifndef _INCLUDED_SSGPARSER_H_
#define _INCLUDED_SSGPARSER_H_


// Be sure to read the ssg-documentation, especially the chapter
// on loaders/writers

// The _ssgParserSpec contains the rules how to extract the tokens:

struct _ssgParserSpec
{ // delimiters; Thats is, chars that delimit tokens:
  const char* delim_chars_skipable ;     // these are "swallowed" by the parser
  const char* delim_chars_non_skipable ; // These are handed to the app.
  const char* open_brace_chars ;
  const char* close_brace_chars ;
  char quote_char ;
  char comment_char ;           // For ex. ';' or '#'
	const char* comment_string;   // For example "//"
} ;


class _ssgParser
{
	enum { MAX_TOKENS = 50000 } ; 
  enum { MAX_DELIMITER_CHARS = 5000 } ;

  char path[ 256 ] ;
  _ssgParserSpec spec ;
  FILE* fileptr ;

  int linenum ;
  char linebuf[ 50000 ] ;
  char tokbuf[ 50000 ] ;
	char anyDelimiter [ MAX_DELIMITER_CHARS ] ;
  char* tokptr[ MAX_TOKENS ] ;
  int numtok ;
  int curtok ;

	char onechartokenbuf [ 50000 ];
	char *onechartokenbuf_ptr;
	void addOneCharToken ( char *ptr ) ;
  
public :
// ************************** general **************************
	int openFile( const char* fname, const _ssgParserSpec* spec = 0 ); // TRUE = success
  void closeFile();
  
  void error( const char *format, ... );
  void message( const char *format, ... );

	int eof ; // end of file reached yet?
	int eol ; // end of line reached yet?
// ************************** line-by-line API **************************
  int level;

   char* getLine( int startLevel=0 ); // may return NULL
   char* getRawLine();
	// All the name -parameters are only for error-messages

	// The parse... - functions get the next token from the current line
	// that was fetched with getLine and return the token as
	// char*, SGfloat, int, unsigned int or char*

  char* parseToken( const char* name );
	// These return TRUE on success:
  int parseFloat( SGfloat &retVal, const char* name ); 
  int parseDouble( double &retVal, const char* name ); 
  int parseInt(int &retVal, const char* name ); 
  int parseUInt(unsigned int &retVal, const char* name ); 
  int parseString(char *&retVal, const char* name ); 
  
  void expect( const char* name );

// ************************** line structure independant API **************************

  // These six functions get the next token - regardless of what line it is on
  char *getNextToken( const char* name );
  char *peekAtNextToken( const char* name );
  // These return TRUE on success:
  int getNextFloat( SGfloat &retVal, const char* name ); 
  int getNextInt( int & retVal, const char* name );
  int getNextString(char *&retVal, const char* name );
  int getNextUInt( unsigned int & retVal, const char* name );
  void expectNextToken( const char* name );
 } ;


#endif
