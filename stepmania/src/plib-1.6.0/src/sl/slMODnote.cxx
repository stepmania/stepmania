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


#include "slMODPrivate.h"

static SampleInfo *smp ;
static unsigned char chToPlay[32];

static int masterVol = 0x30 * 4/3;
static int globalVol = 0x40;
static int mono;
static int outRate  = DEF_OUTRATE ;
static int tempo    = DEF_TEMPO   ;	/* BPM = rows/minute/4 */
static int speed    = DEF_SPEED   ;	/* frames/row */
static int frameLen = DEF_OUTRATE * 60 / ( DEF_TEMPO * 24 ) ;
static int patRepeat = 0 ;



static void commonWork ( Note *np )
{
  _MOD_instClearPFW();

  if (np->ins)
    _MOD_instSample(&smp[np->ins - 1], 0);

  if (np->note != 255)
    if (np->note == 254)
      _MOD_instNoteOff(0);
    else
      _MOD_instNote((np->note/16) * 12 + np->note%16, 0);

  if (np->vol != 255)
    _MOD_instVol(np->vol, 0);
}


static void noEffect ( Note *np )
{
  if ( np->cmd == 255 )
    _MOD_instEmptyCmd () ;

  commonWork ( np ) ;
}


static void unknownEffect ( Note *np )
{
  ulSetError ( UL_WARNING, "Unknown effect: %c%02X", np->cmd + '@', np->info ) ;
  commonWork ( np ) ;
}

#define X (np->info/16)
#define Y (np->info%16)

/* Dxy, Kxy, Lxy common work */

static void dklCommonWork ( Note *np )
{
  if ( np->info )
  {
    if ( Y == 0 ) /* Dx0=up */
      _MOD_instSetVolSlideParams (  X, 1, 1, 1, 0 ) ;
    else
    if ( X > 0 && Y == 0xf ) /* DxF=fine up */
      _MOD_instSetVolSlideParams (  X, 1, 1, 1, 1 ) ;
    else
    if ( X == 0xf ) /* DFy=fine down */
      _MOD_instSetVolSlideParams ( -Y, 1, 1, 1, 1 ) ;
    else          /* D0y=down, but also D46 or something is here */
      _MOD_instSetVolSlideParams ( -Y, 1, 1, 1, 0 ) ;
  }

  _MOD_instVolSlide () ;
}

/* Dxy = volume slide */

static void dCmd ( Note *np )
{
  commonWork ( np ) ;
  dklCommonWork ( np ) ;
}

/* Exx, Fxx common work */

static void efCommonWork ( Note *np )
{
  if (np->info)
  {
    switch (X)
    {
      case 0xf: /* [EF]Fx = fine */
	_MOD_instSetPeriodSlideParams((np->info%16)*4, 1);
        break;

      case 0xe: /* [EF]Ex = extra fine */
	/* ST3 plays [EF]E0 and [EF]F0 differently */
	/* so it may be incompatible.. */
	_MOD_instSetPeriodSlideParams(np->info%16, 1);
        break;

      default:
	_MOD_instSetPeriodSlideParams(np->info*4, 0);
        break;
    }
  }
}

/* Exx = slide down */

static void eCmd(Note *np)
{
  commonWork(np);
  efCommonWork(np);
  _MOD_instPeriodSlideDown();
}

/* Fxx = slide up */

static void fCmd(Note *np)
{
  commonWork(np);
  efCommonWork(np);
  _MOD_instPeriodSlideUp();
}

/* Gxx = portamento */

static void gCmd(Note *np)
{
  _MOD_instClearPFW();

  if (np->ins) _MOD_instSetPortamentoDefaultVol(); /* only set default volume */
  if (np->vol != 255) _MOD_instVol(np->vol, 0);
  if (np->note < 254 ) _MOD_instSetPortamentoTo((np->note/16)*12 + np->note%16);
  if (np->info) _MOD_instSetPortamentoSpeed(np->info*4);

  _MOD_instPortamento();
}

/* Hxy = vibrato */

