#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: Milkshape

 Desc: Types defined in msLib.h.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/
#include "Model.h"
#include "Milkshape.h"
#include "mathlib.h"
#include "RageDisplay.h"
#include "RageUtil.h"
#include "RageTextureManager.h"


const float FRAMES_PER_SECOND = 30;

Model::Model ()
{
	m_pModel = NULL;
	m_pBones = NULL;
}

Model::~Model ()
{
	Clear ();
}

void Model::Clear ()
{
	delete[] m_pBones;
	m_pBones = NULL;

	delete m_pModel;
	m_pModel = NULL;
}

bool Model::LoadMilkshapeAscii( CString sPath )
{
	CString sDir, sThrowAway;
	splitrelpath( sPath, sDir, sThrowAway, sThrowAway );

	FILE *file = fopen (sPath, "rt");
	if (!file)
		RageException::Throw( "Model:: Could not open '%s'.", sPath.c_str() );

	Clear ();

	m_pModel = new msModel;
// Never zero out a non-POD data type.
//	memset (m_pModel, 0, sizeof (msModel));

    bool bError = false;
    char szLine[256];
    char szName[MS_MAX_NAME];
    int nFlags, nIndex, i, j;

	ClearBounds (m_vMins, m_vMaxs);

    while (fgets (szLine, 256, file) != NULL  && !bError)
    {
        if (!strncmp (szLine, "//", 2))
            continue;

        int nFrame;
        if (sscanf (szLine, "Frames: %d", &nFrame) == 1)
        {
            m_pModel->nTotalFrames = nFrame;
        }
        if (sscanf (szLine, "Frame: %d", &nFrame) == 1)
        {
            m_pModel->nFrame = nFrame;
        }

        int nNumMeshes = 0;
        if (sscanf (szLine, "Meshes: %d", &nNumMeshes) == 1)
        {
            m_pModel->Meshes.resize( nNumMeshes );

            for (i = 0; i < nNumMeshes && !bError; i++)
            {
				msMesh& mesh = m_pModel->Meshes[i];

                if (!fgets (szLine, 256, file))
                {
                    bError = true;
                    break;
                }

                // mesh: name, flags, material index
                if (sscanf (szLine, "\"%[^\"]\" %d %d",szName, &nFlags, &nIndex) != 3)
                {
                    bError = true;
                    break;
                }

                strcpy( mesh.szName, szName );
//                mesh.nFlags = nFlags;
                mesh.nMaterialIndex = nIndex;

                //
                // vertices
                //
                if (!fgets (szLine, 256, file))
                {
                    bError = true;
                    break;
                }

                int nNumVertices = 0;
                if (sscanf (szLine, "%d", &nNumVertices) != 1)
                {
                    bError = true;
                    break;
                }

				mesh.Vertices.resize( nNumVertices );

                for (j = 0; j < nNumVertices; j++)
                {
                    if (!fgets (szLine, 256, file))
                    {
                        bError = true;
                        break;
                    }

                    msVec3 Vertex;
                    msVec2 uv;
                    if (sscanf (szLine, "%d %f %f %f %f %f %d",
                        &nFlags,
                        &Vertex[0], &Vertex[1], &Vertex[2],
                        &uv[0], &uv[1],
                        &nIndex
                        ) != 7)
                    {
                        bError = true;
                        break;
                    }

					msVertex& vertex = mesh.Vertices[j];
//                  vertex.nFlags = nFlags;
                    memcpy( vertex.Vertex, Vertex, sizeof(vertex.Vertex) );
                    memcpy( vertex.uv, uv, sizeof(vertex.uv) );
                    vertex.nBoneIndex = nIndex;
					AddPointToBounds (Vertex, m_vMins, m_vMaxs);
                }



                //
                // normals
                //
                if (!fgets (szLine, 256, file))
                {
                    bError = true;
                    break;
                }

                int nNumNormals = 0;
                if (sscanf (szLine, "%d", &nNumNormals) != 1)
                {
                    bError = true;
                    break;
                }
                
				vector<msVec3> Normals;
				Normals.resize( nNumNormals );
                for (j = 0; j < nNumNormals; j++)
                {
                    if (!fgets (szLine, 256, file))
                    {
                        bError = true;
                        break;
                    }

                    msVec3 Normal;
                    if (sscanf (szLine, "%f %f %f", &Normal[0], &Normal[1], &Normal[2]) != 3)
                    {
                        bError = true;
                        break;
                    }

					VectorNormalize (Normal);
                    Normals[j] = Normal;
                }



                //
                // triangles
                //
                if (!fgets (szLine, 256, file))
                {
                    bError = true;
                    break;
                }

                int nNumTriangles = 0;
                if (sscanf (szLine, "%d", &nNumTriangles) != 1)
                {
                    bError = true;
                    break;
                }

				mesh.Triangles.resize( nNumTriangles );

                for (j = 0; j < nNumTriangles; j++)
                {
                    if (!fgets (szLine, 256, file))
                    {
                        bError = true;
                        break;
                    }

                    word nIndices[3];
                    word nNormalIndices[3];
                    if (sscanf (szLine, "%d %hd %hd %hd %hd %hd %hd %d",
                        &nFlags,
                        &nIndices[0], &nIndices[1], &nIndices[2],
                        &nNormalIndices[0], &nNormalIndices[1], &nNormalIndices[2],
                        &nIndex
                        ) != 8)
                    {
                        bError = true;
                        break;
                    }

					// deflate the normals into vertices
					for( int k=0; k<3; k++ )
					{

						msVertex& Vertex = mesh.Vertices[ nIndices[k] ];
						msVec3& Normal = Normals[ nNormalIndices[k] ];
						Vertex.Normal = Normal;
					}

					msTriangle& Triangle = mesh.Triangles[j];
//                  Triangle.nFlags = nFlags;
                    memcpy( &Triangle.nVertexIndices, nIndices, sizeof(Triangle.nVertexIndices) );
//                  Triangle.nSmoothingGroup = nIndex;
                }
            }
        }

        //
        // materials
        //
        int nNumMaterials = 0;
        if (sscanf (szLine, "Materials: %d", &nNumMaterials) == 1)
        {
            m_pModel->Materials.resize( nNumMaterials );
      
			int i;
            char szName[MS_MAX_NAME];

            for (i = 0; i < nNumMaterials && !bError; i++)
            {
				msMaterial& Material = m_pModel->Materials[i];

                // name
                if (!fgets (szLine, 256, file))
                {
                    bError = true;
                    break;
                }
                if (sscanf (szLine, "\"%[^\"]\"", szName) != 1)
                {
                    bError = true;
                    break;
                }
                strcpy( Material.szName, szName );

                // ambient
                if (!fgets (szLine, 256, file))
                {
                    bError = true;
                    break;
                }
                msVec4 Ambient;
                if (sscanf (szLine, "%f %f %f %f", &Ambient.v[0], &Ambient.v[1], &Ambient.v[2], &Ambient.v[3]) != 4)
                {
                    bError = true;
                    break;
                }
                memcpy( &Material.Ambient, &Ambient, sizeof(Material.Ambient) );

                // diffuse
                if (!fgets (szLine, 256, file))
                {
                    bError = true;
                    break;
                }
                msVec4 Diffuse;
                if (sscanf (szLine, "%f %f %f %f", &Diffuse.v[0], &Diffuse.v[1], &Diffuse.v[2], &Diffuse.v[3]) != 4)
                {
                    bError = true;
                    break;
                }
                memcpy( &Material.Diffuse, &Diffuse, sizeof(Material.Diffuse) );

                // specular
                if (!fgets (szLine, 256, file))
                {
                    bError = true;
                    break;
                }
                msVec4 Specular;
                if (sscanf (szLine, "%f %f %f %f", &Specular.v[0], &Specular.v[1], &Specular.v[2], &Specular.v[3]) != 4)
                {
                    bError = true;
                    break;
                }
                memcpy( &Material.Specular, &Specular, sizeof(Material.Specular) );

                // emissive
                if (!fgets (szLine, 256, file))
                {
                    bError = true;
                    break;
                }
                msVec4 Emissive;
                if (sscanf (szLine, "%f %f %f %f", &Emissive.v[0], &Emissive.v[1], &Emissive.v[2], &Emissive.v[3]) != 4)
                {
                    bError = true;
                    break;
                }
                memcpy( &Material.Emissive, &Emissive, sizeof(Material.Emissive) );

                // shininess
                if (!fgets (szLine, 256, file))
                {
                    bError = true;
                    break;
                }
                float fShininess;
                if (sscanf (szLine, "%f", &fShininess) != 1)
                {
                    bError = true;
                    break;
                }
                Material.fShininess = fShininess;

                // transparency
                if (!fgets (szLine, 256, file))
                {
                    bError = true;
                    break;
                }
                float fTransparency;
                if (sscanf (szLine, "%f", &fTransparency) != 1)
                {
                    bError = true;
                    break;
                }
                Material.fTransparency = fTransparency;

                // diffuse texture
                if (!fgets (szLine, 256, file))
                {
                    bError = true;
                    break;
                }
                strcpy (szName, "");
                sscanf (szLine, "\"%[^\"]\"", szName);
                strcpy( Material.szDiffuseTexture, szName );

                // alpha texture
                if (!fgets (szLine, 256, file))
                {
                    bError = true;
                    break;
                }
                strcpy (szName, "");
                sscanf (szLine, "\"%[^\"]\"", szName);
                strcpy( Material.szAlphaTexture, szName );

				Material.pTexture = NULL;
				if( strcmp(Material.szDiffuseTexture, "")!=0 )
				{
					RageTextureID ID;
					ID.filename = sDir+Material.szDiffuseTexture;
					ID.bStretch = true;
					if( DoesFileExist(ID.filename) )
						Material.pTexture = TEXTUREMAN->LoadTexture( ID );
				}
            }
        }

        //
        // bones
        //
        int nNumBones = 0;
        if (sscanf (szLine, "Bones: %d", &nNumBones) == 1)
        {
            int i;
            char szName[MS_MAX_NAME];

            m_pModel->Bones.resize( nNumBones );

            for (i = 0; i < nNumBones && !bError; i++)
            {
				msBone& Bone = m_pModel->Bones[i];

                // name
                if (!fgets (szLine, 256, file))
                {
                    bError = true;
                    break;
                }
                if (sscanf (szLine, "\"%[^\"]\"", szName) != 1)
                {
                    bError = true;
                    break;
                }
                strcpy( Bone.szName, szName );

                // parent
                if (!fgets (szLine, 256, file))
                {
                    bError = true;
                    break;
                }
                strcpy (szName, "");
                sscanf (szLine, "\"%[^\"]\"", szName);

                strcpy( Bone.szParentName, szName );

                // flags, position, rotation
                msVec3 Position, Rotation;
                if (!fgets (szLine, 256, file))
                {
                    bError = true;
                    break;
                }
                if (sscanf (szLine, "%d %f %f %f %f %f %f",
                    &nFlags,
                    &Position.v[0], &Position.v[1], &Position.v[2],
                    &Rotation.v[0], &Rotation.v[1], &Rotation.v[2]) != 7)
                {
                    bError = true;
                    break;
                }
                Bone.nFlags = nFlags;
                memcpy( &Bone.Position, &Position, sizeof(Bone.Position) );
                memcpy( &Bone.Rotation, &Rotation, sizeof(Bone.Rotation) );

                float fTime;

                // position key count
                if (!fgets (szLine, 256, file))
                {
                    bError = true;
                    break;
                }
                int nNumPositionKeys = 0;
                if (sscanf (szLine, "%d", &nNumPositionKeys) != 1)
                {
                    bError = true;
                    break;
                }

                Bone.PositionKeys.resize( nNumPositionKeys );

                for (j = 0; j < nNumPositionKeys; j++)
                {
                    if (!fgets (szLine, 256, file))
                    {
                        bError = true;
                        break;
                    }
                    if (sscanf (szLine, "%f %f %f %f", &fTime, &Position[0], &Position[1], &Position[2]) != 4)
                    {
                        bError = true;
                        break;
                    }

					msPositionKey key = { fTime, { Position[0], Position[1], Position[2] } };
					Bone.PositionKeys[j] = key;
                }

                // rotation key count
                if (!fgets (szLine, 256, file))
                {
                    bError = true;
                    break;
                }
                int nNumRotationKeys = 0;
                if (sscanf (szLine, "%d", &nNumRotationKeys) != 1)
                {
                    bError = true;
                    break;
                }

                Bone.RotationKeys.resize( nNumRotationKeys );

                for (j = 0; j < nNumRotationKeys; j++)
                {
                    if (!fgets (szLine, 256, file))
                    {
                        bError = true;
                        break;
                    }
                    if (sscanf (szLine, "%f %f %f %f", &fTime, &Rotation[0], &Rotation[1], &Rotation[2]) != 4)
                    {
                        bError = true;
                        break;
                    }

					msRotationKey key = { fTime, { Rotation[0], Rotation[1], Rotation[2] } };
                    Bone.RotationKeys[j] = key;
                }
            }
        }
    }

	fclose (file);

	LoadMilkshapeAsciiBones( sPath );

    // Setup temp vertices
	m_vTempVerticesByBone.resize( m_pModel->Meshes.size() );
	for (i = 0; i < m_pModel->Meshes.size(); i++)
    {
		msMesh& Mesh = m_pModel->Meshes[i];
		m_vTempVerticesByBone[i].resize( Mesh.Vertices.size() );
	}	

	return true;
}

