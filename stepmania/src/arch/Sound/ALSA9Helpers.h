#ifndef ALSA9_HELPERS_H
#define ALSA9_HELPERS_H

#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>

#include "SDL_types.h"

class Alsa9Buf
{
private:
	int channels, samplerate, samplebits;
	int buffersize, last_cursor_pos;

	snd_pcm_sframes_t total_frames;


	snd_pcm_t *pcm;

	bool Recover( int r );
	bool SetHWParams();

public:
	static void GetSoundCardDebugInfo();
		
	enum hw { HW_HARDWARE, HW_SOFTWARE, HW_DONT_CARE };

	/* If samplerate is DYNAMIC_SAMPLERATE, then call SetSampleRate before
	 * you use the sample. */
	enum { DYNAMIC_SAMPLERATE = -1 };
	Alsa9Buf( hw hardware, int channels, int samplerate );
	~Alsa9Buf();

	int GetNumFramesToFill( int writeahead );
	void Write( const Sint16 *buffer, int frames );

	void Reset();
	void Play();
	void Stop();
	void SetVolume(float vol);
	void SetSampleRate(int hz);

	int GetPosition() const;
	int GetPlayPos() const { return last_cursor_pos; }
};
#endif
