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
#include "slMODfile.h"

#define NOTE_MAX    (12*5-1) /* 5 octave */
#define shift(x,n)  ((n)>= 0 ? (x) << (n) : (x) >> -(n)) /* x * 2**n */

#define u16LittleEndian(x) (((unsigned char *)(x))[0] + \
                            ((unsigned char *)(x))[1]*(unsigned short)256)
#define u16BigEndian(x)    (((unsigned char *)(x))[0]*(unsigned short)256 + \
                            ((unsigned char *)(x))[1])

static const char *transTab    = "JFEGHLKRXODB.C"   ;
static const char *transTabE   = "SFESSSSSSQ..SSSS" ;
static const char transTabEX[] = { 0,0xF,0xF,1,3,2,0xB,4,8,0,0,0,0xC,0xD,0xE,0xF } ;
static int  oct[12]      = { 1712, 1616, 1524, 1440, 1356, 1280,
                             1208, 1140, 1076, 1016,  960,  907 } ;
static int  freq[16]     = { 8363, 8413, 8463, 8529, 8581, 8651, 8723, 8757,
                             7895, 7941, 7985, 8046, 8107, 8169, 8232, 8280 } ;
static unsigned char emptySample ;

class SlmInfo ;

static SlmInfo *top = NULL ;

class SlmInfo
{
  char    *data ;
  SlmInfo *next ;
public:

  SlmInfo ( unsigned int size )
  {
    data = new char [ size ] ;
    next = top  ;
    top  = this ;
  }

  ~SlmInfo ()
  {
    if ( this == top )
      top = NULL ;

    delete [] data ;
    delete next ;
  }

  void *get () { return (void *) data ; }
} ;


/* Allocate song-life memory */

static void *memSong ( unsigned size )
{
  SlmInfo *sip = new SlmInfo ( size ) ;
  return sip -> get () ;
}

/* free all song-life memory */

static void memSongFree(void)
{
  delete top ;
}


/* LPF fc=fs/4 impulse response * Hamming window (N=19) */
static int h2[][8] = /* h(n) = */
{
    /* n = */
    /*-7  -5   -3   -1    1    3   5   7*/
    { -4, 15, -42, 158, 158, -42, 15, -4}
};
/* LPF fc=fs/6 impulse response * Hamming window (N=29) */
static int h3[][8] =
{
    /*-10  -7   -4   -1    2    5   8  11*/
    {  -5, 16, -44, 209, 101, -31, 12, -3},
    /*-11  -8   -5   -2    1    4   7  10*/
    {  -3, 12, -31, 101, 209, -44, 16, -5}
};
/* LPF fc=fs/8 impulse response * Hamming window (N=35) */
static int h4[][8] =
{
    /*-13  -9   -5   -1    3    7  11  15*/
    {  -4, 13, -38, 229,  72, -22,  7, -2},
    /*-14 -10   -6   -2    2    6  10  14*/
    {  -3, 13, -40, 158, 158, -40, 13, -3},
    /*-15 -11   -7   -3    1    5   9  13*/
    {  -2,  7, -22,  72, 229, -38, 13, -4}
};
static int (*hn[])[8] = { h2, h3, h4 };

static SampleInfo sis;

static void convolute ( int mag, char *dp )
{
    unsigned char *sp0;
    int i,j;

    for (sp0 = sis.beg; sp0 < sis.end; sp0++) {
	*dp++ = *sp0 ^ sis.x_or;
	for (i = 0; i < mag-1; i++,dp++) {
	    int *hp = hn[mag-2][i];
	    unsigned char *sp = sp0 - 3;
	    int sum = 0;
	    for (j = 8; j > 0; j--,sp++,hp++) {
		if (sp < sis.beg) continue;
		if (sp >= sis.end) {
		    if (sis.loopBeg) sp = sis.loopBeg;
		    else break;
		}		
		sum += *hp * (char)(*sp ^ sis.x_or);
	    }
	    sum /= 256;
	    if (sum > 127) sum = 127;
	    else if (sum < -128) sum = -128;
	    *dp = sum;
	}
    }
}