bool Model::LoadMilkshapeAsciiBones( CString sPath )
{
	CString sDir, sThrowAway;
	splitrelpath( sPath, sDir, sThrowAway, sThrowAway );

	FILE *file = fopen (sPath, "rt");
	if (!file)
		RageException::Throw( "Model:: Could not open '%s'.", sPath.c_str() );

    bool bError = false;
    char szLine[256];
    int nFlags, j;

    while (fgets (szLine, 256, file) != NULL  && !bError)
    {
        if (!strncmp (szLine, "//", 2))
            continue;

        //
        // bones
        //
        int nNumBones = 0;
        if (sscanf (szLine, "Bones: %d", &nNumBones) == 1)
        {
            int i;
            char szName[MS_MAX_NAME];

            m_pModel->Bones.resize( nNumBones );

            for (i = 0; i < nNumBones && !bError; i++)
            {
				msBone& Bone = m_pModel->Bones[i];

                // name
                if (!fgets (szLine, 256, file))
                {
                    bError = true;
                    break;
                }
                if (sscanf (szLine, "\"%[^\"]\"", szName) != 1)
                {
                    bError = true;
                    break;
                }
                strcpy( Bone.szName, szName );

                // parent
                if (!fgets (szLine, 256, file))
                {
                    bError = true;
                    break;
                }
                strcpy (szName, "");
                sscanf (szLine, "\"%[^\"]\"", szName);

                strcpy( Bone.szParentName, szName );

                // flags, position, rotation
                msVec3 Position, Rotation;
                if (!fgets (szLine, 256, file))
                {
                    bError = true;
                    break;
                }
                if (sscanf (szLine, "%d %f %f %f %f %f %f",
                    &nFlags,
                    &Position.v[0], &Position.v[1], &Position.v[2],
                    &Rotation.v[0], &Rotation.v[1], &Rotation.v[2]) != 7)
                {
                    bError = true;
                    break;
                }
                Bone.nFlags = nFlags;
                memcpy( &Bone.Position, &Position, sizeof(Bone.Position) );
                memcpy( &Bone.Rotation, &Rotation, sizeof(Bone.Rotation) );

                float fTime;

                // position key count
                if (!fgets (szLine, 256, file))
                {
                    bError = true;
                    break;
                }
                int nNumPositionKeys = 0;
                if (sscanf (szLine, "%d", &nNumPositionKeys) != 1)
                {
                    bError = true;
                    break;
                }

                Bone.PositionKeys.resize( nNumPositionKeys );

                for (j = 0; j < nNumPositionKeys; j++)
                {
                    if (!fgets (szLine, 256, file))
                    {
                        bError = true;
                        break;
                    }
                    if (sscanf (szLine, "%f %f %f %f", &fTime, &Position[0], &Position[1], &Position[2]) != 4)
                    {
                        bError = true;
                        break;
                    }

					msPositionKey key = { fTime, { Position[0], Position[1], Position[2] } };
					Bone.PositionKeys[j] = key;
                }

                // rotation key count
                if (!fgets (szLine, 256, file))
                {
                    bError = true;
                    break;
                }
                int nNumRotationKeys = 0;
                if (sscanf (szLine, "%d", &nNumRotationKeys) != 1)
                {
                    bError = true;
                    break;
                }

                Bone.RotationKeys.resize( nNumRotationKeys );

                for (j = 0; j < nNumRotationKeys; j++)
                {
                    if (!fgets (szLine, 256, file))
                    {
                        bError = true;
                        break;
                    }
                    if (sscanf (szLine, "%f %f %f %f", &fTime, &Rotation[0], &Rotation[1], &Rotation[2]) != 4)
                    {
                        bError = true;
                        break;
                    }

					msRotationKey key = { fTime, { Rotation[0], Rotation[1], Rotation[2] } };
                    Bone.RotationKeys[j] = key;
                }
            }
		}
	}

	// ignore "Frames:" in file
	m_pModel->nTotalFrames = 0;
    for (int i = 0; i < m_pModel->Bones.size(); i++)
    {
		msBone& Bone = m_pModel->Bones[i];
		for( int j=0; j<Bone.PositionKeys.size(); j++ )
		{
			m_pModel->nTotalFrames = max( m_pModel->nTotalFrames, Bone.PositionKeys[j].fTime );
			m_pModel->nTotalFrames = max( m_pModel->nTotalFrames, Bone.RotationKeys[j].fTime );
		}
	}

	SetupBones ();

	return true;
}

