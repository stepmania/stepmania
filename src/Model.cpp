#include "global.h"
#include "Model.h"
#include "RageMath.hpp"
#include "ModelTypes.h"
#include "RageMath.h"
#include "RageMatrix.hpp"
#include "RageDisplay.h"
#include "RageUtil.h"
#include "RageTextureManager.h"
#include "XmlFile.h"
#include "RageFile.h"
#include "RageLog.h"
#include "ActorUtil.h"
#include "ModelManager.h"
#include "LuaBinding.h"
#include "PrefsManager.h"

#include <numeric>

using std::vector;

REGISTER_ACTOR_CLASS( Model );

static const float FRAMES_PER_SECOND = 30;
static const RString DEFAULT_ANIMATION_NAME = "default";

Model::Model()
{
	m_bTextureWrapping = true;
	SetUseZBuffer( true );
	SetCullMode( CULL_BACK );
	m_pGeometry = nullptr;
	m_pCurAnimation = nullptr;
	m_fDefaultAnimationRate = 1;
	m_fCurAnimationRate = 1;
	m_bLoop = true;
	m_bDrawCelShaded = false;
	m_pTempGeometry = nullptr;
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
		m_pGeometry = nullptr;
	}
	m_vpBones.clear();
	m_Materials.clear();
	m_mapNameToAnimation.clear();
	m_pCurAnimation = nullptr;
	RecalcAnimationLengthSeconds();

	if( m_pTempGeometry )
		DISPLAY->DeleteCompiledGeometry( m_pTempGeometry );
}

void Model::Load( const RString &sFile )
{
	if( sFile == "" ) return;

	RString sExt = GetExtension(sFile);
	sExt.MakeLower();
	if( sExt=="txt" )
		LoadMilkshapeAscii( sFile );
	RecalcAnimationLengthSeconds();
}

#define THROW RageException::Throw( "Parse error in \"%s\" at line %d: \"%s\".", sPath.c_str(), iLineNum, sLine.c_str() )

// TODO: Move MS3D loading into its own class. - Colby
void Model::LoadMilkshapeAscii( const RString &sPath )
{
	LoadPieces( sPath, sPath, sPath );
}

void Model::LoadPieces( const RString &sMeshesPath, const RString &sMaterialsPath, const RString &sBonesPath )
{
	Clear();

	// TRICKY: Load materials before geometry so we can figure out whether the materials require normals.
	LoadMaterialsFromMilkshapeAscii( sMaterialsPath );

	ASSERT( m_pGeometry == nullptr );
	m_pGeometry = MODELMAN->LoadMilkshapeAscii( sMeshesPath, this->MaterialsNeedNormals() );

	// Validate material indices.
	for( unsigned i = 0; i < m_pGeometry->m_Meshes.size(); ++i )
	{
		const msMesh *pMesh = &m_pGeometry->m_Meshes[i];

		if( pMesh->nMaterialIndex >= (int) m_Materials.size() )
			RageException::Throw( "Model \"%s\" mesh \"%s\" references material index %i, but there are only %i materials.",
				sMeshesPath.c_str(), pMesh->sName.c_str(), pMesh->nMaterialIndex, (int)m_Materials.size() );
	}

	if( LoadMilkshapeAsciiBones( DEFAULT_ANIMATION_NAME, sBonesPath ) )
		PlayAnimation( DEFAULT_ANIMATION_NAME );

	// Setup temp vertices (if necessary)
	if( m_pGeometry->HasAnyPerVertexBones() )
	{
		m_vTempMeshes = m_pGeometry->m_Meshes;
		m_pTempGeometry = DISPLAY->CreateCompiledGeometry();
		m_pTempGeometry->Set( m_vTempMeshes, this->MaterialsNeedNormals() );
	}
	RecalcAnimationLengthSeconds();
}

