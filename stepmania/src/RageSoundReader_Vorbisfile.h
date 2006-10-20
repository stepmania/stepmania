/* RageSoundReader_Vorbisfile - Read from a Vorbisfile interface. */

#ifndef RAGE_SOUND_READER_VORBISFILE_H
#define RAGE_SOUND_READER_VORBISFILE_H

#include "RageSoundReader_FileReader.h"

typedef struct OggVorbis_File OggVorbis_File;
class RageFileBasic;

class RageSoundReader_Vorbisfile: public SoundReader_FileReader
{
public:
	OpenResult Open(RString filename);
	OpenResult Open( RageFileBasic *f );

	int GetLength() const;
	int GetLength_Fast() const;
	int SetPosition_Accurate(int ms) { return SetPosition( ms, true ); }
	int SetPosition_Fast(int ms) { return SetPosition( ms, false ); }
	int Read(char *buf, unsigned len);
	int GetSampleRate() const;
	unsigned GetNumChannels() const { return channels; }
	RageSoundReader_Vorbisfile();
	~RageSoundReader_Vorbisfile();
	RageSoundReader_Vorbisfile *Copy() const;

private:
	OggVorbis_File *vf;
	bool eof;
	int SetPosition( int ms, bool accurate );
	bool FillBuf();
	RString filename;
	int read_offset;
	unsigned channels;
};

#endif

/*
 * Copyright (c) 2003 Glenn Maynard
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

