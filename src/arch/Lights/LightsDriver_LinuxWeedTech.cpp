#include "global.h"
#include <cstddef>
#include <stdio.h>
#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>

#if defined(HAVE_FCNTL_H)
#include <fcntl.h>
#endif

#include <termios.h>
#include <errno.h>
#include "LightsDriver_LinuxWeedTech.h"
#include "RageLog.h"

REGISTER_LIGHTS_DRIVER_CLASS(LinuxWeedTech);

// Begin serial driver
static int fd = -1;
static LightsState CurLights;

static inline void SerialClose()
{
	if( fd != 1 )
		close( fd );
	fd = -1;
}

static inline void SerialOut( const char *str, std::size_t len )
{
	if( fd ==-1 )
		return;
	while( len )
	{
		ssize_t result = write( fd, str, len );
		if( result == -1 )
		{
			LOG->Trace( "Failed to write to lights driver: %s", strerror(errno) );
			SerialClose();
			return;
		}
		len -= result;
		str += result;
	}
	usleep( 2000 );
}

static inline void SerialOpen()
{
	// Make sure we've not already opened the port
	SerialClose();

	// Open a fresh instance..
	fd = open( "/dev/ttyS0", O_WRONLY | O_NOCTTY | O_NDELAY );
	if( fd < 0 )
	{
		LOG->Warn( "Error opening serial port for lights. Error:: %d %s", errno, strerror(errno) );
	}
	else
	{
		struct termios my_termios;
		tcgetattr( fd, &my_termios );
		tcflush( fd, TCIFLUSH );
		my_termios.c_cflag = B9600 | CS8 | CLOCAL | HUPCL;

		cfsetospeed( &my_termios, B9600 );
		tcsetattr( fd, TCSANOW, &my_termios );
	}
}
// End serial driver //

/* Module maps (Dance mode)
	MODULE #A
		Channel A:	Marquee (Up-Left)
		Channel B:	Marquee (Up-Right)
		Channel C:	Marquee (Down-Left)
		Channel D:	Marquee (Down-Right)
		Channel E:	MenuButtons (P1)
		Channel F:	MenuButtons (P2)
		Channel G:	Bass (Left)
		Channel H:	Bass (Right)
		Channel I:	DancePad P1-Up
		Channel J:	DancePad P1-Down
		Channel K:	DancePad P1-Left
		Channel L:	DancePad P1-Right
		Channel M:	DancePad P2-Up
		Channel N:	DancePad P2-Down

	MODULE #B
		Channel A:	DancePad P2-Left
		Channel B:	DancePad P2-Right
		Channel C:	<not used>
		Channel D:	<not used>
		Channel E:	<not used>
		Channel F:	<not used>
		Channel G:	<not used>
		Channel H:	<not used>
		Channel I:	<not used>
		Channel J:	<not used>
		Channel K:	<not used>
		Channel L:	<not used>
		Channel M:	<not used>
		Channel N:	<not used>
*/

LightsDriver_LinuxWeedTech::LightsDriver_LinuxWeedTech()
{
	// Open port
	SerialOpen();

	// Disable device echoing
	char strinit[5] = { 'A', 'X', '0', 0x0d, 0x00 };
	SerialOut( strinit, 5 );
	strinit[0] = 'B';
	SerialOut( strinit, 5 );
}

LightsDriver_LinuxWeedTech::~LightsDriver_LinuxWeedTech()
{
	// Turn off all lights
	char strkill[5] = { 'A', 'W', '0', 0x0d, 0x00 };
	SerialOut (strkill, 5 );
	strkill[0] = 'B';
	SerialOut( strkill, 5 );

	// Close port
	SerialClose();
}

