#include "global.h"
#include "Model.h"
#include "ModelTypes.h"
#include "RageMath.h"
#include "RageDisplay.h"
#include "RageUtil.h"
#include "RageTextureManager.h"
#include "XmlFile.h"
#include "RageFile.h"
#include "RageLog.h"
#include "ActorUtil.h"
#include "ModelManager.h"
#include "Foreach.h"
#include "LuaBinding.h"
#include "PrefsManager.h"

REGISTER_ACTOR_CLASS( Model )

static const float FRAMES_PER_SECOND = 30;
static const RString DEFAULT_ANIMATION_NAME = "default";

Model::Model()
{
	m_bTextureWrapping = true;
	SetUseZBuffer( true );
	SetCullMode( CULL_BACK );
	m_pGeometry = NULL;
	m_pCurAnimation = NULL;
	m_bRevertToDefaultAnimation = false;
	m_fDefaultAnimationRate = 1;
	m_fCurAnimationRate = 1;
	m_pTempGeometry = NULL;
}

Model::~Model()
{
	Clear();
}

void Model::Clear()
{
	if( m_pGeometry )
	{
		MODELMAN->UnloadModel( m_pGeometry );
		m_pGeometry = NULL;
	}
	m_vpBones.clear();
	m_Materials.clear();
	m_mapNameToAnimation.clear();
	m_pCurAnimation = NULL;

	if( m_pTempGeometry )
		DISPLAY->DeleteCompiledGeometry( m_pTempGeometry );
}

void Model::Load( RString sFile )
{
	if( sFile == "" ) return;

	RString sExt = GetExtension(sFile);
	sExt.MakeLower();
	if( sExt=="txt" )
		LoadMilkshapeAscii( sFile );
}

#define THROW RageException::Throw( "Parse error in \"%s\" at line %d: '%s'", sPath.c_str(), iLineNum, sLine.c_str() )

void Model::LoadMilkshapeAscii( RString sPath )
{
	LoadPieces( sPath, sPath, sPath );
}

void Model::LoadPieces( RString sMeshesPath, RString sMaterialsPath, RString sBonesPath )
{
	Clear();

	// TRICKY: Load materials before geometry so we can figure out whether the materials require normals.
	
	LoadMaterialsFromMilkshapeAscii( sMaterialsPath );

	ASSERT( m_pGeometry == NULL );
	m_pGeometry = MODELMAN->LoadMilkshapeAscii( sMeshesPath, this->MaterialsNeedNormals() );

	/* Validate material indices. */
	for( unsigned i = 0; i < m_pGeometry->m_Meshes.size(); ++i )
	{
		const msMesh *pMesh = &m_pGeometry->m_Meshes[i];
		
		if( pMesh->nMaterialIndex >= (int) m_Materials.size() )
			RageException::Throw( "Model \"%s\" mesh \"%s\" references material index %i, but there are only %i materials",
				sMeshesPath.c_str(), pMesh->sName.c_str(), pMesh->nMaterialIndex, m_Materials.size() );
	}

	if( LoadMilkshapeAsciiBones( DEFAULT_ANIMATION_NAME, sBonesPath ) )
		PlayAnimation( DEFAULT_ANIMATION_NAME );


	//
    // Setup temp vertices (if necessary)
	//
	if( m_pGeometry->HasAnyPerVertexBones() )
	{
		m_vTempMeshes = m_pGeometry->m_Meshes;
		m_pTempGeometry = DISPLAY->CreateCompiledGeometry();
		m_pTempGeometry->Set( m_vTempMeshes, this->MaterialsNeedNormals() );
	}
}

void Model::LoadFromNode( const RString& sDir, const XNode* pNode )
{
	Actor::LoadFromNode( sDir, pNode );

	RString s1, s2, s3;
	pNode->GetAttrValue( "Meshes", s1 );
	pNode->GetAttrValue( "Materials", s2 );
	pNode->GetAttrValue( "Bones", s3 );
	if( !s1.empty() || !s2.empty() || !s3.empty() )
	{
		LuaHelpers::RunAtExpressionS( s1 );
		LuaHelpers::RunAtExpressionS( s2 );
		LuaHelpers::RunAtExpressionS( s3 );

		ASSERT( !s1.empty() && !s2.empty() && !s3.empty() );

		if( s1.Left(1) != "/" )
			s1 = sDir+s1;
		if( s2.Left(1) != "/" )
			s2 = sDir+s2;
		if( s3.Left(1) != "/" )
			s3 = sDir+s3;

		LoadPieces( s1, s2, s3 );
	}
}


