#include "global.h"
#include "RageSoundManager.h"
#include "RageSounds.h"
#include "RageSound.h"

RageSounds *SOUND = NULL;

RageSounds::RageSounds()
{
	/* Init RageSoundMan first: */
	ASSERT( SOUNDMAN );
	music = new RageSound;
}

RageSounds::~RageSounds()
{
	delete music;
}


CString RageSounds::GetMusicPath() const
{
	return music->GetLoadedFilePath();
}

void RageSounds::PlayMusic(CString file, bool force_loop, float start_sec, float length_sec, float fade_len)
{
//	LOG->Trace("play '%s' (current '%s')", file.c_str(), music->GetLoadedFilePath().c_str());
	if(music->IsPlaying())
	{
		if( music->GetLoadedFilePath() == file )
			return;		// do nothing

		music->StopPlaying();
	}

	/* If file is blank, just stop. */
	if(file.empty())
	{
		music->Unload();
		return;
	}

	music->Load( file, false );
	if( force_loop )
		music->SetStopMode( RageSound::M_LOOP );

	if(start_sec == -1)
		music->SetStartSeconds();
	else
		music->SetStartSeconds(start_sec);

	if(length_sec == -1)
		music->SetLengthSeconds();
	else
		music->SetLengthSeconds(length_sec);

	music->SetFadeLength(fade_len);
	music->SetPositionSeconds();
	music->StartPlaying();
}

void RageSounds::PlayOnce( CString sPath )
{
	SOUNDMAN->PlayOnce( sPath );
}

void RageSounds::PlayOnceFromDir( CString PlayOnceFromDir )
{
	SOUNDMAN->PlayOnceFromDir( PlayOnceFromDir );
}

/*
-----------------------------------------------------------------------------
 Copyright (c) 2002-2003 by the person(s) listed below.  All rights reserved.
        Glenn Maynard
-----------------------------------------------------------------------------
*/