void LightsDriver_LinuxWeedTech::Set( const LightsState *ls )
{
	// Re-used var's
	char str[6] = { 0x00, 0x00, 0x00, '1', 0x0d, 0x00 };
	bool bOn = false;

	{
		LightsMode lm = LIGHTSMAN->GetLightsMode();
		if( lm == LIGHTSMODE_GAMEPLAY )
		{
			// Since all cabinet lights flash together during gameplay.. If 1 light is on, all are on.
			// However, the player's menu buttons do NOT flash. This section allows us to turn
			// on multiple lights without bogging down the system with delays. ((2ms between commands req.))
			FOREACH_CabinetLight( cl )
			{
				bOn |= ls->m_bCabinetLights[cl];
				CurLights.m_bCabinetLights[cl] = ls->m_bCabinetLights[cl];
			}

			str[0]='A';
			str[1]='W';

			if( bOn )
			{
				str[2]='C';
				str[3]='F';
			}
			else
			{
				str[2]='0';
				str[3]='0';
			}

			// Send command
			puts( str );
			SerialOut( str, 6 );
			return;
		}

		FOREACH_CabinetLight( cl )
		{
			// Only send the command if the light has changed states (on/off)
			bOn = ls->m_bCabinetLights[cl];
			if( bOn != CurLights.m_bCabinetLights[cl] )
			{
				if(cl == LIGHT_MARQUEE_UP_LEFT)		{str[0] = 'A'; str[2] = 'A';}
				else if(cl == LIGHT_MARQUEE_UP_RIGHT)	{str[0] = 'A'; str[2] = 'B';}
				else if(cl == LIGHT_MARQUEE_LR_LEFT)	{str[0] = 'A'; str[2] = 'C';}
				else if(cl == LIGHT_MARQUEE_LR_RIGHT)	{str[0] = 'A'; str[2] = 'D';}
				else if(cl == LIGHT_BASS_LEFT)		{str[0] = 'A'; str[2] = 'G';}
				else if(cl == LIGHT_BASS_RIGHT)		{str[0] = 'A'; str[2] = 'H';}

				if( bOn )
					str[1]='L';
				else
					str[1]='H';

				if( str[0] != 0x00 )
				{
					SerialOut( str, 6 );
					str[0]=0x00;
				}
				CurLights.m_bCabinetLights[cl] = bOn;
			}
		}
	}

	FOREACH_ENUM( GameController,  gc )
	{
		FOREACH_ENUM( GameButton,  gb )
		{
			// Only send the command if the light has changed states (on/off)
			bool bOn = ls->m_bGameButtonLights[gc][gb];
			if(bOn != CurLights.m_bGameButtonLights[gc][gb]) {
				if(gc == GameController_1) {
					if(gb == DANCE_BUTTON_LEFT)		{str[0] = 'A'; str[2] = 'I';}
					if(gb == DANCE_BUTTON_RIGHT)		{str[0] = 'A'; str[2] = 'J';}
					if(gb == DANCE_BUTTON_UP)			{str[0] = 'A'; str[2] = 'K';}
					if(gb == DANCE_BUTTON_DOWN)		{str[0] = 'A'; str[2] = 'L';}
					if(gb == GAME_BUTTON_START)		{str[0] = 'A'; str[2] = 'E';}
				}
				else if(gc == GameController_2) {
					if(gb == DANCE_BUTTON_LEFT)		{str[0] = 'A'; str[2] = 'M';}
					if(gb == DANCE_BUTTON_RIGHT)		{str[0] = 'A'; str[2] = 'N';}
					if(gb == DANCE_BUTTON_UP)			{str[0] = 'B'; str[2] = 'A';}
					if(gb == DANCE_BUTTON_DOWN)		{str[0] = 'B'; str[2] = 'B';}
					if(gb == GAME_BUTTON_START)		{str[0] = 'A'; str[2] = 'F';}
				}

				str[1] = bOn ? 'L' : 'H';

				if( str[0] != 0x00 )
				{
					//SerialOut(str, 6);
					str[0]=0x00;
				}
				CurLights.m_bGameButtonLights[gc][gb] = bOn;
			}
		}
	}
}

/*
 * (c) 2003-2004 Kevin Slaughter
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
 */
