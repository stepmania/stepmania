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


#ifndef _SSGAUX_H_
#define _SSGAUX_H_ 1

#include "ssg.h"
#include "ssgaShapes.h"
#include "ssgaParticleSystem.h"
#include "ssgaFire.h"
#include "ssgaWaveSystem.h"
#include "ssgaLensFlare.h"


#define _SSGA_TYPE_SHAPE          0x00008000
#define _SSGA_TYPE_CUBE           0x00004000
#define _SSGA_TYPE_SPHERE         0x00002000
#define _SSGA_TYPE_CYLINDER       0x00001000
#define _SSGA_TYPE_PATCH          0x00010000
#define _SSGA_TYPE_TEAPOT         0x00020000
#define _SSGA_TYPE_PARTICLESYSTEM 0x00040000
#define _SSGA_TYPE_WAVESYSTEM     0x00080000
#define _SSGA_TYPE_LENSFLARE      0x00100000

inline int ssgaTypeShape   () { return _SSGA_TYPE_SHAPE    | ssgTypeBranch ();}
inline int ssgaTypeCube    () { return _SSGA_TYPE_CUBE     | ssgaTypeShape ();}
inline int ssgaTypeSphere  () { return _SSGA_TYPE_SPHERE   | ssgaTypeShape ();}
inline int ssgaTypeCylinder() { return _SSGA_TYPE_CYLINDER | ssgaTypeShape ();}
inline int ssgaTypePatch   () { return _SSGA_TYPE_PATCH    | ssgaTypeShape ();}
inline int ssgaTypeTeapot  () { return _SSGA_TYPE_TEAPOT   | ssgaTypeShape ();}
inline int ssgaTypeParticleSystem ()
                        { return _SSGA_TYPE_PARTICLESYSTEM | ssgaTypeShape ();}
inline int ssgaTypeWaveSystem ()
                        { return _SSGA_TYPE_WAVESYSTEM | ssgaTypeShape ();}
inline int ssgaTypeLensFlare ()
                        { return _SSGA_TYPE_LENSFLARE | ssgaTypeShape ();}

void ssgaInit () ;

#endif

