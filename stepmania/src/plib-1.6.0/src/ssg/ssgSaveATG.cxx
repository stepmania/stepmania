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
/* 
 .scenery writer for SSG/PLIB
 ATG = ascii Terra Gear
 
 Written by Wolfram Kuss (Wolfram.Kuss@t-online.de) in May 2001

*/

#include <stdio.h>
#include "ssgLocal.h"
#include "ssgLoaderWriterStuff.h"



int ssgSaveATG( const char* fname, ssgEntity *ent ) {
  FILE *fd = fopen ( fname, "w" ) ;
	int i;
 
  if ( fd == NULL ) {
    ulSetError ( UL_WARNING, "ssgSaveATG: Failed to open '%s' for writing", 
		 fname ); 
    return FALSE ;
  }

  ssgVertexArray* vertices = new ssgVertexArray();
  ssgIndexArray*  indices  = new ssgIndexArray ();

  fprintf(fd, "# Created by ssgSaveATG. Original graph structure was:\n");
  ent->print(fd, "#", 0);

  sgMat4 ident;
  sgMakeIdentMat4( ident );
	ssgSimpleStateArray ssa; // = new ssgSimpleStateArray();
	ssgIndexArray*  materialIndices = new  ssgIndexArray();
	ssgTexCoordArray * texCoordArray = new ssgTexCoordArray();

  ssgAccumVerticesAndFaces( ent, ident, vertices, indices, /* epsilon = */ -1, &ssa,
		                  materialIndices, texCoordArray);
	assert ( vertices->getNum() == texCoordArray->getNum() );
  for (i = 0; i < vertices->getNum(); i++) {
    fprintf(fd, "v %f %f %f\n", vertices->get(i)[0],
															vertices->get(i)[1],
															vertices->get(i)[2]);
  }
// hack calc normal

  ssgNormalArray *na=new ssgNormalArray (vertices->getNum());
	sgVec3 myVec3;
	myVec3[0]=0.0; myVec3[1]=0.0; myVec3[2]=1.0;
	sgVec3 n;
  int ind1, ind2, ind3;
	for (i = 0; i < vertices->getNum(); i++)
		na->add(myVec3);


  for (i = 0; i < indices->getNum(); i += 3) {
		ind1 = *indices->get(i);
    ind2 = *indices->get(i + 1);
		ind3 = *indices->get(i + 2);
		if ((vertices->get(ind1)!=NULL) && (vertices->get(ind2)!=NULL) &&
			  (vertices->get(ind3)!=NULL)) // wk: kludge: why can it be NULL?
		{  sgMakeNormal( n, 
        vertices->get(ind1),
        vertices->get(ind2),
        vertices->get(ind3) );
    
    sgCopyVec3( na->get(ind1), n );
    sgCopyVec3( na->get(ind2), n );
    sgCopyVec3( na->get(ind3), n );
		}
  }







  for (i = 0; i < vertices->getNum(); i++) 
	{ float *f = na->get(i);
    fprintf(fd, "vn     %f %f %f\n", f[0], f[1], f[2]); 
	}
	for (i = 0; i < texCoordArray->getNum(); i++) 
    fprintf(fd, "vt %f %f\n", texCoordArray->get(i)[0], texCoordArray->get(i)[1]);
  
	// output all faces
	int runningIndex=0, lastIndex = -1;
  for (i = 0; i < indices->getNum(); i += 3) {

		// output material
		int matIndex = *(materialIndices->get(runningIndex++));
		if (( matIndex >= 0 ) && ( lastIndex != matIndex ))
		{	ssgSimpleState * ss = ssa.get( matIndex );
		  if ( ss->getTextureFilename() != NULL)
			{ // remove .rgb
				char *s1, *s2, * s = new char [ strlen(ss->getTextureFilename()) +1 ];
				assert ( s != NULL );
			  strcpy(s, ss->getTextureFilename());
				char *p = strrchr(s, '.');
				if ( p != NULL ) 
				{ 
					if (((p[2]=='a') || (p[2]=='A')) && ((p[3]=='f') || (p[3]=='F')))
					// *.?af textures are special:
					// the name before the ':' is not unique, you have to use the first character of the extension as
					// well. The tool af2rgb, which converts *.?af into rgb-files, also appends this char to the filename.
					{	p[0]=p[1];
						p[1]=0;
					}
				  else
					  *p = 0;
				}
				s2 = s;
				s1 = strrchr(s, '/');
				if ( s1 != NULL )
					s2 = ++s1;
				s1 = strrchr(s2, '\\');
				if ( s1 != NULL )
					s2 = ++s1;
				fprintf(fd, "# usemtl %s\n", s2); 
			  delete [] s;
			}
			lastIndex = matIndex;
		}
    // output face
		fprintf(fd, "f %d/%d %d/%d %d/%d\n", *indices->get(i), *indices->get(i),
															  *indices->get(i + 1), *indices->get(i + 1),
															  *indices->get(i + 2), *indices->get(i + 2) );
  }
	assert ( runningIndex == materialIndices->getNum() );
	delete materialIndices;
  
  fclose( fd ) ;

  delete vertices;
  delete indices ;
 
  return TRUE;
}
