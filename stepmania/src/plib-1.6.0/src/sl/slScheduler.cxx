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

const char *__slPendingError = NULL ;

slScheduler *slScheduler::current = NULL ;

void slScheduler::init ()
{
  int mi;
  mixer = NULL ;

  mixer_buffer  = NULL ;
  /* Note there is a required null element on the end of array */
  for ( mi=0; mi <= SL_MAX_MIXERINPUTS; mi++ )
	mixer_inputs[mi] = NULL ;

  current = this ;
  mixer_gain = 0x100 ; /* When we have to make sound quieter */

  if ( notWorking () )
  {
    ulSetError ( UL_WARNING, "slScheduler: soundcard init failed." ) ;
    setError () ;
    return ;
  }
  
  if ( getBps() != 8 )
  {
    ulSetError ( UL_WARNING, "slScheduler: Needs a sound card that supports 8 bits per sample." ) ;
    setError () ;
    return ;
  }
  
  if ( getStereo() )
  {
    ulSetError ( UL_WARNING, "slScheduler: Needs a sound card that supports monophonic replay." ) ;
    setError () ;
    return ;
  }
  
  music = NULL ;
  
  for ( int i = 0 ; i < SL_MAX_SAMPLES ; i++ )
    player [ i ] = NULL ;
  
  amount_left = 0 ;
  now = 0 ;
  num_pending_callbacks = 0 ;
  safety_margin = 1.0 ;
  
  initBuffers () ;
}

void slScheduler::initBuffers ()
{
  if ( notWorking () )
    return ;

  delete [] mixer_buffer ;
  setMaxConcurrent ( 0 );

  mixer_buffer_size = getDriverBufferSize () ;

  mixer_buffer = new Uchar [ mixer_buffer_size ] ;
  memset ( mixer_buffer, 0x80, mixer_buffer_size ) ;
  setMaxConcurrent ( 3 );
}

void slScheduler::setMaxConcurrent ( int mc )
{
  int mi;
  for ( mi = 0; mi < SL_MAX_MIXERINPUTS; mi ++ )
    if ( mi < mc )
    { if ( mixer_inputs[mi] == NULL )
           mixer_inputs[mi] = new Uchar [ mixer_buffer_size ] ;
    } else
    { if ( mixer_inputs[mi] != NULL )
           delete [] mixer_inputs[mi] ;
           mixer_inputs[mi] = NULL;
    }
}

int slScheduler::getMaxConcurrent ( void ) const
{
  int mi, mc = 0;
  for ( mi = 0; mi < SL_MAX_MIXERINPUTS; mi ++ )
    if ( NULL != mixer_inputs[mi] )
      mc ++;
  return mc;
}

slScheduler::~slScheduler ()
{
  delete [] mixer_buffer ;
  setMaxConcurrent(0);
  if ( current == this )
    current = NULL ;
}