#include "SDL_opengl.h"
#include "RageTimer.h"

void Model::DrawPrimitives()
{
	if (!m_pModel)
		return;

	DISPLAY->SetBlendModeNormal();
	DISPLAY->EnableZBuffer();

	for (int i = 0; i < (int)m_pModel->Meshes.size(); i++)
	{
		msMesh *pMesh = &m_pModel->Meshes[i];
		RageVertexVector& TempVertices = m_vTempVerticesByBone[i];

		// apply material
		if( pMesh->nMaterialIndex != -1 )
		{
			msMaterial& mat = m_pModel->Materials[ pMesh->nMaterialIndex ];
			DISPLAY->SetMaterial( 
				mat.Emissive,
				mat.Ambient,
				mat.Diffuse,
				mat.Specular,
				mat.fShininess );
			DISPLAY->SetTexture( mat.pTexture );
		}
		else
		{
			float emissive[4] = {0,0,0,0};
			float ambient[4] = {0.2f,0.2f,0.2f,1};
			float diffuse[4] = {0.7f,0.7f,0.7f,1};
			float specular[4] = {0.2f,0.2f,0.2f,1};
			float shininess = 1;
			DISPLAY->SetMaterial(
				emissive,
				ambient,
				diffuse,
				specular,
				shininess );
			DISPLAY->SetTexture( NULL );
		}

		// process vertices
		for (int j = 0; j < (int)pMesh->Vertices.size(); j++)
		{
			RageVertex& tempVert = TempVertices[j];
			msVertex& originalVert = pMesh->Vertices[j];

			tempVert.c = RageColor(1,1,1,1);

			memcpy( &tempVert.t, originalVert.uv, sizeof(originalVert.uv) );
			
			if( originalVert.nBoneIndex == -1 )
			{
				memcpy( &tempVert.n, originalVert.Normal, sizeof(originalVert.Normal) );
			
				memcpy( &tempVert.p, originalVert.Vertex, sizeof(originalVert.Vertex) );
			}
			else
			{
				VectorRotate (originalVert.Normal, m_pBones[originalVert.nBoneIndex].mFinal, tempVert.n);

				int bone = originalVert.nBoneIndex;
				VectorRotate (originalVert.Vertex, m_pBones[originalVert.nBoneIndex].mFinal, tempVert.p);
				tempVert.p[0] += m_pBones[bone].mFinal[0][3];
				tempVert.p[1] += m_pBones[bone].mFinal[1][3];
				tempVert.p[2] += m_pBones[bone].mFinal[2][3];
			}
		}

		DISPLAY->DrawIndexedTriangles( &TempVertices[0], (Uint16*)&pMesh->Triangles[0], pMesh->Triangles.size()*3 );
	}

	DISPLAY->DisableZBuffer();

}