void Model::LoadFromNode( const XNode* pNode )
{
	RString s1, s2, s3;
	ActorUtil::GetAttrPath( pNode, "Meshes", s1 );
	ActorUtil::GetAttrPath( pNode, "Materials", s2 );
	ActorUtil::GetAttrPath( pNode, "Bones", s3 );
	if( !s1.empty() || !s2.empty() || !s3.empty() )
	{
		ASSERT( !s1.empty() && !s2.empty() && !s3.empty() );
		LoadPieces( s1, s2, s3 );
	}

	Actor::LoadFromNode( pNode );
	RecalcAnimationLengthSeconds();
}


void Model::LoadMaterialsFromMilkshapeAscii( const RString &_sPath )
{
	RString sPath = _sPath;

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

		// materials
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
				if( sscanf(sLine, "\"%255[^\"]\"", szName) != 1 )
					THROW;
				Material.sName = szName;

				// ambient
				if( f.GetLine( sLine ) <= 0 )
					THROW;
				Rage::Vector4 Ambient;
				if( sscanf(sLine, "%f %f %f %f", &Ambient.x, &Ambient.y, &Ambient.z, &Ambient.w) != 4 )
					THROW;
				memcpy( &Material.Ambient, &Ambient, sizeof(Material.Ambient) );

				// diffuse
				if( f.GetLine( sLine ) <= 0 )
					THROW;
				Rage::Vector4 Diffuse;
				if( sscanf(sLine, "%f %f %f %f", &Diffuse.x, &Diffuse.y, &Diffuse.z, &Diffuse.w) != 4 )
					THROW;
				memcpy( &Material.Diffuse, &Diffuse, sizeof(Material.Diffuse) );

				// specular
				if( f.GetLine( sLine ) <= 0 )
					THROW;
				Rage::Vector4 Specular;
				if( sscanf(sLine, "%f %f %f %f", &Specular.x, &Specular.y, &Specular.z, &Specular.w) != 4 )
					THROW;
				memcpy( &Material.Specular, &Specular, sizeof(Material.Specular) );

				// emissive
				if( f.GetLine( sLine ) <= 0 )
					THROW;
				Rage::Vector4 Emissive;
				if( sscanf (sLine, "%f %f %f %f", &Emissive.x, &Emissive.y, &Emissive.z, &Emissive.w) != 4 )
					THROW;
				memcpy( &Material.Emissive, &Emissive, sizeof(Material.Emissive) );

				// shininess
				if( f.GetLine( sLine ) <= 0 )
					THROW;
				float fShininess;
				if( !StringConversion::FromString(sLine, fShininess) )
					THROW;
				Material.fShininess = fShininess;

				// transparency
				if( f.GetLine( sLine ) <= 0 )
					THROW;
				float fTransparency;
				if( !StringConversion::FromString(sLine, fTransparency) )
					THROW;
				Material.fTransparency = fTransparency;

				// diffuse texture
				if( f.GetLine( sLine ) <= 0 )
					THROW;
				strcpy( szName, "" );
				sscanf( sLine, "\"%255[^\"]\"", szName );
				RString sDiffuseTexture = szName;

				if( sDiffuseTexture == "" )
				{
					Material.diffuse.LoadBlank();
				}
				else
				{
					RString sTexturePath = sDir + sDiffuseTexture;
					FixSlashesInPlace( sTexturePath );
					CollapsePath( sTexturePath );
					if( !IsAFile(sTexturePath) )
						RageException::Throw( "\"%s\" references a texture \"%s\" that does not exist.", sPath.c_str(), sTexturePath.c_str() );

					Material.diffuse.Load( sTexturePath );
				}

				// alpha texture
				if( f.GetLine( sLine ) <= 0 )
					THROW;
				strcpy( szName, "" );
				sscanf( sLine, "\"%255[^\"]\"", szName );
				RString sAlphaTexture = szName;

				if( sAlphaTexture == "" )
				{
					Material.alpha.LoadBlank();
				}
				else
				{
					RString sTexturePath = sDir + sAlphaTexture;
					FixSlashesInPlace( sTexturePath );
					CollapsePath( sTexturePath );
					if( !IsAFile(sTexturePath) )
						RageException::Throw( "\"%s\" references a texture \"%s\" that does not exist.", sPath.c_str(), sTexturePath.c_str() );

					Material.alpha.Load( sTexturePath );
				}
			}
		}
	}
}

