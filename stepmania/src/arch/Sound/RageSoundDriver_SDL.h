#ifndef RAGE_SOUND_SDL
#define RAGE_SOUND_SDL
/*
 *  RageSoundDriver_SDL.h
 *  stepmania
 *
 *  Created by Steve Checkoway on Fri Jun 13 2003.
 *  Judicially copying from RageSoundDriver_Null.*
 *  Copyright (c) 2003 Steve Checkoway. All rights reserved.
 *
 */

#include "RageSound.h"
#include <time.h>
#include "RageSoundDriver.h"
#include <SDL.h>

class RageSound_SDL: public RageSoundDriver {
private:
  struct sound {
    RageSound *snd;
    bool stopping;
    int flush_pos;
    sound() { snd=NULL; stopping=false; flush_pos=0; }
  };

  vector<sound *> sounds;
  SDL_AudioSpec *specs;
  bool initedSDL_Audio;
  int last_cursor_pos;
  
protected:
  virtual void StartMixing(RageSound *snd);
  virtual void StopMixing(RageSound *snd);
  virtual int GetPosition(const RageSound *snd) const;
  virtual void Update(float delta);
  virtual float GetPlayLatency() const;

public:
  RageSound_SDL();
  virtual ~RageSound_SDL();
  static void GetData(void *userdata, Uint8 *stream, int len);
};

#endif /*RAGE_SOUND_SDL*/  