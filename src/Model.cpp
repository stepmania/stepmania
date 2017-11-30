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
static const std::string DEFAULT_ANIMATION_NAME = "default";

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
	m_loaded_safely= false;
	m_bInverseBindPose = true;
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

void Model::Load( const std::string &sFile )
{
	if( sFile == "" ) return;

	std::string sExt = Rage::make_lower(GetExtension(sFile));
	if( sExt=="txt" )
	{
		std::string load_fail_reason;
		if(!LoadMilkshapeAscii(sFile, load_fail_reason))
		{
			LuaHelpers::ReportScriptErrorFmt("Could not load model file %s: %s",
				sFile.c_str(), load_fail_reason.c_str());
			m_loaded_safely= false;
			return;
		}
	}
	RecalcAnimationLengthSeconds();
}

// TODO: Move MS3D loading into its own class. - Colby
bool Model::LoadMilkshapeAscii(const std::string &file, std::string& load_fail_reason)
{
	return LoadPieces(file, file, file, load_fail_reason);
}

bool Model::LoadPieces(const std::string &sMeshesPath,
	const std::string &sMaterialsPath, const std::string &sBonesPath,
	std::string& load_fail_reason)
{
	Clear();

	// TRICKY: Load materials before geometry so we can figure out whether the materials require normals.
	if(!LoadMaterialsFromMilkshapeAscii(sMaterialsPath, load_fail_reason))
	{
		return false;
	}

	ASSERT( m_pGeometry == nullptr );
	m_pGeometry = MODELMAN->LoadMilkshapeAscii( sMeshesPath, this->MaterialsNeedNormals() );

	// Validate material indices.
	for (auto const mesh: m_pGeometry->m_Meshes)
	{
		if( mesh.nMaterialIndex >= (int) m_Materials.size() )
		{
			load_fail_reason= fmt::sprintf("Model \"%s\" mesh \"%s\" references"
				"material index %i, but there are only %i materials.",
				sMeshesPath.c_str(), mesh.sName.c_str(), mesh.nMaterialIndex,
				static_cast<int>(m_Materials.size()));
			m_loaded_safely= false;
			return false;
		}
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
	return true;
}

void Model::LoadFromNode( const XNode* pNode )
{
	std::string mesh_path, material_path, bone_path;
	ActorUtil::GetAttrPath(pNode, "Meshes", mesh_path);
	ActorUtil::GetAttrPath(pNode, "Materials", material_path);
	ActorUtil::GetAttrPath(pNode, "Bones", bone_path);
	if(mesh_path.empty() || material_path.empty() || bone_path.empty())
	{
		LuaHelpers::ReportScriptErrorFmt("%s: Def.Model must have Meshes, "
			"Materials, and Bones paths.", ActorUtil::GetWhere(pNode).c_str());
		m_loaded_safely= false;
		return;
	}
	std::string load_fail_reason;
	if(!LoadPieces(mesh_path, material_path, bone_path, load_fail_reason))
	{
		LuaHelpers::ReportScriptErrorFmt("%s: Def.Model loading failed: %s",
			ActorUtil::GetWhere(pNode).c_str(), load_fail_reason.c_str());
		m_loaded_safely= false;
		return;
	}
	m_loaded_safely= true;

	Actor::LoadFromNode( pNode );
	RecalcAnimationLengthSeconds();
}


bool Model::LoadMaterialsFromMilkshapeAscii(const std::string &_sPath,
	std::string& load_fail_reason)
{
#define LOAD_FAIL load_fail_reason= fmt::sprintf("Parse error in \"%s\" at line %d: \"%s\".", sPath.c_str(), iLineNum, sLine.c_str()); return false;
	std::string sPath = _sPath;

	FixSlashesInPlace(sPath);
	const std::string sDir = Rage::dir_name( sPath );

	RageFile f;
	if( !f.Open( sPath ) )
	{
		load_fail_reason= fmt::sprintf("Model::LoadMilkshapeAscii Could not open \"%s\": %s", sPath.c_str(), f.GetError().c_str());
		return false;
	}

	std::string sLine;
	int iLineNum = 0;

	while( f.GetLine( sLine ) > 0 )
	{
		iLineNum++;

		if( !strncmp (sLine.c_str(), "//", 2) )
		{
			continue;
		}
		int nFrame;
		if( sscanf(sLine.c_str(), "Frames: %d", &nFrame) == 1 )
		{
			// ignore
			// m_pModel->nTotalFrames = nFrame;
		}
		if( sscanf(sLine.c_str(), "Frame: %d", &nFrame) == 1 )
		{
			// ignore
			// m_pModel->nFrame = nFrame;
		}

		// materials
		int nNumMaterials = 0;
		if( sscanf(sLine.c_str(), "Materials: %d", &nNumMaterials) == 1 )
		{
			m_Materials.resize( nNumMaterials );

			char szName[256];

			for( int i = 0; i < nNumMaterials; i++ )
			{
				msMaterial& Material = m_Materials[i];

				// name
				if( f.GetLine( sLine ) <= 0 )
				{
					LOAD_FAIL;
				}
				if( sscanf(sLine.c_str(), "\"%255[^\"]\"", szName) != 1 )
				{
					LOAD_FAIL;
				}
				Material.sName = szName;

				// ambient
				if( f.GetLine( sLine ) <= 0 )
				{
					LOAD_FAIL;
				}
				Rage::Vector4 Ambient;
				if( sscanf(sLine.c_str(), "%f %f %f %f", &Ambient.x, &Ambient.y, &Ambient.z, &Ambient.w) != 4 )
				{
					LOAD_FAIL;
				}
				memcpy( &Material.Ambient, &Ambient, sizeof(Material.Ambient) );

				// diffuse
				if( f.GetLine( sLine ) <= 0 )
				{
					LOAD_FAIL;
				}
				Rage::Vector4 Diffuse;
				if( sscanf(sLine.c_str(), "%f %f %f %f", &Diffuse.x, &Diffuse.y, &Diffuse.z, &Diffuse.w) != 4 )
				{
					LOAD_FAIL;
				}
				memcpy( &Material.Diffuse, &Diffuse, sizeof(Material.Diffuse) );

				// specular
				if( f.GetLine( sLine ) <= 0 )
				{
					LOAD_FAIL;
				}
				Rage::Vector4 Specular;
				if( sscanf(sLine.c_str(), "%f %f %f %f", &Specular.x, &Specular.y, &Specular.z, &Specular.w) != 4 )
				{
					LOAD_FAIL;
				}
				memcpy( &Material.Specular, &Specular, sizeof(Material.Specular) );

				// emissive
				if( f.GetLine( sLine ) <= 0 )
				{
					LOAD_FAIL;
				}
				Rage::Vector4 Emissive;
				if( sscanf (sLine.c_str(), "%f %f %f %f", &Emissive.x, &Emissive.y, &Emissive.z, &Emissive.w) != 4 )
				{
					LOAD_FAIL;
				}
				memcpy( &Material.Emissive, &Emissive, sizeof(Material.Emissive) );

				// shininess
				if( f.GetLine( sLine ) <= 0 )
				{
					LOAD_FAIL;
				}
				float fShininess;
				if( !StringConversion::FromString(sLine, fShininess) )
				{
					LOAD_FAIL;
				}
				Material.fShininess = fShininess;

				// transparency
				if( f.GetLine( sLine ) <= 0 )
				{
					LOAD_FAIL;
				}
				float fTransparency;
				if( !StringConversion::FromString(sLine, fTransparency) )
				{
					LOAD_FAIL;
				}
				Material.fTransparency = fTransparency;

				// diffuse texture
				if( f.GetLine( sLine ) <= 0 )
				{
					LOAD_FAIL;
				}
				strcpy( szName, "" );
				sscanf( sLine.c_str(), "\"%255[^\"]\"", szName );
				std::string sDiffuseTexture = szName;

				if( sDiffuseTexture == "" )
				{
					Material.diffuse.LoadBlank();
				}
				else
				{
					std::string sTexturePath = sDir + sDiffuseTexture;
					FixSlashesInPlace( sTexturePath );
					CollapsePath( sTexturePath );
					if( !IsAFile(sTexturePath) )
					{
						load_fail_reason= fmt::sprintf("\"%s\" references a texture \"%s\" that does not exist.", sPath.c_str(), sTexturePath.c_str());
						return false;
					}

					Material.diffuse.Load( sTexturePath );
				}

				// alpha texture
				if( f.GetLine( sLine ) <= 0 )
				{
					LOAD_FAIL;
				}
				strcpy( szName, "" );
				sscanf( sLine.c_str(), "\"%255[^\"]\"", szName );
				std::string sAlphaTexture = szName;

				if( sAlphaTexture == "" )
				{
					Material.alpha.LoadBlank();
				}
				else
				{
					std::string sTexturePath = sDir + sAlphaTexture;
					FixSlashesInPlace( sTexturePath );
					CollapsePath( sTexturePath );
					if( !IsAFile(sTexturePath) )
					{
						load_fail_reason= fmt::sprintf("\"%s\" references a texture \"%s\" that does not exist.", sPath.c_str(), sTexturePath.c_str());
						return false;
					}

					Material.alpha.Load( sTexturePath );
				}
			}
		}
	}
#undef LOAD_FAIL
	return true;
}

bool Model::LoadMilkshapeAsciiBones( const std::string &sAniName, const std::string &sPath )
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
	return m_pGeometry == nullptr || m_pGeometry->m_Meshes.empty() || !m_loaded_safely;
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

		if (m_bInverseBindPose)
		{
			Rage::Matrix inverse = m_vpBones[pMesh->m_iBoneIndex].m_Absolute.GetTranspose();
			Rage::Vector3 vTmp = Rage::Vector3( inverse.m[3][0], inverse.m[3][1], inverse.m[3][2] );
			vTmp = vTmp.TransformNormal( inverse );
			inverse.m[3][0] = -vTmp.x;
			inverse.m[3][1] = -vTmp.y;
			inverse.m[3][2] = -vTmp.z;
			DISPLAY->PreMultMatrix( inverse );
		}
	}

	// Draw it
	const RageCompiledGeometry* TempGeometry = m_pTempGeometry ? m_pTempGeometry : m_pGeometry->m_pCompiledGeometry;
	DISPLAY->DrawCompiledGeometry( TempGeometry, i, m_pGeometry->m_Meshes );

	if( pMesh->m_iBoneIndex != -1 )
		DISPLAY->PopMatrix();
}

