#ifndef RAGE_MUSIC_H

/* This contains all of the generally useful user-level SOUNDMAN calls, as well
 * as some other simple helper stuff.  Don't include RageSoundManager for normal
 * use; it's only for drivers and sound code.  This file should be very lightweight. */
 */
class RageSound;
class RageSounds
{
	RageSound *music;

public:
	RageSounds();
	~RageSounds();

	void PlayMusic(CString file, bool force_loop = false, float start_sec = -1, float length_sec = -1, float fade_len = 0);
	void StopMusic() { PlayMusic(""); }
	CString GetMusicPath() const;

	void PlayOnce( CString sPath );
	void PlayOnceFromDir( CString sDir );
};

extern RageSounds *SOUND;
#endif
/*
-----------------------------------------------------------------------------
 Copyright (c) 2002-2003 by the person(s) listed below.  All rights reserved.
        Glenn Maynard
-----------------------------------------------------------------------------
*/
