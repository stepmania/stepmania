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

#include <stdio.h>
#include "ssgLocal.h"
#include "ssgLoaderWriterStuff.h"



int ssgSaveQHI( const char* fname, ssgEntity *ent ) {
  FILE *fd = fopen ( fname, "w" ) ;
	int i;
 
  if ( fd == NULL ) {
    ulSetError ( UL_WARNING, "ssgSaveQHI: Failed to open '%s' for writing", 
		 fname ); 
    return FALSE ;
  }

  ssgVertexArray* vertices = new ssgVertexArray();
  
  
  sgMat4 ident;
  sgMakeIdentMat4( ident );
  ssgAccumVerticesAndFaces( ent, ident, vertices, NULL, 0.0001f );

	fprintf(fd, "3\n");  // Dimension
	fprintf(fd, "%d\n", vertices->getNum()); // No of points

	// Points:
  for (i = 0; i < vertices->getNum(); i++) {
    fprintf(fd, "%f %f %f\n", vertices->get(i)[0],
															vertices->get(i)[1],
															vertices->get(i)[2]);
  }


  fclose( fd ) ;

  delete vertices;
 
  return TRUE;
}
