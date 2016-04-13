/* LightsDriver_Win32Minimaid - Minimaid based lights with libmmmagic */

#ifndef LightsDriver_Win32Minimaid_H
#define LightsDriver_Win32Minimaid_H

#include "LightsDriver.h"

#define DDR_DOUBLE_BASS_LIGHTS			0 //unknown but guessed
#define DDR_DOUBLE_PLAYER1_PANEL		2
#define DDR_DOUBLE_PLAYER2_PANEL		3
#define DDR_DOUBLE_MARQUEE_LOWER_RIGHT	4
#define DDR_DOUBLE_MARQUEE_UPPER_RIGHT	5
#define DDR_DOUBLE_MARQUEE_LOWER_LEFT	6
#define DDR_DOUBLE_MARQUEE_UPPER_LEFT	7

//PADX_LIGHTS
#define DDR_DOUBLE_PAD_UP		0
#define DDR_DOUBLE_PAD_DOWN		1
#define DDR_DOUBLE_PAD_LEFT		2
#define DDR_DOUBLE_PAD_RIGHT	3
#define DDR_DOUBLE_PAD_RESET	4 

#define BIT(i) (1<<(i))
#define BIT_IS_SET(v,i) ((v&BIT(i))!=0)

static bool _mmmagic_loaded=false;

typedef bool (*__BITVALID)(int);
static __BITVALID __bitValid;

// minimaid prototypes
typedef void (*MM_SETDDRPAD1LIGHT)(int, int);
typedef void (*MM_SETDDRPAD2LIGHT)(int, int);
typedef void (*MM_SETCABINETLIGHT)(int, int);
typedef void (*MM_SETDDRBASSLIGHT)(int, int);
static MM_SETDDRPAD1LIGHT mm_setDDRPad1Light;
static MM_SETDDRPAD2LIGHT mm_setDDRPad2Light;
static MM_SETCABINETLIGHT mm_setDDRCabinetLight;
static MM_SETDDRBASSLIGHT mm_setDDRBassLight;

typedef bool (*MM_CONNECT_MINIMAID)();
typedef bool (*MM_SETKB)(bool val);
static MM_CONNECT_MINIMAID mm_connect_minimaid;
static MM_SETKB mm_setKB;

typedef void (*MM_SETDDRALLON)();
typedef void (*MM_SETDDRALLOFF)();
static MM_SETDDRALLON mm_setDDRAllOn;
static MM_SETDDRALLOFF mm_setDDRAllOff;

typedef void (*MM_SETBLUELED)(unsigned char);
typedef void (*MM_SETMMOUTPUTREPORTS)(unsigned char, unsigned char, unsigned char, unsigned char);
typedef bool (*MM_SENDDDRMINIMAIDUPDATE)();
static MM_SETBLUELED mm_setBlueLED;
static MM_SETMMOUTPUTREPORTS mm_setMMOutputReports;
static MM_SENDDDRMINIMAIDUPDATE mm_sendDDRMiniMaidUpdate;

typedef void (*MM_INIT)();
typedef void (*MM_TURNON)(unsigned char, int);
typedef bool (*MM_TURNOFF)(unsigned char, int);
static MM_INIT mm_init;
static MM_TURNON mm_turnON;
static MM_TURNOFF mm_turnOFF;

class LightsDriver_Win32Minimaid : public LightsDriver
{
public:
	LightsDriver_Win32Minimaid();

	virtual ~LightsDriver_Win32Minimaid();
	virtual void Set( const LightsState *ls );
};

#endif

/*
 * (c) 2013 pkgingo
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE. 
 * 
 * (c) 2016- Electromuis, Anton Grootes
 * This branch of https://github.com/stepmania/stepmania
 * will from here on out be released as GPL v3 (wich converts from the previous MIT license)
 */
