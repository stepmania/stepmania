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

static ssgLoaderOptions* current_options;

ssgEntity* ssgLoadM( const char* fname, 
                    const ssgLoaderOptions* options ) {
  ssgSetCurrentOptions ( (ssgLoaderOptions*)options ) ;
  current_options = ssgGetCurrentOptions () ;

  char filename [ 1024 ] ;
  current_options -> makeModelPath ( filename, fname ) ;

  FILE* model_file = fopen(filename, "r");
  if(!model_file) {
    ulSetError(UL_WARNING, "ssgLoadM: Couldn't open file '%s'.",
      filename);
    return NULL;
  }
  
  ssgVertexArray* vertices = new ssgVertexArray;
  ssgNormalArray* normals  = new ssgNormalArray;
  ssgIndexArray*  indices  = new ssgIndexArray;
  
  int i, index;
  char line[256];
  sgVec3 zero = {0.0f, 0.0f, 0.0f};
  
  fgets(line, 256, model_file);
  while ( !feof(model_file) ) {
    char* token;
    
    switch (line[0]) {
    case '#':                    // comment, skip this line
      break;
    case 'V':
      sgVec3 vtx;
      token = strtok(line, " ");  // token should now be "Vertex"
      token = strtok(NULL, " ");  // token is vertex index now
      
      index = atoi(token) - 1;
      
      // fill out non-declared vertices with zero vectors
      while (index > vertices->getNum()) {
        vertices->add(zero);
        normals ->add(zero);
      }
      
      // get vertex coordinate
      for (i = 0; i < 3; i++) {
        token = strtok(NULL, " ");
        vtx[i] = (float) atof(token);
      }
      
      vertices->add(vtx) ;
      normals ->add(zero);
      
      break;
      
    case 'F':                   // face
      token = strtok(line, " ");  // token should now be "Face"
      token = strtok(NULL, " ");  // face index, ignored
      
      for (i = 0; i < 3; i++) {
        token = strtok(NULL, " ");
        indices->add( atoi(token)-1 );
      }
      
      break;
      
    case 'E':                  // Edge, ignored
      break;
      
    default:
      ulSetError(UL_WARNING, "ssgLoadM: Syntax error on line \"%s\".", line);
    }
    
    fgets(line, 256, model_file);
  }
  
  ssgSimpleState* state = new ssgSimpleState();
  state->setOpaque();
  state->disable(GL_BLEND);
  state->disable(GL_ALPHA_TEST);
  state->disable(GL_TEXTURE_2D);
  state->enable(GL_COLOR_MATERIAL);
  state->enable(GL_LIGHTING);
  state->setShadeModel(GL_SMOOTH);
  state->setMaterial(GL_AMBIENT , 0.7f, 0.7f, 0.0f, 1.0f);
  state->setMaterial(GL_DIFFUSE , 0.7f, 0.7f, 0.0f, 1.0f);
  state->setMaterial(GL_SPECULAR, 1.0f, 1.0f, 1.0f, 1.0f);
  state->setMaterial(GL_EMISSION, 0.0f, 0.0f, 0.0f, 1.0f);
  state->setShininess(50);
  
  // now calculate smooth normals (this code belongs elsewhere)
  for (i = 0; i < indices->getNum(); i += 3) {
    short idx0 = *indices->get(i    );
    short idx1 = *indices->get(i + 1);
    short idx2 = *indices->get(i + 2);
    
    sgVec3 normal;
    sgMakeNormal( normal,
      vertices->get(idx0),
      vertices->get(idx1),
      vertices->get(idx2) );
    
    sgAddVec3( normals->get(idx0), normal );
    sgAddVec3( normals->get(idx1), normal );
    sgAddVec3( normals->get(idx2), normal );
  }
  
  for (i = 0; i < vertices->getNum(); i++) {
    sgNormaliseVec3( normals->get(i) );
  }
  
  ssgVtxArray* leaf = new ssgVtxArray( GL_TRIANGLES,
    vertices,
    normals,
    NULL,
    NULL,
    indices );
  
  leaf->setCullFace( TRUE );
  leaf->setState( state );
  
  ssgLeaf* model = current_options -> createLeaf( leaf, NULL );
  
  return model;
}