bool Model::LoadMilkshapeAsciiBones( const RString &sAniName, const RString &sPath )
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
	return m_pGeometry == nullptr || m_pGeometry->m_Meshes.empty();
}

void Model::DrawCelShaded()
{
	// First pass: shell. We only want the backfaces for this.
	DISPLAY->SetCelShaded(1);
	DISPLAY->SetCullMode(CULL_FRONT);
	this->SetZWrite(false); // XXX: Why on earth isn't the culling working? -Colby
	this->Draw();

	// Second pass: cel shading
	DISPLAY->SetCelShaded(2);
	DISPLAY->SetCullMode(CULL_BACK);
	this->SetZWrite(true);
	this->Draw();

	DISPLAY->SetCelShaded(0);
}

void Model::DrawPrimitives()
{
	Actor::SetGlobalRenderStates();	// set Actor-specified render states

	// Don't if we're fully transparent
	if( m_pTempState->diffuse[0].a < 0.001f && m_pTempState->glow.a < 0.001f )
		return;

	DISPLAY->Scale( 1, -1, 1 );	// flip Y so positive is up

	//////////////////////
	// render the diffuse pass
	//////////////////////
	if( m_pTempState->diffuse[0].a > 0 )
	{
		DISPLAY->SetTextureMode( TextureUnit_1, TextureMode_Modulate );

		for( unsigned i = 0; i < m_pGeometry->m_Meshes.size(); ++i )
		{
			const msMesh *pMesh = &m_pGeometry->m_Meshes[i];

			if( pMesh->nMaterialIndex != -1 )	// has a material
			{
				// apply material
				msMaterial& mat = m_Materials[ pMesh->nMaterialIndex ];

				Rage::Color Emissive = mat.Emissive;
				Rage::Color Ambient = mat.Ambient;
				Rage::Color Diffuse = mat.Diffuse;

				Emissive *= m_pTempState->diffuse[0];
				Ambient *= m_pTempState->diffuse[0];
				Diffuse *= m_pTempState->diffuse[0];

				DISPLAY->SetMaterial( Emissive, Ambient, Diffuse, mat.Specular, mat.fShininess );

				Rage::Vector2 vTexTranslate = mat.diffuse.GetTextureTranslate();
				if( vTexTranslate.x != 0  ||  vTexTranslate.y != 0 )
				{
					DISPLAY->TexturePushMatrix();
					DISPLAY->TextureTranslate( vTexTranslate.x, vTexTranslate.y );
				}

				/* There's some common code that could be folded out here, but
				 * it seems clearer to keep it separate. */
				bool bUseMultitexture = PREFSMAN->m_bAllowMultitexture  &&  DISPLAY->GetNumTextureUnits() >= 2;
				if( bUseMultitexture )
				{
					// render the diffuse texture with texture unit 1
					DISPLAY->SetTexture( TextureUnit_1, mat.diffuse.GetCurrentTexture() ? mat.diffuse.GetCurrentTexture()->GetTexHandle() : 0 );
					Actor::SetTextureRenderStates();	// set Actor-specified render states
					DISPLAY->SetSphereEnvironmentMapping( TextureUnit_1, mat.diffuse.m_bSphereMapped );

					// render the additive texture with texture unit 2
					if( mat.alpha.GetCurrentTexture() )
					{
						DISPLAY->SetTexture( TextureUnit_2, mat.alpha.GetCurrentTexture() ? mat.alpha.GetCurrentTexture()->GetTexHandle() : 0 );
						Actor::SetTextureRenderStates(); // set Actor-specified render states
						DISPLAY->SetSphereEnvironmentMapping( TextureUnit_2, mat.alpha.m_bSphereMapped );
						DISPLAY->SetTextureMode( TextureUnit_2, TextureMode_Add );
						DISPLAY->SetTextureFiltering( TextureUnit_2, true );
					}
					else
					{
						DISPLAY->SetTexture( TextureUnit_2, 0 );

						// set current texture back to 0 or else texture
						// transform applied above  isn't used. Why?!?
						DISPLAY->SetTexture( TextureUnit_1, mat.diffuse.GetCurrentTexture() ? mat.diffuse.GetCurrentTexture()->GetTexHandle() : 0 );
					}

					// go
					DrawMesh(i);

					// Turn off environment mapping on tex unit 0.
					DISPLAY->SetSphereEnvironmentMapping( TextureUnit_1, false );
				}
				else
				{
					// render the diffuse texture
					DISPLAY->SetTexture( TextureUnit_1, mat.diffuse.GetCurrentTexture() ? mat.diffuse.GetCurrentTexture()->GetTexHandle() : 0 );
					Actor::SetTextureRenderStates();	// set Actor-specified render states
					DISPLAY->SetSphereEnvironmentMapping( TextureUnit_1, mat.diffuse.m_bSphereMapped );
					DrawMesh( i );

					// render the additive texture
					if( mat.alpha.GetCurrentTexture() )
					{
						DISPLAY->SetTexture( TextureUnit_1, mat.alpha.GetCurrentTexture() ? mat.alpha.GetCurrentTexture()->GetTexHandle() : 0 );
						Actor::SetTextureRenderStates();	// set Actor-specified render states

						DISPLAY->SetSphereEnvironmentMapping( TextureUnit_1, mat.alpha.m_bSphereMapped );
						// UGLY: This overrides the Actor's BlendMode.
						DISPLAY->SetBlendMode( BLEND_ADD );
						DISPLAY->SetTextureFiltering( TextureUnit_1, true );
						DrawMesh( i );
					}
				}

				if( vTexTranslate.x != 0  ||  vTexTranslate.y != 0 )
					DISPLAY->TexturePopMatrix();
			}
			else
			{
				static const Rage::Color emissive( 0,0,0,0 );
				static const Rage::Color ambient( 0.2f,0.2f,0.2f,1 );
				static const Rage::Color diffuse( 0.7f,0.7f,0.7f,1 );
				static const Rage::Color specular( 0.2f,0.2f,0.2f,1 );
				static const float shininess = 1;
				DISPLAY->SetMaterial( emissive, ambient, diffuse, specular, shininess );
				DISPLAY->ClearAllTextures();
				DISPLAY->SetSphereEnvironmentMapping( TextureUnit_1, false );
				DrawMesh( i );
			}

			DISPLAY->SetSphereEnvironmentMapping( TextureUnit_1, false );
			DISPLAY->SetBlendMode( BLEND_NORMAL );
		}
	}

	// render the glow pass
	if( m_pTempState->glow.a > 0.0001f )
	{
		DISPLAY->SetTextureMode( TextureUnit_1, TextureMode_Glow );

		for( unsigned i = 0; i < m_pGeometry->m_Meshes.size(); ++i )
		{
			const msMesh *pMesh = &m_pGeometry->m_Meshes[i];

			// apply material
			Rage::Color emissive = Rage::Color(0,0,0,0);
			Rage::Color ambient = Rage::Color(0,0,0,0);
			Rage::Color diffuse = m_pTempState->glow;
			Rage::Color specular = Rage::Color(0,0,0,0);
			float shininess = 1;

			DISPLAY->SetMaterial( emissive, ambient, diffuse, specular, shininess );
			DISPLAY->ClearAllTextures();

			if( pMesh->nMaterialIndex != -1 )
			{
				msMaterial& mat = m_Materials[ pMesh->nMaterialIndex ];
				DISPLAY->SetTexture( TextureUnit_1, mat.diffuse.GetCurrentTexture() ? mat.diffuse.GetCurrentTexture()->GetTexHandle() : 0 );
				Actor::SetTextureRenderStates();	// set Actor-specified render states
			}
			else
			{
				// hey why is this otherwise empty else block here? -aj
			}

			DrawMesh( i );
		}
	}
}

