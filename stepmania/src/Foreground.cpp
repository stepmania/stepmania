#include "global.h"
#include "Foreground.h"
#include "BGAnimation.h"
#include "RageUtil.h"
#include "IniFile.h"
#include "GameState.h"
#include "RageTextureManager.h"

Foreground::~Foreground()
{
	Unload();
}

void Foreground::Unload()
{
	for( unsigned i=0; i < m_BGAnimations.size(); ++i )
		delete m_BGAnimations[i].m_bga;
	m_BGAnimations.clear();
	m_SubActors.clear();
	m_fLastMusicSeconds = -9999;
	m_pSong = NULL;
}

void Foreground::LoadFromSong( const Song *pSong )
{
        /* Song graphics can get very big; never keep them in memory. */
	RageTextureID::TexPolicy OldPolicy = TEXTUREMAN->GetDefaultTexturePolicy();
	TEXTUREMAN->SetDefaultTexturePolicy( RageTextureID::TEX_VOLATILE );

	m_pSong = pSong;
	for( unsigned i=0; i<pSong->m_ForegroundChanges.size(); i++ )
	{
		const BackgroundChange &change = pSong->m_ForegroundChanges[i];
		CString sBGName = change.m_sBGName;
		
		const CString sAniDir = pSong->GetSongDir()+sBGName;
		if( !IsADirectory(sAniDir) )
			RageException::Throw( "The song foreground BGA \"%s\" is not a directory", sAniDir.c_str() );

		LoadedBGA bga;
		bga.m_bga = new BGAnimation;
		bga.m_bga->LoadFromAniDir( sAniDir );
		bga.m_fStartBeat = change.m_fStartBeat;

		const float fStartSecond = pSong->m_Timing.GetElapsedTimeFromBeat( bga.m_fStartBeat );
		const float fStopSecond = fStartSecond + bga.m_bga->GetLengthSeconds();
		bga.m_fStopBeat = pSong->m_Timing.GetBeatFromElapsedTime( fStopSecond );

		bga.m_bga->SetHidden( true );

		this->AddChild( bga.m_bga );
		m_BGAnimations.push_back( bga );
	}

	TEXTUREMAN->SetDefaultTexturePolicy( OldPolicy );

	this->SortByZ();
}

void Foreground::Update( float fDeltaTime )
{
	/* Calls to Update() should *not* be scaled by music rate. Undo it. */
	const float fRate = GAMESTATE->m_SongOptions.m_fMusicRate;

	for( unsigned i=0; i < m_BGAnimations.size(); ++i )
	{
		LoadedBGA &bga = m_BGAnimations[i];

		const bool Shown = bga.m_fStartBeat <= GAMESTATE->m_fSongBeat && GAMESTATE->m_fSongBeat <= bga.m_fStopBeat;
		if( !Shown )
		{
			bga.m_bga->SetHidden( true );
			continue;
		}

		float fDeltaTime;
		if( bga.m_bga->GetHidden() )
		{
			bga.m_bga->SetHidden( false );

			const float fStartSecond = m_pSong->m_Timing.GetElapsedTimeFromBeat( bga.m_fStartBeat );
			fDeltaTime = GAMESTATE->m_fMusicSeconds - fStartSecond;
		}
		else
		{
			fDeltaTime = GAMESTATE->m_fMusicSeconds - m_fLastMusicSeconds;
		}
		bga.m_bga->Update( fDeltaTime / fRate );
	}

	m_fLastMusicSeconds = GAMESTATE->m_fMusicSeconds;
}

