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

/* note-period table */

#define NOTE_MAX (12*8-1) /* 8octave */
static short *note ;

#define shift(x,n) ((n) >= 0? (x) << (n) : (x) >> -(n))    /* x * 2**n */

static struct
{
  int *p0 ;
  int *p  ;
  int len ;
} hirev_buf ;

void _MOD_instHirevInit ( void ) 
{
  hirev_buf.p0 = dacioGetBuffer () ;
}

void _MOD_instHirevSetFrameLen ( int l ) 
{
  dacioIncomingBufLen ( l ) ;
  hirev_buf.len = l ;
}

void _MOD_instHirevEraseBuf ( void ) 
{
  memset ( hirev_buf.p0, 0, sizeof ( int )  * hirev_buf.len * 2 ) ;
}

void _MOD_instHirevFlushBuf ( void ) 
{
  dacioOut () ;
}


static void fadeout ( InstHirevInfo *ihip ) 
{
  int lastL = ihip->lastL / 64 ;
  int lastR = ihip->lastR / 64 ;
  int f = ihip->fadeout ;

  if ( f > 63 ) 
    f = 63 ;

  if ( lastL || lastR ) 
  {
    int *bufp = hirev_buf.p ;
    int n = ( hirev_buf.p0 + hirev_buf.len - bufp )  / 2 ;

    if ( n > f )  n = f ;

    for ( ; n > 0 ; n--,f-- ) 
    {
      *bufp++ += lastL * f ;
      *bufp++ += lastR * f ;
    }
  }

  ihip->fadeout = f ;

  if ( !f ) 
    ihip->lastL = ihip->lastR = 0 ;
}

/* vol zero optimization: just calculates phase progress within this frame */

static void vol0Opt ( InstHirevInfo *ihip ) 
{
  if ( ihip->lastL || ihip->lastR ) 
  {
    /* suddenly volume is turned to 0 -> click ( chi_mai.s3m )  */

    ihip->fadeout = 256 ;
    fadeout ( ihip ) ;
    ihip->fadeout = 0 ; /* don't note off ( ambient_power.mod )  */
  }

  ihip->wAcc = ( unsigned int ) ( unsigned short ) ihip->wAcc + ( unsigned short )( ihip->w * hirev_buf.len ) ;
  ihip->ptr += ( ihip->w >> 16 )  * hirev_buf.len
         + ( ( unsigned short ) ihip->w * hirev_buf.len >> 16 ) 
         + ( ihip->wAcc >> 16 ) ;

  if ( ihip->ptr >= ihip->end ) 
  {
    if ( ihip->loopBeg ) 
      ihip->ptr = ihip->loopBeg + ( ihip->ptr - ihip->end )  % ( ihip->end - ihip->loopBeg ) ;
    else
      ihip->ptr = NULL ; /* note off */
  }

  ihip->lastL = ihip->lastR = 0 ;
}

static InstHirevInfo ihi ; /* cacheing */

static void hirevLoop0 ( unsigned int n ) 
{
  int *bufp = hirev_buf.p ;
  unsigned char *ihiPtr = ihi.ptr ;
  unsigned int ihiWAcc = ihi.wAcc ;

  for ( ; n > 0 ; n-- ) 
  {
    int d = ( signed char ) *ihiPtr ;
    *bufp++ += d * ihi.volL ;
    *bufp++ += d * ihi.volR ;

    ihiWAcc = ( unsigned short ) ihiWAcc + ihi.w ; /*ihiWAcc & 0xffff + w*/
    ihiPtr += ihiWAcc >> 16 ;
  }

  hirev_buf.p    = bufp   ;
  ihi.ptr  = ihiPtr ;
  ihi.wAcc = ihiWAcc ;
}

static void hirevLoop80 ( unsigned int n ) 
{
  int *bufp = hirev_buf.p ;
  unsigned char *ihiPtr = ihi.ptr ;
  unsigned int ihiWAcc = ihi.wAcc ;

  for ( ; n > 0 ; n-- ) 
  {
    int d = ( signed char ) ( *ihiPtr ^ 0x80 ) ;
    *bufp++ += d * ihi.volL ;
    *bufp++ += d * ihi.volR ;

    ihiWAcc = ( unsigned short ) ihiWAcc + ihi.w ; /*ihiWAcc & 0xffff + w*/
    ihiPtr += ihiWAcc >> 16 ;
  }

  hirev_buf.p    = bufp   ;
  ihi.ptr  = ihiPtr ;
  ihi.wAcc = ihiWAcc ;
}

