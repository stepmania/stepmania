#include "global.h"
#include "Foreground.h"
#include "BGAnimation.h"
#include "RageUtil.h"
#include "IniFile.h"
#include "GameState.h"

Foreground::~Foreground()
{
	Unload();
}

void Foreground::Unload()
{
	for( unsigned i=0; i < m_BGAnimations.size(); ++i )
		delete m_BGAnimations[i].m_bga;
	m_BGAnimations.clear();
}

void Foreground::LoadFromSong( const Song *pSong )
{
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

	this->SortByZ();
}

void Foreground::Update( float fDeltaTime )
{
	if( GAMESTATE->m_fMusicSeconds == GameState::MUSIC_SECONDS_INVALID )
		return; /* hasn't been updated yet */

	for( unsigned i=0; i < m_BGAnimations.size(); ++i )
	{
		LoadedBGA &bga = m_BGAnimations[i];

		const bool Shown = bga.m_fStartBeat <= GAMESTATE->m_fSongBeat && GAMESTATE->m_fSongBeat <= bga.m_fStopBeat;
		bga.m_bga->SetHidden( !Shown );
		if( Shown )
			bga.m_bga->Update( fDeltaTime );
	}
}

