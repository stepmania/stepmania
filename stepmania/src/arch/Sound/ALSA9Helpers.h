#ifndef ALSA9_HELPERS_H
#define ALSA9_HELPERS_H

#define ALSA_PCM_NEW_HW_PARAMS_API
#define ALSA_PCM_NEW_SW_PARAMS_API
#include <alsa/asoundlib.h>

#include "SDL_types.h"

class Alsa9Buf
{
private:
	int channels, samplerate, samplebits;
	int buffersize;
	int64_t last_cursor_pos;
	bool samplerate_set_explicitly;

	snd_pcm_sframes_t total_frames;
	snd_pcm_uframes_t xfer_align;

	snd_pcm_t *pcm;

	bool Recover( int r );
	bool SetHWParams();
	bool SetSWParams();

	static void ErrorHandler(const char *file, int line, const char *function, int err, const char *fmt, ...);
		
public:
	static void GetSoundCardDebugInfo();
		
	enum hw { HW_HARDWARE, HW_SOFTWARE, HW_DONT_CARE };

	/* Call SetSampleRate before you use the sample. */
	Alsa9Buf( hw hardware, int channels );
	~Alsa9Buf();

	int GetNumFramesToFill( int writeahead );
	void Write( const Sint16 *buffer, int frames );
	unsigned FindSampleRate( unsigned rate );
	
	void Reset();
	void Play();
	void Stop();
	void SetVolume(float vol);
	void SetSampleRate(int hz);
	int GetSampleRate() const { return samplerate; }

	int64_t GetPosition() const;
	int64_t GetPlayPos() const { return last_cursor_pos; }
};
#endif