void Model::LoadMaterialsFromMilkshapeAscii( RString sPath )
{
	FixSlashesInPlace(sPath);
	const RString sDir = Dirname( sPath );

	RageFile f;
	if( !f.Open( sPath ) )
		RageException::Throw( "Model::LoadMilkshapeAscii Could not open \"%s\": %s", sPath.c_str(), f.GetError().c_str() );

	RString sLine;
	int iLineNum = 0;

    while( f.GetLine( sLine ) > 0 )
    {
		iLineNum++;

        if( !strncmp (sLine, "//", 2) )
            continue;

        int nFrame;
        if( sscanf(sLine, "Frames: %d", &nFrame) == 1 )
        {
			// ignore
			// m_pModel->nTotalFrames = nFrame;
        }
        if( sscanf(sLine, "Frame: %d", &nFrame) == 1 )
        {
			// ignore
			// m_pModel->nFrame = nFrame;
        }


        //
        // materials
        //
        int nNumMaterials = 0;
        if( sscanf(sLine, "Materials: %d", &nNumMaterials) == 1 )
        {
            m_Materials.resize( nNumMaterials );
      
            char szName[256];

            for( int i = 0; i < nNumMaterials; i++ )
            {
				msMaterial& Material = m_Materials[i];

                // name
			    if( f.GetLine( sLine ) <= 0 )
					THROW;
                if( sscanf(sLine, "\"%[^\"]\"", szName) != 1 )
					THROW;
                Material.sName = szName;

                // ambient
			    if( f.GetLine( sLine ) <= 0 )
					THROW;
                RageVector4 Ambient;
                if( sscanf(sLine, "%f %f %f %f", &Ambient[0], &Ambient[1], &Ambient[2], &Ambient[3]) != 4 )
					THROW;
                memcpy( &Material.Ambient, &Ambient, sizeof(Material.Ambient) );

                // diffuse
			    if( f.GetLine( sLine ) <= 0 )
					THROW;
                RageVector4 Diffuse;
                if( sscanf(sLine, "%f %f %f %f", &Diffuse[0], &Diffuse[1], &Diffuse[2], &Diffuse[3]) != 4 )
					THROW;
                memcpy( &Material.Diffuse, &Diffuse, sizeof(Material.Diffuse) );

                // specular
			    if( f.GetLine( sLine ) <= 0 )
					THROW;
                RageVector4 Specular;
                if( sscanf(sLine, "%f %f %f %f", &Specular[0], &Specular[1], &Specular[2], &Specular[3]) != 4 )
					THROW;
                memcpy( &Material.Specular, &Specular, sizeof(Material.Specular) );

                // emissive
			    if( f.GetLine( sLine ) <= 0 )
					THROW;
                RageVector4 Emissive;
                if( sscanf (sLine, "%f %f %f %f", &Emissive[0], &Emissive[1], &Emissive[2], &Emissive[3]) != 4 )
					THROW;
                memcpy( &Material.Emissive, &Emissive, sizeof(Material.Emissive) );

                // shininess
			    if( f.GetLine( sLine ) <= 0 )
					THROW;
				char *p;
                float fShininess = strtof( sLine, &p );
                if( p == sLine )
					THROW;
                Material.fShininess = fShininess;

                // transparency
			    if( f.GetLine( sLine ) <= 0 )
					THROW;
                float fTransparency = strtof( sLine, &p );
                if( p == sLine )
					THROW;
                Material.fTransparency = fTransparency;

                // diffuse texture
			    if( f.GetLine( sLine ) <= 0 )
					THROW;
                strcpy( szName, "" );
                sscanf( sLine, "\"%[^\"]\"", szName );
                RString sDiffuseTexture = szName;

				if( sDiffuseTexture != "" )
				{
					RString sTexturePath = sDir + sDiffuseTexture;
					FixSlashesInPlace( sTexturePath );
					CollapsePath( sTexturePath );
					if( IsAFile(sTexturePath) )
						Material.diffuse.Load( sTexturePath );
					else
					{
						RString sError = ssprintf( "'%s' references a texture '%s' that does not exist", sPath.c_str(), sTexturePath.c_str() );
						RageException::Throw( sError );
					}
				}

                // alpha texture
			    if( f.GetLine( sLine ) <= 0 )
					THROW;
                strcpy( szName, "" );
                sscanf( sLine, "\"%[^\"]\"", szName );
				RString sAlphaTexture = szName;

				if( sAlphaTexture != "" )
				{
					RString sTexturePath = sDir + sAlphaTexture;
					FixSlashesInPlace( sTexturePath );
					CollapsePath( sTexturePath );
					if( IsAFile(sTexturePath) )
						Material.alpha.Load( sTexturePath );
					else
					{
						RString sError = ssprintf( "'%s' references a texture '%s' that does not exist", sPath.c_str(), sTexturePath.c_str() );
						RageException::Throw( sError );
					}
				}
            }
        }
    }

	f.Close();
}

