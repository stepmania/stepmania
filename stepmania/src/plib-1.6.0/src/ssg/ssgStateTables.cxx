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


#include "ssgLocal.h"

/*
  Well, it would be nice to say that these tables
  were automatically generated - but they weren't.

  So, there is some particularly nasty Macro-processing
  going on to save my poor aching fingers.
*/

#define E0  { if ( ! _ssgCurrentContext->textureOverridden ()) glEnable ( GL_TEXTURE_2D ) ; }
#define E1  { if ( ! _ssgCurrentContext->cullfaceOverridden()) glEnable ( GL_CULL_FACE  ) ; }
#define E2  glEnable ( GL_COLOR_MATERIAL  ) ;
#define E3  glEnable ( GL_BLEND           ) ;
#define E4  glEnable ( GL_ALPHA_TEST      ) ;
#define E5  glEnable ( GL_LIGHTING        ) ;

#define D0  glDisable ( GL_TEXTURE_2D     ) ;
#define D1  glDisable ( GL_CULL_FACE      ) ;
#define D2  glDisable ( GL_COLOR_MATERIAL ) ;
#define D3  glDisable ( GL_BLEND          ) ;
#define D4  glDisable ( GL_ALPHA_TEST     ) ;
#define D5  glDisable ( GL_LIGHTING       ) ;

/* Do any combination of useful glEnable/glDisable commands */

static void enable_00 () {                }
static void enable_01 () { E0             }
static void enable_02 () {    E1          }
static void enable_03 () { E0 E1          }
static void enable_04 () {       E2       }
static void enable_05 () { E0    E2       }
static void enable_06 () {    E1 E2       }
static void enable_07 () { E0 E1 E2       }
static void enable_08 () {          E3    }
static void enable_09 () { E0       E3    }
static void enable_10 () {    E1    E3    }
static void enable_11 () { E0 E1    E3    }
static void enable_12 () {       E2 E3    }
static void enable_13 () { E0    E2 E3    }
static void enable_14 () {    E1 E2 E3    }
static void enable_15 () { E0 E1 E2 E3    }
static void enable_16 () {             E4 }
static void enable_17 () { E0          E4 }
static void enable_18 () {    E1       E4 }
static void enable_19 () { E0 E1       E4 }
static void enable_20 () {       E2    E4 }
static void enable_21 () { E0    E2    E4 }
static void enable_22 () {    E1 E2    E4 }
static void enable_23 () { E0 E1 E2    E4 }
static void enable_24 () {          E3 E4 }
static void enable_25 () { E0       E3 E4 }
static void enable_26 () {    E1    E3 E4 }
static void enable_27 () { E0 E1    E3 E4 }
static void enable_28 () {       E2 E3 E4 }
static void enable_29 () { E0    E2 E3 E4 }
static void enable_30 () {    E1 E2 E3 E4 }
static void enable_31 () { E0 E1 E2 E3 E4 }

static void enable_32 () {                E5 }
static void enable_33 () { E0             E5 }
static void enable_34 () {    E1          E5 }
static void enable_35 () { E0 E1          E5 }
static void enable_36 () {       E2       E5 }
static void enable_37 () { E0    E2       E5 }
static void enable_38 () {    E1 E2       E5 }
static void enable_39 () { E0 E1 E2       E5 }
static void enable_40 () {          E3    E5 }
static void enable_41 () { E0       E3    E5 }
static void enable_42 () {    E1    E3    E5 }
static void enable_43 () { E0 E1    E3    E5 }
static void enable_44 () {       E2 E3    E5 }
static void enable_45 () { E0    E2 E3    E5 }
static void enable_46 () {    E1 E2 E3    E5 }
static void enable_47 () { E0 E1 E2 E3    E5 }
static void enable_48 () {             E4 E5 }
static void enable_49 () { E0          E4 E5 }
static void enable_50 () {    E1       E4 E5 }
static void enable_51 () { E0 E1       E4 E5 }
static void enable_52 () {       E2    E4 E5 }
static void enable_53 () { E0    E2    E4 E5 }
static void enable_54 () {    E1 E2    E4 E5 }
static void enable_55 () { E0 E1 E2    E4 E5 }
static void enable_56 () {          E3 E4 E5 }
static void enable_57 () { E0       E3 E4 E5 }
static void enable_58 () {    E1    E3 E4 E5 }
static void enable_59 () { E0 E1    E3 E4 E5 }
static void enable_60 () {       E2 E3 E4 E5 }
static void enable_61 () { E0    E2 E3 E4 E5 }
static void enable_62 () {    E1 E2 E3 E4 E5 }
static void enable_63 () { E0 E1 E2 E3 E4 E5 }

