#ifndef RAGE_SOUND_OBJ_H
#define RAGE_SOUND_OBJ_H

#include "RageThreads.h"
#include "RageSoundManager.h"

#include <deque>

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

class SoundReader;
class SpeedChanger;

class RageSound
{
public:
	/* M_STOP (default) stops the sound after m_LengthSamples have been played.
	 * M_LOOP restarts from m_StartSample.
	 * M_CONTINUE feeds silence, which is useful to continue timing longer than the
	 * actual sound. */
	enum StopMode_t {
		M_STOP, M_LOOP, M_CONTINUE
	} StopMode;

	RageSound();
	~RageSound();
	RageSound(const RageSound &cpy);

	/* If cache is true, we'll preload the entire file into memory if it's
	 * small enough.  False is only generally used when we're going to do
	 * operations on a file but not actually play it (eg. to find out
	 * its length).
	 * 
	 * If the file failed to load, false is returned, Error() is set
	 * and a null sample will be loaded.  (This makes failed loads nonfatal;
	 * they can be ignored most of the time, so we continue to work if a file
	 * is broken or missing.) */
	bool Load(CString fn, bool cache = true);

	void LoadAndPlayIfNotAlready( CString sSoundFilePath );
	void Unload();

	void SetStopMode(StopMode_t m) { StopMode = m; }
	StopMode_t GetStopMode() const { return StopMode; }

	void SetStartSeconds(float secs = 0); /* default = beginning */
	void SetLengthSeconds(float secs = -1); /* default = no length limit */
	void StartPlaying();
	void StopPlaying();

	CString GetError() const { return error; }
	bool Error() const { return !error.empty(); }

	RageSound *Play();
	void Stop();

	float GetLengthSeconds();
	float GetPositionSeconds() const;
	bool SetPositionSeconds( float fSeconds = -1);
	void SetAccurateSync(bool yes=true) { AccurateSync = yes; }
	void SetPlaybackRate( float fScale );
	float GetPlaybackRate() const { return speed; }
	bool IsPlaying() const { return playing; }
	CString GetLoadedFilePath() const { return m_sFilePath; }
	
	/* Query only: */
	bool IsStreaming() const { return big; }

private:
	/* If we were copied from another RageSound, this will point to it; otherwise
	 * this is ourself. */
	RageSound *original;

	/* These are only used when big == true: */
	struct stream_t {
		SoundReader *Sample;
		CircBuf buf;
	} stream;
	int FillBuf(int bytes);

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
	deque<pos_map_t> pos_map;

	CString m_sFilePath;

	/* The amount of data to play (or loop): */
	int m_StartSample, m_LengthSamples;
	
	/* Current position of the output sound; if < 0, nothing will play until it
	 * becomes positive.  This is recorded in samples, to avoid rounding error. */
	int		position;
	bool    playing;

	float speed;
	SpeedChanger *speedchanger; /* only if speed != 1 */

	bool AccurateSync;

	CString error;

	bool SetPositionSamples( int samples = -1 );
	int GetData(char *buffer, int size);
	void Fail(CString reason);
	int Bytes_Available() const;

public:
	/* Used by RageSoundManager: */
	RageSound *GetOriginal() { return original; }

	/* Called only by the sound drivers: */
	/* This function should return the number of bytes actually put into buffer.
	 * If less than size is returned, it signals the stream to stop; once it's
	 * flushed, SoundStopped will be called.  Until then, SOUNDMAN->GetPosition
	 * can still be called (the sound is still playing). */
	int GetPCM(char *buffer, int size, int sampleno);

	void Update(float delta);
};

#endif
