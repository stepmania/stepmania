#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: LightsDriver_LinuxSerial

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "LightsDriver_LinuxSerial.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <errno.h>
#include "adrport.h"

static int fd = 0;

LightsDriver_LinuxSerial::LightsDriver_LinuxSerial()
{
    sprintf(sPortName, "/dev/ttyS1", sPortNumber);

    fd = open(sPortName, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0)
		RageException::Throw( "open error %d %s\n", errno, strerror(errno) );

	struct termios my_termios;
    tcgetattr(fd, &my_termios);
    tcflush(fd, TCIFLUSH);
    
    my_termios.c_cflag = B2400 | CS8 |CREAD | CLOCAL | HUPCL;
    
    cfsetospeed(&my_termios, B2400);
    tcsetattr(fd, TCSANOW, &my_termios);

	LOG->Info("Lights info:");
	LOG->Info("  new cflag=%08x\n", my_termios.c_cflag);
    LOG->Info("  new oflag=%08x\n", my_termios.c_oflag);
    LOG->Info("  new iflag=%08x\n", my_termios.c_iflag);
    LOG->Info("  new lflag=%08x\n", my_termios.c_lflag);
    LOG->Info("  new line=%02x\n", my_termios.c_line);

	WriteString( "Yo", strlen("Yo" );
}

LightsDriver_LinuxSerial::~LightsDriver_LinuxSerial()
{
	if (fd > 0)
		close(fd);
}

void WriteString( char* sz, int len )
{
    int iOut;
    if (fd < 1)
    {
        LOG->Warn("Lights port is not open\n");
        return ;
    }

    iOut = write(fd, sz, len);
    if (iOut < 0)
    {
        LOG->Warn("Lights write error %d %s\n", errno, strerror(errno));
    }

	ASSERT( iOut > 0 );
}

char g_data[2] = { 0, 0 };


void LightsDriver_LinuxSerial::SetLight( Light light, bool bOn )
{
	int byte_index = light / 8;
	int bit_index = light % 8;
	char &data = g_data[byte_index];
	BYTE mask = 0x01 << bit_index;
	if( bOn )
		data |= mask;
	else
		data &= ~mask;

	WriteString( data, sizeof(data) );
}

void LightsDriver_LinuxSerial::Flush()
{
	WriteString( data, sizeof(data) );
}
