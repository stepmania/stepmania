/*
 * RageSoundReader_WAV - WAV reader
 */
#ifndef RAGE_SOUND_READER_WAV_H
#define RAGE_SOUND_READER_WAV_H

#include "RageSoundReader_FileReader.h"
#include "RageFile.h"

class RageSoundReader_WAV: public SoundReader_FileReader
{
    mutable RageFile rw;
	struct
	{
		int16_t wFormatTag;
		uint16_t wChannels;
		uint32_t dwAvgBytesPerSec;
		uint16_t wBlockAlign, wBitsPerSample;

		uint32_t adpcm_sample_frame_size;
		uint32_t data_starting_offset;
	} fmt;

	struct ADPCMBLOCKHEADER {
		uint8_t bPredictor;
		uint16_t iDelta;
		int16_t iSamp[2];
	};
	struct adpcm_t
	{
		uint16_t cbSize;
		uint16_t wSamplesPerBlock;
		vector<int16_t> Coef1, Coef2;

		ADPCMBLOCKHEADER blockheaders[2]; /* 2 channels */
		uint32_t samples_left_in_block;
		int nibble_state;
		int8_t nibble;

		adpcm_t();
	};
	adpcm_t adpcm;
	CString filename;

	enum DataType_t { FORMAT_PCM=0, FORMAT_ADPCM=1 } DataType;

	int SampleRate;
	int Channels;
	int BytesPerSample;

	/* Number of bytes to read to get one output buffer byte.  If converting from 8-
	 * to 16-bit, *2; if 1- to 2-channel, another *2. */
	int Input_Buffer_Ratio;

	enum { CONV_NONE, CONV_8BIT_TO_16BIT, CONV_16LSB_TO_16SYS } Conversion;

	int read_sample_fmt_normal( char *buf, unsigned len );
	bool read_le16( RageFile &f, int16_t *si16 ) const;
	bool read_le16( RageFile &f, uint16_t *ui16 ) const;
	bool read_le32( RageFile &f, int32_t *si32 ) const;
	bool read_le32( RageFile &f, uint32_t *ui32 ) const;
	bool read_uint8( RageFile &f, uint8_t *ui8 ) const;

	bool read_adpcm_block_headers( adpcm_t &out ) const;
	bool decode_adpcm_sample_frame();
	uint32_t read_sample_fmt_adpcm( char *buf, unsigned len );
	void do_adpcm_nibble( uint8_t nib, ADPCMBLOCKHEADER *header, int32_t lPredSamp );
	void put_adpcm_sample_frame( uint16_t *buf, int frame );

	int seek_sample_fmt_adpcm( uint32_t ms );
	int get_length_fmt_adpcm() const;
	int find_chunk( uint32_t id, int32_t &size );
	bool read_fmt_chunk();

	int seek_sample_fmt_normal( uint32_t ms );
	int get_length_fmt_normal() const;

	OpenResult WAV_open_internal();

	int SetPosition(int ms);

	bool FindChunk( int32_t ID, int32_t &Length );

	uint32_t ConvertMsToBytePos(int BytesPerSample, int channels, uint32_t ms) const;
	uint32_t ConvertBytePosToMs(int BytesPerSample, int channels, uint32_t pos) const;

public:
	OpenResult Open(CString filename);
	void Close();
	int GetLength() const;
	int GetLength_Fast() const { return GetLength(); }
	int SetPosition_Accurate(int ms)  { return SetPosition(ms); }
	int SetPosition_Fast(int ms) { return SetPosition(ms); }
	int Read(char *buf, unsigned len);
	int GetSampleRate() const { return SampleRate; }
	RageSoundReader_WAV();
	~RageSoundReader_WAV();
	RageSoundReader_WAV( const RageSoundReader_WAV & ); /* not defined; don't use */
	SoundReader *Copy() const;
};

#endif

/*
 * Copyright (C) 2001  Ryan C. Gordon (icculus@clutteredmind.org)
 * Copyright (C) 2003-2004 Glenn Maynard
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

