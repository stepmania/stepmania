/*
     PLIB - A Suite of Portable Game Libraries
     Copyright (C) 1998,2002  Steve Baker
 
     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Library General Public
     License as published by the Free Software Foundation; either
     version 2 of the License, or (at your option) any later version.
 
     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Library General Public License for more details.
 
     You should have received a copy of the GNU Library General Public
     License along with this library; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 
     For further information visit http://plib.sourceforge.net

     $Id$
*/


#ifndef __SL_H__
#define __SL_H__ 1

#include "slPortability.h"

#ifdef SL_USING_OSS_AUDIO
#define SLDSP_DEFAULT_DEVICE "/dev/dsp"
#elif defined(WIN32)
#define SLDSP_DEFAULT_DEVICE "dsp"	
#elif defined(BSD)
#define SLDSP_DEFAULT_DEVICE "/dev/audio"
#elif defined(sgi)
#define SLDSP_DEFAULT_DEVICE "dsp"		// dummy ...
#elif defined(SOLARIS)
#define SLDSP_DEFAULT_DEVICE "/dev/audio"
#elif defined(macintosh) || defined(__APPLE__)
#define SLDSP_DEFAULT_DEVICE "dsp" // dummy
#else
#error "Port me !"
#endif

# define SL_TRUE  1
# define SL_FALSE 0

typedef unsigned char  Uchar  ;
typedef unsigned short Ushort ;

#define SL_DEFAULT_SAMPLING_RATE 11025

class slSample       ;
class slPlayer       ;
class slSamplePlayer ;
class slEnvelope     ;
class slScheduler    ;
class slDSP          ;

extern const char *__slPendingError ;

class slDSP
{
private:

  int stereo ;
  int rate ;
  int bps ;

  int error ;
  int fd ;

#if defined (__NetBSD__) || defined(__OpenBSD__)
  audio_info_t    ainfo;        // ioctl structure
  audio_offset_t  audio_offset; // offset in audiostream
  long            counter;      // counter-written packets
#elif defined(SOLARIS)
  audio_info_t	  ainfo;
  long            counter;
#elif defined(SL_USING_OSS_AUDIO)
  audio_buf_info buff_info ;
#elif defined(sgi)
  ALconfig        config;       // configuration stuff
  ALport          port;         // .. we are here 
#elif defined(macintosh) || defined(__APPLE__)

// Size of the data chunks written with write().
// This should be a multiple of 1024.
#define BUFFER_SIZE 8192

// Size of storage space for sound data.
// This should be evenly divisible by BUFFER_SIZE.
#define VIRTUAL_BUFFER_SIZE 131072

  SndChannelPtr  sndChannel   ;   // sound channel we are using
  ExtSoundHeader extSndHeader ;   // sound header for bufferCmd
  SndCommand     currentCmd   ;   // command we are sending to chnl
  OSErr          osErr ;

  char           *buf  ;          // buffer containing sound data
  char           *rpos ;          // read position for buffer
  char           *wpos ;          // write position for buffer
  char           *ptr  ;          // ptr to data from write()

  float          bytesPerSample ; // conversions
  float          bytesPerSecond ;
  float          secondsPerPacket ;

  float          secLeft  ;       // seconds left in buffer
  float          secUsed  ;       // seconds used in buffer
  UnsignedWide   lastTime ;       // used for timing in secondsUsed()
  UnsignedWide   currTime ;

#endif


#if !defined(WIN32) && !defined(macintosh)
  int ioctl ( int cmd, int param = 0 )
  {
    if ( error ) return param ;

    if ( ::ioctl ( fd, cmd, & param ) == -1 )
    {
      perror ( "slDSP: ioctl" ) ;
      error = SL_TRUE ;
    }

    return param ;
  }

#elif defined(WIN32)

#define BUFFER_COUNT (3*8)
#define BUFFER_SIZE  (1024*1)

   friend void CALLBACK waveOutProc( HWAVEOUT hwo, UINT uMsg,
         DWORD dwInstance, DWORD dwParam1, DWORD dwParam2 );
   HWAVEOUT       hWaveOut;      // device handle 
   WAVEFORMATEX   Format;        // open needs this
   MMTIME         mmt;	         // timing 
   WAVEHDR       *wavehdr[ BUFFER_COUNT ];  // for round robin ..
   int            curr_header;   // index of actual wavehdr
   int            counter;       // counter-written packets
   DWORD          written;       // counter-written samples

#endif