float
Model::CalcDistance () const
{
	float dx = m_vMaxs[0] - m_vMins[0];
	float dy = m_vMaxs[1] - m_vMins[1];
	float dz = m_vMaxs[2] - m_vMins[2];
	float d = dx;
	if (dy > d)
		d = dy;
	if (dz > d)
		d = dz;

	return d;
}

void
Model::SetupBones ()
{
	int nBoneCount = (int)m_pModel->Bones.size();
	if (!m_pBones)
	{
		m_pBones = new myBone_t[nBoneCount];
	}

	int i, j;
	for (i = 0; i < nBoneCount; i++)
	{
		msBone *pBone = &m_pModel->Bones[i];
		msVec3 vRot;
		vRot[0] = pBone->Rotation[0] * 180 / (float) Q_PI;
		vRot[1] = pBone->Rotation[1] * 180 / (float) Q_PI;
		vRot[2] = pBone->Rotation[2] * 180 / (float) Q_PI;
		AngleMatrix (vRot, m_pBones[i].mRelative);
		m_pBones[i].mRelative[0][3] = pBone->Position[0];
		m_pBones[i].mRelative[1][3] = pBone->Position[1];
		m_pBones[i].mRelative[2][3] = pBone->Position[2];
		
		int nParentBone = m_pModel->FindBoneByName( pBone->szParentName );
		if (nParentBone != -1)
		{
			R_ConcatTransforms (m_pBones[nParentBone].mAbsolute, m_pBones[i].mRelative, m_pBones[i].mAbsolute);
			memcpy (m_pBones[i].mFinal, m_pBones[i].mAbsolute, sizeof (matrix_t));
		}
		else
		{
			memcpy (m_pBones[i].mAbsolute, m_pBones[i].mRelative, sizeof (matrix_t));
			memcpy (m_pBones[i].mFinal, m_pBones[i].mRelative, sizeof (matrix_t));
		}
	}

	for (i = 0; i < m_pModel->Meshes.size(); i++)
	{
		msMesh *pMesh = &m_pModel->Meshes[i];
		for (j = 0; j < (int)pMesh->Vertices.size(); j++)
		{
			msVertex *pVertex = &pMesh->Vertices[j];
			if (pVertex->nBoneIndex != -1)
			{
				pVertex->Vertex[0] -= m_pBones[pVertex->nBoneIndex].mAbsolute[0][3];
				pVertex->Vertex[1] -= m_pBones[pVertex->nBoneIndex].mAbsolute[1][3];
				pVertex->Vertex[2] -= m_pBones[pVertex->nBoneIndex].mAbsolute[2][3];
				msVec3 vTmp;
				VectorIRotate (pVertex->Vertex, m_pBones[pVertex->nBoneIndex].mAbsolute, vTmp);
				VectorCopy (vTmp, pVertex->Vertex);
			}
		}
	}

	m_fCurrFrame = (float) m_pModel->nFrame;
}