static void hCmd(Note *np)
{
  commonWork(np);
  if (np->info) _MOD_instSetVibratoParams(X, Y*8);
  _MOD_instVibrato();
}

/* Ixy = tremor */

static void iCmd(Note *np)
{
  commonWork(np);
  if (np->info) _MOD_instSetTremorParams(X+1, Y+1);
  _MOD_instTremor();
}

/* Jxy = arpeggio */

static void jCmd(Note *np)
{
  commonWork(np);
  if (np->info) _MOD_instSetArpeggioParams(X, Y);
  _MOD_instArpeggio();
}

/* Kxy = H00 and Dxy */

static void kCmd(Note *np)
{
  commonWork(np);
  _MOD_instVibrato(); /* H00 */
  dklCommonWork(np);
}

/* Lxy = G00 and Dxy */

static void lCmd(Note *np)
{
  commonWork(np);
  _MOD_instPortamento(); /* G00 */
  dklCommonWork(np);
}

/* Oxx = sample offset */

static void oCmd(Note *np)
{
  commonWork(np);
  _MOD_instSampleOffset(np->info * 0x100);
}

/* Qxy = retrig + volumeslide */

static void qCmd(Note *np)
{
  static int add[16]={0,-1,-2,-4,-8,-16,0,0,0,1,2,4,8,16,0,0};
  static int mul[16]={1,1,1,1,1,1,2,1,1,1,1,1,1,1,3,2};
  static int div[16]={1,1,1,1,1,1,3,2,1,1,1,1,1,1,2,1};

  commonWork(np);

  if (np->info)
  {
    _MOD_instSetVolSlideParams(add[X], mul[X], div[X], Y, 0);
    _MOD_instSetRetrigParam(Y);
  }

  _MOD_instVolSlide();
  _MOD_instRetrig();
}

/* Rxy = tremolo */

static void rCmd(Note *np)
{
  commonWork(np);
  if (np->info) _MOD_instSetTremoloParams(X, Y*2);
  _MOD_instTremolo();
}

/* Uxy = fine vibrato */

static void uCmd(Note *np)
{
  commonWork(np);
  if (np->info) _MOD_instSetVibratoParams(X, Y*2);
  _MOD_instVibrato();
}

/* Sxy = misc */

static void sCmd(Note *np)
{
  if (X == 0xd) /* notedelay */
  {
    _MOD_instClearPFW();

    if (np->ins)
      _MOD_instSample(&smp[np->ins - 1], Y);

    if (np->note != 255)
      if (np->note == 254)
        _MOD_instNoteOff(Y);
      else
        _MOD_instNote((np->note/16) * 12 + np->note%16, Y);

    if (np->vol != 255)
      _MOD_instVol(np->vol, Y);
  }
  else
  {
    commonWork ( np ) ;

    switch ( X )
    {
    case 1: /* set glissando control */
      _MOD_instSetPortamentoGlissando(Y);
      break;
    case 2: /* set finetune */
      /* ...but not tested yet. which tune use this? */
      ulSetError ( UL_DEBUG, "Got it! Set Finetune");
      { 
        static int freq[16] =
        {
          8363,8413,8463,8529,8581,8651,8723,8757,
            7895,7941,7985,8046,8107,8169,8232,8280
        };
        _MOD_instTuning(freq[Y]);
        /* the tuning effects from next key-on */
      }
      break;
    case 3: /* set vibrato waveform */
      _MOD_instSetVibratoWave(Y%4, Y/4);
      break;
    case 4: /* set tremolo waveform */
      _MOD_instSetTremoloWave(Y%4, Y/4);
      break;
    case 8: /* set pan position */
      _MOD_instPanPosition(Y*64/15);
      break;
    case 0xc: /* notecut */
      _MOD_instNoteCut(Y);
      break;
    case 0xb: /* pattern loop */
    case 0xe: /* pattern delay */
      break;
    default:
      ulSetError ( UL_WARNING, "%c%02X not supported.", np->cmd+'@', np->info);
    }
  }
}

/* Vxx = set global volume */


