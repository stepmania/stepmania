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


#include "sl.h"
#include <errno.h>

static int init_bytes ;

#ifdef SL_USING_OSS_AUDIO

/* ------------------------------------------------------------ */
/* OSSAUDIO - Linux, FreeBSD, etc                               */
/* ------------------------------------------------------------ */

void slDSP::open ( const char *device, int _rate, int _stereo, int _bps )
{
  fd = ::open ( device, O_WRONLY | O_NONBLOCK ) ;

  if ( fd < 0 )
  {
    perror ( "slDSP: open" ) ;
    error = SL_TRUE ;

    stereo     = SL_FALSE ;
    bps        =    1     ;
    rate       =  8000    ;
    init_bytes =    0     ;
  }
  else
  {
    error = SL_FALSE ;

    /* Set up a driver fragment size of 1024 (ie 2^10) */

errno = 0 ;
    ioctl ( SNDCTL_DSP_SETFRAGMENT, 0x7FFF000A ) ;

    stereo = ioctl ( SOUND_PCM_WRITE_CHANNELS, _stereo ? 2 : 1 ) >= 2 ; 
    bps    = ioctl ( SOUND_PCM_WRITE_BITS, _bps ) ; 
    rate   = ioctl ( SOUND_PCM_WRITE_RATE, _rate ) ; 

if ( errno != 0 ) perror ( "slDSP: ioctl" ) ;

    getBufferInfo () ;
    init_bytes = buff_info.bytes ;
  }
}


void slDSP::close ()
{
  if ( fd >= 0 )
    ::close ( fd ) ;
}


int slDSP::getDriverBufferSize ()
{
  if ( error )
    return 0 ;

  getBufferInfo () ;
  return buff_info.fragsize ;
}

void slDSP::getBufferInfo ()
{
  if ( error )
    return ;

  if ( ::ioctl ( fd, SNDCTL_DSP_GETOSPACE, & buff_info ) < 0 )
  {
    perror ( "slDSP: getBufferInfo" ) ;
    error = SL_TRUE ;
    return ;
  }
}


void slDSP::write ( void *buffer, size_t length )
{
  if ( error || (int)length <= 0 )
    return ;

  size_t nwritten = ::write ( fd, (const char *) buffer, length ) ;

  if ( (int)nwritten < 0 )
    perror ( "slDSP: write" ) ;
  else
  if ( nwritten != length )
    perror ( "slDSP: short write" ) ;
}


float slDSP::secondsRemaining ()
{
  if ( error )
    return 0.0f ;

  getBufferInfo () ;

  int samples_left = buff_info.fragments * buff_info.fragsize ;

  if (   stereo  ) samples_left /= 2 ;
  if ( bps == 16 ) samples_left /= 2 ;
  return   (float) samples_left / (float) rate ;
}


float slDSP::secondsUsed ()
{
  if ( error )
    return 0.0f ;

  getBufferInfo () ;

  int samples_used = init_bytes - buff_info.bytes ;

  if (  stereo   ) samples_used /= 2 ;
  if ( bps == 16 ) samples_used /= 2 ;
  return   (float) samples_used / (float) rate ;
}


void slDSP::sync ()
{ 
   if ( !error) ::ioctl ( fd, SOUND_PCM_SYNC , 0 ) ; 
}

void slDSP::stop ()
{ 
   if ( !error) ::ioctl ( fd, SOUND_PCM_RESET, 0 ) ; 
}

#endif

#ifdef WIN32

/* ------------------------------------------------------------ */
/* win32                                                        */
/* ------------------------------------------------------------ */

static	void	wperror(MMRESULT num)
{
   char	buffer[0xff];  // yes, this is hardcoded :-)

   waveOutGetErrorText( num, buffer, sizeof(buffer)-1);

   ulSetError ( UL_WARNING, "SlDSP: %s (%d)", buffer, num );
}



