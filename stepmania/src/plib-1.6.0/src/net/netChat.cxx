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

#include "netChat.h"

void
netChat::setTerminator (const char* t)
{
  if (terminator) delete[] terminator;
  terminator = ulStrDup(t);
}

const char*
netChat::getTerminator (void)
{
  return terminator;
}

// return the size of the largest prefix of needle at the end
// of haystack

#define MAX(a,b) (((a)>(b))?(a):(b))

static int
find_prefix_at_end (const netBuffer& haystack, const char* needle)
{
  const char* hd = haystack.getData();
  int hl = haystack.getLength();
  int nl = strlen(needle);
  
  for (int i = MAX (nl-hl, 0); i < nl; i++) {
    //if (haystack.compare (needle, hl-(nl-i), nl-i) == 0) {
    if (memcmp(needle, &hd[hl-(nl-i)], nl-i) == 0) {
      return (nl-i);
    }
  }
  return 0;
}

static int
find_terminator (const netBuffer& haystack, const char* needle)
{
  if (needle && *needle)
  {
    const char* data = haystack.getData();
    char* ptr = strstr(data,needle);
    if (ptr != NULL)
      return(ptr-data);
  }
  return -1;
}


void
netChat::handleBufferRead (netBuffer& in_buffer)
{
  // Continue to search for terminator in in_buffer,
  // while calling collect_incoming_data.  The while loop is
  // necessary because we might read several data+terminator combos
  // with a single recv().
  
  while (in_buffer.getLength()) {

    // special case where we're not using a terminator
    if (terminator == 0 || *terminator == 0) {
      collectIncomingData (in_buffer.getData(),in_buffer.getLength());
      in_buffer.remove ();
      return;
    }
    
    int terminator_len = strlen(terminator);
    
    int index = find_terminator ( in_buffer, terminator ) ;
    
    // 3 cases:
    // 1) end of buffer matches terminator exactly:
    //    collect data, transition
    // 2) end of buffer matches some prefix:
    //    collect data to the prefix
    // 3) end of buffer does not match any prefix:
    //    collect data
    
    if (index != -1) {
      // we found the terminator
      collectIncomingData ( in_buffer.getData(), index ) ;
      in_buffer.remove (0, index + terminator_len);
      foundTerminator();
    } else {
      // check for a prefix of the terminator
      int num = find_prefix_at_end (in_buffer, terminator);
      if (num) {
        int bl = in_buffer.getLength();
        // we found a prefix, collect up to the prefix
        collectIncomingData ( in_buffer.getData(), bl - num ) ;
        in_buffer.remove (0, bl - num);
        break;
      } else {
        // no prefix, collect it all
        collectIncomingData (in_buffer.getData(), in_buffer.getLength());
        in_buffer.remove();
      }
    }
  }
}

