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

//===========================================================================
//                                                                           
// File: fsPalettes.h                                                        
//                                                                           
// Created: Fri Oct  1 15:34:29 1999                                         
//                                                                           
// Author: Thomas Engh Sevaldrud <tse@math.sintef.no>
//                                                                           
// Revision: $Id$
//                                                                           
// Description:
//                                                                           
//===========================================================================
// Copyright (c) 1999 Thomas E. Sevaldrud <tse@math.sintef.no>
//===========================================================================

#ifndef _SSGLOADMDL_H
#define _SSGLOADMDL_H



//===========================================================================
// Aircraft color palette. The aircraft in MSFS has
// a different palette than the default fs5.pal file.
// Entries with (?) means that I am not sure of the
// values. Entries with (L), means that the color is
// emissive (bright at night).

struct acColor
{
   unsigned char r, g, b, emissive;
};

static const acColor fsAcPalette[53] = {
    { 0,   0,   0   , 0 },		// Black
    { 89,  89,  89  , 0 },		// Dark gray
    { 169, 169, 169 , 0 },		// Gray
    { 211, 211, 211 , 0 },		// Light gray
    { 255, 255, 255 , 0 },		// White
    { 255, 0,   0   , 0 },		// Red
    { 0,   255, 0   , 0 },		// Green
    { 0,   0,   255 , 0 },		// Blue
    { 255, 165, 0   , 0 },              // Orange
    { 255, 255, 0   , 0 },		// Yellow
    { 165, 42,  42  , 0 },		// Brown
    { 210, 180, 140 , 0 },		// Tan
    { 255, 69,  0   , 0 },		// Rust (?)
    { 107, 142, 35  , 0 },		// Olive (?)
    { 0,   87,  112 , 0 },		// Water
    { 255, 0,   0   , 1 },		// Red (L)
    { 0,   255, 0   , 1 },		// Green (L)
    { 0,   0,   255 , 1 },		// Blue (L)
    { 57,  57,  112 , 1 },              // Aqua (L) (?)
    { 255, 165, 0   , 1 },              // Orange (L)
    { 255, 255, 0   , 1 },		// Yellow (L)
    { 255, 255, 255 , 1 },		// White (L)
    { 255, 255, 255 , 1 },		// White (L)
    { 139, 0,   0   , 0 }, 		// Dark red
    { 0,   100, 0   , 0 },		// Dark green
    { 0,   0,   139 , 0 },		// Dark blue
    { 205, 120, 0   , 0 },		// Dark orange
    { 139, 100, 0   , 0 },		// Dark yellow (?)
    { 82,  21,  21  , 0 },		// Dark brown (?)
    { 105, 90,  70  , 0 },		// Dark tan
    { 128, 35,  0   , 0 },              // Dark rust (?)
    { 53,  71,  17  , 0 },		// Dark olive (?)
    { 205, 0,   0   , 0 },		// Medium red
    { 0,   179, 0   , 0 },		// Medium green
    { 0,   0,   205 , 0 },		// Medium blue
    { 255, 140, 0   , 0 },		// Medium orange (?)
    { 205, 179, 0   , 0 },		// Medium yellow (?)
    { 120, 32,  32  , 0 },		// Medium brown (?)
    { 155, 135, 105 , 0 },		// Medium tan (?)
    { 186, 52,  0   , 0 },		// Medium rust (?)
    { 80,  106, 25  , 0 },		// Medium olive (?)
    { 238, 173, 173 , 0 },		// Light red (?)
    { 144, 238, 144 , 0 },		// Light green
    { 173, 216, 230 , 0 },		// Light Blue
    { 255, 162, 68  , 0 },		// Light Orange (?)
    { 255, 255, 224 , 0 },		// Light Yellow
    { 255, 130, 58  , 0 },		// Light Brown (?)
    { 255, 220, 180 , 0 },		// Light Tan (?)
    { 255, 137, 0   , 0 },		// Light rust (?)
    { 157, 200, 54  , 0 },		// Light olive (?)
    { 89,  89,  89  , 1 },		// Dark gray (?) (L)
    { 169, 169, 169 , 1 },              // Gray (L)
    { 211, 211, 211 , 1 }		// Light Gray (L)
};

//===========================================================================
// Alternative palette for transparent colors (?)
// (same as in fs5.pal?)