void CALLBACK waveOutProc( HWAVEOUT hwo, UINT uMsg,	
      DWORD dwInstance, DWORD dwParam1, DWORD dwParam2 )
{   
   slDSP *pDsp = (slDSP *)dwInstance;
   switch( uMsg )
   {
   case    WOM_CLOSE:
      break;

   case    WOM_OPEN:
      break;

   case    WOM_DONE:
      pDsp->counter--;
      break;
   }
}


void slDSP::open ( const char *device, int _rate, int _stereo, int _bps )
{
   MMRESULT	result;
   int i;

   hWaveOut   	= NULL;
   curr_header	= 0;
   counter      = 0;
   written      = 0;
   for ( i = 0; i < BUFFER_COUNT; i++ ) wavehdr[i] = NULL;

   Format.wFormatTag       = WAVE_FORMAT_PCM;		
   Format.nChannels        = _stereo ? 2 : 1;
   Format.nSamplesPerSec   = _rate;
   Format.wBitsPerSample   = _bps;
   Format.nBlockAlign      = 1;
   Format.nAvgBytesPerSec  = _rate * Format.nChannels;
   Format.cbSize           = 0;

   result = waveOutOpen( & hWaveOut, WAVE_MAPPER, & Format, 0, 
      0L, WAVE_FORMAT_QUERY );

   if ( result != MMSYSERR_NOERROR )
   {
      wperror( result);

      error      = SL_TRUE  ;
      stereo     = SL_FALSE ;
      bps        = _bps     ;
      rate       = _rate    ;
      init_bytes =    0     ;
      
      return;
   }

#if 0
   ulSetError ( UL_DEBUG, "Request: stereo=%d bps=%d rate=%d", 
      _stereo, _bps, _rate );
   ulSetError ( UL_DEBUG, "Result: channels=%d bps=%d rate=%d", 
      Format.nChannels, Format.wBitsPerSample, 
      Format.nSamplesPerSec );
#endif

   // Now the hwaveouthandle "should" be valid 

   if ( ( result = waveOutOpen( & hWaveOut, WAVE_MAPPER, 
         (WAVEFORMATEX *)& Format, (DWORD)waveOutProc, 
         (DWORD)this, CALLBACK_FUNCTION )) != MMSYSERR_NOERROR )
   {
      wperror( result);

      error      = SL_TRUE ;
      stereo     = SL_FALSE ;
      bps        = _bps     ;
      rate       = _rate    ;
      init_bytes =    0     ;
      return;
   }
   else
   {
      error  = SL_FALSE ;
      stereo = _stereo;
      bps    = _bps;
      rate   = _rate;

      /* hmm ?! */ 

      init_bytes = BUFFER_SIZE;

      for ( i = 0; i < BUFFER_COUNT; i++ )
      {
         Uchar *p = new Uchar[sizeof(WAVEHDR) + BUFFER_SIZE];

         wavehdr[i] = (WAVEHDR*) p;
         wavehdr[i]->lpData          = (LPSTR) p + sizeof(WAVEHDR);
         wavehdr[i]->dwBufferLength  = (DWORD) BUFFER_SIZE;
         wavehdr[i]->dwBytesRecorded = 0L;
         wavehdr[i]->dwUser          = 0;
         wavehdr[i]->dwFlags         = 0;
         wavehdr[i]->dwLoops         = 0;
         wavehdr[i]->lpNext          = 0;
         wavehdr[i]->reserved        = 0;

         result = waveOutPrepareHeader( hWaveOut, 
            wavehdr[i], sizeof(WAVEHDR) );

         if ( result != MMSYSERR_NOERROR ) 
         {
            wperror ( result );
            error = SL_TRUE;
            return;
         }
      }
   }
}


void slDSP::close ()
{
   if ( hWaveOut != NULL )
   {
      waveOutReset( hWaveOut );

      for ( int i = 0; i < BUFFER_COUNT; i++ )
      {
         waveOutUnprepareHeader( hWaveOut, wavehdr[i], sizeof(WAVEHDR) );

         delete[] (Uchar*) wavehdr[i];
      }

      waveOutClose( hWaveOut );
      hWaveOut = NULL;
   }
}