  void open ( const char *device, int _rate, int _stereo, int _bps ) ;
  void close () ;
  void getBufferInfo () ;
  void write ( void *buffer, size_t length ) ;

protected:

  void setError () { error = SL_TRUE ; }
  int getDriverBufferSize () ;

public:

  slDSP ( int _rate = SL_DEFAULT_SAMPLING_RATE,
          int _stereo = SL_FALSE, int _bps = 8 )
  {
    open ( SLDSP_DEFAULT_DEVICE, _rate, _stereo, _bps ) ;
  } 

  slDSP ( const char *device, int _rate = SL_DEFAULT_SAMPLING_RATE,
          int _stereo = SL_FALSE, int _bps = 8 )
  {
    open ( device, _rate, _stereo, _bps ) ;
  } 

 ~slDSP () { close () ; }
  
  float secondsRemaining () ;
  float secondsUsed () ;

  void play ( void *buffer, size_t length ) { write ( buffer, length ) ; } 

  /* This was a typo - but people use it */
  int not_working() const { return error ; }

  int notWorking () const { return error ; }

  int getBps   () const { return bps    ; }
  int getRate  () const { return rate   ; }
  int getStereo() const { return stereo ; }

  void sync () ;
  void stop () ;
} ;


class slSample
{
  int    ref_count ;
protected:

  char  *comment;
  int    rate   ;
  int    bps    ;
  int    stereo ;

  Uchar *buffer ;
  int    length ;

  void init ()
  {
    ref_count = 0 ;
    comment = NULL  ;
    buffer  = NULL  ;
    length  = 0     ;
    rate    = SL_DEFAULT_SAMPLING_RATE ;
    bps     = 8     ;
    stereo  = SL_FALSE ;
  }

public:

  slSample () { init () ; }

  slSample ( const Uchar *buff, int leng )
  {
    init () ;
    setBuffer ( buff, leng ) ;
  }

  slSample ( const char *fname, const class slDSP *dsp = NULL )
  {
    if ( dsp != NULL && dsp->notWorking () )
      dsp = NULL ;

    init () ;
    loadFile ( fname ) ;
    autoMatch ( dsp ) ;
  }

 ~slSample ()
  {
    if ( ref_count != 0 && __slPendingError == NULL )
      __slPendingError =
        "slSample: FATAL ERROR - Application deleted a sample while it was playing." ;

    delete [] buffer ;
  }
  
  void ref   () { ref_count++ ; }
  void unRef () { ref_count-- ; }

  int getPlayCount () const { return ref_count ; }

  char  *getComment () const { return comment ; }

  void   setComment ( const char *nc )
  {
    delete [] comment ;
    comment = new char [ strlen ( nc ) + 1 ] ;
    strcpy ( comment, nc ) ;
  }

  Uchar *getBuffer () const { return buffer ; }
  int    getLength () const { return length ; }

  void  autoMatch ( const slDSP *dsp ) ;

  void   setBuffer ( const Uchar *buff, int leng )
  {
    delete [] buffer ;

    buffer = new Uchar [ leng ] ;

    if ( buff != NULL )
      memcpy ( buffer, buff, leng ) ;

    length = leng ;
  }

  /* These routines only set flags - use changeXXX () to convert a sound */

  void setRate   ( int r ) { rate   = r ; }
  void setBps    ( int b ) { bps    = b ; }
  void setStereo ( int s ) { stereo = s ; }

  int  getRate   () const  { return rate   ; }
  int  getBps    () const  { return bps    ; }
  int  getStereo () const  { return stereo ; }

  float getDuration () const { return (float) getLength() /
                                      (float) ( (getStereo()?2.0f:1.0f)*
                                                (getBps()/8.0f)*getRate() ) ; }

  int loadFile    ( const char *fname ) ;

  int loadRawFile ( const char *fname ) ;
  int loadAUFile  ( const char *fname ) ;
  int loadWavFile ( const char *fname ) ;

  void changeRate       ( int r ) ;
  void changeBps        ( int b ) ;
  void changeStereo     ( int s ) ;
  void changeToUnsigned () ;

  void adjustVolume ( float vol ) ;

  void print ( FILE *fd ) const
  {
    if ( buffer == NULL )
    {
      fprintf ( fd, "Empty sample buffer\n" ) ;
    }
    else
    {
      fprintf ( fd, "\"%s\"\n",(getComment() == NULL ||
                                getComment()[0]=='\0') ? "Sample" : comment  ) ;
      fprintf ( fd, "%s, %d bits per sample.\n",
			    getStereo() ? "Stereo" : "Mono", getBps() ) ;
      fprintf ( fd, "%gKHz sample rate.\n", (float) getRate() / 1000.0f ) ;
      fprintf ( fd, "%d bytes of samples == %g seconds duration.\n", getLength(), getDuration() ) ;
    }
  }
} ;


