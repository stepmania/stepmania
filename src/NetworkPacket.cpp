#include "global.h"
#include "NetworkPacket.h"
#include "ezsockets.h"

#if defined(WITHOUT_NETWORKING)
void NetworkPacket::Clear() { }
uint8_t NetworkPacket::Read1() { }
uint16_t NetworkPacket::Read2() { }
uint32_t NetworkPacket::Read4() { }
RString NetworkPacket::ReadString() { }
void NetworkPacket::Write1(uint8_t data) { }
void NetworkPacket::Write2(uint16_t data) { }
void NetworkPacket::Write4(uint32_t data) { }
NetworkPacket::WriteString(const RString& str) { }
#else
void NetworkPacket::Clear()
{
	memset((void*)(&Data),0, MAX_BUFFER_SIZE);
	Position = 0;
	Size = 0;
}

// read functions
uint8_t NetworkPacket::Read1()
{
	if( Position >= MAX_BUFFER_SIZE )
		return 0;

	return Data[Position++];
}

uint16_t NetworkPacket::Read2()
{
	if( Position >= MAX_BUFFER_SIZE-1 )
		return 0;

	uint16_t Temp;
	memcpy( &Temp, Data + Position, 2 );
	Position+=2;
	return ntohs(Temp);
}

uint32_t NetworkPacket::Read4()
{
	if( Position >= MAX_BUFFER_SIZE-3 )
		return 0;

	uint32_t Temp;
	memcpy( &Temp, Data + Position,4 );
	Position+=4;
	return ntohl(Temp);
}

RString NetworkPacket::ReadString()
{
	RString TempStr;
	while( (Position < MAX_BUFFER_SIZE) && ( ((char*)Data)[Position]!=0) )
		TempStr = TempStr + (char)Data[Position++];

	++Position;
	return TempStr;
}

// write functions
void NetworkPacket::Write1(uint8_t data)
{
	if(Position >= MAX_BUFFER_SIZE)
		return;
	memcpy( &Data[Position], &data, 1 );
	++Position;
}

void NetworkPacket::Write2(uint16_t data)
{
	if(Position >= MAX_BUFFER_SIZE-1)
		return;
	data = htons(data);
	memcpy( &Data[Position], &data, 2 );
	Position+=2;
}

void NetworkPacket::Write4(uint32_t data)
{
	if(Position >= MAX_BUFFER_SIZE-3)
		return;
	data = htonl(data);
	memcpy( &Data[Position], &data, 4 );
	Position+=4;
}

void NetworkPacket::WriteString(const RString& str)
{
	unsigned int index=0;
	while( (Position < MAX_BUFFER_SIZE) && (index < str.length()) )
		Data[Position++] = (unsigned char)(str.c_str()[index++]);
	Data[Position++] = 0;
}
#endif

/*
 * (c) 2011 AJ Kelly
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