int slDSP::getDriverBufferSize ()
{
   if ( error )
      return 0 ;

   /* hmm ?! */

   return    BUFFER_SIZE;
}

void slDSP::getBufferInfo ()
{
    return ;
}


void slDSP::write ( void *buffer, size_t length )
{
   MMRESULT	result;

   if ( error || (int)length <= 0 )
      return ;

#if 0
   ulSetError ( UL_DEBUG, "written=%ld counter=%d curr_header=%d", 
      written, counter, curr_header );
#endif

   memcpy(wavehdr[curr_header]->lpData, buffer, length);
   wavehdr[curr_header]->dwBufferLength  = (DWORD) length;

   result = waveOutWrite(hWaveOut, wavehdr[curr_header], sizeof(WAVEHDR));

   if ( result != MMSYSERR_NOERROR )
   {
      wperror ( result );
      error = SL_TRUE;
   }
   
   counter ++; 
   written += (DWORD) BUFFER_SIZE;
   
   curr_header = ( curr_header + 1 ) % BUFFER_COUNT;
}


float slDSP::secondsRemaining ()
{
   if ( error )
      return 0.0f ;
   
   return 0.0f ;
}


float slDSP::secondsUsed ()
{
   DWORD    samples_used;
   MMRESULT	result;
   float    samp_time;

   if ( error )
      return 0.0f ;

   mmt.wType = TIME_BYTES;

   result = waveOutGetPosition( hWaveOut, &mmt, sizeof( mmt ));

   if ( mmt.u.cb == 0 || counter == 0)
      return    (float)0.0;

   if ( counter < BUFFER_COUNT )
      samples_used = written - mmt.u.cb ;
   else
      samples_used = BUFFER_COUNT * BUFFER_SIZE ;

   if (  stereo   ) samples_used /= 2 ;
   if ( bps == 16 ) samples_used /= 2 ;

   samp_time  = (float) samples_used / (float) rate ;

#if 0
   ulSetError ( UL_DEBUG, "%0.2f written packets=%ld stereo=%d bps=%d rate=%d", 
      samp_time, counter, stereo, bps, rate );
#endif

   return   samp_time;
}


void slDSP::sync ()
{
}

void slDSP::stop ()
{
   if ( error )
     return ;

   waveOutReset( hWaveOut );
   written = 0 ;
}

/* ------------------------------------------------------------ */
/* NetBSD/OpenBSD 2.3 this should be very close to SUN Audio    */
/* ------------------------------------------------------------ */

#elif defined(__NetBSD__) || defined(__OpenBSD__) || defined(SOLARIS)
void slDSP::open ( const char *device, int _rate, int _stereo, int _bps )
{

  counter = 0;
  
  fd = ::open ( device, O_RDWR | O_NONBLOCK ) ;
    
  if ( fd < 0 )
  {
    perror ( "slDSP: open" ) ;
    error = SL_TRUE ;
  }
  else
  {    
  
    if( ::ioctl( fd, AUDIO_GETINFO, &ainfo) == -1)
    {
      perror("slDSP: open - getinfo");
      stereo     = SL_FALSE ;
      bps        =    8     ;
      rate       =  8000    ;
      init_bytes =    0     ;
      
      return;
    }
      
#ifdef SOLARIS
	AUDIO_INITINFO(&ainfo);
#endif

    ainfo.play.sample_rate  = _rate;
    ainfo.play.precision    = _bps;    
#ifdef SOLARIS
	if ( ainfo.play.port == AUDIO_SPEAKER )
		ainfo.play.channels = 1;
	else
		ainfo.play.channels     = _stereo ? 2 : 1;

	ainfo.play.encoding = AUDIO_ENCODING_ALAW;

//	if ( _bps < 16 )
//	{
//		ainfo.play.encoding	    = AUDIO_ENCODING_LINEAR8;
//	}
//	else
//	{
//		ainfo.play.encoding	= AUDIO_ENCODING_LINEAR;
//	}
#else
    ainfo.play.encoding     = AUDIO_ENCODING_ULINEAR;
#endif

    if( :: ioctl(fd, AUDIO_SETINFO, &ainfo) == -1)
    {
      perror("slDSP: open - setinfo");
      stereo     = SL_FALSE ;
      bps        =    8     ;
      rate       =  8000    ;
      init_bytes =    0     ;
      return;
    }

    rate    = _rate;
    stereo  = _stereo;
    bps     = _bps;

    error = SL_FALSE ;

    getBufferInfo ();
    
    // I could not change the size, 
    // so let's try this ...
    
    init_bytes = 1024 * 8;
  }
}


