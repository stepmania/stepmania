/* (C) 2003 Tim Hentenaar */

#ifndef RAGE_SOUND_LINUX
#define RAGE_SOUND_LINUX

#include <vector>
using namespace std;

#include "RageSoundDriver.h"
#include "SDL_thread.h"
#include "global.h"
#include <alsa/asoundlib.h>


/* A lot of this is from Glenn's RageSoundDriver_WaveOut */

struct sound {	
		RageSound *snd;
		bool stopping;
		int flush_pos; /* state == STOPPING only */
		sound() { snd = NULL; stopping = false; }
};

typedef struct sound sound_t;

class RageSound_Linux : public RageSoundDriver {
	public:
	

	/* Currently Playing Sounds */
	vector<sound_t *> sounds;
	SDL_Thread *MixerThreadPtr;
	snd_pcm_uframes_t buffer_size;
	Sint16 *sbuffer;	/* Sound Buffer */
	Sint16 *chunks[8];	/* chunks of sound buffer */
	snd_pcm_t *playback_handle; /* Handle for PCM device */
	int last_pos;
	bool shutdown;

	void RecoverState(long state); /* Recover from various states */

	void MixerThread();
	static int MixerThread_start(void *p);
	bool GetData();
	void OpenAudio();
	void CloseAudio();
	void StartMixing(RageSound *snd);
	void StopMixing(RageSound *snd);
	int GetPosition(const RageSound *snd) const;
	float GetPlayLatency() const;
	
	void Update(float delta);
	RageSound_Linux();
	~RageSound_Linux();
};

#endif