void slScheduler::realUpdate ( int dump_first )
{
  if ( notWorking () ) return ;
  
  if ( __slPendingError != NULL )
  {
    ulSetError ( UL_FATAL, "%s", __slPendingError ) ;
  }
  
  int i ;

  while ( secondsUsed() <= safety_margin )
  {
    slPlayer *psp [ SL_MAX_MIXERINPUTS ] ;
    int       pri [ SL_MAX_MIXERINPUTS ] ;
    int inputsused = 0 ;
    int lowest = 0 ; /* makes it safe to reference before assignment below */

    /* Decide who we're going to use */

    for ( i = 0 ; i < SL_MAX_SAMPLES ; i++ )
    {
      /* Ignore non-existent channels */

      if ( player [ i ] == NULL )
	continue ;

      /* Clean up dead sample players */

      if ( player [ i ] -> isDone () )
      {
        if ( player [ i ] == music )
          music = NULL ;

	delete player [ i ] ;
	player [ i ] = NULL ;
	continue ;
      }

      /* Skip paused players, but leave them alone */

      if ( player [ i ] -> isPaused () )
	continue ;

      /* This one is viable, make sure we have room */

      int mypri = player[i]->getPriority() ;

      if ( ( inputsused > 0 ) &&
           ( mixer_inputs[inputsused] == NULL ) &&
           ( mypri > pri[lowest] )
         )
      { /* Not enough room; get rid of someone */
        player[lowest] -> preempt ( mixer_buffer_size ) ;
	psp[lowest] = player[i] ;
	pri[lowest] = mypri ;
        int j;
        for ( j = 0; j < inputsused; j++ )
          if ( pri[lowest] < pri[j] )
            lowest = j;
      }
      else if ( mixer_inputs[inputsused] != NULL )
      { /* Ok, we've got room, presumably, for another item */
        psp[inputsused] = player[i];
        pri[inputsused] = mypri;
	if ( ( ( inputsused ++ ) == 0 ) ||
             ( pri[lowest] > mypri ) )
          lowest = inputsused;
      }
      else
      { /* Oops, something very wrong.  Shouldn't get here in normal use.
           One obvious reason is if user specified zero channels to mix! */
        player[i] -> preempt ( mixer_buffer_size ) ;
      }
    }

    /* We've now chosen our sources.  Previously we'd go back through 
       and figure out who to report as pre-empted.  But we already did 
       that as part of getting our list of sources, so that disappeared. */

    /* Now, we actually get the sound samples into the mixer_buffer.
       All the versions here are fastpaths except the two last ones.
       Just read the comments on the last one to understand it. */

 
    /* We put a minus sign on the number of inputs if we're attenuating */
    switch ( mixer_gain != 0x100 ? -inputsused : inputsused )
    {

    case 0:
      /* No sound whatsoever, the attenuation is irrelevant */
      memset ( mixer_buffer, 0x80, mixer_buffer_size ) ;
      amount_left = 0 ;
      if ( mixer_gain < 0x100 ) mixer_gain ++;
      break ;

    case 1:
      /* One sample, so overflow is impossible.  No legacy attenuation */
      psp[0] -> read ( mixer_buffer_size, mixer_buffer ) ;
      break ;

    case 2:
      { register int    l = mixer_buffer_size ;
        register Uchar *d = mixer_buffer, *a, *b ;
        register int t, x = l / 100 ;
        psp[0] -> read ( l, a = mixer_inputs [ 0 ] ) ;
        psp[1] -> read ( l, b = mixer_inputs [ 1 ] ) ;
        while ( l-- )
          { t = (*a++) - 0x80 + (*b++) ;
            *d++ = ( t >= 0x100 ? x--, 0xff :
                   ( t <= 0x000 ? x--, 0x00 : t ) );
          }
        if ( x < 0 ) mixer_gain -- ;
      } break;

    case 3:
      { register int    l = mixer_buffer_size ;
        register Uchar *d = mixer_buffer, *a, *b, *c ;
        register int t, x = l / 100 ;
        psp[0] -> read ( l, a = mixer_inputs [ 0 ] ) ;
        psp[1] -> read ( l, b = mixer_inputs [ 1 ] ) ;
        psp[2] -> read ( l, c = mixer_inputs [ 2 ] ) ;
        while ( l-- ) *d++ = ( t = (*a++) - 0x100 
                                 + (*b++) 
                                 + (*c++) ),
                       t >= 0x100 ? x--, 0xff :
                       t <= 0x000 ? x--, 0x00 : t;
        if ( x < 0 ) mixer_gain -- ;
      } break;

    default:
      { /* How much data to get, and where to put it */
        int l = mixer_buffer_size ;
        register Uchar *d = mixer_buffer ;
        /* We compute one value in here, in the loop below */
        register int t ;
        /* We need to determine the maximum range of values */
	int tmax = 0x80, tmin = 0x80;
        /* A pointer to the element about to be added to "t" */
        register Uchar **p ;
        /* Sound cards use offset binary, so do zero adjustment */
        int z = ( (int) 0x8000 / mixer_gain ) - ( inputsused * 0x80 ) ;
        /* An array of pointers to keep in step with "d" above */
        register Uchar * s [ SL_MAX_MIXERINPUTS + 1 ] ;
        /* First thing is to read individual channels into buffers */
        for ( i = 0; i < inputsused; i++ )
          psp[i] -> read ( l, s [ i ] = mixer_inputs [ i ] );
        /* Put a null on the end of "s" so we don't have to count */
        s [ inputsused ] = NULL;
        /* This while loop is executed for each audio sample */
        while ( l-- )
        { /* Accumulation to zero, and point to first buffer item */
          t = z;
          p = s;
          /* Iterate through all buffers to do accumulation */
          while ( /* Add this buffer element, advance within buffer */
                  t += *((*p)++),
                  /* Also advance to the next channel buffer */
                  *(++p)
                  /* Make the while loop quit when we find NULL */
                ) { };
          /* Apply gain correction if needed */
          if ( mixer_gain != 0x100 )
             t = ( t * mixer_gain ) / 0x100 ;
          /* Store accumulation, but force onscale */
          *(d++) = t >= 0x100 ? 0xff :
                   t <= 0x000 ? 0x00 : t ;
          /* Figure out the maximum signal range */
          if ( tmax < t ) tmax = t ; 
          else
          if ( tmin > t ) tmin = t ;
        }
        /* Consider changing the attenuation */
        if ( ( ( tmin < 0x00 ) || ( tmax > 0xFF ) )
          && ( mixer_gain > 1 ) )
           mixer_gain -- ;
        else
        if ( ( tmin * ( mixer_gain + 1 ) > 0x80 + 0x10 * mixer_gain )
          && ( tmax * ( mixer_gain + 1 ) < 0x80 + 0xE0 * mixer_gain )
          && ( mixer_gain < 0x100 ) )
           mixer_gain ++ ;
        // printf ( "Gain=%03x, Range=%04x,%04x\n", mixer_gain, tmin, tmax );
      } break;
    }

    /* We've created the next buffer of sound data ... now what ? */

    if ( dump_first )
    {
      stop () ;
      dump_first = SL_FALSE ;
    }
    
    play ( mixer_buffer, mixer_buffer_size ) ;
    
    now += mixer_buffer_size ;
  }
  
  flushCallBacks () ;
}

