#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: RageModelGeometry

 Desc: Types defined in msLib.h.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/
#include "RageModelGeometry.h"
#include "RageUtil.h"
#include "RageFile.h"
#include "RageMath.h"
#include "RageDisplay.h"


RageModelGeometry::RageModelGeometry ()
{
	m_iRefCount = 1;
}

RageModelGeometry::~RageModelGeometry ()
{
    for (unsigned i = 0; i < m_Meshes.size(); i++)
    {
		msMesh& mesh = m_Meshes[i];
		RageModelVertexArray *&pVertices = mesh.Vertices;
		DISPLAY->DeleteRageModelVertexArray( pVertices );
		pVertices = NULL;
	}
}

void RageModelGeometry::OptimizeBones()
{
    for (unsigned i = 0; i < m_Meshes.size(); i++)
    {
		msMesh& mesh = m_Meshes[i];

		// check to see if all vertices have the same bone index
		bool bAllVertsUseSameBone = true;
		char nBoneIndex = !mesh.Vertices->sizeVerts()==0 ? mesh.Vertices->Bone(0) : (char) -1;
		if( nBoneIndex != -1 )
		{
			for (unsigned j = 1; j < mesh.Vertices->sizeVerts(); j++)
			{
				if( mesh.Vertices->Bone(j) != nBoneIndex )
				{
					bAllVertsUseSameBone = false;
					break;
				}
			}
		}
		if( bAllVertsUseSameBone )
		{
			mesh.nBoneIndex = nBoneIndex;

			// clear all vertex/bone associations;
			for (unsigned j = 0; j < mesh.Vertices->sizeVerts(); j++)
			{
				mesh.Vertices->Bone(j) = -1;
			}
		}
	}
}

#define THROW RageException::Throw( "Parse at line %d: '%s'", sLine.c_str() );

void RageModelGeometry::LoadMilkshapeAscii( CString sPath )
{
	FixSlashesInPlace(sPath);
	const CString sDir = Dirname( sPath );

	RageFile f;
	if( !f.Open( sPath ) )
		RageException::Throw( "RageModelGeometry::LoadMilkshapeAscii Could not open \"%s\": %s", sPath.c_str(), f.GetError().c_str() );

	CString sLine;
	int iLineNum = 0;
    char szName[MS_MAX_NAME];
    int nFlags, nIndex, i, j;

	RageVec3ClearBounds( m_vMins, m_vMaxs );

    while( f.GetLine( sLine ) > 0 )
    {
		iLineNum++;

        if (!strncmp (sLine, "//", 2))
            continue;

        int nFrame;
        if (sscanf (sLine, "Frames: %d", &nFrame) == 1)
        {
			// ignore
			// m_pRageModelGeometry->nTotalFrames = nFrame;
        }
        if (sscanf (sLine, "Frame: %d", &nFrame) == 1)
        {
			// ignore
			// m_pRageModelGeometry->nFrame = nFrame;
        }

        int nNumMeshes = 0;
        if (sscanf (sLine, "Meshes: %d", &nNumMeshes) == 1)
        {
			ASSERT( m_Meshes.empty() );
            m_Meshes.resize( nNumMeshes );

            for (i = 0; i < nNumMeshes; i++)
            {
				msMesh& mesh = m_Meshes[i];
				RageModelVertexArray *&pVertices = mesh.Vertices;
				pVertices = DISPLAY->CreateRageModelVertexArray();

			    if( f.GetLine( sLine ) <= 0 )
					THROW

                // mesh: name, flags, material index
                if (sscanf (sLine, "\"%[^\"]\" %d %d",szName, &nFlags, &nIndex) != 3)
					THROW

                strcpy( mesh.szName, szName );
//                mesh.nFlags = nFlags;
                mesh.nMaterialIndex = (byte)nIndex;

				mesh.nBoneIndex = -1;

                //
                // vertices
                //
			    if( f.GetLine( sLine ) <= 0 )
					THROW

                int nNumVertices = 0;
                if (sscanf (sLine, "%d", &nNumVertices) != 1)
					THROW

				pVertices->resizeVerts( nNumVertices );

                for (j = 0; j < nNumVertices; j++)
                {
				    if( f.GetLine( sLine ) <= 0 )
						THROW

                    RageVector3 pos;
                    RageVector2 uv;
                    if (sscanf (sLine, "%d %f %f %f %f %f %d",
                        &nFlags,
                        &pos[0], &pos[1], &pos[2],
                        &uv[0], &uv[1],
                        &nIndex
                        ) != 7)
                    {
						THROW
                    }

//                  vertex.nFlags = nFlags;
                    pVertices->Position(j) = pos;
                    pVertices->TexCoord(j) = uv;
                    pVertices->Bone(j) = (byte)nIndex;
					RageVec3AddToBounds( RageVector3(pos), m_vMins, m_vMaxs );
                }


                //
                // normals
                //
			    if( f.GetLine( sLine ) <= 0 )
					THROW

                int nNumNormals = 0;
                if (sscanf (sLine, "%d", &nNumNormals) != 1)
					THROW
                
				vector<RageVector3> Normals;
				Normals.resize( nNumNormals );
                for (j = 0; j < nNumNormals; j++)
                {
				    if( f.GetLine( sLine ) <= 0 )
						THROW

                    RageVector3 Normal;
                    if (sscanf (sLine, "%f %f %f", &Normal[0], &Normal[1], &Normal[2]) != 3)
						THROW

					RageVec3Normalize( (RageVector3*)&Normal, (RageVector3*)&Normal );
                    Normals[j] = Normal;
                }



                //
                // triangles
                //
			    if( f.GetLine( sLine ) <= 0 )
					THROW

                int nNumTriangles = 0;
                if (sscanf (sLine, "%d", &nNumTriangles) != 1)
					THROW

				pVertices->resizeTriangles( nNumTriangles );

                for (j = 0; j < nNumTriangles; j++)
                {
				    if( f.GetLine( sLine ) <= 0 )
						THROW

                    word nIndices[3];
                    word nNormalIndices[3];
                    if (sscanf (sLine, "%d %hd %hd %hd %hd %hd %hd %d",
                        &nFlags,
                        &nIndices[0], &nIndices[1], &nIndices[2],
                        &nNormalIndices[0], &nNormalIndices[1], &nNormalIndices[2],
                        &nIndex
                        ) != 8)
                    {
						THROW
                    }

					// deflate the normals into vertices
					for( int k=0; k<3; k++ )
					{
						//RageModelVertex& vertex = mesh.Vertices[ nIndices[k] ];
						//RageVector3& normal = Normals[ nNormalIndices[k] ];
						//vertex.n = normal;
						mesh.Vertices->Normal( nIndices[k] ) = Normals[ nNormalIndices[k] ];
					}

					msTriangle& Triangle = pVertices->Triangle(j);
//                  Triangle.nFlags = nFlags;
                    memcpy( &Triangle.nVertexIndices, nIndices, sizeof(Triangle.nVertexIndices) );
//                  Triangle.nSmoothingGroup = nIndex;
                }
            }
        }
    }

	OptimizeBones();

	// send the finalized vertices to the graphics card
	{
		for( unsigned i = 0; i < m_Meshes.size(); i++ )
		{
			msMesh& mesh = m_Meshes[i];
			RageModelVertexArray *&pVertices = mesh.Vertices;
			pVertices->OnChanged();
		}
	}

	f.Close();
}