#define hirevLoop(x)  { if ( ihi.x_or )  hirevLoop80(x) ; else hirevLoop0(x) ; }

  
void _MOD_instHirevLoop ( InstHirevInfo *ihip ) 
{
  unsigned int restF ;
  unsigned int restS ;
  int lastD ;

  if (  ihip->ptr == NULL ) 
    return ; /* note is off */

  hirev_buf.p = hirev_buf.p0 ;

  if ( ihip->fadeout ) 
  {
    fadeout ( ihip ) ;

    if ( !ihip->fadeout ) 
      ihip->ptr = NULL ; /* note off */

    return ;
  }

  if ( !ihip->volL && !ihip->volR )  { vol0Opt ( ihip ) ; return ; }

  ihi = *ihip ; /* load to cache */
  restF = hirev_buf.len ;

  do
  {
    int l8, l0 ;

    ihi.wAcc = ( unsigned short ) ihi.wAcc ;

    l8 = ( ihi.end - ihi.ptr )  << 8 ;

    if ( !l8 && !ihi.wAcc ) 
    { /* happens only on empty samples */

      ihip->fadeout = 256 ;
      fadeout ( ihip ) ;

      if ( !ihip->fadeout ) 
        ihip->ptr = NULL ; /* note off */

      return ;
    }

    if ( l8 <= 0 ) 
    {
      ulSetError ( UL_WARNING, "bug: restF=%u",restF ) ;
      ulSetError ( UL_WARNING, "end-ptr=%d w=%u",ihi.end-ihi.ptr,ihi.w ) ;
      ulSetError ( UL_WARNING, "wAcc = %u",ihi.wAcc ) ;
    }

    l0 = ihi.w - 1 - ihi.wAcc ;
    l8 += l0 >> 8 ;
    l0 &= 0xff ;
    restS = ( ( l8 / ihi.w )  << 8 )  + ( ( ( l8 % ihi.w )  << 8 )  + l0 )  / ihi.w ;

    if ( restF < restS ) 
    { /* sample is longer than frame */

      hirevLoop ( restF ) ;
      lastD = ( signed char ) ( * ( ihi.ptr - ( ihi.wAcc >> 16 ) )  ^ ihi.x_or ) ;

      break ;
    }

    /* restF >= restS */
    hirevLoop ( restS ) ;
    lastD = ( signed char ) ( * ( ihi.ptr - ( ihi.wAcc >> 16 ) )  ^ ihi.x_or ) ;

    if ( ihi.ptr < ihi.end || ihi.end <= ihi.ptr - ( ihi.wAcc >> 16 ) ) 
    {
      ulSetError ( UL_FATAL, "SL: Internal Error in _MOD_instHirevLoop." ) ;
/*
      ulSetError ( UL_DEBUG, "bug: restS = %u restF=%u end-ptr = %d, ptr=%p",
          restS, restF, ihi.end-ihi.ptr, ihi.ptr ) ;
      ulSetError ( UL_DEBUG, "last ptr=%p", ( ihi.ptr - ( ihi.wAcc >> 16 ) ) ) ;
*/
    }

    restF -= restS ;

    if ( ihi.loopBeg ) 
    {
      ihi.ptr = ihi.loopBeg + ( ihi.ptr - ihi.end )  % ( ihi.end - ihi.loopBeg ) ;
    }
    else
    {
      ihi.lastL = lastD * ihi.volL ;
      ihi.lastR = lastD * ihi.volR ;
      ihi.fadeout = 256 ;

      fadeout ( &ihi ) ;

      if ( !ihi.fadeout ) 
        ihi.ptr = NULL ; /* note off */

      *ihip = ihi ; /* save cache */
      return ;
    }
  } while ( restF ) ;

  ihi.lastL = lastD * ihi.volL ;
  ihi.lastR = lastD * ihi.volR ;
  *ihip = ihi ; /* save cache */
}