static void perSampleWork(SampleInfo *sip, unsigned int c4req)
{
    static SampleInfo sid;
    unsigned char *dp0;
    unsigned int len;
    int mag;

    mag = c4req / sip->c4spd;

    if (!mag) return; /* no need to oversampling */
    mag++;
    if (mag > 4) mag = 4; /* currently max = 4x */
    sis = *sip;
    len = sis.end - sis.beg;
    if (len <= 2) return; /* too short sample */
    dp0 = (unsigned char *) memSong(len * mag);
    sid.beg = dp0 ;
    sid.end = dp0 + len*mag ;
    convolute(mag, (char *)dp0);
    sid.loopBeg = sis.loopBeg? (sid.beg + (sis.loopBeg - sis.beg)*mag) : (unsigned char *)NULL;
    sid.x_or = 0;
    sid.c4spd = sis.c4spd;
    sid.vol = sis.vol;
    sid.mag = sis.mag * mag;
    *sip = sid;
}

static void oversample ( int insNum, SampleInfo *sip, unsigned int c4req )
{
  for ( ; insNum > 0 ; insNum--, sip++ )
    perSampleWork ( sip, c4req ) ;
}

enum MagicType
{
  MAGIC_S3M, MAGIC_MOD ,MAGIC_MODX, MAGIC_MOD15, MAGIC_MTM
} ;

/* stricter 15-sample mod checking */

static int isMod15(unsigned char *p0, int size)
{
  int ordNum, patNum ;
  unsigned char *p ;

  if ( size < 20+30*15+130+1024 )
    return 0 ; /* check min length */

  ordNum = p0[20+30*15] ;

  if (!ordNum || ordNum >= 128)
    return 0 ;

  patNum = 0 ;
  p = p0 + 20+30*15+2 ; /* pattern table */

  for ( int i = 0 ; i < ordNum ; i++, p++ )
    if (patNum < *p)
      patNum = *p ;

  if (patNum >= 64)
    return 0 ;

  if (size < 20 + 30*15 + 130 + 1024 + 1024*patNum )
    return 0 ;

  /* sample length is not checked here */
  return 1 ;
}


struct MagicInfo
{
  const char *str  ;
  int         off  ;
  MagicType   type ;
} ;


MagicInfo magicInfo[] =
{
  {"SCRM", 0x2c , MAGIC_S3M  } ,
  {"M.K.", 0x438 /*20+30*31+130=1080*/, MAGIC_MOD},
  {"M!K!", 0x438, MAGIC_MOD  },
  {"FLT4", 0x438, MAGIC_MOD  }, /* FLT8's pattern is different */
  {"#CHN", 0x438, MAGIC_MODX },
  {"##CH", 0x438, MAGIC_MODX }, /* dope.mod */
  {"MTM",  0  ,   MAGIC_MTM  }, /* check after above patterns fail. */
                                /* (A mod songname can be "MTMxxx") */
  {0}
} ;


#define isnum(x) ('0' <= (x) && (x) <= '9')

static int magic(unsigned char *p, int size, MagicType *mtp, int *numChp)
{
  MagicInfo *mip;
  int ch;

  for (mip = magicInfo; mip->str != 0; mip++)
  {
    const char *ss ;
    char *sp ;

    if (mip->off + 16 >= size)
      continue ; /* Magicinfo.str must < 16 */

    sp = (char *)p + mip->off;
    ch = 0;

    int got_it = FALSE ;

    for (ss = mip->str; *ss != 0; ss++,sp++)
    {
      if (*ss == '#')
      {
        if (isnum(*sp))
        {
          ch = ch*10 + *sp - '0';
          continue ;
        }
        else
        {
          got_it = TRUE ;
          break ;
        }
      }

      if (*sp != *ss)
      {
        got_it = TRUE ;
        break ;
      }
    }

    if ( ! got_it )
    {
      *mtp    = mip->type ;
      *numChp = ch        ;
      return 0 ;
    }
  }

  if ( isMod15 ( p, size ) )
  {
    *mtp = MAGIC_MOD15 ;
    return 0 ;
  }
  else
    return 1 ;
}