void Model::DrawMesh( int i ) const
{
	const msMesh *pMesh = &m_pGeometry->m_Meshes[i];

	// apply mesh-specific bone (if any)
	if( pMesh->m_iBoneIndex != -1 )
	{
		DISPLAY->PushMatrix();

		const Rage::Matrix &mat = m_vpBones[pMesh->m_iBoneIndex].m_Final;
		DISPLAY->PreMultMatrix( mat );
	}

	// Draw it
	const RageCompiledGeometry* TempGeometry = m_pTempGeometry ? m_pTempGeometry : m_pGeometry->m_pCompiledGeometry;
	DISPLAY->DrawCompiledGeometry( TempGeometry, i, m_pGeometry->m_Meshes );

	if( pMesh->m_iBoneIndex != -1 )
		DISPLAY->PopMatrix();
}

void Model::SetDefaultAnimation( RString sAnimation, float fPlayRate )
{
	m_sDefaultAnimation = sAnimation;
	m_fDefaultAnimationRate = fPlayRate;
}

void Model::PlayAnimation( const RString &sAniName, float fPlayRate )
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
	m_vpBones.resize( m_pCurAnimation->Bones.size() );

	for( unsigned i = 0; i < m_pCurAnimation->Bones.size(); i++ )
	{
		const msBone *pBone = &m_pCurAnimation->Bones[i];
		const Rage::Vector3 &vRot = pBone->Rotation;

		RageMatrixAngles( &m_vpBones[i].m_Relative, vRot );

		m_vpBones[i].m_Relative.m[3][0] = pBone->Position.x;
		m_vpBones[i].m_Relative.m[3][1] = pBone->Position.y;
		m_vpBones[i].m_Relative.m[3][2] = pBone->Position.z;

		int nParentBone = m_pCurAnimation->FindBoneByName( pBone->sParentName );
		if( nParentBone != -1 )
		{
			m_vpBones[i].m_Absolute = m_vpBones[i].m_Relative * m_vpBones[i].m_Absolute;
		}
		else
		{
			m_vpBones[i].m_Absolute = m_vpBones[i].m_Relative;
		}
		m_vpBones[i].m_Final = m_vpBones[i].m_Absolute;
	}

	// subtract out the bone's resting position
	for( unsigned i = 0; i < m_pGeometry->m_Meshes.size(); ++i )
	{
		msMesh *pMesh = &m_pGeometry->m_Meshes[i];
		vector<Rage::ModelVertex> &Vertices = pMesh->Vertices;
		for( unsigned j = 0; j < Vertices.size(); j++ )
		{
			// int iBoneIndex = (pMesh->m_iBoneIndex!=-1) ? pMesh->m_iBoneIndex : bone;
			Rage::Vector3 &pos = Vertices[j].p;
			int8_t bone = Vertices[j].bone;
			if( bone != -1 )
			{
				pos.x -= m_vpBones[bone].m_Absolute.m[3][0];
				pos.y -= m_vpBones[bone].m_Absolute.m[3][1];
				pos.z -= m_vpBones[bone].m_Absolute.m[3][2];

				Rage::Vector3 vTmp;

				Rage::Matrix inverse = m_vpBones[bone].m_Absolute.GetTranspose(); // transpose = inverse for rotation matrices
				vTmp = pos.TransformNormal(inverse);

				pos = vTmp;
			}
		}
	}

	// Set up m_vpBones, just in case we're drawn without being Update()d.
	SetBones( m_pCurAnimation, m_fCurFrame, m_vpBones );
	UpdateTempGeometry();
}

