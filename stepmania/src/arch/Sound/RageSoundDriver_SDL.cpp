/*
 *  RageSoundDriver_SDL.cpp
 *  stepmania
 *
 *  Created by Steve Checkoway on Fri Jun 13 2003.
 *  Copyright (c) 2003 Steve Checkoway. All rights reserved.
 *
 */

#include "global.h"
#include "RageSoundDriver_SDL.h"
#include "RageTimer.h"
#include "RageLog.h"
#include "RageSound.h"
#include "RageUtil.h"

const unsigned channels = 2;
const unsigned samplerate = 44100;
const unsigned samplesize = channels*2;
const unsigned buffersize_frames = 1024; /* in samples */
const unsigned buffersize = buffersize_frames * samplesize;

RageSound_SDL::RageSound_SDL() {
  last_cursor_pos = 0;
  /* Do we need to init SDL_Audio? */
  initedSDL_Audio = SDL_WasInit(SDL_INIT_AUDIO) != SDL_INIT_AUDIO;

  if (initedSDL_Audio)
    if (SDL_InitSubSystem(SDL_INIT_AUDIO)) {
      LOG->Trace("Initializing SDL_Audio subsytem.");
      RageException::ThrowNonfatal(SDL_GetError());
    }

  SDL_AudioSpec *specs = static_cast<SDL_AudioSpec *>(calloc(1, sizeof(SDL_AudioSpec)));
  specs->freq = samplerate;
  specs->format = AUDIO_S16MSB; /* Signed 16-bit big-endian samples */
  specs->channels = channels;
  specs->samples = buffersize_frames;
  specs->callback = GetData;
  specs->userdata = this;
  if (SDL_OpenAudio(specs, NULL))
    RageException::ThrowNonfatal(SDL_GetError());
  ASSERT(specs->size == buffersize);
  SDL_PauseAudio(0);
}

RageSound_SDL::~RageSound_SDL() {
  SDL_CloseAudio();
  LOG->Trace("Mixing thread shut down.");
  /* Did we init SDL_Audio? */
  if (initedSDL_Audio) {
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    LOG->Trace("SDL_Audio subsystem quit.");
  }
}

/* This function is called by the SDL_Audio subsytem to get the data
 * to output.
 * Params: userdata: A pointer to the creating RageSound_SDL.
 *         stream:   The audio buffer to fill.
 *         len:      The length of the buffer.
 */
void RageSound_SDL::GetData(void *userdata, Uint8 *stream, int len) {
  LockMutex L(SOUNDMAN->lock);
  static Sint16 *buf = NULL;
  static int bufsize = buffersize_frames * channels;
  static SoundMixBuffer mix;
  if (!buf)
    buf = new Sint16[bufsize];
  RageSound_SDL *P = static_cast<RageSound_SDL *>(userdata);

  memset(buf, 0, bufsize*sizeof(Uint16));

  /* I don't like this but I can't think of another place to put it. */
  while (!SOUNDMAN)
    SDL_Delay(10);
  
  for (unsigned i = 0; i < P->sounds.size(); ++i) {
    if (P->sounds[i]->stopping)
      continue;

    int got = P->sounds[i]->snd->GetPCM(reinterpret_cast<char *>(buf), len, P->last_cursor_pos);

    mix.write(buf, got/2);
    if (got < len) {
      P->sounds[i]->stopping = true;
      P->sounds[i]->flush_pos = P->last_cursor_pos + (got / samplesize);
    }
  }

  mix.read(buf); 

  memcpy(stream, buf, len);
  P->last_cursor_pos += buffersize_frames;
}

void RageSound_SDL::StartMixing(RageSound *snd) {
  sound *s = new sound;
  s->snd = snd;

  LockMutex L(SOUNDMAN->lock);
  sounds.push_back(s);
}

void RageSound_SDL::StopMixing(RageSound *snd) {
  LockMutex L(SOUNDMAN->lock);

  /* Find the sound. */
  unsigned i;
  for(i = 0; i < sounds.size(); ++i)
    if(sounds[i]->snd == snd) break;
  if(i == sounds.size())
    {
    LOG->Trace("not stopping a sound because it's not playing");
    return;
    }

  delete sounds[i];
  sounds.erase(sounds.begin()+i, sounds.begin()+i+1);
}

void RageSound_SDL::Update(float delta) {
  LockMutex L(SOUNDMAN->lock);

  vector<sound *>snds = sounds;
  for (unsigned i = 0; i < snds.size(); ++i) {
    if (!sounds[i]->stopping)
      continue;
    if (GetPosition(snds[i]->snd) < sounds[i]->flush_pos)
      continue;
    snds[i]->snd->StopPlaying();
  }
}

/* What is this parameter for?
 * This returns the same as is passed to RageSound::GetPCM
 * as required by RageSoundDriver. I suppose I don't really
 * understand the purpose of this method.
 * --Steve
 */
int RageSound_SDL::GetPosition(const RageSound *snd) const {
  return last_cursor_pos;
}

float RageSound_SDL::GetPlayLatency() const {
  return (1.0f / samplerate) * (7/8);
}