static void makeNoteTable ( void )
{
  static int oct4[] =
  {
    1712, 1616, 1524, 1440, 1356, 1280, 1208, 1140, 1076, 1016, 960, 907
  } ;
  
  note = new short [ NOTE_MAX + 1 ] ;

  for ( int i = 0 ; i <= NOTE_MAX ; i++ )
    note[i] = shift ( oct4[i%12], 4-i/12 ) ;
}

/* period -> nearest note */

static int normalizePeriod ( int *pp )
{
  int i ;
  int s ;
  int p = *pp ;

  if ( p >= note [    0     ] ) { *pp = note [    0     ] ; return    0     ; }
  if ( p <= note [ NOTE_MAX ] ) { *pp = note [ NOTE_MAX ] ; return NOTE_MAX ; }

  for ( i = 0, s = 64 ; s > 0 ; s /= 2 )    /* 64: =2**n, and 64*2 > NOTE_MAX */
    if ( i + s < NOTE_MAX && note [ i + s ] > p )    /* i < NOTE_MAX !! */
      i += s ;

  if ( note [ i ] - p > p - note [ i + 1 ] )
    i++ ; /* choose nearest */

  *pp = note [ i ] ;

  return i ;
}



struct ModulateInfo
{
  int type     ;  /* sine,ramp,square,... */
  int noRetrig ;
  int phase    ;
  int d        ;  /* phase accumlates this for each frame */
  int depth    ;  /* org * table[phase] * depth/MAX_IN_TABLE = cur */
} ;


/* output rate */

static unsigned int mclk    ;

#define MCLK0 ((unsigned int)3579545*4)

void _MOD_instOutRate ( unsigned int rate )
{
  mclk = MCLK0 / rate * 65536 + MCLK0 % rate * 65536 / rate ;
}

#define PFW_MAX    3 /* note delay = note delay + sample delay + vol delay */

struct InstInfo
{
  InstHirevInfo hirev ; /* for hirev loop */

  struct
  {
    int cur     ;    /* current period */
    int org     ;    /* original period */
    int note    ;    /* note */
    int notePer ;    /* note period */
    int mclk    ;    /* master clock in Hz (c4spd * 1712) */

    struct
    {
      int speed ;    /* added/sub'ed for each frame */
      int fine  ;    /* fine slide */
    } slide ;

    struct
    {
      int speed     ;
      int glissando ;
      int nextNote  ;    /* (glissando) next note */
    } port ;             /* portamento */

    ModulateInfo mod ;

    struct
    {
      int baseNote ;
      int plus[2]  ;  /* baseNote + plus[n] halfnote is played */
    } rpgo ;          /* arpeggio */

    struct
    {
      int n       ;
      int newNote ;
    } delay ;

  } per ;        /* period */

  struct
  {
    int cur ;    /* current volume */
    int org ;
    int pan ;    /* 0(left)..64(right) */

    struct
    {
      int d        ;  /* added for each frame    */
      int mul      ;  /* multiply for each frame */
      int div      ;  /* divide for each frame   */
      int nthFrame ;  /* slide every nth frame   */
      int fine     ;  /* fine slide    */
      int count    ;  /* frame counter */
    } slide ;

    ModulateInfo mod ;

    struct
    {
      int onOff   ;
      int count   ;
      int onTime  ;
      int offTime ;
    } tremor ;

    struct
    {
      int n      ;
      int newest ;
    } delay ;

  } vol ;

  struct
  {
    int cur   ;    /* current frame */
    int count ;    /* decrement counter, 0 starts next frame */
  } frame ;

  struct
  {
    struct
    {
      int nthFrame ;  /* retrig every nth frame */
      int count    ;  /* frame counter */
    } retrig ;

    SampleInfo *sip    ;
    SampleInfo *newSip ;
    int         c4spd  ;

    struct
    {
      int         n   ;
      SampleInfo *sip ;
    } delay ;

    int cutFrame ;
  } smp ;

  struct
  {
    void (*func[PFW_MAX])(void) ;
    int n                       ;
  } pfw ;        /* per frame works */

} ;

static InstInfo *instBank ;
static InstInfo *instp    ;


/* select channel */

void _MOD_instSelectCh(int ch)
{
  instp = &instBank[ch];
}


/* per frame work */

