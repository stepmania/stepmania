/* This is just a simple wrapper to handle a single music track conveniently. */
#include "global.h"
#include "RageMusic.h"
#include "RageSound.h"

RageMusic::RageMusic()
{
	music = new RageSound;
}

RageMusic::~RageMusic()
{
	delete music;
}


CString RageMusic::GetPath() const
{
	return music->GetLoadedFilePath();
}

void RageMusic::Play(CString file, bool force_loop, float start_sec, float length_sec, float fade_len)
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

/*
-----------------------------------------------------------------------------
 Copyright (c) 2002-2003 by the person(s) listed below.  All rights reserved.
        Glenn Maynard
-----------------------------------------------------------------------------
*/

