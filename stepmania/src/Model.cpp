#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: Model

 Desc: Types defined in msLib.h.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/
#include "Model.h"
#include "ModelTypes.h"
#include "mathlib.h"
#include "RageDisplay.h"
#include "RageUtil.h"
#include "RageTextureManager.h"
#include "IniFile.h"
#include "RageFile.h"
#include "RageLog.h"

const float FRAMES_PER_SECOND = 30;
const CString DEFAULT_ANIMATION_NAME = "default";

Model::Model ()
{
	m_pBones = NULL;
	m_bTextureWrapping = true;
	m_bUseZBuffer = true;
	m_pCurAnimation = NULL;
	m_bRevertToDefaultAnimation = false;
	m_fCurrAnimPlayRate = 1.0f;
	m_fDefaultAnimPlayRate = 1.0f;
}

Model::~Model ()
{
	Clear ();
}

void Model::Clear ()
{
	delete[] m_pBones;
	m_pBones = NULL;

	m_Meshes.clear();
	m_Materials.clear();
	m_mapNameToAnimation.clear();
	m_pCurAnimation = NULL;
}

bool Model::LoadFromModelFile( CString sPath )
{
	CString sDir, sThrowAway;
	splitrelpath( sPath, sDir, sThrowAway, sThrowAway );

	IniFile ini;
	ini.SetPath( sPath );
	ini.ReadFile();
	
	if( !ini.GetKey("Model") )
		RageException::Throw( "The model file '%s' is invalid.", sPath.c_str() );

	CString sFileName;
	ini.GetValue( "Model", "File", sFileName );
	LoadMilkshapeAscii( sDir+sFileName );

	float f;
	if( ini.GetValueF( "Model", "BaseRotationXDegrees", f ) )	Actor::SetBaseRotationX( f );
	if( ini.GetValueF( "Model", "BaseRotationYDegrees", f ) )	Actor::SetBaseRotationY( f );
	if( ini.GetValueF( "Model", "BaseRotationZDegrees", f ) )	Actor::SetBaseRotationZ( f );
	if( ini.GetValueF( "Model", "BaseZoomX", f ) )				Actor::SetBaseZoomX( f );
	if( ini.GetValueF( "Model", "BaseZoomY", f ) )				Actor::SetBaseZoomY( f );
	if( ini.GetValueF( "Model", "BaseZoomZ", f ) )				Actor::SetBaseZoomZ( f );
	
	return true;
}

bool Model::LoadMilkshapeAscii( CString sPath )
{
	CString sDir, sThrowAway;
	splitrelpath( sPath, sDir, sThrowAway, sThrowAway );

	FILE *file = fopen (sPath, "rt");
	if (!file)
		RageException::Throw( "Model::LoadMilkshapeAscii Could not open '%s'.", sPath.c_str() );

	Clear ();

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
			// ignore
			// m_pModel->nTotalFrames = nFrame;
        }
        if (sscanf (szLine, "Frame: %d", &nFrame) == 1)
        {
			// ignore
			// m_pModel->nFrame = nFrame;
        }

        int nNumMeshes = 0;
        if (sscanf (szLine, "Meshes: %d", &nNumMeshes) == 1)
        {
            m_Meshes.resize( nNumMeshes );

            for (i = 0; i < nNumMeshes && !bError; i++)
            {
				msMesh& mesh = m_Meshes[i];

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
                mesh.nMaterialIndex = (byte)nIndex;

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
                    vertex.nBoneIndex = (byte)nIndex;
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
            m_Materials.resize( nNumMaterials );
      
			int i;
            char szName[MS_MAX_NAME];

            for (i = 0; i < nNumMaterials && !bError; i++)
            {
				msMaterial& Material = m_Materials[i];

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

				if( strcmp(Material.szDiffuseTexture, "")!=0 )
				{
					CString sFile = Material.szDiffuseTexture;
					FixSlashesInPlace( sFile );
					CString sPath = sDir + Material.szDiffuseTexture;
					CollapsePath( sPath );
					if( IsAFile(sPath) )
						Material.aniTexture.Load( sPath );
				}

                // alpha texture
                if (!fgets (szLine, 256, file))
                {
                    bError = true;
                    break;
                }
                strcpy (szName, "");
                sscanf (szLine, "\"%[^\"]\"", szName);
                strcpy( Material.szAlphaTexture, szName );
            }
        }
    }

	fclose (file);

	LoadMilkshapeAsciiBones( DEFAULT_ANIMATION_NAME, sPath );

    // Setup temp vertices
	m_vTempVerticesByBone.resize( m_Meshes.size() );
	for (i = 0; i < (int)m_Meshes.size(); i++)
    {
		msMesh& Mesh = m_Meshes[i];
		m_vTempVerticesByBone[i].resize( Mesh.Vertices.size() );
	}	

	return true;
}