void _MOD_instClearPFW(void)
{
  instp->pfw.n = 0;
}

void _MOD_instDoPerFrameWorks(int frame)
{
  instp->frame.cur = frame;

  for (int i = 0; i < instp->pfw.n; i++)
    (*instp->pfw.func[i])();
}

static void addPerFrameWork(void (*f)(void))
{
  if (instp->pfw.n >= PFW_MAX)
  {
    ulSetError ( UL_FATAL, "Too many PFWs");
  }

  instp->pfw.func[instp->pfw.n++] = f;
}


/* hirev loop */

void _MOD_instLoop(void)
{
  _MOD_instHirevLoop(&instp->hirev);
}


/* initialize */

void _MOD_instInit(void)
{
  static SampleInfo si0 ;

  _MOD_instHirevInit () ;
  makeNoteTable () ;

  instBank = new InstInfo [ 32 ] ;

  si0.beg   = si0.end = NULL ;
  si0.c4spd = 8363 ;
  si0.mag   = 1 ;

  for ( int i = 0 ; i < 32 ; i++ )
  {
    /* prepare for "note w/o inst and volume" */
    instBank[i].smp.sip = instBank[i].smp.newSip = &si0 ;
    instBank[i].smp.c4spd = 8363; /* dope.mod ch14 begins with GF0 */
    instBank[i].hirev.end = instBank[i].hirev.ptr = NULL;
    instBank[i].vol.slide.div = 1;
  }
}


static void setW(void)
{
  instp->hirev.w = (mclk * instp->smp.sip->mag) /
                   (instp->per.cur < 16? 16 : instp->per.cur);
}

#define noteToPeriod(n) ((int)note[(n)] * 8363 / instp->smp.c4spd)

static void setPeriod(void)
{
  if (instp->smp.sip != instp->smp.newSip)
  {
    /* actual sample switching is carried out here */
    instp->smp.sip       = instp->smp.newSip       ;
    instp->hirev.end     = instp->smp.sip->end     ;
    instp->hirev.loopBeg = instp->smp.sip->loopBeg ;
    instp->hirev.x_or    = instp->smp.sip->x_or    ;
  }

  instp->per.note = instp->per.delay.newNote;
  instp->per.cur  =
    instp->per.org =
      instp->per.notePer = noteToPeriod(instp->per.note);

  instp->hirev.ptr = instp->smp.sip->beg; /* key-on */
  instp->hirev.wAcc = 0;
  instp->hirev.fadeout = 0;

  if ( ! instp->per.mod.noRetrig ) instp->per.mod.phase = 0 ;
  if ( ! instp->vol.mod.noRetrig ) instp->vol.mod.phase = 0 ;

  setW () ;
}

static void setPeriodPFW(void)
{
  if (instp->per.delay.n == instp->frame.cur)
    setPeriod() ;
}


void _MOD_instNote(int n, int delay)
{
  instp->per.delay.newNote = n;

  if (delay == 0)
    setPeriod();
  else
  {
    instp->per.delay.n = delay;
    addPerFrameWork(setPeriodPFW);
  }
}


/* set volume */

static int mono;

void _MOD_instMono(int n)
{
  mono = n;
}

static void setHirevVol(void)
{
  if ( mono )
  {
    instp->hirev.volL = instp->vol.cur ;
    return ;
  }

  /* currently linear, which makes front sounds rather weaker */

  if (instp->vol.pan >= 0)
  {
    instp->hirev.volL = instp->vol.cur * (64 - instp->vol.pan)/64;
    instp->hirev.volR = instp->vol.cur * instp->vol.pan/64;
  }
  else
  { /* surround!! */
    instp->hirev.volL =  instp->vol.cur / 2;
    instp->hirev.volR = -instp->vol.cur / 2;
  }
}

static void setVol(void)
{
  instp->vol.cur = instp->vol.org = instp->vol.delay.newest;
  setHirevVol();
}

static void setVolPFW(void)
{
  if (instp->vol.delay.n == instp->frame.cur) setVol();
}

void _MOD_instVol(int v, int delay)
{
  instp->vol.delay.newest = v > 64? 64: v;

  if (delay == 0)
    setVol();
  else 
  {
    instp->vol.delay.n = delay;
    addPerFrameWork(setVolPFW);
  }
}


