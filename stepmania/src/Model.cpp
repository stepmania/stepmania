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
#include "RageMath.h"
#include "RageDisplay.h"
#include "RageUtil.h"
#include "RageTextureManager.h"
#include "IniFile.h"
#include "RageFile.h"
#include "RageLog.h"
#include "ActorUtil.h"
#include <errno.h>

const float FRAMES_PER_SECOND = 30;
const CString DEFAULT_ANIMATION_NAME = "default";

Model::Model ()
{
	m_pBones = NULL;
	m_bTextureWrapping = true;
	SetUseZBuffer( true );
	m_pCurAnimation = NULL;
	m_bRevertToDefaultAnimation = false;
	m_fDefaultAnimationRate = 1;
	m_fCurAnimationRate = 1;
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

#define THROW RageException::Throw( "Parse at line %d: '%s'", sLine.c_str() );

bool Model::LoadMilkshapeAscii( CString sPath )
{
	FixSlashesInPlace(sPath);
	const CString sDir = Dirname( sPath );

	RageFile f;
	if( !f.Open( sPath ) )
		RageException::Throw( "Model::LoadMilkshapeAscii Could not open \"%s\": %s", sPath.c_str(), f.GetError().c_str() );

	Clear ();

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
			// m_pModel->nTotalFrames = nFrame;
        }
        if (sscanf (sLine, "Frame: %d", &nFrame) == 1)
        {
			// ignore
			// m_pModel->nFrame = nFrame;
        }

        int nNumMeshes = 0;
        if (sscanf (sLine, "Meshes: %d", &nNumMeshes) == 1)
        {
            m_Meshes.resize( nNumMeshes );

            for (i = 0; i < nNumMeshes; i++)
            {
				msMesh& mesh = m_Meshes[i];

			    if( f.GetLine( sLine ) <= 0 )
					THROW

                // mesh: name, flags, material index
                if (sscanf (sLine, "\"%[^\"]\" %d %d",szName, &nFlags, &nIndex) != 3)
					THROW

                strcpy( mesh.szName, szName );
//                mesh.nFlags = nFlags;
                mesh.nMaterialIndex = (byte)nIndex;

                //
                // vertices
                //
			    if( f.GetLine( sLine ) <= 0 )
					THROW

                int nNumVertices = 0;
                if (sscanf (sLine, "%d", &nNumVertices) != 1)
					THROW

				mesh.Vertices.resize( nNumVertices );

                for (j = 0; j < nNumVertices; j++)
                {
				    if( f.GetLine( sLine ) <= 0 )
						THROW

                    RageVector3 Vertex;
                    RageVector2 uv;
                    if (sscanf (sLine, "%d %f %f %f %f %f %d",
                        &nFlags,
                        &Vertex[0], &Vertex[1], &Vertex[2],
                        &uv[0], &uv[1],
                        &nIndex
                        ) != 7)
                    {
						THROW
                    }

					msVertex& vertex = mesh.Vertices[j];
//                  vertex.nFlags = nFlags;
                    memcpy( vertex.Vertex, Vertex, sizeof(vertex.Vertex) );
                    memcpy( vertex.uv, uv, sizeof(vertex.uv) );
                    vertex.nBoneIndex = (byte)nIndex;
					RageVec3AddToBounds( RageVector3(Vertex), m_vMins, m_vMaxs );
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

				mesh.Triangles.resize( nNumTriangles );

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

						msVertex& Vertex = mesh.Vertices[ nIndices[k] ];
						RageVector3& Normal = Normals[ nNormalIndices[k] ];
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
        if (sscanf (sLine, "Materials: %d", &nNumMaterials) == 1)
        {
            m_Materials.resize( nNumMaterials );
      
			int i;
            char szName[256];

            for (i = 0; i < nNumMaterials; i++)
            {
				msMaterial& Material = m_Materials[i];

                // name
			    if( f.GetLine( sLine ) <= 0 )
					THROW
                if (sscanf (sLine, "\"%[^\"]\"", szName) != 1)
					THROW
                strcpy( Material.szName, szName );

                // ambient
			    if( f.GetLine( sLine ) <= 0 )
					THROW
                RageVector4 Ambient;
                if (sscanf (sLine, "%f %f %f %f", &Ambient[0], &Ambient[1], &Ambient[2], &Ambient[3]) != 4)
					THROW
                memcpy( &Material.Ambient, &Ambient, sizeof(Material.Ambient) );

                // diffuse
			    if( f.GetLine( sLine ) <= 0 )
					THROW
                RageVector4 Diffuse;
                if (sscanf (sLine, "%f %f %f %f", &Diffuse[0], &Diffuse[1], &Diffuse[2], &Diffuse[3]) != 4)
					THROW
                memcpy( &Material.Diffuse, &Diffuse, sizeof(Material.Diffuse) );

                // specular
			    if( f.GetLine( sLine ) <= 0 )
					THROW
                RageVector4 Specular;
                if (sscanf (sLine, "%f %f %f %f", &Specular[0], &Specular[1], &Specular[2], &Specular[3]) != 4)
					THROW
                memcpy( &Material.Specular, &Specular, sizeof(Material.Specular) );

                // emissive
			    if( f.GetLine( sLine ) <= 0 )
					THROW
                RageVector4 Emissive;
                if (sscanf (sLine, "%f %f %f %f", &Emissive[0], &Emissive[1], &Emissive[2], &Emissive[3]) != 4)
					THROW
                memcpy( &Material.Emissive, &Emissive, sizeof(Material.Emissive) );

                // shininess
			    if( f.GetLine( sLine ) <= 0 )
					THROW
                float fShininess;
                if (sscanf (sLine, "%f", &fShininess) != 1)
					THROW
                Material.fShininess = fShininess;

                // transparency
			    if( f.GetLine( sLine ) <= 0 )
					THROW
                float fTransparency;
                if (sscanf (sLine, "%f", &fTransparency) != 1)
					THROW
                Material.fTransparency = fTransparency;

                // diffuse texture
			    if( f.GetLine( sLine ) <= 0 )
					THROW
                strcpy (szName, "");
                sscanf (sLine, "\"%[^\"]\"", szName);
                strcpy( Material.szDiffuseTexture, szName );

				if( strcmp(Material.szDiffuseTexture, "")!=0 )
				{
					CString sTexturePath = sDir + Material.szDiffuseTexture;
					FixSlashesInPlace( sTexturePath );
					CollapsePath( sTexturePath );
					if( IsAFile(sTexturePath) )
						Material.diffuse.Load( sTexturePath );
					else
					{
						CString sError = ssprintf( "'%s' references a texture '%s' that does not exist", sPath.c_str(), sTexturePath.c_str() );
						RageException::Throw( sError );
					}
				}

                // alpha texture
			    if( f.GetLine( sLine ) <= 0 )
					THROW
                strcpy (szName, "");
                sscanf (sLine, "\"%[^\"]\"", szName);
                strcpy( Material.szAlphaTexture, szName );

				if( strcmp(Material.szAlphaTexture, "")!=0 )
				{
					CString sTexturePath = sDir + Material.szAlphaTexture;
					FixSlashesInPlace( sTexturePath );
					CollapsePath( sTexturePath );
					if( IsAFile(sTexturePath) )
						Material.alpha.Load( sTexturePath );
					else
					{
						CString sError = ssprintf( "'%s' references a texture '%s' that does not exist", sPath.c_str(), sTexturePath.c_str() );
						RageException::Throw( sError );
					}
				}
            }
        }
    }

	f.Close();

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
	FixSlashesInPlace(sPath);
	const CString sDir = Dirname( sPath );

	RageFile f;
	if ( !f.Open(sPath) )
		RageException::Throw( "Model:: Could not open \"%s\": %s", sPath.c_str(), f.GetError().c_str() );

	CString sLine;
	int iLineNum = 0;
    int nFlags, j;

    while( f.GetLine( sLine ) > 0 )
    {
		iLineNum++;

        if (!strncmp (sLine, "//", 2))
            continue;

        //
        // bones
        //
        int nNumBones = 0;
        if (sscanf (sLine, "Bones: %d", &nNumBones) == 1)
        {
			m_mapNameToAnimation[sAniName] = msAnimation();
			msAnimation &Animation = m_mapNameToAnimation[sAniName];

            int i;
            char szName[MS_MAX_NAME];

            Animation.Bones.resize( nNumBones );

            for (i = 0; i < nNumBones; i++)
            {
				msBone& Bone = Animation.Bones[i];

                // name
			    if( f.GetLine( sLine ) <= 0 )
                {
					THROW;
                }
                if (sscanf (sLine, "\"%[^\"]\"", szName) != 1)
                {
					THROW;
                }
                strcpy( Bone.szName, szName );

                // parent
			    if( f.GetLine( sLine ) <= 0 )
                {
					THROW;
                }
                strcpy (szName, "");
                sscanf (sLine, "\"%[^\"]\"", szName);

                strcpy( Bone.szParentName, szName );

                // flags, position, rotation
                RageVector3 Position, Rotation;
			    if( f.GetLine( sLine ) <= 0 )
					THROW;
                if (sscanf (sLine, "%d %f %f %f %f %f %f",
                    &nFlags,
                    &Position[0], &Position[1], &Position[2],
                    &Rotation[0], &Rotation[1], &Rotation[2]) != 7)
                {
					THROW;
                }
                Bone.nFlags = nFlags;
                memcpy( &Bone.Position, &Position, sizeof(Bone.Position) );
                memcpy( &Bone.Rotation, &Rotation, sizeof(Bone.Rotation) );

                float fTime;

                // position key count
			    if( f.GetLine( sLine ) <= 0 )
					THROW;
                int nNumPositionKeys = 0;
                if (sscanf (sLine, "%d", &nNumPositionKeys) != 1)
					THROW;

                Bone.PositionKeys.resize( nNumPositionKeys );

                for (j = 0; j < nNumPositionKeys; j++)
                {
				    if( f.GetLine( sLine ) <= 0 )
						THROW;
                    if (sscanf (sLine, "%f %f %f %f", &fTime, &Position[0], &Position[1], &Position[2]) != 4)
						THROW;

					msPositionKey key;
					key.fTime = fTime;
					key.Position = RageVector3( Position[0], Position[1], Position[2] );
					Bone.PositionKeys[j] = key;
                }

                // rotation key count
			    if( f.GetLine( sLine ) <= 0 )
					THROW;
                int nNumRotationKeys = 0;
                if (sscanf (sLine, "%d", &nNumRotationKeys) != 1)
					THROW;

                Bone.RotationKeys.resize( nNumRotationKeys );

                for (j = 0; j < nNumRotationKeys; j++)
                {
				    if( f.GetLine( sLine ) <= 0 )
						THROW;
                    if (sscanf (sLine, "%f %f %f %f", &fTime, &Rotation[0], &Rotation[1], &Rotation[2]) != 4)
						THROW;

					msRotationKey key;
					key.fTime = fTime;
					key.Rotation = RageVector3( Rotation[0], Rotation[1], Rotation[2] );
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

	return true;
}

void Model::DrawPrimitives()
{
	if(m_Meshes.empty())
		return;	// bail early

	/* Don't if we're fully transparent */
	if( m_pTempState->diffuse[0].a < 0.001f && m_pTempState->glow.a < 0.001f )
		return;

	Actor::SetRenderStates();	// set Actor-specified render states

	DISPLAY->Scale( 1, -1, 1 );	// flip Y so positive is up

	//
	// process vertices
	//
	for (int i = 0; i < (int)m_Meshes.size(); i++)
	{
		msMesh *pMesh = &m_Meshes[i];
		RageModelVertexVector& TempVertices = m_vTempVerticesByBone[i];
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

				RageVec3TransformNormal( &tempVert.n, &originalVert.Normal, &m_pBones[bone].mFinal );

				RageVec3TransformCoord( &tempVert.p, &originalVert.Vertex, &m_pBones[bone].mFinal );
			}
		}
	}

	//////////////////////
	// render the diffuse pass
	//////////////////////
	if( m_pTempState->diffuse[0].a > 0 )
	{
		DISPLAY->SetTextureModeModulate();

		for (int i = 0; i < (int)m_Meshes.size(); i++)
		{
			msMesh *pMesh = &m_Meshes[i];
			RageModelVertexVector& TempVertices = m_vTempVerticesByBone[i];

			if( pMesh->nMaterialIndex != -1 )	// has a material
			{
			// apply material
				msMaterial& mat = m_Materials[ pMesh->nMaterialIndex ];

				RageColor Emissive = mat.Emissive;
				RageColor Ambient = mat.Ambient;
				RageColor Diffuse = mat.Diffuse;
				
				Emissive *= m_pTempState->diffuse[0];
				Ambient *= m_pTempState->diffuse[0];
				Diffuse *= m_pTempState->diffuse[0];

				DISPLAY->SetMaterial( 
					Emissive,
					Ambient,
					Diffuse,
					mat.Specular,
					mat.fShininess );

				// render the first pass with texture 1
				DISPLAY->SetTexture( 0, mat.diffuse.ani.GetCurrentTexture() );
				DISPLAY->SetSphereEnironmentMapping( mat.diffuse.bSphereMapped );
				// UGLY:  This overrides the Actor's BlendMode
//				DISPLAY->SetBlendMode( mat.diffuse.blendMode );

				// render the second pass with texture 2
				if( mat.alpha.ani.GetCurrentTexture() )
				{
					DISPLAY->SetTexture( 1, mat.alpha.ani.GetCurrentTexture() );
					DISPLAY->SetSphereEnironmentMapping( mat.alpha.bSphereMapped );
					// UGLY:  This overrides the Actor's BlendMode
					DISPLAY->SetTextureModeAdd();
				}

				DISPLAY->DrawIndexedTriangles( &TempVertices[0], pMesh->Vertices.size(), (Uint16*)&pMesh->Triangles[0], pMesh->Triangles.size()*3 );
				DISPLAY->SetSphereEnironmentMapping( false );
			}
			else
			{
				const static RageColor emissive( 0,0,0,0 );
				const static RageColor ambient( 0.2f,0.2f,0.2f,1 );
				const static RageColor diffuse( 0.7f,0.7f,0.7f,1 );
				const static RageColor specular( 0.2f,0.2f,0.2f,1 );
				const static float shininess = 1;
				DISPLAY->SetMaterial(
					emissive,
					ambient,
					diffuse,
					specular,
					shininess );
				DISPLAY->ClearAllTextures();
				DISPLAY->SetSphereEnironmentMapping( false );
				DISPLAY->DrawIndexedTriangles( &TempVertices[0], pMesh->Vertices.size(), (Uint16*)&pMesh->Triangles[0], pMesh->Triangles.size()*3 );
			}
		}
	}

	//////////////////////
	// render the glow pass
	//////////////////////
	if( m_pTempState->glow.a > 0.0001f )
	{
		DISPLAY->SetTextureModeGlow();

		for (int i = 0; i < (int)m_Meshes.size(); i++)
		{
			msMesh *pMesh = &m_Meshes[i];
			RageModelVertexVector& TempVertices = m_vTempVerticesByBone[i];

			// apply material
			if( pMesh->nMaterialIndex != -1 )
			{
				msMaterial& mat = m_Materials[ pMesh->nMaterialIndex ];

				RageColor Emissive = mat.Emissive;
				RageColor Ambient = mat.Ambient;
				RageColor Diffuse = mat.Diffuse;
				
				Emissive = m_pTempState->glow;
				Ambient = m_pTempState->glow;
				Diffuse = m_pTempState->glow;

				DISPLAY->SetMaterial( 
					Emissive,
					Ambient,
					Diffuse,
					mat.Specular,
					mat.fShininess );
				DISPLAY->ClearAllTextures();
				DISPLAY->SetTexture( 0, mat.diffuse.ani.GetCurrentTexture() );
			}
			else
			{
				RageColor emissive = m_pTempState->glow;
				RageColor ambient = m_pTempState->glow;
				RageColor diffuse = m_pTempState->glow;
				RageColor specular = m_pTempState->glow;
				float shininess = 1;
				DISPLAY->SetMaterial(
					emissive,
					ambient,
					diffuse,
					specular,
					shininess );
				DISPLAY->ClearAllTextures();
			}

			DISPLAY->DrawIndexedTriangles( &TempVertices[0], pMesh->Vertices.size(), (Uint16*)&pMesh->Triangles[0], pMesh->Triangles.size()*3 );
		}

	}
}

void Model::SetDefaultAnimation( CString sAnimation, float fPlayRate )
{
	m_sDefaultAnimation = sAnimation;
	m_fDefaultAnimationRate = fPlayRate;
}

void Model::PlayAnimation( CString sAniName, float fPlayRate )
{
	msAnimation *pNewAnimation = NULL;
	if( m_mapNameToAnimation.find(sAniName) == m_mapNameToAnimation.end() )
		return;
	else
		pNewAnimation = &m_mapNameToAnimation[sAniName];

	m_fCurrFrame = 0;
	m_fCurAnimationRate = fPlayRate;

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
		RageVector3 vRot;
		vRot[0] = pBone->Rotation[0] * 180 / (float) PI;
		vRot[1] = pBone->Rotation[1] * 180 / (float) PI;
		vRot[2] = pBone->Rotation[2] * 180 / (float) PI;

		RageMatrixAngles( &m_pBones[i].mRelative, vRot );
		
		m_pBones[i].mRelative.m[3][0] = pBone->Position[0];
		m_pBones[i].mRelative.m[3][1] = pBone->Position[1];
		m_pBones[i].mRelative.m[3][2] = pBone->Position[2];
		
		int nParentBone = m_pCurAnimation->FindBoneByName( pBone->szParentName );
		if (nParentBone != -1)
		{
			RageMatrixMultiply( &m_pBones[i].mAbsolute, &m_pBones[nParentBone].mAbsolute, &m_pBones[i].mRelative );

			m_pBones[i].mFinal = m_pBones[i].mAbsolute;
		}
		else
		{
			m_pBones[i].mAbsolute = m_pBones[i].mRelative;
			m_pBones[i].mFinal = m_pBones[i].mRelative;
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
				pVertex->Vertex[0] -= m_pBones[pVertex->nBoneIndex].mAbsolute.m[3][0];
				pVertex->Vertex[1] -= m_pBones[pVertex->nBoneIndex].mAbsolute.m[3][1];
				pVertex->Vertex[2] -= m_pBones[pVertex->nBoneIndex].mAbsolute.m[3][2];
				RageVector3 vTmp;

				RageMatrix inverse;
				RageMatrixTranspose( &inverse, &m_pBones[pVertex->nBoneIndex].mAbsolute );	// transpose = inverse for rotation matrices
				RageVec3TransformNormal( &vTmp, &pVertex->Vertex, &inverse );
				
				pVertex->Vertex = vTmp;
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

	m_fCurrFrame += FRAMES_PER_SECOND * dt * m_fCurAnimationRate;
	if (m_fCurrFrame >= (float)m_pCurAnimation->nTotalFrames)
	{
		if( (m_bRevertToDefaultAnimation) && (m_sDefaultAnimation != "") )
		{
			this->PlayAnimation( m_sDefaultAnimation, m_fDefaultAnimationRate );
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
			m_pBones[i].mFinal = m_pBones[i].mAbsolute;
		}
		else
		{
			RageVector3 vPos;
			RageVector3 vRot;
			//
			// search for the adjacent position keys
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
				vPos = pThisPositionKey->Position;
			}
			else if (pThisPositionKey == 0)
			{
				vPos = pLastPositionKey->Position;
			}
			//
			// search for the adjacent rotation keys
			//
			RageMatrix m;
			RageMatrixIdentity( &m );
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
				const float s = SCALE( m_fCurrFrame, pLastRotationKey->fTime, pThisRotationKey->fTime, 0, 1 );

				RageVector4 q1, q2, q;
				RageQuatFromHPR( &q1, RageVector3(pLastRotationKey->Rotation) * (180 / PI) );
				RageQuatFromHPR( &q2, RageVector3(pThisRotationKey->Rotation) * (180 / PI) );
				RageQuatSlerp( &q, q1, q2, s );

				RageMatrixFromQuat( &m, q );
			}
			else if (pLastRotationKey == 0)
			{
				vRot[0] = pThisRotationKey->Rotation[0] * 180 / (float) PI;
				vRot[1] = pThisRotationKey->Rotation[1] * 180 / (float) PI;
				vRot[2] = pThisRotationKey->Rotation[2] * 180 / (float) PI;
				RageMatrixAngles( &m, vRot );
			}
			else if (pThisRotationKey == 0)
			{
				vRot[0] = pLastRotationKey->Rotation[0] * 180 / (float) PI;
				vRot[1] = pLastRotationKey->Rotation[1] * 180 / (float) PI;
				vRot[2] = pLastRotationKey->Rotation[2] * 180 / (float) PI;
				RageMatrixAngles( &m, vRot );
			}
			m.m[3][0] = vPos[0];
			m.m[3][1] = vPos[1];
			m.m[3][2] = vPos[2];

			RageMatrixMultiply( &m_pBones[i].mRelativeFinal, &m_pBones[i].mRelative, &m );

			int nParentBone = m_pCurAnimation->FindBoneByName( pBone->szParentName );
			if (nParentBone == -1)
			{
				m_pBones[i].mFinal = m_pBones[i].mRelativeFinal;
			}
			else
			{
				RageMatrixMultiply( &m_pBones[i].mFinal, &m_pBones[nParentBone].mFinal, &m_pBones[i].mRelativeFinal );
			}
		}
	}
}

void Model::Update( float fDelta )
{
	Actor::Update( fDelta );
	AdvanceFrame( fDelta );

	for( int i=0; i<(int)m_Materials.size(); i++ )
	{
		m_Materials[i].diffuse.ani.Update( fDelta );
		m_Materials[i].alpha.ani.Update( fDelta );
	}
}

void Model::SetState( int iNewState )
{
	for( int i=0; i<(int)m_Materials.size(); i++ )
	{
		m_Materials[i].diffuse.ani.SetState( iNewState );
		m_Materials[i].alpha.ani.SetState( iNewState );
	}
}

int Model::GetNumStates()
{
	int iMaxStates = 0;
	for( int i=0; i<(int)m_Materials.size(); i++ )
		iMaxStates = max( iMaxStates, m_Materials[i].diffuse.ani.GetNumStates() );
	return iMaxStates;
}

void Model::HandleCommand( const ParsedCommand &command )
{
	HandleParams;

	/* XXX: It would be very useful to be able to tween animations, eg:
	 *
	 * play,Dance,1;sleep,2;linear,.5;play,Collapse,1
	 *
	 * to play "Dance" for two seconds, then tween to playing "Collapse" over half
	 * a second, with the tween percentage weighting the animations.
	 *
	 * Also, being able to queue animations cleanly without knowing the exact duration
	 * of the animation, eg:
	 *
	 * play,Dance,1;finishanim;play,Collapse,1
	 *
	 * to play "Dance", and then play "Collapse" when "Dance" finishes.  (In this case,
	 * Dance would presumably end on the same keyframe that Collapse begins on, since
	 * it isn't queuing a tween.)
	 *
	 * We need more architecture for this, so we can put custom items in the Actor
	 * tween queue.
	 */

	const CString& sName = sParam(0);
	if( sName=="play" )
		PlayAnimation( sParam(1),fParam(2) );
	else
	{
		Actor::HandleCommand( command );
		return;
	}

	CheckHandledParams;
}