static void setGlobalVol(void)
{
  dacioGlobalVol ( masterVol * globalVol ) ;
}


void _MOD_playNoteSetMono(int m)
{
  mono = m;
  _MOD_instMono(m);
}

void _MOD_playNoteSetMasterVol(int mv)
{
  masterVol = mono? mv : mv * 4 / 3;
  setGlobalVol();
}

void _MOD_playNoteSetGlobalVol(int gv)
{
  globalVol = gv;
  setGlobalVol();
}

static void vCmd(Note *np)
{
  commonWork(np);
  _MOD_playNoteSetGlobalVol(np->info);
}

/* Xxx = DMP style pan position */

static void xCmd(Note *np)
{
  commonWork(np);

  if (np->info <= 0x80)
    _MOD_instPanPosition(np->info * 64 / 0x80);
  else
  if (np->info == 0xa4)
    _MOD_instPanPosition(-1); /* surround */
  else
    _MOD_instPanPosition(32); /* unknown -> center */
}


static void (*cmdTbl[])( Note *np ) =
{
 /*@*/ unknownEffect, /*A*/ noEffect     , /*B*/ noEffect, /*C*/noEffect,
 /*D*/ dCmd         , /*E*/ eCmd         , /*F*/ fCmd    , /*G*/gCmd,
 /*H*/ hCmd         , /*I*/ iCmd         , /*J*/ jCmd    , /*K*/kCmd,
 /*L*/ lCmd         , /*M*/ unknownEffect, /*N*/ unknownEffect, /*O*/oCmd,
 /*P*/ unknownEffect, /*Q*/ qCmd         , /*R*/ rCmd    , /*S*/sCmd,
 /*T*/ noEffect     , /*U*/ uCmd         , /*V*/ vCmd    , /*W*/unknownEffect,
 /*X*/ xCmd         , /*Y*/ unknownEffect, /*Z*/ unknownEffect
} ;


void _MOD_playNoteSetSample ( SampleInfo *sip )
{
  smp = sip ;
}


void _MOD_playNoteInit ( void )
{
  _MOD_instInit () ;
}

static void setFrameLen ( void )
{
  frameLen = outRate * 60 / (tempo * 24) ;
  _MOD_instHirevSetFrameLen ( frameLen ) ;
}


void _MOD_playNoteSetOutRate ( int _or )
{
  if ( _or > MAX_OUTRATE )
  {
    ulSetError ( UL_FATAL, "Too high output sample rate." ) ;
  }

  _MOD_instOutRate ( _or ) ;
  outRate = _or ;
  setFrameLen () ;
}


void _MOD_playNoteSetTempo ( int n )
{
  if ( n < MIN_TEMPO )
  {
    ulSetError ( UL_WARNING, "Illegal tempo (%d) ignored.", n ) ;
    return ;
  }

  tempo = n ;
  setFrameLen () ;
}


void _MOD_playNoteSetSpeed ( int n )
{
  speed = n ;
}


void _MOD_playNoteSetNote ( int ch, Note *np )
{
  chToPlay [ ch ] = 1 ;
  _MOD_instSelectCh ( ch ) ;

  if ( np->cmd == 255 )
    noEffect ( np ) ;
  else
    (*cmdTbl[np->cmd]) ( np ) ;
}


void _MOD_playNoteSetPatRepeat ( int n )
{
  patRepeat = n ;
}

void _MOD_playNote ( void )
{
  for ( int r = 0 ; r <= patRepeat ; r++ )
    for ( int f = 0 ; f < speed ; f++ )
    {
      _MOD_instHirevEraseBuf () ;

      for ( int ch = 0 ; ch < 32 ; ch++ )
	if ( chToPlay [ ch ] )
        {
	  _MOD_instSelectCh ( ch ) ;
	  _MOD_instDoPerFrameWorks ( f ) ;
	  _MOD_instLoop () ;
	}

      _MOD_instHirevFlushBuf () ;
    }

  patRepeat = 0 ;

  for ( int ch = 0 ; ch < 32 ; ch++ )
    chToPlay [ ch ] = 0 ;
}


