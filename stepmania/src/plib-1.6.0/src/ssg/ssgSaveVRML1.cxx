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
     License along with this library; if not, write to the Free
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 
     For further information visit http://plib.sourceforge.net

     $Id$
     
     --------------------------------------------------------------------

     This save routine was written by Warren Wilbur to support a sub-set
     of the standard Virtual Reality Modelling Language v1.0 (i.e. VRML1.0).
     The Version 1.0 Specification, 9-Nov-95 was used and should be available
     at www.vrml.org

     When reporting bugs/difficulties please mention 'VRML' in the subject
     of your email and post to the plib developers mailing list.
*/

#include <stdio.h>
#include "ssgLocal.h"
#include "ssgLoaderWriterStuff.h"

/* Function name:	SaveVRML1MaterialNode
 *
 * Limitations:		Before calling this function you must verify that
 *             		at least one material exists whose textureName
 *             		matches the one passed in to this function,
 *             		otherwise you might save a empty Material node!
 *
 * Notes:		Saving empty Material notes will create an
 *       		unnecessary choking hazard for VRML loaders :(
 */

static void SaveVRML1MaterialNode(FILE *fd, ssgIndexArray *materials_ptr,
                           ssgSimpleStateArray *ssa_ptr,
                           char *textureName, bool saveDiffuse,
                           bool saveAmbient,  bool saveEmission,
                           bool saveSpecular)
{
  ssgSimpleState *ss_ptr;
  int            i;

  /* Tell VRML loaders that we are providing a single color for each
   * face in the array of indices written below. */

  fprintf(fd, "    MaterialBinding { value PER_FACE }\n");
  fprintf(fd, "    Material {\n");

  if (saveDiffuse)
  {
    fprintf(fd, "        diffuseColor [\n");
    for (i = 0; i < materials_ptr->getNum(); i++)
    {
      ss_ptr = ssa_ptr->get(*(materials_ptr->get(i)));

           /* If we are trying to save all untextured materials then check
            * if either ptr is NULL */

      if ( ( (textureName == NULL)&&
             ((ss_ptr == NULL)||(ss_ptr->getTextureFilename() == NULL)) )||

           /* If we are trying to save all materials that are textured by
            * a specific texture then check if the texture filename matches */

           ( (textureName != NULL)&&(ss_ptr != NULL)&&
             (ss_ptr->getTextureFilename() != NULL)&&
             (!strcmp(textureName, ss_ptr->getTextureFilename())) ) )
      {
        float diffuse0, diffuse1, diffuse2;

        diffuse0 = ss_ptr->diffuse_colour[0];
        diffuse1 = ss_ptr->diffuse_colour[1];
        diffuse2 = ss_ptr->diffuse_colour[2];

#ifdef EXPERIMENTAL_ADD_AMBIENT_TO_DIFFUSE
        diffuse0 += ss_ptr->ambient_colour[0];
        diffuse1 += ss_ptr->ambient_colour[1];
        diffuse2 += ss_ptr->ambient_colour[2];
#endif //EXPERIMENTAL_ADD_AMBIENT_TO_DIFFUSE

#ifdef EXPERIMENTAL_ADD_EMISSION_TO_DIFFUSE
        diffuse0 += ss_ptr->emission_colour[0];
        diffuse1 += ss_ptr->emission_colour[1];
        diffuse2 += ss_ptr->emission_colour[2];
#endif //EXPERIMENTAL_ADD_EMISSION_TO_DIFFUSE

#ifdef EXPERIMENTAL_ADD_SPECULAR_TO_DIFFUSE
        diffuse0 += ss_ptr->specular_colour[0];
        diffuse1 += ss_ptr->specular_colour[1];
        diffuse2 += ss_ptr->specular_colour[2];
#endif //EXPERIMENTAL_ADD_SPECULAR_TO_DIFFUSE

        /* OpenGL caps the maximum RGB value for a colour to 1.0 when it
         * calculates colours in a scene. If we don't cap the value OpenGL
         * will do it for us. */

        fprintf(fd, "            %f %f %f,\n", diffuse0 > 1.0 ? 1.0:diffuse0,
                diffuse1 > 1.0 ? 1.0:diffuse1, diffuse2 > 1.0 ? 1.0:diffuse2);
      }
    }
    fprintf(fd, "        ]\n"); //close diffuseColor array
  }

  if (saveAmbient)
  {
    fprintf(fd, "        ambientColor [\n");
    for (i = 0; i < materials_ptr->getNum(); i++)
    {
      ss_ptr = ssa_ptr->get(*(materials_ptr->get(i)));

       /* If we are trying to save all untextured materials then check
        * if either ptr is NULL */

      if ( ( (textureName == NULL)&&
             ((ss_ptr == NULL)||(ss_ptr->getTextureFilename() == NULL)) )||

           /* If we are trying to save all materials that are textured by
            * a specific texture then check if the texture filename matches */

           ( (textureName != NULL)&&(ss_ptr != NULL)&&
             (ss_ptr->getTextureFilename() != NULL)&&
             (!strcmp(textureName, ss_ptr->getTextureFilename())) ) )
      {
        fprintf(fd, "            %f %f %f,\n", ss_ptr->ambient_colour[0],
                ss_ptr->ambient_colour[1], ss_ptr->ambient_colour[2]);
      }
      fprintf(fd, "        ]\n"); //close ambientColor array
    }
  }

  if(saveEmission)
  {
    fprintf(fd, "        emissiveColor [\n");
    for (i = 0; i < materials_ptr->getNum(); i++)
    {
      ss_ptr = ssa_ptr->get(*(materials_ptr->get(i)));

         /* If we are trying to save all untextured materials then check
          * if either ptr is NULL */

      if ( ( (textureName == NULL)&&
             ((ss_ptr == NULL)||(ss_ptr->getTextureFilename() == NULL)) )||

           /* If we are trying to save all materials that are textured by
            * a specific texture then check if the texture filename matches */

           ( (textureName != NULL)&&(ss_ptr != NULL)&&
             (ss_ptr->getTextureFilename() != NULL)&&
             (!strcmp(textureName, ss_ptr->getTextureFilename())) ) )
      {
        fprintf(fd, "            %f %f %f,\n", ss_ptr->emission_colour[0],
                ss_ptr->emission_colour[1], ss_ptr->emission_colour[2]);
      }
      fprintf(fd, "        ]\n"); //close emissionColor array
    }
  }

  if(saveSpecular)
  {
    fprintf(fd, "        specularColor [\n");
    for (i = 0; i < materials_ptr->getNum(); i++)
    {
      ss_ptr = ssa_ptr->get(*(materials_ptr->get(i)));

           /* If we are trying to save all untextured materials then check
            * if either ptr is NULL */

      if ( ( (textureName == NULL)&&
             ((ss_ptr == NULL)||(ss_ptr->getTextureFilename() == NULL)) )||

           /* If we are trying to save all materials that are textured by
            * a specific texture then check if the texture filename matches */

           ( (textureName != NULL)&&(ss_ptr != NULL)&&
             (ss_ptr->getTextureFilename() != NULL)&&
             (!strcmp(textureName, ss_ptr->getTextureFilename())) ) )
      {
        fprintf(fd, "            %f %f %f,\n", ss_ptr->specular_colour[0],
                ss_ptr->specular_colour[1], ss_ptr->specular_colour[2]);
      }
      fprintf(fd, "        ]\n"); //close specularColor array
    }
  }

  fprintf(fd, "    }\n"); //close Material node
  return;
}

