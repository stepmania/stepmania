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

/*******************************************************
 **  ssg3ds.h
 **  
 *   Common data for ssgLoad3ds.cxx and ssgSave3ds.cxx
 *******************************************************/

// 3ds chunk identifiers
static const int CHUNK_VERSION         = 0x0002;
static const int CHUNK_RGB1            = 0x0010;  // 3 floats of RGB
static const int CHUNK_RGB2            = 0x0011;  // 3 bytes of RGB
static const int CHUNK_RGB3            = 0x0012;  // 3 bytes of RGB (gamma)
static const int CHUNK_AMOUNT          = 0x0030;
static const int CHUNK_MAIN            = 0x4D4D;
static const int CHUNK_OBJMESH         = 0x3D3D;
static const int CHUNK_ONEUNIT         = 0x0100;
static const int CHUNK_BKGCOLOR        = 0x1200;
static const int CHUNK_AMBCOLOR        = 0x2100;
static const int CHUNK_OBJBLOCK        = 0x4000;
static const int CHUNK_TRIMESH         = 0x4100;
static const int CHUNK_VERTLIST        = 0x4110;
static const int CHUNK_FACELIST        = 0x4120;
static const int CHUNK_FACEMAT         = 0x4130;
static const int CHUNK_MAPLIST         = 0x4140;
static const int CHUNK_SMOOLIST        = 0x4150;
static const int CHUNK_TRMATRIX        = 0x4160;
static const int CHUNK_LIGHT           = 0x4600;
static const int CHUNK_SPOTLIGHT       = 0x4610;
static const int CHUNK_CAMERA          = 0x4700;
static const int CHUNK_MATERIAL        = 0xAFFF;
static const int CHUNK_MATNAME         = 0xA000;
static const int CHUNK_AMBIENT         = 0xA010;
static const int CHUNK_DIFFUSE         = 0xA020;
static const int CHUNK_SPECULAR        = 0xA030;
static const int CHUNK_SHININESS       = 0xA040;
static const int CHUNK_SHINE_STRENGTH  = 0xA041;
static const int CHUNK_TRANSPARENCY    = 0xA050;
static const int CHUNK_TRANSP_FALLOFF  = 0xA052;
static const int CHUNK_DOUBLESIDED     = 0xA081;
static const int CHUNK_TEXTURE         = 0xA200;
static const int CHUNK_BUMPMAP         = 0xA230;
static const int CHUNK_MAPFILENAME     = 0xA300;
static const int CHUNK_MAPOPTIONS      = 0xA351;
static const int CHUNK_MAP_VSCALE      = 0xA354;
static const int CHUNK_MAP_USCALE      = 0xA356;
static const int CHUNK_MAP_UOFFST      = 0xA358;
static const int CHUNK_MAP_VOFFST      = 0xA35A;
static const int CHUNK_KEYFRAMER       = 0xB000;
static const int CHUNK_FRAMES          = 0xB008;
static const int CHUNK_KEYFRAME_MESH   = 0xB002;
static const int CHUNK_FRAME_OBJNAME   = 0xB010;
static const int CHUNK_FRAME_PIVOT     = 0xB013;
static const int CHUNK_FRAME_POSITION  = 0xB020;
static const int CHUNK_FRAME_ROTATION  = 0xB021;
static const int CHUNK_FRAME_SCALE     = 0xB022;
static const int CHUNK_FRAME_HIERARCHY = 0xB030;