void Model::SetPosition( float fSeconds )
{
	m_fCurFrame = FRAMES_PER_SECOND * fSeconds;
	m_fCurFrame = Rage::clamp( m_fCurFrame, 0.f, static_cast<float>(m_pCurAnimation->nTotalFrames) );
}

void Model::AdvanceFrame( float fDeltaTime )
{
	if( m_pGeometry == nullptr ||
		m_pGeometry->m_Meshes.empty() ||
		!m_pCurAnimation )
	{
		return; // bail early
	}

	// LOG->Trace( "m_fCurFrame = %f", m_fCurFrame );

	m_fCurFrame += FRAMES_PER_SECOND * fDeltaTime * m_fCurAnimationRate;
	if( m_fCurFrame < 0 || m_fCurFrame >= m_pCurAnimation->nTotalFrames )
	{
		if( m_sDefaultAnimation != "" )
		{
			this->PlayAnimation( m_sDefaultAnimation, m_fDefaultAnimationRate );
			/* XXX: add to m_fCurFrame the wrapover from the previous
			 * m_fCurFrame-m_pCurAnimation->nTotalFrames, so it doesn't skip */
		}
		else if( m_bLoop )
			wrap( m_fCurFrame, (float) m_pCurAnimation->nTotalFrames );
		else
			m_fCurFrame = Rage::clamp( m_fCurFrame, 0.f, static_cast<float>(m_pCurAnimation->nTotalFrames) );
	}

	SetBones( m_pCurAnimation, m_fCurFrame, m_vpBones );
	UpdateTempGeometry();
}

