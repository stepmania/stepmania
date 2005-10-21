#include "global.h"
#include "ModelTypes.h"
#include "IniFile.h"
#include "RageUtil.h"
#include "RageFile.h"
#include "RageMath.h"
#include "RageTexture.h"
#include "RageTextureManager.h"
#include "RageLog.h"
#include "RageDisplay.h"
#include "Foreach.h"

#define MS_MAX_NAME	32

AnimatedTexture::AnimatedTexture()
{
	m_iCurState = 0;
	m_fSecsIntoFrame = 0;
	m_bSphereMapped = false;
	m_vTexOffset = RageVector2(0,0);
	m_vTexVelocity = RageVector2(0,0);
	m_BlendMode = BLEND_NORMAL;
}

AnimatedTexture::~AnimatedTexture()
{
	Unload();
}

void AnimatedTexture::Load( CString sTexOrIniPath )
{
	ASSERT( vFrames.empty() );	// don't load more than once

	m_bSphereMapped = sTexOrIniPath.Find("sphere") != -1;
	if( sTexOrIniPath.Find("add") != -1 )
		m_BlendMode = BLEND_ADD;
	else
		m_BlendMode = BLEND_NORMAL;

	if( GetExtension(sTexOrIniPath).CompareNoCase("ini")==0 )
	{
		IniFile ini;
		if( !ini.ReadFile( sTexOrIniPath ) )
			RageException::Throw( "Error reading %s: %s", sTexOrIniPath.c_str(), ini.GetError().c_str() );

		const XNode* pAnimatedTexture = ini.GetChild("AnimatedTexture");
		if( pAnimatedTexture == NULL )
			RageException::Throw( "The animated texture file '%s' doesn't contain a section called 'AnimatedTexture'.", sTexOrIniPath.c_str() );
		
		pAnimatedTexture->GetAttrValue( "TexVelocityX", m_vTexVelocity.x );
		pAnimatedTexture->GetAttrValue( "TexVelocityY", m_vTexVelocity.y );
		pAnimatedTexture->GetAttrValue( "TexOffsetX", m_vTexOffset.x );
		pAnimatedTexture->GetAttrValue( "TexOffsetY", m_vTexOffset.y );
		
		for( int i=0; i<1000; i++ )
		{
			CString sFileKey = ssprintf( "Frame%04d", i );
			CString sDelayKey = ssprintf( "Delay%04d", i );

			CString sFileName;
			float fDelay = 0;
			if( pAnimatedTexture->GetAttrValue( sFileKey, sFileName ) &&
				pAnimatedTexture->GetAttrValue( sDelayKey, fDelay ) ) 
			{
				CString sTranslateXKey = ssprintf( "TranslateX%04d", i );
				CString sTranslateYKey = ssprintf( "TranslateY%04d", i );

				RageVector2 vOffset(0,0);
				pAnimatedTexture->GetAttrValue( sTranslateXKey, vOffset.x );
				pAnimatedTexture->GetAttrValue( sTranslateYKey, vOffset.y );

				RageTextureID ID;
				ID.filename = Dirname(sTexOrIniPath) + sFileName;
				ID.bStretch = true;
				ID.bHotPinkColorKey = true;
				ID.bMipMaps = true;	// use mipmaps in Models
				AnimatedTextureState state( 
					TEXTUREMAN->LoadTexture( ID ),
					fDelay,
					vOffset
					);
				vFrames.push_back( state );
			}
			else
			{
				break;
			}
		}
	}
	else
	{
		RageTextureID ID;
		ID.filename = sTexOrIniPath;
		ID.bHotPinkColorKey = true;
		ID.bStretch = true;
		ID.bMipMaps = true;	// use mipmaps in Models
		AnimatedTextureState state(
			TEXTUREMAN->LoadTexture( ID ),
			1,
			RageVector2(0,0)
			);
		vFrames.push_back( state );
	}
}


void AnimatedTexture::Update( float fDelta )
{
	if( vFrames.empty() )
		return;
	ASSERT( m_iCurState < (int)vFrames.size() );
	m_fSecsIntoFrame += fDelta;
	if( m_fSecsIntoFrame > vFrames[m_iCurState].fDelaySecs )
	{
		m_fSecsIntoFrame -= vFrames[m_iCurState].fDelaySecs;
		m_iCurState = (m_iCurState+1) % vFrames.size();
	}
}

RageTexture* AnimatedTexture::GetCurrentTexture()
{
	if( vFrames.empty() )
		return NULL;
	ASSERT( m_iCurState < (int)vFrames.size() );
	return vFrames[m_iCurState].pTexture;
}

int AnimatedTexture::GetNumStates() const
{
	return vFrames.size();
}

void AnimatedTexture::SetState( int iState )
{
	CLAMP( iState, 0, GetNumStates()-1 );
	m_iCurState = iState;
}

float AnimatedTexture::GetAnimationLengthSeconds() const
{
	float fTotalSeconds = 0;
	FOREACH_CONST( AnimatedTextureState, vFrames, ats )
		fTotalSeconds += ats->fDelaySecs;
	return fTotalSeconds;
}

void AnimatedTexture::SetSecondsIntoAnimation( float fSeconds )
{
	fSeconds = fmodf( fSeconds, GetAnimationLengthSeconds() );

	m_iCurState = 0;
	for( unsigned i=0; i<vFrames.size(); i++ )
	{
		AnimatedTextureState& ats = vFrames[i];
		if( fSeconds >= ats.fDelaySecs )
		{
			fSeconds -= ats.fDelaySecs;
			m_iCurState = i+1;
		}
		else
		{
			break;
		}
	}
	m_fSecsIntoFrame = fSeconds;	// remainder
}

