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

static char            _ulErrorBuffer [ 1024 ] = { '\0' } ;
static ulErrorCallback _ulErrorCB = 0 ;

static const char* _ulSeverityText [ UL_MAX_SEVERITY ] =
{
  "DEBUG",
  "WARNING",
  "FATAL",
};
 

void ulSetError ( enum ulSeverity severity, const char *fmt, ... )
{
  va_list argp;
  va_start ( argp, fmt ) ;
  vsprintf ( _ulErrorBuffer, fmt, argp ) ;
  va_end ( argp ) ;
 
  if ( _ulErrorCB )
  {
    (*_ulErrorCB)( severity, _ulErrorBuffer ) ;
  }
  else
  {
    fprintf ( stderr, "%s: %s\n",
       _ulSeverityText[ severity ], _ulErrorBuffer ) ;
    if ( severity == UL_FATAL )
      exit (1) ;
  }
}
 

char* ulGetError ( void )
{
  return _ulErrorBuffer ;
}
 

void ulClearError ( void )
{
  _ulErrorBuffer [0] = 0 ;
}
 

ulErrorCallback ulGetErrorCallback ( void )
{
  return _ulErrorCB ;
}
 

void ulSetErrorCallback ( ulErrorCallback cb )
{
  _ulErrorCB = cb ;
}