bool Model::LoadMilkshapeAsciiBones( CString sAniName, CString sPath )
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
			m_mapNameToAnimation[sAniName] = msAnimation();
			msAnimation &Animation = m_mapNameToAnimation[sAniName];

            int i;
            char szName[MS_MAX_NAME];

            Animation.Bones.resize( nNumBones );

            for (i = 0; i < nNumBones && !bError; i++)
            {
				msBone& Bone = Animation.Bones[i];

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

			// Ignore "Frames:" in file.  Calculate it ourself
			Animation.nTotalFrames = 0;
			for ( i = 0; i < (int)Animation.Bones.size(); i++)
			{
				msBone& Bone = Animation.Bones[i];
				for( int j=0; j<(int)Bone.PositionKeys.size(); j++ )
				{
					Animation.nTotalFrames = max( Animation.nTotalFrames, (int)Bone.PositionKeys[j].fTime+1 );
					Animation.nTotalFrames = max( Animation.nTotalFrames, (int)Bone.RotationKeys[j].fTime+1 );
				}
			}

			PlayAnimation( sAniName );
		}
	}

	fclose (file);

	return true;
}

void Model::DrawPrimitives()
{
	if(m_Meshes.empty())
		return;	// bail early

	Actor::SetRenderStates();	// set Actor-specified render states

	DISPLAY->Scale( 1, -1, 1 );	// flip Y so positive is up

	for (int i = 0; i < (int)m_Meshes.size(); i++)
	{
		msMesh *pMesh = &m_Meshes[i];
		RageModelVertexVector& TempVertices = m_vTempVerticesByBone[i];

		// apply material
		if( pMesh->nMaterialIndex != -1 )
		{
			msMaterial& mat = m_Materials[ pMesh->nMaterialIndex ];
			DISPLAY->SetMaterial( 
				mat.Emissive,
				mat.Ambient,
				mat.Diffuse,
				mat.Specular,
				mat.fShininess );
			DISPLAY->SetTexture( mat.aniTexture.GetCurrentTexture() );
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
			RageModelVertex& tempVert = TempVertices[j];
			msVertex& originalVert = pMesh->Vertices[j];

			memcpy( &tempVert.t, originalVert.uv, sizeof(originalVert.uv) );
			
			if( originalVert.nBoneIndex == -1 )
			{
				memcpy( &tempVert.n, originalVert.Normal, sizeof(originalVert.Normal) );
			
				memcpy( &tempVert.p, originalVert.Vertex, sizeof(originalVert.Vertex) );
			}
			else
			{
				int bone = originalVert.nBoneIndex;

				VectorRotate (originalVert.Normal, m_pBones[bone].mFinal, tempVert.n);

				VectorRotate (originalVert.Vertex, m_pBones[bone].mFinal, tempVert.p);
				tempVert.p[0] += m_pBones[bone].mFinal[0][3];
				tempVert.p[1] += m_pBones[bone].mFinal[1][3];
				tempVert.p[2] += m_pBones[bone].mFinal[2][3];
			}
		}

		DISPLAY->DrawIndexedTriangles( &TempVertices[0], pMesh->Vertices.size(), (Uint16*)&pMesh->Triangles[0], pMesh->Triangles.size()*3 );
	}

	DISPLAY->SetZBuffer( false );

}

