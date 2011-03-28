#ifndef NETWORKPACKET_H
#define NETWORKPACKET_H

const int MAX_BUFFER_SIZE = 1020; // 1024 - 4 bytes for EzSockets

class NetworkPacket
{
public:
	unsigned char Data[MAX_BUFFER_SIZE];
	int Position;
	uint32_t Size; // Hacky; Only valid for packets read from the network.

	void Clear();

	uint8_t Read1();
	uint16_t Read2();
	uint32_t Read4();
	RString ReadString();	// null-terminated

	void Write1(uint8_t data);
	void Write2(uint16_t data);
	void Write4(uint32_t data);
	void WriteString(const RString& str);	// null-terminated
};

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