float AnimatedTexture::GetSecondsIntoAnimation() const
{
	float fSeconds = 0;

	for( unsigned i=0; i<vFrames.size(); i++ )
	{
		const AnimatedTextureState& ats = vFrames[i];
		if( int(i) >= m_iCurState )
			break;

		fSeconds += ats.fDelaySecs;
	}
	fSeconds += m_fSecsIntoFrame;
	return fSeconds;
}

void AnimatedTexture::Unload()
{
	for(unsigned i = 0; i < vFrames.size(); ++i)
		TEXTUREMAN->UnloadTexture(vFrames[i].pTexture);
	vFrames.clear();
	m_iCurState = 0;
	m_fSecsIntoFrame = 0;
}

RageVector2 AnimatedTexture::GetTextureTranslate()
{
	float fPercentIntoAnimation = GetSecondsIntoAnimation() / GetAnimationLengthSeconds();
	RageVector2 v = m_vTexVelocity * fPercentIntoAnimation + m_vTexOffset;

	if( vFrames.empty() )
		return v;

	ASSERT( m_iCurState < (int)vFrames.size() );
	v += vFrames[m_iCurState].vTranslate;

	return v;
}

msMesh::msMesh()
{
}

msMesh::~msMesh()
{
}

#define THROW RageException::Throw( "Parse error in \"%s\" at line %d: '%s'", sPath.c_str(), iLineNum, sLine.c_str() )

bool msAnimation::LoadMilkshapeAsciiBones( CString sAniName, CString sPath )
{
	FixSlashesInPlace(sPath);
	const CString sDir = Dirname( sPath );

	RageFile f;
	if ( !f.Open(sPath) )
		RageException::Throw( "Model:: Could not open \"%s\": %s", sPath.c_str(), f.GetError().c_str() );

	CString sLine;
	int iLineNum = 0;

	msAnimation &Animation = *this;

	bool bLoaded = false;
    while( f.GetLine( sLine ) > 0 )
    {
		iLineNum++;

        if (!strncmp (sLine, "//", 2))
            continue;

        //
        // bones
        //
        int nNumBones = 0;
        if( sscanf (sLine, "Bones: %d", &nNumBones) != 1 )
			continue;

        char szName[MS_MAX_NAME];

        Animation.Bones.resize( nNumBones );

        for( int i = 0; i < nNumBones; i++ )
        {
			msBone& Bone = Animation.Bones[i];

            // name
			if( f.GetLine( sLine ) <= 0 )
				THROW;
            if (sscanf (sLine, "\"%[^\"]\"", szName) != 1)
				THROW;
            Bone.sName = szName;

            // parent
			if( f.GetLine( sLine ) <= 0 )
				THROW;
            strcpy (szName, "");
            sscanf (sLine, "\"%[^\"]\"", szName);

            Bone.sParentName = szName;

            // flags, position, rotation
            RageVector3 Position, Rotation;
			if( f.GetLine( sLine ) <= 0 )
				THROW;

			int nFlags;
            if (sscanf (sLine, "%d %f %f %f %f %f %f",
                &nFlags,
                &Position[0], &Position[1], &Position[2],
                &Rotation[0], &Rotation[1], &Rotation[2]) != 7)
            {
				THROW;
            }
			Rotation = RadianToDegree(Rotation);

			Bone.nFlags = nFlags;
            memcpy( &Bone.Position, &Position, sizeof(Bone.Position) );
            memcpy( &Bone.Rotation, &Rotation, sizeof(Bone.Rotation) );

            // position key count
			if( f.GetLine( sLine ) <= 0 )
				THROW;
            int nNumPositionKeys = 0;
            if (sscanf (sLine, "%d", &nNumPositionKeys) != 1)
				THROW;

            Bone.PositionKeys.resize( nNumPositionKeys );

            for( int j = 0; j < nNumPositionKeys; ++j )
            {
				if( f.GetLine( sLine ) <= 0 )
					THROW;

				float fTime;
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

            for( int j = 0; j < nNumRotationKeys; ++j )
            {
				if( f.GetLine( sLine ) <= 0 )
					THROW;

				float fTime;
                if (sscanf (sLine, "%f %f %f %f", &fTime, &Rotation[0], &Rotation[1], &Rotation[2]) != 4)
					THROW;
				Rotation = RadianToDegree(Rotation);

				msRotationKey key;
				key.fTime = fTime;
				key.Rotation = RageVector3( Rotation[0], Rotation[1], Rotation[2] );
                Bone.RotationKeys[j] = key;
            }
        }

		// Ignore "Frames:" in file.  Calculate it ourself
		Animation.nTotalFrames = 0;
		for( int i = 0; i < (int)Animation.Bones.size(); i++ )
		{
			msBone& Bone = Animation.Bones[i];
			for( unsigned j = 0; j < Bone.PositionKeys.size(); ++j )
				Animation.nTotalFrames = max( Animation.nTotalFrames, (int)Bone.PositionKeys[j].fTime );
			for( unsigned j = 0; j < Bone.RotationKeys.size(); ++j )
				Animation.nTotalFrames = max( Animation.nTotalFrames, (int)Bone.RotationKeys[j].fTime );
		}
	}

	return bLoaded;
}

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