bool Model::LoadMilkshapeAsciiBones( RString sAniName, RString sPath )
{
	m_mapNameToAnimation[sAniName] = msAnimation();
	msAnimation &Animation = m_mapNameToAnimation[sAniName];

	if( Animation.LoadMilkshapeAsciiBones( sAniName, sPath ) )
	{
		m_mapNameToAnimation.erase( sAniName );
		return false;
	}

	return true;
}

bool Model::EarlyAbortDraw() const
{
	return m_pGeometry == NULL || m_pGeometry->m_Meshes.empty();
}

void Model::DrawCelShaded()
{
	this->SetGlow(RageColor(0,0,0,1));
	this->SetDiffuseAlpha(0);
	DISPLAY->SetPolygonMode( POLYGON_LINE );
	DISPLAY->SetLineWidth( 4 );
	this->SetZWrite( false );
	this->Draw();
	this->SetDiffuseAlpha(1);
	this->SetGlow(RageColor(1,1,1,0));
	DISPLAY->SetPolygonMode( POLYGON_FILL );
	this->SetZWrite( true );
	this->Draw();
}

void Model::DrawPrimitives()
{
	Actor::SetGlobalRenderStates();	// set Actor-specified render states

	/* Don't if we're fully transparent */
	if( m_pTempState->diffuse[0].a < 0.001f && m_pTempState->glow.a < 0.001f )
		return;

	DISPLAY->Scale( 1, -1, 1 );	// flip Y so positive is up

	//////////////////////
	// render the diffuse pass
	//////////////////////
	if( m_pTempState->diffuse[0].a > 0 )
	{
		DISPLAY->SetTextureModeModulate();

		for( unsigned i = 0; i < m_pGeometry->m_Meshes.size(); ++i )
		{
			const msMesh *pMesh = &m_pGeometry->m_Meshes[i];


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

				DISPLAY->SetMaterial( Emissive, Ambient, Diffuse, mat.Specular, mat.fShininess );

				RageVector2 vTexTranslate = mat.diffuse.GetTextureTranslate();
				if( vTexTranslate.x != 0  ||  vTexTranslate.y != 0 )
				{
					DISPLAY->TexturePushMatrix();
					DISPLAY->TextureTranslate( vTexTranslate.x, vTexTranslate.y );
				}

				/* There's some common code that could be folded out here, but it seems
				 * clearer to keep it separate. */
				bool bUseMultitexture = PREFSMAN->m_bAllowMultitexture  &&  DISPLAY->GetNumTextureUnits() >= 2;
				if( bUseMultitexture )
				{
					// render the diffuse texture with texture unit 1
					DISPLAY->SetTexture( TextureUnit_1, mat.diffuse.GetCurrentTexture() );
					Actor::SetTextureRenderStates();	// set Actor-specified render states
					DISPLAY->SetSphereEnvironmentMapping( mat.diffuse.m_bSphereMapped );
					
					// render the additive texture with texture unit 2
					if( mat.alpha.GetCurrentTexture() )
					{
						DISPLAY->SetTexture( TextureUnit_2, mat.alpha.GetCurrentTexture() );
						Actor::SetTextureRenderStates();	// set Actor-specified render states
						DISPLAY->SetSphereEnvironmentMapping( mat.alpha.m_bSphereMapped );
						DISPLAY->SetTextureModeAdd();
						DISPLAY->SetTextureFiltering( true );
					}
					else
					{
						DISPLAY->SetTexture( TextureUnit_2, NULL );

						// set current texture back to 0 or else texture transform applied above 
						// isn't used.  Why?!?
						DISPLAY->SetTexture( TextureUnit_1, mat.diffuse.GetCurrentTexture() );
					}

					/* go */
					DrawMesh( i );

					// Turn off Environment mapping on tex unit 0.  Is there a better way to reset?
					DISPLAY->SetTexture( TextureUnit_1, NULL );
					DISPLAY->SetSphereEnvironmentMapping( 0 );
				}
				else
				{
					// render the diffuse texture
					DISPLAY->SetTexture( TextureUnit_1, mat.diffuse.GetCurrentTexture() );
					Actor::SetTextureRenderStates();	// set Actor-specified render states
					DISPLAY->SetSphereEnvironmentMapping( mat.diffuse.m_bSphereMapped );
					DrawMesh( i );
				
					// render the additive texture
					if( mat.alpha.GetCurrentTexture() )
					{
						DISPLAY->SetTexture( TextureUnit_1, mat.alpha.GetCurrentTexture() );
						Actor::SetTextureRenderStates();	// set Actor-specified render states

						DISPLAY->SetSphereEnvironmentMapping( mat.alpha.m_bSphereMapped );
						// UGLY:  This overrides the Actor's BlendMode
						DISPLAY->SetBlendMode( BLEND_ADD );
						DISPLAY->SetTextureFiltering( true );
						DrawMesh( i );
					}
				}

				if( vTexTranslate.x != 0  ||  vTexTranslate.y != 0 )
					DISPLAY->TexturePopMatrix();
			}
			else
			{
				static const RageColor emissive( 0,0,0,0 );
				static const RageColor ambient( 0.2f,0.2f,0.2f,1 );
				static const RageColor diffuse( 0.7f,0.7f,0.7f,1 );
				static const RageColor specular( 0.2f,0.2f,0.2f,1 );
				static const float shininess = 1;
				DISPLAY->SetMaterial( emissive, ambient, diffuse, specular, shininess );
				DISPLAY->ClearAllTextures();
				DISPLAY->SetSphereEnvironmentMapping( false );
				DrawMesh( i );
			}

			DISPLAY->SetSphereEnvironmentMapping( false );
			DISPLAY->SetBlendMode( BLEND_NORMAL );
		}
	}

	//////////////////////
	// render the glow pass
	//////////////////////
	if( m_pTempState->glow.a > 0.0001f )
	{
		DISPLAY->SetTextureModeGlow();

		for( unsigned i = 0; i < m_pGeometry->m_Meshes.size(); ++i )
		{
			const msMesh *pMesh = &m_pGeometry->m_Meshes[i];

			// apply material
			RageColor emissive = RageColor(0,0,0,0);
			RageColor ambient = m_pTempState->glow;
			RageColor diffuse = RageColor(0,0,0,0);
			RageColor specular = RageColor(0,0,0,0);
			float shininess = 1;

			DISPLAY->SetMaterial( emissive, ambient, diffuse, specular, shininess );
			DISPLAY->ClearAllTextures();

			if( pMesh->nMaterialIndex != -1 )
			{
				msMaterial& mat = m_Materials[ pMesh->nMaterialIndex ];
				DISPLAY->SetTexture( TextureUnit_1, mat.diffuse.GetCurrentTexture() );
				Actor::SetTextureRenderStates();	// set Actor-specified render states
			}
			else
			{
			}

			DrawMesh( i );
		}
	}
}