enum slSampleStatus
{
  SL_SAMPLE_WAITING,  /* Sound hasn't started playing yet */
  SL_SAMPLE_RUNNING,  /* Sound has started playing */
  SL_SAMPLE_DONE   ,  /* Sound is complete */
  SL_SAMPLE_PAUSED    /* Sound hasn't started playing yet */
} ;


enum slPreemptMode
{
  SL_SAMPLE_CONTINUE,  /* Don't allow yourself to be preempted   */
  SL_SAMPLE_ABORT   ,  /* Abort playing the sound when preempted */
  SL_SAMPLE_RESTART ,  /* Restart the sound when load permits    */
  SL_SAMPLE_MUTE    ,  /* Continue silently until load permits   */
  SL_SAMPLE_DELAY      /* Pause until load permits               */
} ;


enum slReplayMode
{
  SL_SAMPLE_LOOP,      /* Loop sound so that it plays forever */
  SL_SAMPLE_ONE_SHOT   /* Play sound just once */
} ;

enum slEvent
{
  SL_EVENT_COMPLETE,  /* Sound finished playing */
  SL_EVENT_LOOPED,    /* Sound looped back to the start */
  SL_EVENT_PREEMPTED  /* Sound was preempted */
} ;

typedef void (*slCallBack) ( slSample *, slEvent, int ) ;

class slEnvelope
{
protected:

  float *time  ;
  float *value ;
  int   nsteps ;
  int   ref_count ;

  slReplayMode replay_mode ;

  int getStepDelta ( float *_time, float *delta ) const ;

public:

  slEnvelope ( int _nsteps, slReplayMode _rm, float *_times, float *_values )
  {
    ref_count = 0 ;
    nsteps = _nsteps ;
    time  = new float [ nsteps ] ;
    value = new float [ nsteps ] ;
    memcpy ( time , _times , sizeof(float) * nsteps ) ;
    memcpy ( value, _values, sizeof(float) * nsteps ) ;

    replay_mode = _rm ;
  }


  slEnvelope ( int _nsteps = 1, slReplayMode _rm = SL_SAMPLE_ONE_SHOT )
  {
    ref_count = 0 ;
    nsteps = _nsteps ;
    time  = new float [ nsteps ] ;
    value = new float [ nsteps ] ;

    for ( int i = 0 ; i < nsteps ; i++ )
      time [ i ] = value [ i ] = 0.0 ;

    replay_mode = _rm ;
  }
 
 ~slEnvelope ()
  {
    if ( ref_count != 0 && __slPendingError == NULL )
      __slPendingError =
      "slEnvelope: FATAL ERROR - Application deleted an envelope while it was playing.\n" ;

    delete [] time ;
    delete [] value ;
  }

  void ref   () { ref_count++ ; }
  void unRef () { ref_count-- ; }

  int getPlayCount () const { return ref_count ; }

  void setStep ( int n, float _time, float _value )
  {
    if ( n >= 0 && n < nsteps )
    {
      time  [ n ] = _time  ;
      value [ n ] = _value ;
    }
  }

  float getStepValue ( int s ) const { return value [ s ] ; }
  float getStepTime  ( int s ) const { return time  [ s ] ; }

  int   getNumSteps  () const { return nsteps ; }

  float getValue ( float _time ) const ;

  void applyToPitch     ( Uchar *dst, slPlayer *src, int nframes, int start, int next_env ) ;
  void applyToInvPitch  ( Uchar *dst, slPlayer *src, int nframes, int start, int next_env ) ;
  void applyToVolume    ( Uchar *dst, Uchar *src, int nframes, int start ) ;
  void applyToInvVolume ( Uchar *dst, Uchar *src, int nframes, int start ) ;
} ;

#define SL_MAX_PRIORITY  16
#define SL_MAX_SAMPLES   16
#define SL_MAX_CALLBACKS (SL_MAX_SAMPLES * 2)
#define SL_MAX_ENVELOPES 4
#define SL_MAX_MIXERINPUTS 10