void Model::SetDefaultAnimation( std::string sAnimation, float fPlayRate )
{
	m_sDefaultAnimation = sAnimation;
	m_fDefaultAnimationRate = fPlayRate;
}

void Model::PlayAnimation( const std::string &sAniName, float fPlayRate )
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
			RageMatrixMultiply( &m_vpBones[i].m_Absolute, &m_vpBones[nParentBone].m_Absolute, &m_vpBones[i].m_Relative );
		}
		else
		{
			m_vpBones[i].m_Absolute = m_vpBones[i].m_Relative;
		}
		m_vpBones[i].m_Final = m_vpBones[i].m_Absolute;
	}

	// subtract out the bone's resting position
	for (auto &mesh: m_pGeometry->m_Meshes)
	{
		vector<Rage::ModelVertex> &Vertices = mesh.Vertices;
		for (auto &vertex: Vertices)
		{
			// int iBoneIndex = (pMesh->m_iBoneIndex!=-1) ? pMesh->m_iBoneIndex : bone;
			Rage::Vector3 &pos = vertex.p;
			int8_t bone = vertex.bone;
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
		for (auto const &positionKey: pBone->PositionKeys)
		{
			if( positionKey.fTime >= fFrame )
			{
				pThisPositionKey = &positionKey;
				break;
			}
			pLastPositionKey = &positionKey;
		}

		Rage::Vector3 vPos;
		if( pLastPositionKey != nullptr && pThisPositionKey != nullptr )
		{
			const float s = Rage::scale( fFrame, pLastPositionKey->fTime, pThisPositionKey->fTime, 0.f, 1.f );
			vPos = pLastPositionKey->Position + (pThisPositionKey->Position - pLastPositionKey->Position) * s;
		}
		else if( pLastPositionKey == nullptr )
		{
			vPos = pThisPositionKey->Position;
		}
		else if( pThisPositionKey == nullptr )
		{
			vPos = pLastPositionKey->Position;
		}
		// search for the adjacent rotation keys
		const msRotationKey *pLastRotationKey = nullptr, *pThisRotationKey = nullptr;
		for (auto const &rotationKey: pBone->RotationKeys)
		{
			if( rotationKey.fTime >= fFrame )
			{
				pThisRotationKey = &rotationKey;
				break;
			}
			pLastRotationKey = &rotationKey;
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

		Rage::Matrix RelativeFinal;
		RageMatrixMultiply( &RelativeFinal, &vpBones[i].m_Relative, &m );

		int iParentBone = pAnimation->FindBoneByName( pBone->sParentName );
		if( iParentBone == -1 )
			vpBones[i].m_Final = RelativeFinal;
		else
			RageMatrixMultiply( &vpBones[i].m_Final, &vpBones[iParentBone].m_Final, &RelativeFinal );
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

void Model::UpdateInternal(float delta)
{
	Actor::UpdateInternal(delta);
	float effect_delta= GetEffectDeltaTime();
	AdvanceFrame(effect_delta);
	for(auto&& mat : m_Materials)
	{
		mat.diffuse.Update(effect_delta);
		mat.alpha.Update(effect_delta);
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
	static int position( T* p, lua_State *L )
	{
		p->SetPosition( FArg(1) );
		COMMON_RETURN_SELF;
	}
	static int playanimation( T* p, lua_State *L )
	{
		p->PlayAnimation(SArg(1),FArg(2));
		COMMON_RETURN_SELF;
	}
	static int SetDefaultAnimation( T* p, lua_State *L )
	{
		p->SetDefaultAnimation(SArg(1),FArg(2));
		COMMON_RETURN_SELF;
	}
	static int GetDefaultAnimation( T* p, lua_State *L )
	{
		lua_pushstring( L, p->GetDefaultAnimation().c_str() );
		return 1;
	}
	static int loop( T* p, lua_State *L )
	{
		p->SetLoop(BArg(1));
		COMMON_RETURN_SELF;
	}
	static int rate( T* p, lua_State *L )
	{
		p->SetRate(FArg(1));
		COMMON_RETURN_SELF;
	}
	static int GetNumStates( T* p, lua_State *L )
	{
		lua_pushnumber( L, p->GetNumStates() );
		return 1;
	}
	static int InverseBindPose( T* p, lua_State *L )
	{
		p->SetInverseBindPose(BArg(1));
		COMMON_RETURN_SELF;
	}
	/*
	static int CelShading( T* p, lua_State *L )
	{
		p->SetCelShading(BArg(1));
		COMMON_RETURN_SELF;
	}
	*/
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
		ADD_METHOD( InverseBindPose );
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