void Model::DrawMesh( int i ) const
{
	const msMesh *pMesh = &m_pGeometry->m_Meshes[i];

	// apply mesh-specific bone (if any)
	if( pMesh->nBoneIndex != -1 )
	{
		DISPLAY->PushMatrix();

		const RageMatrix &mat = m_vpBones[pMesh->nBoneIndex].mFinal;
		DISPLAY->PreMultMatrix( mat );
	}

	// Draw it
	const RageCompiledGeometry* TempGeometry = m_pTempGeometry ? m_pTempGeometry : m_pGeometry->m_pCompiledGeometry;
	DISPLAY->DrawCompiledGeometry( TempGeometry, i, m_pGeometry->m_Meshes );

	if( pMesh->nBoneIndex != -1 )
		DISPLAY->PopMatrix();
}

void Model::SetDefaultAnimation( RString sAnimation, float fPlayRate )
{
	m_sDefaultAnimation = sAnimation;
	m_fDefaultAnimationRate = fPlayRate;
}

void Model::PlayAnimation( RString sAniName, float fPlayRate )
{
	if( m_mapNameToAnimation.find(sAniName) == m_mapNameToAnimation.end() )
		return;

	const msAnimation *pNewAnimation = &m_mapNameToAnimation[sAniName];

	m_fCurFrame = 0;
	m_fCurAnimationRate = fPlayRate;

	if( m_pCurAnimation == pNewAnimation )
		return;

	m_pCurAnimation = pNewAnimation;

	// setup bones
	unsigned nBoneCount = m_pCurAnimation->Bones.size();
	m_vpBones.resize( nBoneCount );

	for( unsigned i = 0; i < nBoneCount; i++ )
	{
		const msBone *pBone = &m_pCurAnimation->Bones[i];
		const RageVector3 &vRot = pBone->Rotation;

		RageMatrixAngles( &m_vpBones[i].mRelative, vRot );
		
		m_vpBones[i].mRelative.m[3][0] = pBone->Position[0];
		m_vpBones[i].mRelative.m[3][1] = pBone->Position[1];
		m_vpBones[i].mRelative.m[3][2] = pBone->Position[2];
		
		int nParentBone = m_pCurAnimation->FindBoneByName( pBone->sParentName );
		if( nParentBone != -1 )
		{
			RageMatrixMultiply( &m_vpBones[i].mAbsolute, &m_vpBones[nParentBone].mAbsolute, &m_vpBones[i].mRelative );

			m_vpBones[i].mFinal = m_vpBones[i].mAbsolute;
		}
		else
		{
			m_vpBones[i].mAbsolute = m_vpBones[i].mRelative;
			m_vpBones[i].mFinal = m_vpBones[i].mRelative;
		}
	}

	// subtract out the bone's resting position
	for( unsigned i = 0; i < m_pGeometry->m_Meshes.size(); ++i )
	{
		msMesh *pMesh = &m_pGeometry->m_Meshes[i];
		vector<RageModelVertex> &Vertices = pMesh->Vertices;
		for( unsigned j = 0; j < Vertices.size(); j++ )
		{
//			int nBoneIndex = (pMesh->nBoneIndex!=-1) ? pMesh->nBoneIndex : bone;
			RageVector3 &pos = Vertices[j].p;
			int8_t bone = Vertices[j].bone;
			if( bone != -1 )
			{
				pos[0] -= m_vpBones[bone].mAbsolute.m[3][0];
				pos[1] -= m_vpBones[bone].mAbsolute.m[3][1];
				pos[2] -= m_vpBones[bone].mAbsolute.m[3][2];

				RageVector3 vTmp;

				RageMatrix inverse;
				RageMatrixTranspose( &inverse, &m_vpBones[bone].mAbsolute );	// transpose = inverse for rotation matrices
				RageVec3TransformNormal( &vTmp, &pos, &inverse );
				
				pos = vTmp;
			}
		}
	}
	
	/* Set up m_vpBones, just in case we're drawn without being Update()d. */
	SetBones( m_pCurAnimation, m_fCurFrame, m_vpBones );
	UpdateTempGeometry();
}

