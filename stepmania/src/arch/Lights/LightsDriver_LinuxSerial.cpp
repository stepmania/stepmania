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
#include "RageTimer.h"

static int fd = -1;

void WriteString( const char *sz, int len )
{
    int iOut;
    if( fd == -1 )
    {
        LOG->Warn("Lights port is not open\n");
        return;
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
    fd = open("/dev/ttyS0", O_RDWR | O_NOCTTY);	// COM1
    if (fd < 0)
		RageException::Throw( "open error %d %s\n", errno, strerror(errno) );

    tcflush(fd, TCIFLUSH);

    struct termios my_termios;
    tcgetattr(fd, &my_termios);
    
//	my_termios.c_iflag = IGNBRK;
//    my_termios.c_lflag = 0;
//    my_termios.c_oflag = 0;
//    my_termios.c_cflag |= CREAD | CLOCAL;
    my_termios.c_cflag &= ~CRTSCTS;
	my_termios.c_iflag &= ~(IXON | IXOFF | IXANY); /* no flow control */
//	my_termios.sg_ispeed = my_termios.sg_ispeed = B2400;

//    my_termios.c_cflag = B2400 | CS8 | CREAD | CLOCAL | HUPCL;
//	my_termios.c_oflag &= ~0; /* no flow control */
//	fcntl(fd, F_SETFL, 0);

    cfsetispeed(&my_termios, B2400);
    cfsetospeed(&my_termios, B2400);
    tcsetattr(fd, TCSANOW, &my_termios);
    LOG->Info("Lights info:");
    LOG->Info("  new cflag=%08lx", my_termios.c_cflag);
    LOG->Info("  new oflag=%08lx", my_termios.c_oflag);
    LOG->Info("  new iflag=%08lx", my_termios.c_iflag);
    LOG->Info("  new lflag=%08lx", my_termios.c_lflag);
}

LightsDriver_LinuxSerial::~LightsDriver_LinuxSerial()
{
	if( fd != -1 )
		close( fd );
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
	static RageTimer tm;
	if( tm.PeekDeltaTime() < 0.02f )
		return;
	tm.Touch();

//	LOG->Trace( "Entering LightsDriver_LinuxSerial::Flush()" );

	char temp_data_to_write[2+NUM_DATA_BYTES] = "";
	temp_data_to_write[0] = 'Y';
	temp_data_to_write[1] = 'o';

	for( int i=0; i<NUM_DATA_BYTES; i++ )
		temp_data_to_write[2+i] = ~g_data[i];	// flip bits so that 0=on, 1=off.

	RageTimer testing;
	WriteString( temp_data_to_write, sizeof(temp_data_to_write) );
	if( testing.PeekDeltaTime() > 0.1f )
		LOG->Warn("WriteString took %F", testing.GetDeltaTime());

//	LOG->Trace( "Leaving LightsDriver_LinuxSerial::Flush()" );
}