/* tuning */

void _MOD_instTuning(int c4spd)
{
  instp->smp.c4spd = c4spd;
}


/* set sample */

static void setSample()
{
  /* actual sample switching is not done here.. */

  instp->smp.newSip = instp->smp.delay.sip;

  /* set smp's default vol.. */

  instp->vol.cur = instp->vol.org = instp->smp.newSip->vol;

  /* set smp's c4spd.. */

  instp->smp.c4spd = instp->smp.newSip->c4spd;
  setHirevVol();
}


static void setSamplePFW(void)
{
  if (instp->smp.delay.n == instp->frame.cur) setSample();
}

void _MOD_instSample(SampleInfo *sip, int delay)
{
  instp->smp.delay.sip = sip;

  if (delay)
  {
    instp->smp.delay.n = delay;
    addPerFrameWork(setSamplePFW);
  } else setSample();
}


/* volume slide */

inline int limitVol ( int v )
{
  if ( v > 64 )
    return 64 ;

  if ( v < 0 )
    return 0 ;

  return v ;
}

static int fastVolSlide;

static void volSlidePFW(void)
{
  if (!fastVolSlide && !instp->frame.cur) return; /* skip frame 0 */
  if (--instp->vol.slide.count <= 0) {
    instp->vol.slide.count = instp->vol.slide.nthFrame;
    instp->vol.cur =
      instp->vol.cur * instp->vol.slide.mul / instp->vol.slide.div
        + instp->vol.slide.d;

    instp->vol.cur = limitVol ( instp->vol.cur ) ;
    setHirevVol();
  }
}

void _MOD_instVolSlide(void)
{
  if (instp->vol.slide.fine) {
    instp->vol.cur =
      instp->vol.cur * instp->vol.slide.mul / instp->vol.slide.div
        + instp->vol.slide.d;

    instp->vol.cur = limitVol ( instp->vol.cur ) ;
    setHirevVol();
  } else addPerFrameWork(volSlidePFW);
}

void _MOD_instSetVolSlideParams(int d, int mul, int div, int nthFrame, int fine)
{
  instp->vol.slide.d = d;
  instp->vol.slide.mul = mul;
  instp->vol.slide.div = div;
  instp->vol.slide.nthFrame = instp->vol.slide.count = nthFrame;
  instp->vol.slide.fine = fine;
}

void _MOD_instSetVolSlideFast(int onOff)
{
  fastVolSlide = onOff;
}


/* period slide */

#define noteOff() { instp->hirev.fadeout = 256; } /* eventually note-off */

static int amigaLimit;

static void limitPeriod(void)
{
#define p instp->per.cur
  if (amigaLimit) {
    if ((p) > note[3*12]) (p) = note[3*12];
    else if ((p) < note[5*12+11]) (p) = note[5*12+11];
  } else {
    if ((p) > 32000) (p) = 32000;
    else if ((p) < 0) { (p) = 0; noteOff(); /*panic.s3m needs this*/ }
  }
#undef p
}

static void periodSlideUpPFW(void)
{
  if (!instp->frame.cur) return; /* skip frame 0 */
  instp->per.cur -= instp->per.slide.speed; /* ignore per.org */
  limitPeriod(/*instp->per.cur*/);
  instp->per.org = instp->per.cur;
  setW();
}

static void periodSlideDownPFW(void)
{
  if (!instp->frame.cur) return; /* skip frame 0 */
  instp->per.cur += instp->per.slide.speed;
  limitPeriod(/*instp->per.cur*/);
  instp->per.org = instp->per.cur;
  setW();
}

void _MOD_instPeriodSlideUp(void)
{
  if (instp->per.slide.fine) {
    instp->per.cur -= instp->per.slide.speed;
    limitPeriod(/*instp->per.cur*/);
    instp->per.org = instp->per.cur;
    setW();
  } else addPerFrameWork(periodSlideUpPFW);
}

void _MOD_instPeriodSlideDown(void)
{
  if (instp->per.slide.fine) {
    instp->per.cur += instp->per.slide.speed;
    limitPeriod(/*instp->per.cur*/);
    instp->per.org = instp->per.cur;
    setW();
  } else addPerFrameWork(periodSlideDownPFW);
}