void slDSP::close ()
{
  if ( fd >= 0 )
    ::close ( fd ) ;
}


int slDSP::getDriverBufferSize ()
{
  if ( error )
    return 0 ;

  getBufferInfo () ;
  
  // HW buffer is 0xffff on my box

#ifdef SOLARIS
  return ainfo.play.buffer_size;
#else
  return  1024 * 8;
#endif

}

void slDSP::getBufferInfo ()
{
  if ( error )
    return ;

  if( ::ioctl( fd, AUDIO_GETINFO, &ainfo) < 0)
  {
    perror ( "slDSP: getBufferInfo" ) ;
    error = SL_TRUE ;
    return ;
  }
    
#ifndef SOLARIS
  if( ::ioctl( fd, AUDIO_GETOOFFS, &audio_offset ) < 0)
  {
    perror ( "slDSP: getBufferInfo" ) ;
    error = SL_TRUE ;
    return ;
  }
#endif
}


void slDSP::write ( void *buffer, size_t length )
{
  if ( error || (int)length <= 0 )
    return ;
  
  size_t nwritten = ::write ( fd, (const char *) buffer, length ) ;

  if ( (int)nwritten < 0 )
    perror ( "slDSP: write" ) ;
  else if ( nwritten != length )
    perror ( "slDSP: short write" ) ;
      
  counter ++; /* hmmm */
}


float slDSP::secondsRemaining ()
{
    return 0.0f ;
}


float slDSP::secondsUsed ()
{
  /*
   * original formula from Steve:
   * -----------------------------
   *
   * int samples_used = init_bytes - buff_info.bytes ;
   *                    |            |
   *                    |            +--- current available
   *                    |                 space in bytes !
   *                    +---------------- available space
   *                                      when empty;
   * 
   * sample_used contains the number of bytes which are
   * "used" or in the DSP "pipeline".
   */


  int samples_used;
  
  if ( error )
    return 0.0f ;

  getBufferInfo () ;

  //This is wrong: this is the hw queue in the kernel !
  //samples_used   = ainfo.play.buffer_size - audio_offset.offset ;

  // This is: all data written minus where we are now in the queue
  
  if ( counter == 0 )
      return 0.0;

#ifndef SOLARIS
  samples_used = ( counter * init_bytes ) - audio_offset.samples;
#else
  samples_used = ( counter * init_bytes ) - ainfo.play.samples;
#endif
  
  if (  stereo   ) samples_used /= 2 ;
  if ( bps == 16 ) samples_used /= 2 ;

  return   (float) samples_used / (float) rate ;
}


void slDSP::sync ()
{ 
#ifndef SOLARIS
   if ( !error) ::ioctl ( fd, AUDIO_FLUSH , 0 ) ; 
#else
   if ( !error) ::ioctl ( fd, I_FLUSH, FLUSHRW ) ;
#endif
}

void slDSP::stop ()
{ 
   // nothing found yet 
}

/* ------------------------------------------------------------ */
/* SGI IRIX audio                                               */
/* ------------------------------------------------------------ */

#elif defined(sgi)

