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
#include "RageLog.h"

static int fd = 0;

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

LightsDriver_LinuxSerial::LightsDriver_LinuxSerial()
{
    fd = open("/dev/ttyS0", O_RDWR | O_NOCTTY | O_NDELAY);	// COM1
    if (fd < 0)
		RageException::Throw( "open error %d %s\n", errno, strerror(errno) );

    struct termios my_termios;
    tcgetattr(fd, &my_termios);
    tcflush(fd, TCIFLUSH);
    
    my_termios.c_cflag = B2400 | CS8 |CREAD | CLOCAL | HUPCL;
    
    cfsetospeed(&my_termios, B2400);
    tcsetattr(fd, TCSANOW, &my_termios);

    LOG->Info("Lights info:");
    LOG->Info("  new cflag=%08lx", my_termios.c_cflag);
    LOG->Info("  new oflag=%08lx", my_termios.c_oflag);
    LOG->Info("  new iflag=%08lx", my_termios.c_iflag);
    LOG->Info("  new lflag=%08lx", my_termios.c_lflag);
    // c_line dosen't seem to be defined in every termios.h file
    // Is it really needed in info?
#if 0
    LOG->Info("  new line=%02x", my_termios.c_line);
#endif
}

LightsDriver_LinuxSerial::~LightsDriver_LinuxSerial()
{
	if (fd > 0)
		close(fd);
}

#define NUM_DATA_BYTES 2

char g_data[NUM_DATA_BYTES] = { 0, 0 };

void LightsDriver_LinuxSerial::SetLight( Light light, bool bOn )
{
	int byte_index = light / 8;
	ASSERT( byte_index <= NUM_DATA_BYTES );
	int bit_index = light % 8;
	char &data = g_data[byte_index];
	char mask = 0x01 << bit_index;
	if( bOn )
		data |= mask;
	else
		data &= ~mask;
}

void LightsDriver_LinuxSerial::Flush()
{
	// temp HACK: only write once every 30 times
	static int counter = 0;
	if( counter < 30 )
	{
		counter++;
		return;
	}
	else
	{
		counter = 0;
		// fall through and write
	}

	LOG->Trace( "Entering LightsDriver_LinuxSerial::Flush()" );

	char temp_data_to_write[2+NUM_DATA_BYTES] = "";
	temp_data_to_write[0] = 'Y';
	temp_data_to_write[1] = 'o';
	for( int i=0; i<NUM_DATA_BYTES; i++ )
		temp_data[2+i] = ~g_data[i];	// flip bits so that 0=on, 1=off.

	WriteString( temp_data, sizeof(temp_data) );

	LOG->Trace( "Leaving LightsDriver_LinuxSerial::Flush()" );
}
