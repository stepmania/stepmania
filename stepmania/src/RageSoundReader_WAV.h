#ifndef RAGE_SOUND_READER_WAV_H
#define RAGE_SOUND_READER_WAV_H

#include "RageSoundReader_FileReader.h"
#include "SDL_utils.h"

struct fmt_t;
struct ADPCMCOEFSET;
struct ADPCMBLOCKHEADER;

class RageSoundReader_WAV: public SoundReader_FileReader
{
    FILE *rw;
	struct
	{
		Sint16 wFormatTag;
		Uint16 wChannels;
		Uint32 dwAvgBytesPerSec;
		Uint16 wBlockAlign, wBitsPerSample;

		Uint32 adpcm_sample_frame_size;
		Uint32 data_starting_offset;
	} fmt;

	struct adpcm_t
	{
		Uint16 cbSize;
		Uint16 wSamplesPerBlock;
		Uint16 wNumCoef;
		ADPCMCOEFSET *aCoef;
		ADPCMBLOCKHEADER *blockheaders;
		Uint32 samples_left_in_block;
		int nibble_state;
		Sint8 nibble;
	};
	struct adpcm_t adpcm;
	CString filename;

	enum DataType_t { FORMAT_PCM=0, FORMAT_ADPCM=1 } DataType;

	/* Amount of input data, in bytes: */
	int SampleRate;
	int Channels;
	int BytesPerSample;

	/* Number of bytes to read to get one output buffer byte.  If converting from 8-
	 * to 16-bit, *2; if 1- to 2-channel, another *2. */
	int Input_Buffer_Ratio;

	enum { CONV_NONE, CONV_8BIT_TO_16BIT, CONV_16LSB_TO_16SYS } Conversion;

	int read_sample_fmt_normal( char *buf, unsigned len );
	bool read_le16( FILE *rw, Sint16 *si16 ) const;
	bool read_le16( FILE *rw, Uint16 *ui16 ) const;
	bool read_le32( FILE *rw, Sint32 *si32 ) const;
	bool read_le32( FILE *rw, Uint32 *ui32 ) const;
	bool read_uint8( FILE *rw, Uint8 *ui8 ) const;

	bool read_adpcm_block_headers( adpcm_t &out ) const;
	bool decode_adpcm_sample_frame();
	Uint32 read_sample_fmt_adpcm( char *buf, unsigned len );
	void do_adpcm_nibble( Uint8 nib, ADPCMBLOCKHEADER *header, Sint32 lPredSamp );
	void put_adpcm_sample_frame( Uint16 *buf, int frame );

	int seek_sample_fmt_adpcm( Uint32 ms );
	int get_length_fmt_adpcm() const;
	int find_chunk( Uint32 id, Sint32 &size );
	bool read_fmt_chunk();

	int seek_sample_fmt_normal( Uint32 ms );
	int get_length_fmt_normal() const;

	bool WAV_open_internal();

	int SetPosition(int ms);

	bool FindChunk( Sint32 ID, Sint32 &Length );

	Uint32 ConvertMsToBytePos(int BytesPerSample, int channels, Uint32 ms) const;
	Uint32 ConvertBytePosToMs(int BytesPerSample, int channels, Uint32 pos) const;

public:
	bool Open(CString filename);
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