void slDSP::open ( const char *device, int _rate, int _stereo, int _bps )
{
  if ( _bps != 8 )
  {
    perror ( "slDSP: supports only 8bit audio for sgi" ) ;
    error = SL_TRUE;
    return;
  }

  init_bytes = 1024 * 16;

  config  = ALnewconfig();
 
  ALsetchannels (  config, _stereo ? AL_STEREO : AL_MONO );
  ALsetwidth    (  config, _bps == 8 ? AL_SAMPLE_8 : AL_SAMPLE_16 );
  ALsetqueuesize(  config, init_bytes );

  port = ALopenport( device, "w", config );
  
  if ( port == NULL )
  {
    perror ( "slDSP: open" ) ;
    error = SL_TRUE ;
  }
  else
  {
    long params[2] = {AL_OUTPUT_RATE, 0 };

    params[1] = _rate;

    if ( ALsetparams(AL_DEFAULT_DEVICE, params, 2) != 0 )
        {
       perror ( "slDSP: open - ALsetparams" ) ;
       error = SL_TRUE ;
       return;
        }
 
    rate    = _rate;
    stereo  = _stereo;
    bps     = _bps;

    error = SL_FALSE ;

  }
}


void slDSP::close ()
{
  if ( port != NULL )
  {
     ALcloseport ( port   );
     ALfreeconfig( config );
     port = NULL;
  }
}


int slDSP::getDriverBufferSize ()
{
  if ( error )
    return 0 ;

  return  ALgetqueuesize( config );
}

void slDSP::getBufferInfo ()
{
  if ( error )
    return ;
}


#define swap_half(a) ( ((a & 0xff) << 8) | ((unsigned short)(a) >> 8) )

void slDSP::write ( void *buffer, size_t length )
{
  char *buf = (char *)buffer;

  if ( error || (int)length <= 0 )
    return ;

  // Steve: is this a problem ??

  for ( int i = 0; i < (int)length; i++ ) {
    buf[i] = buf[i] >> 1;
    if (bps == 16)
       buf[i] = swap_half( buf[i] );
  }

  ALwritesamps(port, (void *)buf, length/2 );
}


float slDSP::secondsRemaining ()
{
  int   samples_remain;

  if ( error )
    return 0.0f ;

  samples_remain = ALgetfillable(port);

  if (  stereo   ) samples_remain /= 2 ;
  if ( bps == 16 ) samples_remain /= 2 ;

  return   (float) samples_remain / (float) rate ;
}


float slDSP::secondsUsed ()
{
  int   samples_used;

  if ( error )
    return 0.0f ;

  samples_used = ALgetfilled(port);

  if (  stereo   ) samples_used /= 2 ;
  if ( bps == 16 ) samples_used /= 2 ;

  return   (float) samples_used / (float) rate ;
}


void slDSP::sync ()
{ 
   if ( error )
     return ;

  /* found this in the header file - but no description
   * or example for the long parameter.
   */

  // ALflush(ALport, long);
}

void slDSP::stop ()
{ 
}


#endif


#if defined(macintosh) || defined(__APPLE__)
	

// Print out debugging info when secondsUsed is called,
// track useful information, and used extended error checking.
//#define SL_MAC_DEBUG

pascal void sndCallbackProc ( SndChannelPtr theChan, SndCommand *theCmd );
       void doError         ( OSErr theError );

int   bytesUsed;
bool  playing;

//SCStatus sndChanStatus;

#ifdef SL_MAC_DEBUG
  int   underWrites;
  int   queued;
  int   writes;
  int   callBacks;
  float roughTime;
  int   qLength;
  int   qHead;
  int   qTail;
#endif

void doError( OSErr theError ) {

  const char* msg = 0 ;
  
  switch( theError ) {
  
  case 0:
    msg = "No Error." ;
    break;
  case notEnoughHardwareErr:
    msg = "Insufficient hardware available." ;
    break;
  case badChannel:
    msg = "Channel is corrupt or unusable." ;
    break;
  case badFormat:
    msg = "Resource is corrupt or unusable." ;
    break;
  case queueFull:
    msg = "No room in the queue." ;
    break;
  case channelBusy:
    msg = "Channel is busy." ;
    break;
  case siInvalidCompression:
    msg = "Invalid compression type." ;
    break;
  case notEnoughBufferSpace:
    msg = "Insufficient memory available." ;
    break;
  case buffersTooSmall:
    msg = "Buffer is too small." ;
    break;
  case paramErr:
    msg = "Invalid parameter specified." ;
    break;
  }

  if ( msg != 0 )
    ulSetError ( UL_WARNING, "OSErr : %s", msg ) ;
  else
    ulSetError ( UL_WARNING, "OSErr : Unknown Error : %d.", theError ); 
}