static const acColor fsAltPalette[113] = {
    { 0, 0, 0},
    { 8, 8, 8},
    { 16, 16, 16},
    { 24, 24, 24},
    { 32, 32, 32},
    { 40, 40, 40},
    { 48, 48, 48},
    { 56, 56, 56},
    { 65, 65, 65},
    { 73, 73, 73},
    { 81, 81, 81},
    { 89, 89, 89},
    { 97, 97, 97},
    { 105, 105, 105},
    { 113, 113, 113},
    { 121, 121, 121},
    { 130, 130, 130},
    { 138, 138, 138},
    { 146, 146, 146},
    { 154, 154, 154},
    { 162, 162, 162},
    { 170, 170, 170},
    { 178, 178, 178},
    { 186, 186, 186},
    { 195, 195, 195},
    { 203, 203, 203},
    { 211, 211, 211},
    { 219, 219, 219},
    { 227, 227, 227},
    { 235, 235, 235},
    { 247, 247, 247},
    { 255, 255, 255},
    { 21, 5, 5},
    { 42, 10, 10},
    { 63, 15, 15},
    { 84, 20, 20},
    { 105, 25, 25},
    { 126, 30, 30},
    { 147, 35, 35},
    { 168, 40, 40},
    { 189, 45, 45},
    { 210, 50, 50},
    { 231, 55, 55},
    { 252, 60, 60},
    { 5, 21, 5},
    { 10, 42, 10},
    { 15, 63, 15},
    { 20, 84, 20},
    { 25, 105, 25},
    { 30, 126, 30},
    { 35, 147, 35},
    { 40, 168, 40},
    { 45, 189, 45},
    { 50, 210, 50},
    { 55, 231, 55},
    { 60, 252, 60},
    { 0, 7, 23},
    { 0, 15, 40},
    { 0, 23, 58},
    { 0, 40, 84},
    { 0, 64, 104},
    { 0, 71, 122},
    { 0, 87, 143},
    { 0, 99, 156},
    { 0, 112, 179},
    { 0, 128, 199},
    { 0, 143, 215},
    { 0, 153, 230},
    { 28, 14, 0},
    { 56, 28, 0},
    { 84, 42, 0},
    { 112, 56, 0},
    { 140, 70, 0},
    { 168, 84, 0},
    { 196, 98, 0},
    { 224, 112, 0},
    { 252, 126, 0},
    { 28, 28, 0},
    { 56, 56, 0},
    { 84, 84, 0},
    { 112, 112, 0},
    { 140, 140, 0},
    { 168, 168, 0},
    { 196, 196, 0},
    { 224, 224, 0},
    { 252, 252, 0},
    { 25, 21, 16},
    { 50, 42, 32},
    { 75, 63, 48},
    { 100, 84, 64},
    { 125, 105, 80},
    { 150, 126, 96},
    { 175, 147, 112},
    { 200, 168, 128},
    { 225, 189, 144},
    { 28, 11, 7},
    { 56, 22, 14},
    { 84, 33, 21},
    { 112, 44, 28},
    { 140, 55, 35},
    { 168, 66, 42},
    { 196, 77, 49},
    { 224, 88, 56},
    { 252, 99, 63},
    { 17, 22, 9},
    { 34, 44, 18},
    { 51, 66, 27},
    { 68, 88, 36},
    { 85, 110, 45},
    { 102, 132, 54},
    { 119, 154, 63},
    { 136, 176, 72},
    { 153, 198, 81}
};

struct _ssgBGLOpCode {
  unsigned short opcode;
  const char     *name;
  int            size;   /* size includes opcode (2 bytes)
			    -1 indicates that special treatment is needed
			    to find the size */
                        
};