static void disable_00 () {                }
static void disable_01 () { D0             }
static void disable_02 () {    D1          }
static void disable_03 () { D0 D1          }
static void disable_04 () {       D2       }
static void disable_05 () { D0    D2       }
static void disable_06 () {    D1 D2       }
static void disable_07 () { D0 D1 D2       }
static void disable_08 () {          D3    }
static void disable_09 () { D0       D3    }
static void disable_10 () {    D1    D3    }
static void disable_11 () { D0 D1    D3    }
static void disable_12 () {       D2 D3    }
static void disable_13 () { D0    D2 D3    }
static void disable_14 () {    D1 D2 D3    }
static void disable_15 () { D0 D1 D2 D3    }
static void disable_16 () {             D4 }
static void disable_17 () { D0          D4 }
static void disable_18 () {    D1       D4 }
static void disable_19 () { D0 D1       D4 }
static void disable_20 () {       D2    D4 }
static void disable_21 () { D0    D2    D4 }
static void disable_22 () {    D1 D2    D4 }
static void disable_23 () { D0 D1 D2    D4 }
static void disable_24 () {          D3 D4 }
static void disable_25 () { D0       D3 D4 }
static void disable_26 () {    D1    D3 D4 }
static void disable_27 () { D0 D1    D3 D4 }
static void disable_28 () {       D2 D3 D4 }
static void disable_29 () { D0    D2 D3 D4 }
static void disable_30 () {    D1 D2 D3 D4 }
static void disable_31 () { D0 D1 D2 D3 D4 }

static void disable_32 () {                D5 }
static void disable_33 () { D0             D5 }
static void disable_34 () {    D1          D5 }
static void disable_35 () { D0 D1          D5 }
static void disable_36 () {       D2       D5 }
static void disable_37 () { D0    D2       D5 }
static void disable_38 () {    D1 D2       D5 }
static void disable_39 () { D0 D1 D2       D5 }
static void disable_40 () {          D3    D5 }
static void disable_41 () { D0       D3    D5 }
static void disable_42 () {    D1    D3    D5 }
static void disable_43 () { D0 D1    D3    D5 }
static void disable_44 () {       D2 D3    D5 }
static void disable_45 () { D0    D2 D3    D5 }
static void disable_46 () {    D1 D2 D3    D5 }
static void disable_47 () { D0 D1 D2 D3    D5 }
static void disable_48 () {             D4 D5 }
static void disable_49 () { D0          D4 D5 }
static void disable_50 () {    D1       D4 D5 }
static void disable_51 () { D0 D1       D4 D5 }
static void disable_52 () {       D2    D4 D5 }
static void disable_53 () { D0    D2    D4 D5 }
static void disable_54 () {    D1 D2    D4 D5 }
static void disable_55 () { D0 D1 D2    D4 D5 }
static void disable_56 () {          D3 D4 D5 }
static void disable_57 () { D0       D3 D4 D5 }
static void disable_58 () {    D1    D3 D4 D5 }
static void disable_59 () { D0 D1    D3 D4 D5 }
static void disable_60 () {       D2 D3 D4 D5 }
static void disable_61 () { D0    D2 D3 D4 D5 }
static void disable_62 () {    D1 D2 D3 D4 D5 }
static void disable_63 () { D0 D1 D2 D3 D4 D5 }


void (*__ssgEnableTable[64])() =
{
  enable_00, enable_01, enable_02, enable_03, enable_04,
  enable_05, enable_06, enable_07, enable_08, enable_09,
  enable_10, enable_11, enable_12, enable_13, enable_14,
  enable_15, enable_16, enable_17, enable_18, enable_19,
  enable_20, enable_21, enable_22, enable_23, enable_24,
  enable_25, enable_26, enable_27, enable_28, enable_29,
  enable_30, enable_31,

  enable_32, enable_33, enable_34, enable_35, enable_36,
  enable_37, enable_38, enable_39, enable_40, enable_41,
  enable_42, enable_43, enable_44, enable_45, enable_46,
  enable_47, enable_48, enable_49, enable_50, enable_51,
  enable_52, enable_53, enable_54, enable_55, enable_56,
  enable_57, enable_58, enable_59, enable_60, enable_61,
  enable_62, enable_63
} ;

void (*__ssgDisableTable[64])() =
{
  disable_00, disable_01, disable_02, disable_03, disable_04,
  disable_05, disable_06, disable_07, disable_08, disable_09,
  disable_10, disable_11, disable_12, disable_13, disable_14,
  disable_15, disable_16, disable_17, disable_18, disable_19,
  disable_20, disable_21, disable_22, disable_23, disable_24,
  disable_25, disable_26, disable_27, disable_28, disable_29,
  disable_30, disable_31,

  disable_32, disable_33, disable_34, disable_35, disable_36,
  disable_37, disable_38, disable_39, disable_40, disable_41,
  disable_42, disable_43, disable_44, disable_45, disable_46,
  disable_47, disable_48, disable_49, disable_50, disable_51,
  disable_52, disable_53, disable_54, disable_55, disable_56,
  disable_57, disable_58, disable_59, disable_60, disable_61,
  disable_62, disable_63
} ;