void Model::SetBones( const msAnimation* pAnimation, float fFrame, vector<myBone_t> &vpBones )
{
	for( size_t i = 0; i < pAnimation->Bones.size(); ++i )
	{
		const msBone *pBone = &pAnimation->Bones[i];
		if( pBone->PositionKeys.size() == 0 && pBone->RotationKeys.size() == 0 )
		{
			vpBones[i].m_Final = vpBones[i].m_Absolute;
			continue;
		}

		// search for the adjacent position keys
		const msPositionKey *pLastPositionKey = nullptr, *pThisPositionKey = nullptr;
		for( size_t j = 0; j < pBone->PositionKeys.size(); ++j )
		{
			const msPositionKey *pPositionKey = &pBone->PositionKeys[j];
			if( pPositionKey->fTime >= fFrame )
			{
				pThisPositionKey = pPositionKey;
				break;
			}
			pLastPositionKey = pPositionKey;
		}

		Rage::Vector3 vPos;
		if( pLastPositionKey != nullptr && pThisPositionKey != nullptr )
		{
			const float s = Rage::scale( fFrame, pLastPositionKey->fTime, pThisPositionKey->fTime, 0.f, 1.f );
			vPos = pLastPositionKey->Position + (pThisPositionKey->Position - pLastPositionKey->Position) * s;
		}
		else if( pLastPositionKey == nullptr )
			vPos = pThisPositionKey->Position;
		else if( pThisPositionKey == nullptr )
			vPos = pLastPositionKey->Position;

		// search for the adjacent rotation keys
		const msRotationKey *pLastRotationKey = nullptr, *pThisRotationKey = nullptr;
		for( size_t j = 0; j < pBone->RotationKeys.size(); ++j )
		{
			const msRotationKey *pRotationKey = &pBone->RotationKeys[j];
			if( pRotationKey->fTime >= fFrame )
			{
				pThisRotationKey = pRotationKey;
				break;
			}
			pLastRotationKey = pRotationKey;
		}

		Rage::Vector4 vRot;
		if( pLastRotationKey != nullptr && pThisRotationKey != nullptr )
		{
			const float s = Rage::scale( fFrame, pLastRotationKey->fTime, pThisRotationKey->fTime, 0.f, 1.f );
			RageQuatSlerp( &vRot, pLastRotationKey->Rotation, pThisRotationKey->Rotation, s );
		}
		else if( pLastRotationKey == nullptr )
		{
			vRot = pThisRotationKey->Rotation;
		}
		else if( pThisRotationKey == nullptr )
		{
			vRot = pLastRotationKey->Rotation;
		}

		auto m = Rage::Matrix::GetIdentity();
		RageMatrixFromQuat( &m, vRot );
		m.m[3][0] = vPos.x;
		m.m[3][1] = vPos.y;
		m.m[3][2] = vPos.z;

		Rage::Matrix RelativeFinal = m * vpBones[i].m_Relative;

		int iParentBone = pAnimation->FindBoneByName( pBone->sParentName );
		if( iParentBone == -1 )
			vpBones[i].m_Final = RelativeFinal;
		else
			vpBones[i].m_Final = RelativeFinal * vpBones[iParentBone].m_Final;
	}
}

