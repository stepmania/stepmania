#ifndef RAGE_SOUND_OBJ_H
#define RAGE_SOUND_OBJ_H

#include "SDL_sound-1.0.0/SDL_sound.h"

#include "RageThreads.h"
#include "RageSoundManager.h"

class CircBuf
{
	string buf;
	unsigned cnt, start;
	
public:

	CircBuf() { clear(); }
	unsigned size() const { return cnt; }
	unsigned capacity() const { return buf.size(); }
	void reserve(unsigned n);
	void clear();
	void write(const char *buf, unsigned size);
	void read(char *buf, unsigned size);
};

class RageSound
{
	/* These are only used when big == true: */
	struct stream_t {
		Sound_Sample *Sample;
		int FillBuf(int bytes);
		CircBuf buf;
	} stream;

	/* These are only used when big == false: */
	basic_string<char> full_buf;

	/* If true, we're streaming data through sample and buf.  If false, the entire
	 * file is pre-loaded into full_buf. */
	bool big;

	/* Sound blocks we've sent out recently through GetPCM.  We keep track
	 * of each block for the last four calls of GetPCM. */
	struct pos_map_t {
		/* Sample number from the POV of the sound driver: */
		int sampleno;

		/* Actual sound position within the sample: */
		int position;

		/* The number of samples in this block: */
		int samples;

		pos_map_t(int samp, int pos, int cnt) { sampleno=samp, position=pos; samples=cnt; }
	};
	vector<pos_map_t> pos_map[4];

	CString m_sFilePath;
//	float m_Rate;

	/* If looping, the range to loop: */
	float m_StartSeconds, m_LengthSeconds;
	bool Loop;

	/* Current position of the output sound; if < 0, nothing will play until it
	 * becomes positive.  This is recorded in samples, to avoid rounding error. */
	int		position;
	bool    playing;

	float speed;

public:
	RageSound();
	~RageSound();
	RageSound(const RageSound &cpy);

	/* If cache is true, we'll preload the entire file into memory if it's
	 * small enough.  False is only generally used when we're going to do
	 * operations on a file but not actually play it (eg. to find out
	 * its length). */
	void Load(CString fn, bool cache = true);

	void LoadAndPlayIfNotAlready( CString sSoundFilePath );
	void Unload();

	void Play( bool bLoop = false, float fStartSeconds = -1000, float fLengthSeconds = -1 );
	void Pause();
	void Stop();
	float GetLengthSeconds();
	float GetPositionSeconds();
	void SetPositionSeconds( float fSeconds );
	void SetPlaybackRate( float fScale );
	float GetPlaybackRate() const { return speed; }
	bool IsPlaying() const { return playing; }
	CString GetLoadedFilePath() const { return m_sFilePath; };

	/* Called only by the sound drivers: */
	/* This function should return the number of bytes actually put into buffer.
	 * If less than size is returned, it signals the stream to stop; once it's
	 * flushed, SoundStopped will be called.  Until then, SOUNDMAN->GetPosition
	 * can still be called (the sound is still playing). */
	int GetPCM(char *buffer, int size, int sampleno);

	/* Called by the sound driver when a sound has actually finished stopping
	 * normally.  Not called when Stop() is called to stop the sound prematurely. */
	void SoundStopped();

	void Update(float delta);
};

#endif