static const _ssgBGLOpCode opcodes[] = {
  { 0x00, "BGL_EOF"                             , 2  },
  { 0x01, "OBSOLETE 0x01"                       , 2  },
  { 0x02, "BGL_NOOP"                            , 2  },
  { 0x03, "BGL_CASE"                            , -1 },
  { 0x04, "RESERVED 0x04"                       , 2  },
  { 0x05, "BGL_SURFACE - Area"                  , 2  },
  { 0x06, "BGL_SPNT"                            , 8  },
  { 0x07, "BGL_CPNT"                            , 8  },
  { 0x08, "BLG_CLOSURE"                         , 2  },
  { 0x09, "OBSOLETE 0x09"                       , 2  },
  { 0x0a, "OBSOLETE 0x0a"                       , 10 },
  { 0x0b, "OBSOLETE 0x0b"                       , 10 },
  { 0x0c, "OBSOLETE 0x0c"                       , 2  },
  { 0x0d, "BGL_JUMP"                            , 4  },
  { 0x0e, "BGL_DEFRES"                          , 8  },
  { 0x0f, "BGL_STRRES"                          , 4  },
  { 0x10, "BGL_CNTRES"                          , 4  },
  { 0x11, "OBSOLETE 0x11"                       , 10 },
  { 0x12, "OBSOLETE 0x12"                       , 4  },
  { 0x13, "OBSOLETE 0x13"                       , 4  },
  { 0x14, "BGL_SCOLOR"                          , 4  },
  { 0x15, "BGL_ELEVATION_MAP"                   , -1 },
  { 0x16, "RESERVED 0x16"                       , -1 },
  { 0x17, "BGL_TEXTURE_ENABLE"                  , 4  },
  { 0x18, "BGL_TEXTURE"                         , 24 },
  { 0x19, "BGL_PALETTE"                         , 16 },
  { 0x1a, "BGL_RESLIST"                         , -1 },
  { 0x1b, "BGL_IFIN_BOX_RAW_PLANE"              , 22 },
  { 0x1c, "BGL_IFIN2"                           , 16 },
  { 0x1d, "BGL_FACE"                            , -1 },
  { 0x1e, "BGL_HAZE"                            , 4  },
  { 0x1f, "RESERVED 0x1f"                       , 4  },
  { 0x20, "BGL_FACET_TMAP"                      , -1 },
  { 0x21, "BGL_IFIN3"                           , 22 },
  { 0x22, "BGL_RETURN - Return"                 , 2  },
  { 0x23, "BGL_CALL"                            , 4  },
  { 0x24, "BGL_IFIN1"                           , 10 },
  { 0x25, "BGL_SEPARATION_PLANE"                , 14 },
  { 0x26, "BGL_SETWRD"                          , 6  },
  { 0x27, "OBSOLETE 0x27"                       , -1 },
  { 0x28, "OBSOLETE 0x28"                       , 10 },
  { 0x29, "BGL_GRESLIST"                        , -1 },
  { 0x2a, "BGL_GFACET"                          , -1 },
  { 0x2b, "BGL_ADDOBJ32"                        , 6  },
  { 0x2c, "BGL_REJECT"                          , 12 },
  { 0x2d, "BGL_SCOLOR24"                        , 6  },
  { 0x2e, "BGL_LCOLOR24"                        , 6  },
  { 0x2f, "BGL_SCALE"                           , 32 },
  { 0x30, "OBSOLETE 0x30"                       , 4  },
  { 0x31, "BGL_RESROW"                          , 18 },
  { 0x32, "BGL_ADDOBJ - PerspectiveCall"        , 4  },
  { 0x33, "BGL_INSTANCE - RotatedCall"          , 10 },
  { 0x34, "BGL_SUPER_SCALE"                     , 10 },
  { 0x35, "BGL_PNTROW"                          , 16 },
  { 0x36, "OBSOLETE 0x36"                       , -1 },
  { 0x37, "BGL_POINT"                           , 8  },
  { 0x38, "BGL_CONCAVE"                         , 2  },
  { 0x39, "BGL_IFMSK"                           , 8  },
  { 0x3a, "BGL_VPOSITION"                       , 12 },
  { 0x3b, "BGL_VINSTANCE"                       , 6  },
  { 0x3c, "BGL_POSITION"                        , 28 },
  { 0x3d, "BGL_SEED"                            , 24 },
  { 0x3e, "BGL_FACET"                           , -1 },
  { 0x3f, "BGL_SHADOW_CALL"                     , 4  },
  { 0x40, "BGL_SHADOW_VPOSITION"                , 12 },
  { 0x41, "BGL_SHADOW_VICALL"                   , 6  },
  { 0x42, "BGL_POLYGON_RUNWAY"                  , 40 },
  { 0x43, "BGL_TEXTURE2"                        , -1 },
  { 0x44, "BGL_TEXTURE_RUNWAY"                  , 64 }, // 40
  { 0x45, "OBSOLETE 0x45"                       , 16 },
  { 0x46, "BGL_POINT_VICALL"                    , 22 },
  { 0x47, "RESERVED"                            , 4  },
  { 0x48, "BGL_VAR_SEG"                         , 4  },
  { 0x49, "BGL_BUILDING"                        , 18 },
  { 0x4a, "OBSOLETE 0x4A"                       , 22 },
  { 0x4b, "OBSOLETE 0x4B"                       , -1 },
  { 0x4c, "BGL_VSCALE"                          , 16 },
  { 0x4d, "BGL_MOVEL2G"                         , 6  },
  { 0x4e, "BGL_MOVEG2L"                         , 6  },
  { 0x4f, "BGL_MOVEWORD"                        , 6  },
  { 0x50, "BGL_GCOLOR"                          , 4  },
  { 0x51, "BGL_NEW_LCOLOR"                      , 4  },
  { 0x52, "BGL_NEW_SCOLOR"                      , 4  },
  { 0x53, "OBSOLETE 0x53"                       , 4  },
  { 0x54, "OBSOLETE 0x54"                       , 6  },
  { 0x55, "BGL_SURFACE_TYPE"                    , 10 },
  { 0x56, "BGL_SET_WEATHER"                     , 4  },
  { 0x57, "BGL_SET_WEATHER"                     , 10 },
  { 0x58, "BLG_TEXTURE_BOUNDS"                  , 10 },
  { 0x59, "OBSOLETE 0x59"                       , 4  },
  { 0x5a, "OBSOLETE 0x5a"                       , 4  },
  { 0x5b, "OBSOLETE 0x5b"                       , 4  },
  { 0x5c, "OBSOLETE 0x5c"                       , 6  },
  { 0x5d, "BGL_TEXTURE_REPEAT"                  , 8  },
  { 0x5e, "OBSOLETE 0x5e"                       , 4  },
  { 0x5f, "BGL_IFSIZEV"                         , 8  },
  { 0x60, "BGL_FACE_TMAP"                       , -1 },
  { 0x61, "RESERVED 0x53"                       , 6  },
  { 0x62, "BGL_IFVIS"                           , -1 },
  { 0x63, "BGL_LIBRARY_CALL"                    , 20 },
  { 0x64, "BGL_LIST"                            , -1 },
  { 0x65, "BGL_VSCOLOR"                         , 4  },
  { 0x66, "BGL_VGCOLOR"                         , 4  },
  { 0x67, "BGL_VLCOLOR"                         , 4  },
  { 0x68, "OBSOLETE"                            , 8  },
  { 0x69, "BGL_ROAD_START"                      , 10 },
  { 0x6a, "BGL_ROAD_CONT"                       , 8  },
  { 0x6b, "BGL_RIVER_START"                     , 10 },
  { 0x6c, "BGL_RIVER_CONT"                      , 8  },
  { 0x6d, "BGL_IFSIZEH"                         , 8  },
  { 0x6e, "BGL_TAXIWAY_START"                   , 10 },
  { 0x6f, "BGL_TAXIWAY_CONT"                    , 8  },
  { 0x70, "BGL_AREA_SENSE"                      , -1 },
  { 0x71, "BGL_ALTITUDE_SET"                    , 4  },
  { 0x72, "OBSOLETE 0x72"                       , 20 },
  { 0x73, "BGL_IFINBOXP"                        , 16 },
  { 0x74, "BGL_ADDCAT"                          , 6  },
  { 0x75, "BGL_ADDMNT"                          , 4  },
  { 0x76, "BGL_BGL - Perspective"               , 2  },
  { 0x77, "BGL_SCALE_AGL - RefPoint"            , 32 },
  { 0x78, "BGL_ROAD_CONTW"                      , 10 },
  { 0x79, "BGL_RIVER_CONTW"                     , 10 },
  { 0x7a, "BGL_GFACET_TMAP"                     , -1 },
  { 0x7b, "OBSOLETE 0x7b"                       , -1 },
  { 0x7c, "BGL_SELECT"                          , 10 },
  { 0x7d, "BGL_PERSPECTIVE"                     , 2  },
  { 0x7e, "BGL_SETWRD_GLOBAL"                   , 6  },
  { 0x7f, "OBSOLETE 0x7f"                       , 2  },
  { 0x80, "BGL_RESPNT"                          , 4  },
  { 0x81, "OBSOLETE 0x81"                       , 4  },
  { 0x82, "OBSOLETE 0x82"                       , 28 },
  { 0x83, "BGL_RESCALE"                         , 32 },
  { 0x84, "OBSOLETE 0x84"                       , 8  },
  { 0x85, "OBSOLETE 0x85"                       , 2  },
  { 0x86, "OBSOLETE 0x86"                       , 3  },
  { 0x87, "BGL_FIXED_COLORS"                    , 4  },
  { 0x88, "BGL_JUMP32"                          , 6  },
  { 0x89, "BGL_VAR_BASE32"                      , 6  },
  { 0x8a, "BGL_CALL32"                          , 6  },
  { 0x8b, "BGL_ADDCAT32"                        , 8  },
  { 0x8c, "RESERVED 0x8c"                       , 6  },
  { 0x8d, "RESERVED 0x8d"                       , 6  },
  { 0x8e, "BGL_VFILE_MARKER"                    , 4  },
  { 0x8f, "BGL_ALPHA"                           , 6  },
  { 0x90, "RESERVED 0x90"                       , -1 },
  { 0x91, "BGL_TEXT"                            , 12 },
  { 0x92, "OBSOLETE 0x92"                       , 4  },
  { 0x93, "RESERVED 0x93"                       , 4  },
  { 0x94, "BGL_CRASH"                           , 12 },
  { 0x95, "BGL_CRASH_INDIRECT"                  , 10 },
  { 0x96, "BGL_CRASH_START"                     , 6  },
  { 0x97, "BGL_CRASH_SPHERE"                    , 6  },
  { 0x98, "BGL_CRASH_BOX"                       , 22 },
  { 0x99, "BGL_SET_CRASH"                       , 4  },
  { 0x9a, "OBSOLETE 0x9a"                       , -1 },
  { 0x9b, "RESERVED 0x9b"                       , 8  },
  { 0x9c, "RESERVED 0x9c"                       , 6  },
  { 0x9d, "RESERVED 0x9d"                       , 14 },
  { 0x9e, "BGL_INTERPOLATE"                     , 20 },
  { 0x9f, "BGL_OVERRIDE"                        , 6  },
  { 0xa0, "BGL_OBJECT"                          , -1 },
  { 0xa1, "OBSOLETE 0xA1"                       , 10 },
  { 0xa2, "OBSOLETE 0xA2"                       , 8  },
  { 0xa3, "OBSOLETE 0xA3"                       , -1 },
  { 0xa4, "BGL_VALPHA"                          , 4  },
  { 0xa5, "OBSOLETE 0xA5"                       , 4  },
  { 0xa6, "OBSOLETE 0xA6"                       , 8  },
  { 0xa7, "BGL_SPRITE_VCALL"                    , 22 },
  { 0xa8, "BGL_TEXTURE_ROAD_START"              , 12},
  { 0xa9, "BGL_IFIN_INSTANCE_BOX_PLANE"         , 22},
  { 0xaa, "BGL_NEW_RUNWAY"                      , -1 },
  { 0xab, "RESERVED 0xAB"                       , 38 },
  { 0xac, "BGL_ZBIAS"                           , 4  },
  { 0xad, "BGL_ANIMATE"                         , 30 },
  { 0xae, "BGL_TRANSFORM_END"                   , 2  },
  { 0xaf, "BGL_TRANSFORM_MATRIX"                , 50 },
  { 0xb0, "UNKNOWN 0xB0"                        , -1 },
  { 0xb1, "UNKNOWN 0xB1"                        , -1 },
  { 0xb2, "BGL_LIGHT"                           , 44 },
  { 0xb3, "BGL_IFINF1"                          , 14 },
  { 0xb4, "UNKNOWN 0xB4"                        , -1 },
  { 0xb5, "UNKNOWN 0xB5"                        , -1 },
  { 0xb6, "UNKNOWN 0xB6"                        , -1 },
  { 0xb7, "UNKNOWN 0xB7"                        , -1 },
  { 0xb8, "UNKNOWN 0xB8"                        , -1 },
  { 0xb9, "UNKNOWN 0xB9"                        , -1 },
  { 0xba, "UNKNOWN 0xBA"                        , -1 },
  { 0xbb, "UNKNOWN 0xBB"                        , -1 },
  { 0xbc, "UNKNOWN 0xBC"                        , -1 },
  { 0xbd, "UNKNOWN 0xBD"                        , -1 },
  { 0xbe, "UNKNOWN 0xBE"                        , -1 },
  { 0xbf, "UNKNOWN 0xBF"                        , -1 },
  { 0xc0, "UNKNOWN 0xC0"                        , -1 },
  { 0xc1, "UNKNOWN 0xC1"                        , -1 },
  { 0xc2, "UNKNOWN 0xC2"                        , -1 },
  { 0xc3, "UNKNOWN 0xC3"                        , -1 },
  { 0xc4, "UNKNOWN 0xC4"                        , -1 },
  { 0xc5, "UNKNOWN 0xC5"                        , -1 },
  { 0xc6, "UNKNOWN 0xC6"                        , -1 },
  { 0xc7, "UNKNOWN 0xC7"                        , -1 },
  { 0xc8, "UNKNOWN 0xC8"                        , -1 },
  { 0xc9, "UNKNOWN 0xC9"                        , -1 },
  { 0xca, "UNKNOWN 0xCA"                        , -1 },
  { 0xcb, "UNKNOWN 0xCB"                        , -1 },
  { 0xcc, "UNKNOWN 0xCC"                        , -1 },
  { 0xcd, "UNKNOWN 0xCD"                        , -1 },
  { 0xce, "UNKNOWN 0xCE"                        , -1 },
  { 0xcf, "UNKNOWN 0xCF"                        , -1 },
  { 0xd0, "UNKNOWN 0xD0"                        , -1 },
  { 0xd1, "UNKNOWN 0xD1"                        , -1 },
  { 0xd2, "UNKNOWN 0xD2"                        , -1 },
  { 0xd3, "UNKNOWN 0xD3"                        , -1 },
  { 0xd4, "UNKNOWN 0xD4"                        , -1 },
  { 0xd5, "UNKNOWN 0xD5"                        , -1 },
  { 0xd6, "UNKNOWN 0xD6"                        , -1 },
  { 0xd7, "UNKNOWN 0xD7"                        , -1 },
  { 0xd8, "UNKNOWN 0xD8"                        , -1 },
  { 0xd9, "UNKNOWN 0xD9"                        , -1 },
  { 0xda, "UNKNOWN 0xDA"                        , -1 },
  { 0xdb, "UNKNOWN 0xDB"                        , -1 },
  { 0xdc, "UNKNOWN 0xDC"                        , -1 },
  { 0xdd, "UNKNOWN 0xDD"                        , -1 },
  { 0xde, "UNKNOWN 0xDE"                        , -1 },
  { 0xdf, "UNKNOWN 0xDF"                        , -1 },
  { 0xe0, "UNKNOWN 0xE0"                        , -1 },
  { 0xe1, "UNKNOWN 0xE1"                        , -1 },
  { 0xe2, "UNKNOWN 0xE2"                        , -1 },
  { 0xe3, "UNKNOWN 0xE3"                        , -1 },
  { 0xe4, "UNKNOWN 0xE4"                        , -1 },
  { 0xe5, "UNKNOWN 0xE5"                        , -1 },
  { 0xe6, "UNKNOWN 0xE6"                        , -1 },
  { 0xe7, "UNKNOWN 0xE7"                        , -1 },
  { 0xe8, "UNKNOWN 0xE8"                        , -1 },
  { 0xe9, "UNKNOWN 0xE9"                        , -1 },
  { 0xea, "UNKNOWN 0xEA"                        , -1 },
  { 0xeb, "UNKNOWN 0xEB"                        , -1 },
  { 0xec, "UNKNOWN 0xEC"                        , -1 },
  { 0xed, "UNKNOWN 0xED"                        , -1 },
  { 0xee, "UNKNOWN 0xEE"                        , -1 },
  { 0xef, "UNKNOWN 0xEF"                        , -1 },
  { 0xf0, "UNKNOWN 0xF0"                        , -1 },
  { 0xf1, "UNKNOWN 0xF1"                        , -1 },
  { 0xf2, "UNKNOWN 0xF2"                        , -1 },
  { 0xf3, "UNKNOWN 0xF3"                        , -1 },
  { 0xf4, "UNKNOWN 0xF4"                        , -1 },
  { 0xf5, "UNKNOWN 0xF5"                        , -1 },
  { 0xf6, "UNKNOWN 0xF6"                        , -1 },
  { 0xf7, "UNKNOWN 0xF7"                        , -1 },
  { 0xf8, "UNKNOWN 0xF8"                        , -1 },
  { 0xf9, "UNKNOWN 0xF9"                        , -1 },
  { 0xfa, "UNKNOWN 0xFA"                        , -1 },
  { 0xfb, "UNKNOWN 0xFB"                        , -1 },
  { 0xfc, "UNKNOWN 0xFC"                        , -1 },
  { 0xfd, "UNKNOWN 0xFD"                        , -1 },
  { 0xfe, "UNKNOWN 0xFE"                        , -1 },
  { 0xff, "UNKNOWN 0xFF"                        , -1 }
};

//===========================================================================

#endif // _SSGLOADMDL_H