void
Model::AdvanceFrame (float dt)
{
	if (!m_pModel)
		return;

	m_fCurrFrame += FRAMES_PER_SECOND * dt;
	if (m_fCurrFrame > (float) m_pModel->nTotalFrames)
		m_fCurrFrame = 0.0f;

	int nBoneCount = (int)m_pModel->Bones.size();
	int i, j;
	for (i = 0; i < nBoneCount; i++)
	{
		msBone *pBone = &m_pModel->Bones[i];
		int nPositionKeyCount = pBone->PositionKeys.size();
		int nRotationKeyCount = pBone->RotationKeys.size();
		if (nPositionKeyCount == 0 && nRotationKeyCount == 0)
		{
			memcpy (m_pBones[i].mFinal, m_pBones[i].mAbsolute, sizeof (matrix_t));
		}
		else
		{
			msVec3 vPos;
			msVec4 vRot;
			//
			// search for the adjaced position keys
			//
			msPositionKey *pLastPositionKey = 0, *pThisPositionKey = 0;
			for (j = 0; j < nPositionKeyCount; j++)
			{
				msPositionKey *pPositionKey = &pBone->PositionKeys[j];
				if (pPositionKey->fTime >= m_fCurrFrame)
				{
					pThisPositionKey = pPositionKey;
					break;
				}
				pLastPositionKey = pPositionKey;
			}
			if (pLastPositionKey != 0 && pThisPositionKey != 0)
			{
				float d = pThisPositionKey->fTime - pLastPositionKey->fTime;
				float s = (m_fCurrFrame - pLastPositionKey->fTime) / d;
				vPos[0] = pLastPositionKey->Position[0] + (pThisPositionKey->Position[0] - pLastPositionKey->Position[0]) * s;
				vPos[1] = pLastPositionKey->Position[1] + (pThisPositionKey->Position[1] - pLastPositionKey->Position[1]) * s;
				vPos[2] = pLastPositionKey->Position[2] + (pThisPositionKey->Position[2] - pLastPositionKey->Position[2]) * s;
			}
			else if (pLastPositionKey == 0)
			{
				VectorCopy (pThisPositionKey->Position, vPos);
			}
			else if (pThisPositionKey == 0)
			{
				VectorCopy (pLastPositionKey->Position, vPos);
			}
			//
			// search for the adjaced rotation keys
			//
			matrix_t m;
			msRotationKey *pLastRotationKey = 0, *pThisRotationKey = 0;
			for (j = 0; j < nRotationKeyCount; j++)
			{
				msRotationKey *pRotationKey = &pBone->RotationKeys[j];
				if (pRotationKey->fTime >= m_fCurrFrame)
				{
					pThisRotationKey = pRotationKey;
					break;
				}
				pLastRotationKey = pRotationKey;
			}
			if (pLastRotationKey != 0 && pThisRotationKey != 0)
			{
				float d = pThisRotationKey->fTime - pLastRotationKey->fTime;
				float s = (m_fCurrFrame - pLastRotationKey->fTime) / d;
#if 1
				msVec4 q1, q2, q;
				AngleQuaternion (pLastRotationKey->Rotation, q1);
				AngleQuaternion (pThisRotationKey->Rotation, q2);
				QuaternionSlerp (q1, q2, s, q);
				QuaternionMatrix (q, m);
#else
				vRot[0] = pLastRotationKey->Rotation[0] + (pThisRotationKey->Rotation[0] - pLastRotationKey->Rotation[0]) * s;
				vRot[1] = pLastRotationKey->Rotation[1] + (pThisRotationKey->Rotation[1] - pLastRotationKey->Rotation[1]) * s;
				vRot[2] = pLastRotationKey->Rotation[2] + (pThisRotationKey->Rotation[2] - pLastRotationKey->Rotation[2]) * s;
				vRot[0] *= 180 / (float) Q_PI;
				vRot[1] *= 180 / (float) Q_PI;
				vRot[2] *= 180 / (float) Q_PI;
				AngleMatrix (vRot, m);
#endif
			}
			else if (pLastRotationKey == 0)
			{
				vRot[0] = pThisRotationKey->Rotation[0] * 180 / (float) Q_PI;
				vRot[1] = pThisRotationKey->Rotation[1] * 180 / (float) Q_PI;
				vRot[2] = pThisRotationKey->Rotation[2] * 180 / (float) Q_PI;
				AngleMatrix (vRot, m);
			}
			else if (pThisRotationKey == 0)
			{
				vRot[0] = pLastRotationKey->Rotation[0] * 180 / (float) Q_PI;
				vRot[1] = pLastRotationKey->Rotation[1] * 180 / (float) Q_PI;
				vRot[2] = pLastRotationKey->Rotation[2] * 180 / (float) Q_PI;
				AngleMatrix (vRot, m);
			}
			m[0][3] = vPos[0];
			m[1][3] = vPos[1];
			m[2][3] = vPos[2];
			R_ConcatTransforms (m_pBones[i].mRelative, m, m_pBones[i].mRelativeFinal);
			int nParentBone = m_pModel->FindBoneByName( pBone->szParentName );
			if (nParentBone == -1)
			{
				memcpy (m_pBones[i].mFinal, m_pBones[i].mRelativeFinal, sizeof (matrix_t));
			}
			else
			{
				R_ConcatTransforms (m_pBones[nParentBone].mFinal, m_pBones[i].mRelativeFinal, m_pBones[i].mFinal);
			}
		}
	}
}

void Model::Update( float fDelta )
{
	Actor::Update( fDelta );
	AdvanceFrame( fDelta );
}