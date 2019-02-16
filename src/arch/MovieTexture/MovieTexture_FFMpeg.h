/* MovieTexture_FFMpeg - FFMpeg movie renderer. */

#ifndef RAGE_MOVIE_TEXTURE_FFMPEG_H
#define RAGE_MOVIE_TEXTURE_FFMPEG_H

#include "MovieTexture_Generic.h"
struct RageSurface;

namespace avcodec
{
	extern "C"
	{
		#include <libavformat/avformat.h>
		#include <libswscale/swscale.h>
		#include <libavutil/pixdesc.h>

		#if LIBAVCODEC_VERSION_MAJOR >= 58
		#define av_free_packet av_packet_unref
		#define PixelFormat AVPixelFormat
		#define PIX_FMT_YUYV422 AV_PIX_FMT_YUYV422
		#define PIX_FMT_BGRA    AV_PIX_FMT_BGRA
		#define PIX_FMT_ARGB	AV_PIX_FMT_ARGB
		#define PIX_FMT_ABGR	AV_PIX_FMT_ABGR
		#define PIX_FMT_RGBA	AV_PIX_FMT_RGBA
		#define PIX_FMT_RGB24	AV_PIX_FMT_RGB24
		#define PIX_FMT_BGR24	AV_PIX_FMT_BGR24
		#define PIX_FMT_RGB555	AV_PIX_FMT_RGB555
		#define PIX_FMT_NB		AV_PIX_FMT_NB
		#define CODEC_ID_NONE   AV_CODEC_ID_NONE
		#endif
	}
};

#define STEPMANIA_FFMPEG_BUFFER_SIZE 4096
static const int sws_flags = SWS_BICUBIC; // XXX: Reasonable default?

class MovieTexture_FFMpeg: public MovieTexture_Generic
{
public:
	MovieTexture_FFMpeg( RageTextureID ID );

	static void RegisterProtocols();
	static RageSurface *AVCodecCreateCompatibleSurface( int iTextureWidth, int iTextureHeight, bool bPreferHighColor, int &iAVTexfmt, MovieDecoderPixelFormatYCbCr &fmtout );
};

class RageMovieTextureDriver_FFMpeg: public RageMovieTextureDriver
{
public:
	virtual RageMovieTexture *Create( RageTextureID ID, RString &sError );
	static RageSurface *AVCodecCreateCompatibleSurface( int iTextureWidth, int iTextureHeight, bool bPreferHighColor, int &iAVTexfmt, MovieDecoderPixelFormatYCbCr &fmtout );
};

class MovieDecoder_FFMpeg: public MovieDecoder
{
public:
	MovieDecoder_FFMpeg();
	~MovieDecoder_FFMpeg();

	RString Open( RString sFile );
	void Close();
	void Rewind();

	void GetFrame( RageSurface *pOut );
	int DecodeFrame( float fTargetTime );

	int GetWidth() const { return m_pStreamCodec->width; }
	int GetHeight() const { return m_pStreamCodec->height; }

	RageSurface *CreateCompatibleSurface( int iTextureWidth, int iTextureHeight, bool bPreferHighColor, MovieDecoderPixelFormatYCbCr &fmtout );

	float GetTimestamp() const;
	float GetFrameDuration() const;

private:
	void Init();
	RString OpenCodec();
	int ReadPacket();
	int DecodePacket( float fTargetTime );

	avcodec::AVStream *m_pStream;
	avcodec::AVFrame *m_Frame;
	avcodec::PixelFormat m_AVTexfmt; /* PixelFormat of output surface */
	avcodec::SwsContext *m_swsctx;
	avcodec::AVCodecContext *m_pStreamCodec;

	avcodec::AVFormatContext *m_fctx;
	float m_fTimestamp;
	float m_fTimestampOffset;
	float m_fLastFrameDelay;
	int m_iFrameNumber;

	unsigned char *m_buffer;
	avcodec::AVIOContext *m_avioContext;

	avcodec::AVPacket m_Packet;
	int m_iCurrentPacketOffset;
	float m_fLastFrame;

	/* 0 = no EOF
	 * 1 = EOF from ReadPacket
	 * 2 = EOF from ReadPacket and DecodePacket */
	int m_iEOF;
};

static struct AVPixelFormat_t
{
	int bpp;
	uint32_t masks[4];
	avcodec::PixelFormat pf;
	bool bHighColor;
	bool bByteSwapOnLittleEndian;
	MovieDecoderPixelFormatYCbCr YUV;
} AVPixelFormats[] = {
	{
		32,
		{ 0xFF000000,
		  0x00FF0000,
		  0x0000FF00,
		  0x000000FF },
		avcodec::PIX_FMT_YUYV422,
		false, /* N/A */
		true,
		PixelFormatYCbCr_YUYV422,
	},
	{
		32,
		{ 0x0000FF00,
		  0x00FF0000,
		  0xFF000000,
		  0x000000FF },
		avcodec::PIX_FMT_BGRA,
		true,
		true,
		PixelFormatYCbCr_Invalid,
	},
	{
		32,
		{ 0x00FF0000,
		  0x0000FF00,
		  0x000000FF,
		  0xFF000000 },
		avcodec::PIX_FMT_ARGB,
		true,
		true,
		PixelFormatYCbCr_Invalid,
	},
	/*
	{
		32,
		{ 0x000000FF,
		  0x0000FF00,
		  0x00FF0000,
		  0xFF000000 },
		avcodec::PIX_FMT_ABGR,
		true,
		true,
		PixelFormatYCbCr_Invalid,
	},
	{
		32,
		{ 0xFF000000,
		  0x00FF0000,
		  0x0000FF00,
		  0x000000FF },
		avcodec::PIX_FMT_RGBA,
		true,
		true,
		PixelFormatYCbCr_Invalid,
	}, */
	{
		24,
		{ 0xFF0000,
		  0x00FF00,
		  0x0000FF,
		  0x000000 },
		avcodec::PIX_FMT_RGB24,
		true,
		true,
		PixelFormatYCbCr_Invalid,
	},
	{
		24,
		{ 0x0000FF,
		  0x00FF00,
		  0xFF0000,
		  0x000000 },
		avcodec::PIX_FMT_BGR24,
		true,
		true,
		PixelFormatYCbCr_Invalid,
	},
	{
		16,
		{ 0x7C00,
		  0x03E0,
		  0x001F,
		  0x0000 },
		avcodec::PIX_FMT_RGB555,
		false,
		false,
		PixelFormatYCbCr_Invalid,
	},
	{ 0, { 0,0,0,0 }, avcodec::PIX_FMT_NB, true, false, PixelFormatYCbCr_Invalid }
};

#endif

/*
 * (c) 2003-2005 Glenn Maynard
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