void _MOD_instSetPeriodSlideParams(int speed, int fine)
{
  instp->per.slide.speed = speed;
  instp->per.slide.fine = fine;
}

void _MOD_instSetPeriodAmigaLimit(int onOff)
{
  amigaLimit = onOff;
}


/* portamento */

static void portamentoPFW(void)
{
  if (!instp->frame.cur) return; /* which S3M slides at frame 0? */
  if (instp->per.org > instp->per.notePer) { /* port up now */
    instp->per.org -= instp->per.port.speed;
    if (instp->per.org < instp->per.notePer)
      instp->per.cur = instp->per.org = instp->per.notePer;
    else {
      instp->per.cur = instp->per.org;
      if (instp->per.port.glissando) normalizePeriod(&instp->per.cur);
    }
  } else { /* port down now */
    instp->per.org += instp->per.port.speed;
    if (instp->per.org > instp->per.notePer)
      instp->per.cur = instp->per.org = instp->per.notePer;
    else {
      instp->per.cur = instp->per.org;
      if (instp->per.port.glissando) normalizePeriod(&instp->per.cur);
    }
  }
  setW();
}

void _MOD_instPortamento(void)
{
  addPerFrameWork(portamentoPFW);
}

void _MOD_instSetPortamentoTo(int to)
{
  instp->per.note = to;
  instp->per.notePer = noteToPeriod(to);
}

void _MOD_instSetPortamentoSpeed(int speed)
{
  instp->per.port.speed = speed;
}

void _MOD_instSetPortamentoDefaultVol(void)
{
  instp->vol.cur = instp->vol.org = instp->smp.sip->vol;
  setHirevVol();
}

void _MOD_instSetPortamentoGlissando(int onOff)
{
  instp->per.port.glissando = onOff;
}


/* arpeggio */

static void arpeggioPFW(void)
{
  if (instp->frame.cur % 3) {
    instp->per.cur = note[instp->per.note
                + instp->per.rpgo.plus[instp->frame.cur%3 - 1]];
  } else instp->per.cur = instp->per.notePer;
  setW();
}

void _MOD_instArpeggio(void)
{
  addPerFrameWork(arpeggioPFW);
}

void _MOD_instSetArpeggioParams(int plus1, int plus2)
{
  instp->per.rpgo.plus[0] = plus1;
  instp->per.rpgo.plus[1] = plus2;
}


/* retrig */

static void retrigPFW(void)
{
  if (--instp->smp.retrig.count <= 0) {
    instp->smp.retrig.count = instp->smp.retrig.nthFrame;
    instp->hirev.ptr = instp->smp.sip->beg;
    setW();
  }
}

void _MOD_instRetrig(void)
{
  addPerFrameWork(retrigPFW);
}

void _MOD_instSetRetrigParam(int nthFrame)
{
  instp->smp.retrig.nthFrame = nthFrame;
  instp->smp.retrig.count = 0;
}


/* sample offset */

void _MOD_instSampleOffset(int offset)
{
  instp->hirev.ptr = instp->smp.sip->beg + offset * instp->smp.sip->mag;
  if (instp->hirev.ptr >= instp->hirev.end) {
    if (instp->hirev.loopBeg) {
      instp->hirev.ptr =
        instp->hirev.loopBeg +
          (instp->hirev.ptr - instp->hirev.end) %
            (instp->hirev.end - instp->hirev.loopBeg);
    } else { noteOff(); }
  }
}


/* modulation */

static unsigned char sine[] = {
  000,  25,  50,  74,  98, 120, 142, 162,
  180, 197, 212, 225, 236, 244, 250, 254,
  255
}; /* sin(x), 0 <= x < pi/4, max = 255 */


static int wave(ModulateInfo *mip)
{
  int i;

  switch (mip->type) {
  case 1: /* ramp up (period: down) */
    i = (255 * 2 * mip->phase)/63 - 255;
    break;
  case 2: /* square */
    i = (mip->phase < 32)? 255 : 0; /* yes, not 255/-255 */
    break;
  default: /* sine */
    if    (mip->phase < 16) i = sine[mip->phase];
    else if (mip->phase < 32) i = sine[32 - mip->phase];
    else if (mip->phase < 48) i = -sine[mip->phase - 32];
    else            i = -sine[64 - mip->phase];
  }
  return mip->depth * i / 255;
}