void slScheduler::addCallBack ( slCallBack c, slSample *s, slEvent e, int m )
{
  if ( notWorking () ) return ;
  
  if ( num_pending_callbacks >= SL_MAX_CALLBACKS )
  {
    ulSetError ( UL_WARNING, "slScheduler: Too many pending callback events!" ) ;
    return ;
  }
  
  slPendingCallBack *p = & ( pending_callback [ num_pending_callbacks++ ] ) ;
  
  p -> callback = c ;
  p -> sample   = s ;
  p -> event    = e ;
  p -> magic    = m ;
}

void slScheduler::flushCallBacks ()
{
  if ( notWorking () ) return ;
  
  /*
  Execute all the callbacks that we accumulated
  in this iteration.
  
    This is done at the end of 'update' to reduce the risk
    of nasty side-effects caused by 'unusual' activities
    in the application's callback function.
  */
  
  while ( num_pending_callbacks > 0 )
  {
    slPendingCallBack *p = & ( pending_callback [ --num_pending_callbacks ] ) ;
    
    if ( p -> callback )
      (*(p->callback))( p->sample, p->event, p->magic ) ;
  }
}

void slScheduler::addSampleEnvelope ( slSample *s, int magic,
                                     int slot, slEnvelope *e,
                                     slEnvelopeType t)
{
  if ( notWorking () ) return ;
  
  for ( int i = 0 ; i < SL_MAX_SAMPLES ; i++ )
    if ( player [ i ] != NULL && player[i] != music &&
      ( s  == NULL || player [ i ] -> getSample () ==   s   ) &&
      ( magic == 0 || player [ i ] -> getMagic  () == magic ) )
      player [ i ] -> addEnvelope ( slot, e, t ) ;
}

void slScheduler::resumeSample ( slSample *s, int magic)
{
  if ( notWorking () ) return ;
  
  for ( int i = 0 ; i < SL_MAX_SAMPLES ; i++ )
    if ( player [ i ] != NULL && player[i] != music &&
      ( s  == NULL || player [ i ] -> getSample () ==   s   ) &&
      ( magic == 0 || player [ i ] -> getMagic  () == magic ) )
      player [ i ] -> resume () ;
}

void slScheduler::pauseSample ( slSample *s, int magic)
{
  if ( notWorking () ) return ;
  
  for ( int i = 0 ; i < SL_MAX_SAMPLES ; i++ )
    if ( player [ i ] != NULL && player[i] != music &&
      ( s  == NULL || player [ i ] -> getSample () ==   s   ) &&
      ( magic == 0 || player [ i ] -> getMagic  () == magic ) )
      player [ i ] -> pause () ;
}

