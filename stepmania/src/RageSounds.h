#ifndef RAGE_SOUNDS_H
#define RAGE_SOUNDS_H

/* This contains all of the generally useful user-level SOUNDMAN calls, as well
 * as some other simple helper stuff.  Don't include RageSoundManager for normal
 * use; it's only for drivers and sound code.  This file should be very lightweight. */
class TimingData;
class RageSound;
class RageSounds
{
public:
	RageSounds();
	~RageSounds();
	void Update( float fDeltaTime );

	void PlayMusic( const CString &file, bool force_loop = false, float start_sec = 0, float length_sec = -1, float fade_len = 0, bool align_beat = true ) { PlayMusic( file, "", force_loop, start_sec, length_sec, fade_len, align_beat ); }
	void PlayMusic( const CString &file, const CString &timing_file, bool force_loop = false, float start_sec = 0, float length_sec = -1, float fade_len = 0, bool align_beat = true );
	void StopMusic() { PlayMusic(""); }
	CString GetMusicPath() const;

	/* Take ownership of snd; handle timing, free it, etc. */
	void TakeOverSound( RageSound *snd, const TimingData *Timing );

	void PlayOnce( CString sPath );
	void PlayOnceFromDir( CString sDir );

	float GetPlayLatency() const;
	void HandleSongTimer( bool on=true );
};

extern RageSounds *SOUND;
#endif
/*
-----------------------------------------------------------------------------
 Copyright (c) 2002-2003 by the person(s) listed below.  All rights reserved.
        Glenn Maynard
-----------------------------------------------------------------------------
*/