void Model::UpdateTempGeometry()
{
	if( m_pGeometry == nullptr || m_pTempGeometry == nullptr )
		return;

	for( unsigned i = 0; i < m_pGeometry->m_Meshes.size(); ++i )
	{
		const msMesh &origMesh = m_pGeometry->m_Meshes[i];
		msMesh &tempMesh = m_vTempMeshes[i];
		const vector<Rage::ModelVertex> &origVertices = origMesh.Vertices;
		vector<Rage::ModelVertex> &tempVertices = tempMesh.Vertices;
		for( unsigned j = 0; j < origVertices.size(); j++ )
		{
			Rage::Vector3 &tempPos =			tempVertices[j].p;
			Rage::Vector3 &tempNormal =		tempVertices[j].n;
			const Rage::Vector3 &originalPos =	origVertices[j].p;
			const Rage::Vector3 &originalNormal =	origVertices[j].n;
			int8_t bone =				origVertices[j].bone;

			if( bone == -1 )
			{
				tempNormal = originalNormal;
				tempPos = originalPos;
			}
			else
			{
				tempNormal = originalNormal.TransformNormal(m_vpBones[bone].m_Final);
				tempPos = originalPos.TransformCoords(m_vpBones[bone].m_Final);
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
	auto findMax = [](int highest, msMaterial const &m) {
		return std::max(highest, m.diffuse.GetNumStates());
	};
	return std::accumulate(m_Materials.begin(), m_Materials.end(), 0, findMax);
}

void Model::SetState( size_t iNewState )
{
	for (auto &m: m_Materials)
	{
		m.diffuse.SetState( iNewState );
		m.alpha.SetState( iNewState );
	}
}

void Model::RecalcAnimationLengthSeconds()
{
	m_animation_length_seconds= 0;
	for (auto &m: m_Materials)
	{
		m_animation_length_seconds= std::max(m_animation_length_seconds,
			m.diffuse.GetAnimationLengthSeconds());
	}
}

void Model::SetSecondsIntoAnimation( float fSeconds )
{
	for (auto &m: m_Materials)
	{
		m.diffuse.SetSecondsIntoAnimation( fSeconds );
		m.alpha.SetSecondsIntoAnimation( fSeconds );
	}
}

bool Model::MaterialsNeedNormals() const
{
	auto needsNormals = [](msMaterial const &m) {
		return m.NeedsNormals();
	};
	return std::any_of(m_Materials.begin(), m_Materials.end(), needsNormals);
}

// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the Model. */
class LunaModel: public Luna<Model>
{
public:
	static int position( T* p, lua_State *L )	{ p->SetPosition( FArg(1) ); COMMON_RETURN_SELF; }
	static int playanimation( T* p, lua_State *L )	{ p->PlayAnimation(SArg(1),FArg(2)); COMMON_RETURN_SELF; }
	static int SetDefaultAnimation( T* p, lua_State *L )	{ p->SetDefaultAnimation(SArg(1),FArg(2)); COMMON_RETURN_SELF; }
	static int GetDefaultAnimation( T* p, lua_State *L )	{ lua_pushstring( L, p->GetDefaultAnimation() ); return 1; }
	static int loop( T* p, lua_State *L )		{ p->SetLoop(BArg(1)); COMMON_RETURN_SELF; }
	static int rate( T* p, lua_State *L )		{ p->SetRate(FArg(1)); COMMON_RETURN_SELF; }
	static int GetNumStates( T* p, lua_State *L )		{ lua_pushnumber( L, p->GetNumStates() ); return 1; }
	//static int CelShading( T* p, lua_State *L )		{ p->SetCelShading(BArg(1)); COMMON_RETURN_SELF; }

	LunaModel()
	{
		ADD_METHOD( position );
		ADD_METHOD( playanimation );
		ADD_METHOD( SetDefaultAnimation );
		ADD_METHOD( GetDefaultAnimation );
		ADD_METHOD( loop );
		ADD_METHOD( rate );
		// sm-ssc adds:
		ADD_METHOD( GetNumStates );
		//ADD_METHOD( CelShading );
		// LoadMilkshapeAsciiBones?
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