void MODfile::makeNoteTable(void)
{
  note = (short *) memSong ( (NOTE_MAX+1) * sizeof(short) ) ;

  for ( int i = 0 ; i <= NOTE_MAX ; i++ )
    note[i] = shift ( oct[i%12], -i/12 ) ;
}


int MODfile::roundToNote ( int p )
{
  if ( p ==      0          ) return    0     ;
  if ( p >= note [   0    ] ) return    0     ;
  if ( p <= note [NOTE_MAX] ) return NOTE_MAX ;

  /* 32: =2**n, and 32*2 > NOTE_MAX */

  int i, s ;

  for ( i = 0, s = 32 ; s > 0 ; s /= 2 )
    if ( i + s < NOTE_MAX && note[i+s] > p )
      i += s ;	/* i < NOTE_MAX !! */

  if ( note[i] - p > p - note[i+1] )
    i++ ; /* choose nearest */

  return i ;
}


void MODfile::modToS3m(ModNote *mp, Note *np)
{
  int X  = (*mp)[3] / 16 ;
  int Y  = (*mp)[3] % 16 ;
  int XY = (*mp)[3] ;

  int n = roundToNote ( (*mp)[0]%16 * 256 + (*mp)[1] ) ;

  np->note = (n!=0) ?(n/12+2)*16 + n%12 : 255 ;
  np->ins  = ((*mp)[0] & 0xf0) | (*mp)[2]/16;
  np->vol  = 255 ;
  np->cmd  = 255 ;
  np->info =  0  ;

  switch ( (*mp)[2] % 16 )
  {
    case 0x0: if (X || Y) { np->cmd = 'J' - '@'; np->info = XY; } break;
    case 0x1:
    case 0x2: if (XY) { np->cmd = transTab[(*mp)[2]%16] - '@'; np->info = (XY >= 0xE0)? 0xdf : XY; } break;
    case 0xA: if (!XY) break;
    case 0xC: np->vol = XY>64? 64 : XY; np->cmd = 255; np->info = 0; break;
    case 0xE:
      switch (X)
      {
        case 0xA: if (Y) { np->cmd = 'D' - '@'; np->info = Y*16 + 15; } break ;
        case 0xB: if (Y) { np->cmd = 'D' - '@'; np->info = 0xf0 + Y; } break ;
        default:         { np->cmd = transTabE[X] - '@'; np->info = transTabEX[X]*16 + Y; } break ;
      }
      break;
    case 0xF: if (XY) { np->cmd = XY <= 32? 'A' - '@' : 'T' - '@'; np->info = XY? XY : 1; } break;
    default : { np->cmd = transTab[(*mp)[2]%16] - '@'; np->info = XY ; } break ;
  }
}


void MODfile::play_one ( int ppat )
{
  ModNote *np = & pat [ chNum * ( 64 * ppat + play_row ) ] ;

  for ( int ch = 0 ; ch < chNum ; ch++, np++ )
  {
    Note note ;

    modToS3m        ( np, &note ) ;
    _MOD_playNoteSetNote ( ch, &note ) ;

    switch ( note.cmd )
    {
      case 'A' - '@': /* Set speed */
	_MOD_playNoteSetSpeed ( note.info ) ;
	break;

      case 'B' - '@':
	play_nextOrd = note.info | 0x100 ;
	play_row = 64 ;
	break ;

      case 'C' - '@':
	play_row0 = note.info / 16 * 10 + note.info % 16 ;
	play_row = 64 ;
	break ;

      case 'S' - '@':
	switch ( note.info / 16 )
	{
	  case 0xB: /* pattern loop */
	    if ( note.info % 16 )  /* Jump to mark */
	    {
	      if ( play_loopCnt < note.info % 16 )
	      {
		play_row = play_loopBeg - 1 ;
		play_loopCnt++ ;
	      }
	      else
		play_loopCnt = 0 ;
	    }
	    else /* Set mark */
	      play_loopBeg = play_row ;

	    break;

	  case 0xE: /* Pattern delay */
	    _MOD_playNoteSetPatRepeat ( note.info % 16 ) ;
	    break;
	}
	break;

      case 'T' - '@': /* Set tempo */
	_MOD_playNoteSetTempo ( note.info ) ;
	break ;
    }
  }

  _MOD_playNote () ;
}



