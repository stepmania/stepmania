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



int ssgSaveM( const char* fname, ssgEntity *ent ) {
  FILE *fd = fopen ( fname, "w" ) ;
	int i;
 
  if ( fd == NULL ) {
    ulSetError ( UL_WARNING, "ssgSaveM: Failed to open '%s' for writing", 
		 fname ); 
    return FALSE ;
  }

  ssgVertexArray* vertices = new ssgVertexArray();
  ssgIndexArray*  indices  = new ssgIndexArray ();

  fprintf(fd, "# Model output by ssgSaveM. Original graph structure was:\n");
  ent->print(fd, "#", 0);

  sgMat4 ident;
  sgMakeIdentMat4( ident );
  ssgAccumVerticesAndFaces( ent, ident, vertices, indices, -1 );

  for (i = 0; i < vertices->getNum(); i++) {
    fprintf(fd, "Vertex %d  %f %f %f\n", i+1,
	    vertices->get(i)[0],
	    vertices->get(i)[1],
	    vertices->get(i)[2]);
  }

  for (i = 0; i < indices->getNum(); i += 3) {
    fprintf(fd, "Face %d  %d %d %d\n", (i/3)+1,
	    *indices->get(i    ) + 1,
	    *indices->get(i + 1) + 1,
	    *indices->get(i + 2) + 1);
  }

  fclose( fd ) ;

  delete vertices;
  delete indices ;
 
  return TRUE;
}