void Model::SetDefaultAnimation( CString sAnimation )
{
	m_fDefaultAnimPlayRate = PlayRate;
	m_sDefaultAnimation = sAnimation;
}

void Model::PlayAnimation( CString sAniName )
{
	msAnimation *pNewAnimation = NULL;
	if( m_mapNameToAnimation.find(sAniName) == m_mapNameToAnimation.end() )
		return;
	else
		pNewAnimation = &m_mapNameToAnimation[sAniName];

	m_fCurrFrame = 0;

	if ( m_pCurAnimation == pNewAnimation )
		return;

	m_pCurAnimation = pNewAnimation;

	// setup bones
	int nBoneCount = (int)m_pCurAnimation->Bones.size();
	if (!m_pBones)
	{
		m_pBones = new myBone_t[nBoneCount];
	}

	int i, j;
	for (i = 0; i < nBoneCount; i++)
	{
		msBone *pBone = &m_pCurAnimation->Bones[i];
		msVec3 vRot;
		vRot[0] = pBone->Rotation[0] * 180 / (float) Q_PI;
		vRot[1] = pBone->Rotation[1] * 180 / (float) Q_PI;
		vRot[2] = pBone->Rotation[2] * 180 / (float) Q_PI;
		AngleMatrix (vRot, m_pBones[i].mRelative);
		m_pBones[i].mRelative[0][3] = pBone->Position[0];
		m_pBones[i].mRelative[1][3] = pBone->Position[1];
		m_pBones[i].mRelative[2][3] = pBone->Position[2];
		
		int nParentBone = m_pCurAnimation->FindBoneByName( pBone->szParentName );
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

	for (i = 0; i < (int)m_Meshes.size(); i++)
	{
		msMesh *pMesh = &m_Meshes[i];
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
}

float Model::GetCurFrame() { return m_fCurrFrame; };

void Model::SetFrame( float fNewFrame )
{
	m_fCurrFrame = fNewFrame;
}

void
Model::AdvanceFrame (float dt)
{
	if( m_Meshes.empty() || !m_pCurAnimation )
		return;	// bail early

	m_fCurrFrame += FRAMES_PER_SECOND * dt;
	if (m_fCurrFrame >= (float) m_pCurAnimation->nTotalFrames)
	{
		if( (m_bRevertToDefaultAnimation) && (m_sDefaultAnimation != "") )
		{
			this->PlayAnimation( m_sDefaultAnimation, m_fDefaultAnimPlayRate );
			m_fCurrFrame = 0.0f;
			return;
		}
		else
		{
			m_fCurrFrame = 0.0f;
		}
	}


	int nBoneCount = (int)m_pCurAnimation->Bones.size();
	int i, j;
	for (i = 0; i < nBoneCount; i++)
	{
		msBone *pBone = &m_pCurAnimation->Bones[i];
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
			int nParentBone = m_pCurAnimation->FindBoneByName( pBone->szParentName );
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

	for( int i=0; i<(int)m_Materials.size(); i++ )
		m_Materials[i].aniTexture.Update( fDelta );
}

void Model::SetState( int iNewState )
{
	for( int i=0; i<(int)m_Materials.size(); i++ )
		m_Materials[i].aniTexture.SetState( iNewState );
}

int Model::GetNumStates()
{
	int iMaxStates = 0;
	for( int i=0; i<(int)m_Materials.size(); i++ )
		iMaxStates = max( iMaxStates, m_Materials[i].aniTexture.GetNumStates() );
	return iMaxStates;
}