void MODfile::tellChSettings( void )
{
  for ( int i = 0; i < chNum; i++ )
  {
    _MOD_instSelectCh ( i ) ;

    switch ( i % 4 )
    {
      case 0  :
      case 3  : _MOD_instPanPosition ( 3 * 64/15 ) ; break ; /* left */
      default : _MOD_instPanPosition (12 * 64/15 ) ; break ; /* right */
    }
  }
}



void MODfile::makeSampleInfo( int smp15 )
{
  ModSample *msp ;
  SampleInfo *p ;
  unsigned char *sp ;

  p = sip = (SampleInfo *) memSong ( sizeof(SampleInfo) * insNum ) ;
  sp = smp0p ;
  msp = &smpInfop[0] ;

  for ( int i = 0; i < insNum; i++, p++, msp++)
  {
    unsigned int lOff = 0 ;

    p->beg = sp ;

    unsigned int len  = u16BigEndian(*msp+22) * 2 ; /*len*/
    unsigned int lLen = u16BigEndian(*msp+28)     ; /*loopLen*/

    if (lLen > 1)
    {
      lLen *= smp15? 1:2;
      lOff = u16BigEndian(*msp+26)*(smp15? 1:2); /*loopOffset*/
      p->loopBeg = sp + lOff; /*sp+loopOffset*/
      p->end = p->loopBeg + lLen;
    }
    else
    {
      p->loopBeg = NULL;
      p->end = sp + len; /*sp+len*/
    }

    p->x_or  = 0 ;
    p->mag   = 1 ;
    p->c4spd = freq[(*msp)[24]%16] ;
    p->vol   = (*msp)[25] > 64 ? 64 : (*msp)[25] ;

    if (p->end > fileEnd)
    {
      if (p->beg >= fileEnd || p->loopBeg >= fileEnd)
      {
        ulSetError ( UL_WARNING,
          "short file (assigned an empty sample for #%d)", i+1 ) ;
        p->beg = &emptySample   ;
        p->end = &emptySample+1 ;
        p->loopBeg = NULL ;
        p->vol = 0 ;
      }
      else
      {
        ulSetError ( UL_WARNING,
          "short file (sample #%d truncated)", i+1 ) ;
        p->end = fileEnd ;
      }
    }

    sp += u16BigEndian(*msp+22) * 2; /*sp+len*/
  }
}


void MODfile::parseMod(unsigned char *pp0, int smp15)
{
  int i,n;
  unsigned char *p;

  p = pp0;
  p0 = songName = p ;
  p += 20;

  smpInfop = (ModSample *) p ;

  if ( smp15 ) { p += 15*sizeof(ModSample); insNum = 15; }
  else         { p += 31*sizeof(ModSample); insNum = 31; }

  ordNum = *p ; p++ ;
  rstOrd = *p ; p++ ;

  ord = p;

  for ( i = 0, n = 0 ; i < 128 ; i++ )
    if ( n < p[i] )
      n = p[i] ; /* find max pat */

  patNum = n + 1 ;
  p += 128 + ( smp15 ? 0 : 4 ) ;
  pat   = (ModNote *) p ;
  smp0p = pat [ chNum * 64 * patNum ] ;
}