enum slEnvelopeType
{
  SL_PITCH_ENVELOPE , SL_INVERSE_PITCH_ENVELOPE ,
  SL_VOLUME_ENVELOPE, SL_INVERSE_VOLUME_ENVELOPE,
  SL_FILTER_ENVELOPE, SL_INVERSE_FILTER_ENVELOPE,
  SL_PAN_ENVELOPE   , SL_INVERSE_PAN_ENVELOPE   ,
  SL_ECHO_ENVELOPE  , SL_INVERSE_ECHO_ENVELOPE  ,

  SL_NULL_ENVELOPE
} ;

struct slPendingCallBack
{
  slCallBack callback ;
  slSample  *sample   ;
  slEvent    event    ;
  int        magic    ;
} ;

class slPlayer
{
protected:

  slEnvelope    *env            [ SL_MAX_ENVELOPES ] ;
  slEnvelopeType env_type       [ SL_MAX_ENVELOPES ] ;
  int            env_start_time [ SL_MAX_ENVELOPES ] ;

  slReplayMode   replay_mode     ;
  slPreemptMode  preempt_mode    ;
  slSampleStatus status          ;
  int            priority        ;

  slCallBack     callback ;
  int            magic ;

  virtual void  low_read ( int nframes, Uchar *dest ) = 0 ;
  virtual void  skip ( int nframes ) = 0 ;

public:

  slPlayer ( slReplayMode  rp_mode = SL_SAMPLE_ONE_SHOT, 
	     int pri = 0, slPreemptMode pr_mode = SL_SAMPLE_DELAY,
             int _magic = 0, slCallBack cb = NULL )
  {
    magic           = _magic ;
    callback        = cb ;

    for ( int i = 0 ; i < SL_MAX_ENVELOPES ; i++ )
    {
      env [ i ] = NULL ;
      env_type [ i ] = SL_NULL_ENVELOPE ;
    }

    status          = SL_SAMPLE_WAITING ;
    replay_mode     = rp_mode ;
    preempt_mode    = pr_mode ;
    priority        = pri ;
  }

  virtual ~slPlayer () ;

  slPreemptMode getPreemptMode () const { return preempt_mode ; }

  int getPriority () const
  {
    return ( isRunning() &&
             preempt_mode == SL_SAMPLE_CONTINUE ) ? (SL_MAX_PRIORITY+1) :
                                                               priority ;
  }

  void addEnvelope ( int i, slEnvelope *_env, slEnvelopeType _type ) ;

  virtual void pause ()
  {
    if ( status != SL_SAMPLE_DONE )
      status = SL_SAMPLE_PAUSED ;
  }

  virtual void resume ()
  {
    if ( status == SL_SAMPLE_PAUSED )
      status = SL_SAMPLE_RUNNING ;
  }

  virtual void reset ()
  {
    status = SL_SAMPLE_WAITING ;
  }

  virtual void start ()
  {
    status = SL_SAMPLE_RUNNING ;
  } 

  virtual void stop ()
  {
    status = SL_SAMPLE_DONE ;
  } 

  int getMagic  () const { return magic  ; }
  int isWaiting () const { return status == SL_SAMPLE_WAITING ; } 
  int isPaused  () const { return status == SL_SAMPLE_PAUSED  ; } 
  int isRunning () const { return status == SL_SAMPLE_RUNNING ; } 
  int isDone    () const { return status == SL_SAMPLE_DONE    ; }

  virtual int preempt ( int delay ) ;
  
  virtual slSample *getSample () const = 0 ;

  void read ( int nframes, Uchar *dest, int next_env = 0 ) ;
} ;


class MODfile ;

class slMODPlayer : public slPlayer
{
  MODfile *mf ;

  void  low_read ( int nframes, Uchar *dest ) ;
  void  init ( const char *fname ) ;
public:

  slMODPlayer ( const char *fname, slReplayMode rp_mode = SL_SAMPLE_ONE_SHOT, 
		int pri = 0, slPreemptMode pr_mode = SL_SAMPLE_DELAY,
                int _magic = 0, slCallBack cb = NULL ) :
                slPlayer ( rp_mode, pri, pr_mode, _magic, cb )
  {
    init ( fname ) ;
    reset () ;
  }

  ~slMODPlayer () ;

  int preempt ( int delay ) ;
  virtual slSample *getSample () const { return NULL ; }

  void skip ( int nframes ) ;
} ;


class slSamplePlayer : public slPlayer
{
  int            lengthRemaining ;  /* Sample frames remaining until repeat */
  Uchar         *bufferPos       ;  /* Sample frame to replay next */
  slSample      *sample          ;

  void  low_read ( int nframes, Uchar *dest ) ;

public:

  slSamplePlayer ( slSample *s, slReplayMode  rp_mode = SL_SAMPLE_ONE_SHOT, 
		   int pri = 0, slPreemptMode pr_mode = SL_SAMPLE_DELAY,
                   int _magic = 0, slCallBack cb = NULL ) :
                   slPlayer ( rp_mode, pri, pr_mode, _magic, cb )
  {
    sample = s ;

    if ( sample ) sample -> ref () ;

    reset () ;
  }

  ~slSamplePlayer () ;

  int preempt ( int delay ) ;
  
  void reset ()
  {
    slPlayer::reset () ;
    
    lengthRemaining = sample->getLength () ;
    bufferPos       = sample->getBuffer () ;
  } 

  void start ()
  {
    slPlayer::start () ;
    
    lengthRemaining = sample->getLength () ;
    bufferPos       = sample->getBuffer () ;
  } 

  void stop ()
  {
    slPlayer::stop () ;
    
    lengthRemaining = 0    ;
    bufferPos       = NULL ;
  } 

  slSample *getSample () const { return sample ; }

  void skip ( int nframes ) ;
} ;


class slScheduler : public slDSP
{
  slPendingCallBack pending_callback [ SL_MAX_CALLBACKS ] ;
  int num_pending_callbacks ;

  float safety_margin ;

  int mixer_buffer_size, mixer_gain ;

  Uchar *mixer_buffer  ;
  Uchar *mixer_inputs [ SL_MAX_MIXERINPUTS + 1 ] ;
  
  Uchar *mixer ;
  int amount_left ;

  slPlayer *player [ SL_MAX_SAMPLES ] ;
  slPlayer *music ;

  void init () ;

  void realUpdate ( int dump_first = SL_FALSE ) ;

  void initBuffers () ;

  int now ;

  static slScheduler *current ;

public:

  slScheduler ( int _rate = SL_DEFAULT_SAMPLING_RATE ) : slDSP ( _rate, SL_FALSE, 8 ) { init () ; }
  slScheduler ( const char *device,
                int _rate = SL_DEFAULT_SAMPLING_RATE ) : slDSP ( device, _rate, SL_FALSE, 8 ) { init () ; }
 ~slScheduler () ;

  static slScheduler *getCurrent () { return current ; }

  int   getTimeNow     () const { return now ; }
  float getElapsedTime ( int then ) const { return (float)(now-then)/(float)getRate() ; }

  void setMaxConcurrent ( int mc );
  int  getMaxConcurrent () const ;

  void flushCallBacks () ;
  void addCallBack    ( slCallBack c, slSample *s, slEvent e, int m ) ;

  void update     () { realUpdate ( SL_FALSE ) ; }
  void dumpUpdate () { realUpdate ( SL_TRUE  ) ; }

  void resumeSample ( slSample *s = NULL, int magic = 0 ) ;
  void resumeMusic  ( int magic = 0 ) ;
  void pauseSample  ( slSample *s = NULL, int magic = 0 ) ;
  void pauseMusic   ( int magic = 0 ) ;
  void stopSample   ( slSample *s = NULL, int magic = 0 ) ;
  void stopMusic    ( int magic = 0 ) ;

  void addSampleEnvelope ( slSample *s = NULL, int magic = 0,
                           int slot = 1, slEnvelope *e = NULL, 
                           slEnvelopeType t =  SL_VOLUME_ENVELOPE ) ;
  void addMusicEnvelope  ( int magic = 0,
                           int slot = 1, slEnvelope *e = NULL, 
                           slEnvelopeType t =  SL_VOLUME_ENVELOPE ) ;
  int  loopSample        ( slSample *s, int pri = 0,
                           slPreemptMode mode = SL_SAMPLE_MUTE,
                           int magic = 0, slCallBack cb = NULL ) ;
  int  loopMusic         ( const char *fname, int pri = 0,
                           slPreemptMode mode = SL_SAMPLE_MUTE,
                           int magic = 0, slCallBack cb = NULL ) ;
  int  playSample        ( slSample *s, int pri = 1,
                           slPreemptMode mode = SL_SAMPLE_ABORT,
                           int magic = 0, slCallBack cb = NULL ) ;
  int  playMusic         ( const char *fname, int pri = 1,
                           slPreemptMode mode = SL_SAMPLE_ABORT,
                           int magic = 0, slCallBack cb = NULL ) ;

  void setSafetyMargin ( float seconds ) { safety_margin = seconds ; }
} ;

#endif

