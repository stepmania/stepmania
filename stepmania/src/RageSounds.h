/*
 * RageSounds - Sound utilities
 */

#ifndef RAGE_SOUNDS_H
#define RAGE_SOUNDS_H

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
	void PlayOnceFromAnnouncer( CString sFolderName );

	float GetPlayLatency() const;
	void HandleSongTimer( bool on=true );
};

extern RageSounds *SOUND;
#endif

/*
 * Copyright (c) 2003-2004 Glenn Maynard
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