int MODfile::update ()
{
  if ( broken )
    return FALSE ;

  if ( firsttime )
  {
    play_ord = play_ord0 = 0 ;
    memset ( repCounter, 0, ordNum ) ;
    firsttime = FALSE ;
  }

  play_one ( ord [ play_ord ] ) ;
 
  if ( ++play_row >= 64 )
  {
    play_loopBeg = 0 ;
    play_loopCnt = 0 ;
    play_nextOrd = 0 ;

    play_row = play_row0 ;
    play_row0 = 0 ;

    play_ord = (play_nextOrd==0) ? (play_ord+1) : (play_nextOrd & 0xff) ;

    if ( play_ord >= ordNum )
    {
      play_ord0 = rstOrd    ;
      play_ord  = play_ord0 ;

      if ( rstOrd > ordNum )
      { 
        firsttime = TRUE ;
        return FALSE ;
      }
    }
  }

  return TRUE ;         /* Carry on playing! */
}



unsigned char *MODfile::read_whole_file ( const char *fname, int *len )
{
  struct stat statbuf ;

  int l = 0 ;

  int fd = open ( fname, O_RDONLY ) ;

  if ( fd < 0 )
  {
    perror ( "open" ) ;
    ulSetError ( UL_WARNING,
      "SL: Couldn't open MOD file '%s' for reading", fname ) ;
    return NULL ;
  }

  if ( fstat ( fd, &statbuf ) < 0 )
  {
    perror ( "fstat" ) ;
    return NULL ;
  }

  l = statbuf.st_size ;

  unsigned char *p = new unsigned char [ l ] ;
  read  ( fd, (char *)p, l ) ;
  close ( fd ) ;

  if ( len != NULL )
    *len = l ;

  return p ;
}



MODfile::MODfile ( const char *fname, int speed, int stereo )
{
  p0         = NULL ; songName   = NULL ; ord        = NULL ;
  smpInfop   = NULL ; smp0p      = NULL ; pat        = NULL ;
  note       = NULL ; fileEnd    = NULL ; sip        = NULL ;
  repCounter = NULL ;

  play_nextOrd = play_loopBeg = play_loopCnt = play_row0 =
    play_row = play_ord0 = play_ord = ordNum = insNum =
      patNum = chNum = rstOrd = firsttime = broken = 0 ;

  MagicType      mt    ;

  int len = 0 ;

  buffer = read_whole_file ( fname, &len ) ;
  
  if ( buffer == NULL )
  {
    broken = TRUE ;
    return ;
  }  

  if ( magic ( buffer, len, &mt, &chNum ) )
  {
    broken = TRUE ;
    ulSetError ( UL_WARNING, "Unknown format" ) ;
    return ;
  }

  if ( mt == MAGIC_MOD || mt == MAGIC_MOD15 )
    chNum = 4 ;
  else
  if ( mt != MAGIC_MODX )
  {
    ulSetError ( UL_WARNING, "Unknown format" ) ;
    broken = TRUE ;
    return ;
  }

  _MOD_playNoteInit       () ;
  _MOD_playNoteSetOutRate ( speed   ) ;
  _MOD_playNoteSetMono    ( !stereo ) ;

  fileEnd = & buffer [ len ] ;
  repCounter = (unsigned char *) memSong ( 128 ) ;
  parseMod ( buffer, mt == MAGIC_MOD15 ) ;
  _MOD_instSetPeriodAmigaLimit ( 1 ) ;
  tellChSettings () ;

  makeSampleInfo ( mt == MAGIC_MOD15 ) ;

  oversample ( insNum, sip, (unsigned int) speed ) ;

  _MOD_playNoteSetSample    ( sip  ) ;
  _MOD_playNoteSetTempo     ( 125  ) ;
  _MOD_playNoteSetSpeed     (  6   ) ;
  _MOD_playNoteSetMasterVol ( 0x30 ) ;
  _MOD_playNoteSetGlobalVol ( 0x40 ) ;
  makeNoteTable  () ;
}


MODfile::~MODfile ()
{
  if ( broken )
    return ;

  delete [] buffer ;
  memSongFree () ;
}