pascal void sndCallbackProc ( SndChannelPtr chan, SndCommand *cmd )
{
  
  //SndChannelStatus is a BIG waste of cpu time, but I don't know
  //any other way to remedy this. I will leave it out for now.
  //SndChannelStatus(chan, sizeof(SCStatus), &sndChanStatus);
  //playing = sndChanStatus.scChannelBusy ? true : false;
  
  bytesUsed -= BUFFER_SIZE ;
    
#ifdef SL_MAC_DEBUG
  callBacks++;
  queued--;
  qLength = chan->qLength;
  qTail   = chan->qTail;
  qHead   = chan->qHead;
#endif
}

void slDSP::open ( const char *device, int _rate, int _stereo, int _bps )
{

  // Check for valid ranges on inputs
  if ( _rate > 65535 ) 
  {
    ulSetError ( UL_WARNING,
      "slDsp : Sample rate out of bounds! Setting to 44100hz.");
    _rate = 44100;
  }
      
  error  = SL_FALSE;
  stereo = _stereo;
  rate   = _rate;
  bps    = _bps; 
  osErr  = noErr;
   
  bytesPerSample   = (stereo ? 2:1) * (bps/8);
  bytesPerSecond   = bytesPerSample * rate;
  secondsPerPacket = BUFFER_SIZE / bytesPerSecond;
  secUsed = 0;
  secLeft = 0;
  playing = false;
   
  long initOptions = 0;             // Channel init options
  if ( stereo )
    initOptions += initStereo;
  else
    initOptions += initMono;

  // Extra init options available
  //initOptions += initNoInterp;     // No linear interpolation
  //initOptions += initNoDrop;       // No drop-sample conversion
  //initOptions += initChanLeft;     // Left stereo channel
  //initOptions += initChanRight;    // Right stereo channel

  // Define a call-back routine - invoked via callBackCmd
  SndCallBackUPP callBackRoutine = NewSndCallBackUPP ( sndCallbackProc );

  // Allocate a sound channel
  sndChannel = new SndChannel;
  sndChannel -> userInfo = 0;
  sndChannel -> qLength  = 128;  // Queue as many as 128 packets at a time;  
  osErr = SndNewChannel( &sndChannel, sampledSynth,
                          initOptions, callBackRoutine );

  if ( osErr != noErr ) 
  {
    SndDisposeChannel( sndChannel, true );
    ulSetError ( UL_WARNING,
      "slDSP::open() Problem creating sound channel" );
    doError (osErr);    
      error = SL_TRUE;
  }
  
  // Format sound header structure
  extSndHeader.numChannels   = stereo ? 2 : 1;
  extSndHeader.sampleRate    = rate << 16;
  extSndHeader.encode        = extSH;
  extSndHeader.sampleSize    = bps;

  // Allocate the sound buffer
  buf = new char [VIRTUAL_BUFFER_SIZE];
  if ( !buf ) 
  {
    ulSetError ( UL_WARNING,
      "slDSP::open() Not enough memory to allocate sound buffer." );
    SndDisposeChannel( sndChannel, true );
    error = SL_TRUE;
  }
  rpos = buf;
  wpos = buf;
}

void slDSP::close ()
{
  SndDisposeChannel( sndChannel, true );
  delete [] buf;
}