/* The 'main' entry point for saving a model in VRML1.0 */

int ssgSaveVRML1( const char* fname, ssgEntity *ent ) {
  ssgVertexArray        *vertices_ptr;
  ssgIndexArray         *indices_ptr;
  FILE                  *fd;
  ssgSimpleStateArray   ssa;
  ssgTexCoordArray      *texcoord_ptr;
  ssgIndexArray         *materials_ptr;
  bool                  textured_faces_found, untextured_faces_found,
                        textureFacesAlreadySaved;
  int                   i, j, index1, index2, index3;
  ssgSimpleState        *ss_ptr, *ss_ptr2;
 
  fd = fopen ( fname, "w" ) ;
  if ( fd == NULL )
  {
    ulSetError ( UL_WARNING, "ssgSaveVRML1: Failed to open '%s' for writing", 
		 fname ); 
    return FALSE ;
  }

  vertices_ptr  = new ssgVertexArray();
  indices_ptr   = new ssgIndexArray();
  materials_ptr = new ssgIndexArray();
  texcoord_ptr  = new ssgTexCoordArray();
  
  sgMat4 ident;
  sgMakeIdentMat4( ident );
  ssgAccumVerticesAndFaces( ent, ident, vertices_ptr, indices_ptr, -1.0f,
                            &ssa, materials_ptr, texcoord_ptr);

  /* The spec requires every file to begin with these characters */

  fprintf(fd, "#VRML V1.0 ascii\n\n");

  /* Since a VRML file contains only one parent node we must use a
   * node type that can have several child 'nodes' so we can save
   * the materials, texture coordinates, vertices, and indices each
   * as (seperate) child nodes. */

  fprintf(fd, "Separator {\n");

    /* Save all the individual vertices used in the model. It doesn't
     * matter if there are duplicates... */

    fprintf(fd, "    Coordinate3 {\n        point [\n");

      for (i = 0; i < vertices_ptr->getNum(); i++)
      {
        fprintf(fd, "            %f %f %f,\n", vertices_ptr->get(i)[0],
                vertices_ptr->get(i)[1], vertices_ptr->get(i)[2]);
      }

    fprintf(fd, "        ]\n    }\n"); //close point array and
                                               //Coordinate3

    /* Chcck if the model is textured at all. This test will help us parse
     * out how to save the model since it may be totally textured, partially
     * textured, or not textured at all. */

    textured_faces_found = false;
    untextured_faces_found = false;
    for (i = 0; i < materials_ptr->getNum(); i++)
    {
      ss_ptr = ssa.get(*(materials_ptr->get(i)));
      if ( (ss_ptr != NULL)&&(ss_ptr->getTextureFilename() != NULL) )
      {
        textured_faces_found = true;
      }
      else
      {
        untextured_faces_found = true;
      }
    }

    if (untextured_faces_found)
    {
      /* Save all the material node fields which VRML supports. Note that the
       * VRML spec discourages complicated uses of the Material Node. We
       * cannot expect VRML implementations to support the full syntax
       * of the Material Node including ambient, diffuse, specular, emissive,
       * shininess, and transparency. We should be always be okay if we just
       * use diffuse. */

      SaveVRML1MaterialNode(fd, materials_ptr, &ssa, NULL,
                           true, false, false, false);

      /* Save all faces that are not textured in a single IndexedFaceSet node */

      fprintf(fd, "    IndexedFaceSet {\n        coordIndex [\n");
      for (i = 0; i < indices_ptr->getNum(); i+=3)
      {
        ss_ptr = ssa.get(*(materials_ptr->get(i/3)));

        /* Make sure this face doesn't have a texture associated with it */

        if ( (ss_ptr == NULL)||(ss_ptr->getTextureFilename() == NULL) )
	{
          index1 = *indices_ptr->get(i);
          index2 = *indices_ptr->get(i+1);
          index3 = *indices_ptr->get(i+2);

	  /* Check for index overflow since PLIB stores it as a short */

	  if ( (index1 < 0)||(index2 < 0)||(index3 < 0) )
	  {
            ulSetError(UL_WARNING, "ssgSaveVRML1: Save error: index overflow, "
                       "value won't fit in 16bits.");
	  }
	  else
	  {
            fprintf(fd, "            %d, %d, %d, -1,\n", index1, index2,
                                                         index3);
	  }
        }
      }
      fprintf(fd, "        ]\n    }\n"); //close coordIndex array and
                                         //IndexedFaceSet
    }

    if (textured_faces_found)
    {
      /* Save all texture coordinates (per-vertex) for all the textures in
       * the model. It doesn't matter if there is one texture or more than
       * one since we will specify which texture to use with the coordinates
       * before saving the portion of the indexed face set which uses that
       * texture. */

      fprintf(fd, "    TextureCoordinate2 {\n        point [\n");
      for (i = 0; i < texcoord_ptr->getNum(); i++)
      {

/* In VrmlView Pro 3.0 (Linux) textured models appear correct Left-Right
 * but the texture is reversed Top-Bottom. Enabling the INVERSE_REPEAT
 * macro fixes the problem for VrmlView. I don't want to enable this until
 * I figure out where the problem really is!? */

//#define INVERSE_REPEAT(a) (a > 0.0 ? 1.0 - a:a + 1.0)
#define INVERSE_REPEAT(a) a

        fprintf(fd, "            %f %f,\n", texcoord_ptr->get(i)[0],
                INVERSE_REPEAT(texcoord_ptr->get(i)[1]));
      }
      fprintf(fd, "        ]\n    }\n");

      /* Now save separate Texture2 and IndexedFaceSet node pairs for each
       * texture used in the model. Each of the IndexedFaceSet(s) will
       * reference back to the initial vertices, materials, and texture
       * coordinates (due to the lack of Seperator nodes in between).
       * Find the first textured face starting at the i'th face. In this
       * manner we will find the next texture used in the model and save
       * all faces that are textured with it. */

      for (i = 0; i < indices_ptr->getNum(); i+=3)
      {
        ss_ptr = ssa.get(*(materials_ptr->get(i/3)));
        if ( (ss_ptr != NULL)&&(ss_ptr->getTextureFilename() != NULL) )
        {
          /* We've found the next textured face. Since we save all
           * faces using a texture when we find the first face using
           * that texture we must check if the faces for this texture
           * have already been saved. If we can find a face using
           * this texture earlier in the list of faces then we know
           * that it has already been saved. */

          textureFacesAlreadySaved = false;
          for (j = 0; j < i; j+=3)
          {
            ss_ptr2 = ssa.get(*(materials_ptr->get(j/3)));
            if ( (ss_ptr2 != NULL)&&
                 (ss_ptr2->getTextureFilename() != NULL)&&
                 (!strcmp(ss_ptr->getTextureFilename(),
                          ss_ptr2->getTextureFilename())) )
            {
              textureFacesAlreadySaved = true;
              break;
            }
          }

          if (!textureFacesAlreadySaved)
          {
            fprintf(fd, "    Texture2 {\n");
            fprintf(fd, "        filename %s\n", ss_ptr->getTextureFilename());
//TODO: support CLAMP mode as well.
            fprintf(fd, "        wrapS    REPEAT\n");
            fprintf(fd, "        wrapT    REPEAT\n");
            fprintf(fd, "    }\n");

	    /* Save all the materials needed by this following indexed face
	     * set. This will save all materials that have the same texture
	     * filename specified below. */

            SaveVRML1MaterialNode(fd, materials_ptr, &ssa,
                                 ss_ptr->getTextureFilename(),
                                 true, false, false, false);

            fprintf(fd, "    IndexedFaceSet {\n        coordIndex [\n");
            for (j = i; j < indices_ptr->getNum(); j+=3)
            {
              /* Save each face which is textured by the Texture2 node defined
               * above. */

              ss_ptr2 = ssa.get(*(materials_ptr->get(j/3)));
              if ( (ss_ptr2 != NULL)&&
                   (ss_ptr2->getTextureFilename() != NULL)&&
                   (!strcmp(ss_ptr->getTextureFilename(),
                            ss_ptr2->getTextureFilename())) )
              {
                index1 = *indices_ptr->get(j);
                index2 = *indices_ptr->get(j+1);
                index3 = *indices_ptr->get(j+2);

                /* Check for index overflow since PLIB stores it as a
                 * short */

                if ( (index1 < 0)||(index2 < 0)||(index3 < 0) )
                {
                  ulSetError(UL_WARNING, "ssgSaveVRML1: Save error: index "
                             "overflow, value won't fit in 16bits.");
                }
                else
                {
                  fprintf(fd, "            %d, %d, %d, -1,\n",
                          index1, index2, index3);
                }
              }
            }
            fprintf(fd, "        ]\n    }\n"); //close coordIndex array and
                                               //IndexedFaceSet
          }
        }      
      }
    }

  fprintf(fd, "}\n"); //close Seperator
  fclose( fd ) ;

  delete vertices_ptr;
  delete indices_ptr;
  delete materials_ptr;
  delete texcoord_ptr;
  return TRUE;
}