static void vibratoPFW(void)
{
  if (!instp->frame.cur) return; /* skip frame 0 */
  instp->per.mod.phase += instp->per.mod.d;
  instp->per.mod.phase %= 64;
  /* per.org: no change.. */
  instp->per.cur = instp->per.org + wave(&instp->per.mod);
  limitPeriod(/*instp->per.cur*/);
  setW();
}

void _MOD_instVibrato(void)
{
  addPerFrameWork(vibratoPFW);
}

/* depth = period */
void _MOD_instSetVibratoParams(int d, int depth)
{
  if (d) instp->per.mod.d = d;
  instp->per.mod.depth = depth;
  /*if (!instp->per.mod.noRetrig) instp->per.mod.phase = 0;*/
}

void _MOD_instSetVibratoWave(int type, int noRetrig)
{
  if (type == 3) type = rand() % 3;
  instp->per.mod.type = type;
  instp->per.mod.noRetrig = noRetrig;
}


static void tremoloPFW(void)
{
  if (!instp->frame.cur) return; /* skip frame 0 */
  instp->vol.mod.phase += instp->vol.mod.d;
  instp->vol.mod.phase %= 64;
  instp->vol.cur = instp->vol.org + wave(&instp->vol.mod);
  instp->vol.cur = limitVol ( instp->vol.cur ) ;
  setHirevVol();
}

void _MOD_instTremolo(void)
{
  addPerFrameWork(tremoloPFW);
}

void _MOD_instSetTremoloParams(int d, int depth)
{
  if (d) instp->vol.mod.d = d;
  instp->vol.mod.depth = depth;
  /*if (!instp->vol.mod.noRetrig) instp->vol.mod.phase = 0;*/
}

void _MOD_instSetTremoloWave(int type, int noRetrig)
{
  if (type == 3) type = rand() % 3;
  instp->vol.mod.type = type;
  instp->vol.mod.noRetrig = noRetrig;
}


/* note cut */

static void noteCutPFW(void)
{
  /* said to be vol := 0, but ST3 seems to key-off */
  if (instp->smp.cutFrame == instp->frame.cur) noteOff();
}
  
void _MOD_instNoteCut(int frame)
{
  if (frame) {
    instp->smp.cutFrame = frame;
    addPerFrameWork(noteCutPFW);
  } else noteOff();
}


/* tremor */

static void tremorPFW(void)
{
  if (--instp->vol.tremor.count <= 0) {
    if (instp->vol.tremor.onOff) {
      instp->vol.cur = 0;
      setHirevVol();
      instp->vol.tremor.onOff = 0;
      instp->vol.tremor.count = instp->vol.tremor.offTime;
    } else {
      instp->vol.cur = instp->vol.org;
      setHirevVol();
      instp->vol.tremor.onOff = 1;
      instp->vol.tremor.count = instp->vol.tremor.onTime;
    }
  }
}

void _MOD_instTremor(void)
{
  addPerFrameWork(tremorPFW);
}

void _MOD_instSetTremorParams(int onTime, int offTime)
{
  instp->vol.tremor.onTime = onTime;
  instp->vol.tremor.offTime = offTime;
  instp->vol.tremor.count = 0;
  instp->vol.tremor.onOff = 0;
}


/* note off */

static void noteOffPFW(void)
{
  if (instp->per.delay.n == instp->frame.cur) noteOff();
}

void _MOD_instNoteOff(int delay)
{
  if (delay) {
    instp->per.delay.n = delay;
    addPerFrameWork(noteOffPFW);
  } else { noteOff(); }
}

int _MOD_instIsNoteOff(void)
{
  return instp->hirev.ptr == NULL;
}


/* pan position */

/* pos = 0(left)..64(right), -1(surround) */

void _MOD_instPanPosition(int pos)
{
  instp->vol.pan = pos;
  setHirevVol();
}


/* empty command */

void _MOD_instEmptyCmd(void)
{
  instp->per.cur = instp->per.org;
  setW();
  /* empty command has some special meanings...
     (when after glissando portamento or vibrato) */
}

