#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ModelTypes

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ModelTypes.h"
#include "IniFile.h"
#include "RageUtil.h"
#include "RageTexture.h"
#include "RageTextureManager.h"
#include "RageLog.h"

AnimatedTexture::AnimatedTexture()
{
	iCurState = 0;
	fSecsIntoFrame = 0;
}

AnimatedTexture::~AnimatedTexture()
{
	Unload();
}

void AnimatedTexture::Load( CString sTexOrIniPath )
{
LOG->Trace("AnimatedTexture::Load(%s)", sTexOrIniPath.c_str());
	ASSERT( vFrames.empty() );	// don't load more than once

	CString sDir, sFName, sExt;
	splitrelpath( sTexOrIniPath, sDir, sFName, sExt );

	bool bIsIni = sTexOrIniPath.Right(3).CompareNoCase("ini")== 0;
LOG->Trace("sTexOrIniPath: is ini: %i", bIsIni);
	if( bIsIni )
	{
		IniFile ini;
		ini.SetPath( sTexOrIniPath );
		if( !ini.ReadFile() )
			RageException::Throw( "Error reading %s: %s", sTexOrIniPath.c_str(), ini.error.c_str() );

		if( !ini.GetKey("AnimatedTexture") )
			RageException::Throw( "The animated texture file '%s' doesn't contain a section called 'AnimatedTexture'.", sTexOrIniPath.c_str() );
		for( int i=0; i<1000; i++ )
		{
			CString sFileKey = ssprintf( "Frame%04d", i );
			CString sDelayKey = ssprintf( "Delay%04d", i );

			CString sFileName;
			float fDelay = 0;
			if( ini.GetValue( "AnimatedTexture", sFileKey, sFileName ) &&
				ini.GetValueF( "AnimatedTexture", sDelayKey, fDelay ) ) 
			{
				RageTextureID ID;
				ID.filename = sDir+sFileName;
				ID.bStretch = true;
				ID.bHotPinkColorKey = true;
				AnimatedTextureState state = { 
					TEXTUREMAN->LoadTexture( ID ),
					fDelay
				};
				vFrames.push_back( state );
			}
			else
				break;
		}
	}
	else
	{
		RageTextureID ID;
		ID.filename = sTexOrIniPath;
		ID.bHotPinkColorKey = true;
		ID.bStretch = true;
		AnimatedTextureState state = { 
			TEXTUREMAN->LoadTexture( ID ),
			10
		};
		vFrames.push_back( state );
	}
}


void AnimatedTexture::Update( float fDelta )
{
	if( vFrames.empty() )
		return;
	ASSERT( iCurState < (int)vFrames.size() );
	fSecsIntoFrame += fDelta;
	if( fSecsIntoFrame > vFrames[iCurState].fDelaySecs )
	{
		fSecsIntoFrame -= vFrames[iCurState].fDelaySecs;
		iCurState = (iCurState+1) % vFrames.size();
	}
}

RageTexture* AnimatedTexture::GetCurrentTexture()
{
	if( vFrames.empty() )
		return NULL;
	ASSERT( iCurState < (int)vFrames.size() );
	return vFrames[iCurState].pTexture;
}

void AnimatedTexture::SetState( int iState )
{
	CLAMP( iState, 0, GetNumStates()-1 );
	iCurState = iState;
}

int AnimatedTexture::GetNumStates()
{
	return vFrames.size();
}

void AnimatedTexture::Unload()
{
	for(unsigned i = 0; i < vFrames.size(); ++i)
		TEXTUREMAN->UnloadTexture(vFrames[i].pTexture);
	vFrames.clear();
	iCurState = 0;
	fSecsIntoFrame = 0;
}