void slScheduler::stopSample ( slSample *s, int magic )
{
  if ( notWorking () ) return ;
  
  for ( int i = 0 ; i < SL_MAX_SAMPLES ; i++ )
    if ( player [ i ] != NULL && player[i] != music &&
      ( s  == NULL || player [ i ] -> getSample () ==   s   ) &&
      ( magic == 0 || player [ i ] -> getMagic  () == magic ) )
      player [ i ] -> stop () ;
}

int slScheduler::loopSample ( slSample *s, int pri,
                             slPreemptMode mode,
                             int magic, slCallBack cb )
{
  if ( notWorking () ) return -1 ;
  
  for ( int i = 0 ; i < SL_MAX_SAMPLES ; i++ )
    if ( player [ i ] == NULL )
    {
      player [ i ] = new slSamplePlayer ( s, SL_SAMPLE_LOOP, pri, mode, magic, cb ) ;
      return i ;
    }
    
  return -1 ;
}

int slScheduler::playSample ( slSample *s, int pri,
                             slPreemptMode mode,
                             int magic, slCallBack cb)
{
  if ( notWorking () ) return SL_FALSE ;
  
  for ( int i = 0 ; i < SL_MAX_SAMPLES ; i++ )
    if ( player [ i ] == NULL )
    {
      player [ i ] = new slSamplePlayer ( s, SL_SAMPLE_ONE_SHOT, pri, mode, magic, cb ) ;
      return SL_TRUE ;
    }
    
  return SL_FALSE ;
}

void slScheduler::addMusicEnvelope ( int magic,
                                    int slot, slEnvelope *e,
                                    slEnvelopeType t)
{
  if ( notWorking () ) return ;
  
  if ( music != NULL &&
    ( magic == 0 || music -> getMagic  () == magic ) )
    music -> addEnvelope ( slot, e, t ) ;
}

void slScheduler::resumeMusic ( int magic)
{
  if ( notWorking () ) return ;
  
  if ( music != NULL &&
    ( magic == 0 || music -> getMagic  () == magic ) )
    music -> resume () ;
}

void slScheduler::pauseMusic ( int magic)
{
  if ( notWorking () ) return ;
  
  if ( music != NULL &&
    ( magic == 0 || music -> getMagic  () == magic ) )
    music -> pause () ;
}

void slScheduler::stopMusic ( int magic )
{
  if ( notWorking () ) return ;
  
  if ( music != NULL &&
    ( magic == 0 || music -> getMagic  () == magic ) )
  {
    music -> stop () ;
    
    for ( int i = 0 ; i < SL_MAX_SAMPLES ; i++ )
      if ( player [ i ] == music )
        player [ i ] = NULL ;
      
    delete music ;
    music = NULL ;
  }
}

int slScheduler::loopMusic ( const char *fname, int pri,
                            slPreemptMode mode,
                            int magic, slCallBack cb )
{
  if ( notWorking () ) return -1 ;
  
  if ( music != NULL )
  {
    ulSetError ( UL_WARNING, "slScheduler: Can't play two music tracks at once." ) ;
    return -1 ;
  }      
  
  for ( int i = 0 ; i < SL_MAX_SAMPLES ; i++ )
    if ( player [ i ] == NULL )
    {
      music = new slMODPlayer ( fname, SL_SAMPLE_LOOP,
        pri, mode, magic, cb ) ;
      player [ i ] = music ;
      return i ;
    }
    
  return -1 ;
}

int slScheduler::playMusic ( const char *fname, int pri,
                            slPreemptMode mode,
                            int magic, slCallBack cb )
{
  if ( notWorking () ) return SL_FALSE ;
  
  if ( music != NULL )
  {
    ulSetError ( UL_WARNING, "slScheduler: Can't play two music tracks at once." ) ;
    return SL_FALSE ;
  }      
  
  for ( int i = 0 ; i < SL_MAX_SAMPLES ; i++ )
    if ( player [ i ] == NULL )
    {
      music = new slMODPlayer ( fname, SL_SAMPLE_ONE_SHOT,
        pri, mode, magic, cb ) ;
      player [ i ] = music ;
      
      return SL_TRUE ;
    }
    
  return SL_FALSE ;
}