void Model::AdvanceFrame( float fDeltaTime )
{
	if( m_pGeometry == NULL || 
		m_pGeometry->m_Meshes.empty() || 
		!m_pCurAnimation )
	{
		return;	// bail early
	}

//	LOG->Trace( "m_fCurFrame = %f", m_fCurFrame );

	m_fCurFrame += FRAMES_PER_SECOND * fDeltaTime * m_fCurAnimationRate;
	if( m_fCurFrame >= m_pCurAnimation->nTotalFrames )
	{
		if( m_bRevertToDefaultAnimation && m_sDefaultAnimation != "" )
		{
			this->PlayAnimation( m_sDefaultAnimation, m_fDefaultAnimationRate );
			/* XXX: add to m_fCurFrame the wrapover from the previous
			 * m_fCurFrame-m_pCurAnimation->nTotalFrames, so it doesn't skip */
		}
		else
			m_fCurFrame -= m_pCurAnimation->nTotalFrames;
	}

	SetBones( m_pCurAnimation, m_fCurFrame, m_vpBones );
	UpdateTempGeometry();
}

void Model::SetBones( const msAnimation* pAnimation, float fFrame, vector<myBone_t> &vpBones )
{
	unsigned nBoneCount = pAnimation->Bones.size();
	for( unsigned i = 0; i < nBoneCount; i++ )
	{
		const msBone *pBone = &pAnimation->Bones[i];
		int nPositionKeyCount = pBone->PositionKeys.size();
		int nRotationKeyCount = pBone->RotationKeys.size();
		if( nPositionKeyCount == 0 && nRotationKeyCount == 0 )
		{
			vpBones[i].mFinal = vpBones[i].mAbsolute;
		}
		else
		{
			RageVector3 vPos;
			RageVector3 vRot;

			//
			// search for the adjacent position keys
			//
			const msPositionKey *pLastPositionKey = NULL, *pThisPositionKey = NULL;
			for( int j = 0; j < nPositionKeyCount; j++ )
			{
				const msPositionKey *pPositionKey = &pBone->PositionKeys[j];
				if( pPositionKey->fTime >= fFrame )
				{
					pThisPositionKey = pPositionKey;
					break;
				}
				pLastPositionKey = pPositionKey;
			}
			if( pLastPositionKey != NULL && pThisPositionKey != NULL )
			{
				float d = pThisPositionKey->fTime - pLastPositionKey->fTime;
				float s = (fFrame - pLastPositionKey->fTime) / d;
				vPos = pLastPositionKey->Position + (pThisPositionKey->Position - pLastPositionKey->Position) * s;
			}
			else if( pLastPositionKey == NULL )
				vPos = pThisPositionKey->Position;
			else if( pThisPositionKey == NULL )
				vPos = pLastPositionKey->Position;

			//
			// search for the adjacent rotation keys
			//
			RageMatrix m;
			RageMatrixIdentity( &m );
			const msRotationKey *pLastRotationKey = NULL, *pThisRotationKey = NULL;
			for( int j = 0; j < nRotationKeyCount; j++ )
			{
				const msRotationKey *pRotationKey = &pBone->RotationKeys[j];
				if( pRotationKey->fTime >= fFrame )
				{
					pThisRotationKey = pRotationKey;
					break;
				}
				pLastRotationKey = pRotationKey;
			}
			if( pLastRotationKey != 0 && pThisRotationKey != 0 )
			{
				const float s = SCALE( fFrame, pLastRotationKey->fTime, pThisRotationKey->fTime, 0, 1 );

				RageVector4 q1, q2, q;
				RageQuatFromHPR( &q1, pLastRotationKey->Rotation );
				RageQuatFromHPR( &q2, pThisRotationKey->Rotation );
				RageQuatSlerp( &q, q1, q2, s );

				RageMatrixFromQuat( &m, q );
			}
			else if( pLastRotationKey == 0 )
			{
				vRot = pThisRotationKey->Rotation;
				RageMatrixAngles( &m, vRot );
			}
			else if( pThisRotationKey == 0 )
			{
				vRot = pLastRotationKey->Rotation;
				RageMatrixAngles( &m, vRot );
			}
			m.m[3][0] = vPos[0];
			m.m[3][1] = vPos[1];
			m.m[3][2] = vPos[2];

			RageMatrixMultiply( &vpBones[i].mRelativeFinal, &vpBones[i].mRelative, &m );

			int nParentBone = pAnimation->FindBoneByName( pBone->sParentName );
			if( nParentBone == -1 )
				vpBones[i].mFinal = vpBones[i].mRelativeFinal;
			else
				RageMatrixMultiply( &vpBones[i].mFinal, &vpBones[nParentBone].mFinal, &vpBones[i].mRelativeFinal );
		}
	}
}