void slDSP::write ( void *buffer, size_t length )
{
  
  if ( error || (int)length <= 0 || length > VIRTUAL_BUFFER_SIZE  )
    return;
  
  // Make sure data will fit into available space
  if ( length >= VIRTUAL_BUFFER_SIZE - (rpos - buf) )      
    wpos = buf;
  
  // Copy sound data into buffer
  rpos = wpos;
  ptr  = (char*)buffer;
  for ( int i = 0; i < length; i++ )  
    *wpos++ = *ptr++;  
  
  // Format the sound header
  extSndHeader.samplePtr = rpos;
  extSndHeader.numFrames = length / bytesPerSample;
  
  // Format the buffer command
  currentCmd.cmd = bufferCmd;
  currentCmd.param2 = (long)&extSndHeader;
  
  // Do the sound command
  osErr = SndDoCommand (sndChannel, &currentCmd, false);  
  
#ifdef SL_MAC_DEBUG
  if ( osErr != noErr )
  {
      ulSetError ( UL_WARNING, "Error slDsp::write - bufferCmd() : " ); 
      doError( osErr );
      error = SL_TRUE;
  }
#endif
  
  // Issue a callBack command when the bufferCmd has finished
  currentCmd.cmd = callBackCmd;
  osErr = SndDoCommand (sndChannel, &currentCmd, false);
  
#ifdef SL_MAC_DEBUG
  if ( osErr != noErr )
  {
      ulSetError ( UL_WARNING, "Error slDsp::write - callBackCmd() : " ); 
      doError( osErr );
      error = SL_TRUE;
  }
#endif
    
  // Add on time for this packet
  secUsed += length / bytesPerSecond;
  
  // Add on bytes used
  bytesUsed += BUFFER_SIZE;
  
  // Reset the timing for the buffer
  if (!playing)
  {
    Microseconds(&lastTime);
    playing = true;
  }

#ifdef SL_MAC_DEBUG
  queued++;
  writes++;
#endif
  
}

void slDSP::sync ()
{
  if ( error ) return;
  
  // flushCmd will remove all queued commands in a channel,
  // and the command in progress is not affected.
  // This is not exactly what sync() is supposed to do!
  currentCmd.cmd    = flushCmd;  
  osErr = SndDoImmediate ( sndChannel, &currentCmd );
  
#ifdef SL_MAC_DEBUG
  if ( osErr != noErr )
  {
    ulSetError ( UL_WARNING, "Error slDsp::sync() : " ); 
    doError( osErr );  
    error = SL_TRUE;
  }
#endif
}

void slDSP::stop ()
{
  if ( error ) return;
  
  currentCmd.cmd = quietCmd;
  osErr = SndDoImmediate ( sndChannel, &currentCmd );
  playing = false;

#ifdef SL_MAC_DEBUG
  if ( osErr != noErr )
  {
    ulSetError ( UL_WARNING, "Error slDsp::stop() : " ); 
    doError( osErr );
    error = SL_TRUE;  
  }
#endif
}

void slDSP::getBufferInfo ()
{
  return;
}

int slDSP::getDriverBufferSize ()
{  
  return BUFFER_SIZE;
}

float slDSP::secondsRemaining ()
{
  if ( error )
    return 0.0f;
  
  // sl doesn't use this, so I didn't write it!
  return secLeft;
}

float slDSP::secondsUsed ()
{
  if ( error || secUsed <= 0 )
    return secUsed = 0;
  
  Microseconds( &currTime );        
  secUsed  -= (currTime.lo - lastTime.lo) / 1000000.0;
  Microseconds( &lastTime );
  
  // This fixes inaccuracy with Microseconds
  if ( secUsed > bytesUsed / bytesPerSecond + secondsPerPacket )
    secUsed -= secondsPerPacket;
  
#ifdef SL_MAC_DEBUG  
  if (queued == 0)
    underWrites++;      
  roughTime = bytesUsed / bytesPerSecond;
  ulSetError ( UL_DEBUG, "%d\t%d\t%f\t%d\t%d\t%f\t%d\t%d\t%d\t%d", 
    bytesUsed, queued, secUsed, callBacks, 
    writes, roughTime, underWrites, qLength, qHead, qTail );
#endif
  
  return secUsed;
}

#endif


