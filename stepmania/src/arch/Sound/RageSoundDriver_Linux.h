/* (C) 2003 Tim Hentenaar */

#ifndef RAGE_SOUND_LINUX
#define RAGE_SOUND_LINUX

#include <vector>
using namespace std;

#include "RageSoundDriver.h"
#include "SDL_Thread.h"


/* A lot of this is from Glenn's RageSoundDriver_WaveOut */

class RageSound_Linux : public RageSoundDriver {
	public:

	struct sound {	
		RageSound *snd;
		bool stopping;
		int flush_pos; /* state == STOPPING only */
		sound() { snd = NULL; stopping = false; }
	};

	/* Currently Playing Sounds */
	vector<sound *> sounds;
	SDL_Thread *MixerThread;
	snd_pcm_sframes_t buffer_size;
	Sint16 *sbuffer;	/* Sound Buffer */
	Sint16 *chunks[8];	/* chunks of sound buffer */
	snd_pcm_t *playback_handle; /* Handle for PCM device */
	int last_pos;

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