void Model::UpdateTempGeometry()
{
	if( m_pGeometry == NULL || m_pTempGeometry == NULL )
		return;

	for( unsigned i = 0; i < m_pGeometry->m_Meshes.size(); ++i )
	{
		const msMesh &origMesh = m_pGeometry->m_Meshes[i];
		msMesh &tempMesh = m_vTempMeshes[i];
		const vector<RageModelVertex> &origVertices = origMesh.Vertices;
		vector<RageModelVertex> &tempVertices = tempMesh.Vertices;
		for( unsigned j = 0; j < origVertices.size(); j++ )
		{
			RageVector3& tempPos =				tempVertices[j].p;
			RageVector3& tempNormal =			tempVertices[j].n;
			const RageVector3& originalPos =	origVertices[j].p;
			const RageVector3& originalNormal =	origVertices[j].n;
			int8_t bone =						origVertices[j].bone;

			if( bone == -1 )
			{
				tempNormal = originalNormal;
				tempPos = originalPos;
			}
			else
			{
				RageVec3TransformNormal( &tempNormal, &originalNormal, &m_vpBones[bone].mFinal );
				RageVec3TransformCoord( &tempPos, &originalPos, &m_vpBones[bone].mFinal );
			}
		}
	}

	// send the new vertices to the graphics card
	m_pTempGeometry->Change( m_vTempMeshes );
}

void Model::Update( float fDelta )
{
	Actor::Update( fDelta );
	AdvanceFrame( fDelta );

	for( unsigned i = 0; i < m_Materials.size(); ++i )
	{
		m_Materials[i].diffuse.Update( fDelta );
		m_Materials[i].alpha.Update( fDelta );
	}
}

int Model::GetNumStates() const
{
	int iMaxStates = 0;
	FOREACH_CONST( msMaterial, m_Materials, m )
		iMaxStates = max( iMaxStates, m->diffuse.GetNumStates() );
	return iMaxStates;
}

void Model::SetState( int iNewState )
{
	FOREACH( msMaterial, m_Materials, m )
	{
		m->diffuse.SetState( iNewState );
		m->alpha.SetState( iNewState );
	}
}

float Model::GetAnimationLengthSeconds() const
{
	float fSeconds = 0;
	FOREACH_CONST( msMaterial, m_Materials, m )
		fSeconds = max( fSeconds, m->diffuse.GetAnimationLengthSeconds() );
	return fSeconds;
}

void Model::SetSecondsIntoAnimation( float fSeconds )
{
	FOREACH( msMaterial, m_Materials, m )
	{
		m->diffuse.SetSecondsIntoAnimation( fSeconds );
		m->alpha.SetSecondsIntoAnimation( fSeconds );
	}
}

/*
void Model::HandleCommand( const Command &command )
{
	BeginHandleArgs;

	const RString& sName = command.GetName();
	if( sName=="play" )
	{
		PlayAnimation( sArg(1),fArg(2) );
	}
	else
	{
		Actor::HandleCommand( command );
		return;
	}

	EndHandleArgs;
}
*/

bool Model::MaterialsNeedNormals() const
{
	FOREACH_CONST( msMaterial, m_Materials, m )
	{
		if( m->NeedsNormals() )
			return true;
	}
	return false;
}

// lua start
#include "LuaBinding.h"

class LunaModel: public Luna<Model>
{
public:
	LunaModel() { LUA->Register( Register ); }

	static int playanimation( T* p, lua_State *L )	{ p->PlayAnimation(SArg(1),FArg(2)); return 0; }

	static void Register(lua_State *L) 
	{
		ADD_METHOD( playanimation );

		Luna<T>::Register( L );
	}
};

LUA_REGISTER_DERIVED_CLASS( Model, Actor )
// lua end

/*
 * (c) 2003-2004 Chris Danford
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
