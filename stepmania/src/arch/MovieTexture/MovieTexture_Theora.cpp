#include "global.h"
#include "MovieTexture_Theora.h"
#include "RageDisplay.h"
#include "RageFile.h"
#include "RageLog.h"
#include "RageSurface.h"
#include "RageUtil.h"
#include "MovieTexture_FFMpeg.h" /* for AVCodecCreateCompatibleSurface */

#include <theora/theora.h>

#if defined(_MSC_VER)
#pragma comment(lib, OGG_LIB_DIR "ogg_static.lib")
#pragma comment(lib, OGG_LIB_DIR "theora_static.lib")
#endif

namespace avcodec
{
#include <ffmpeg/avcodec.h> /* for avcodec::img_convert */
};

class MovieDecoder_Theora: public MovieDecoder
{
public:
	MovieDecoder_Theora();
	~MovieDecoder_Theora();

	CString Open( CString sFile );
	void Close();

	int GetFrame();
	RageSurface *CreateCompatibleSurface( int iTextureWidth, int iTextureHeight, bool bPreferHighColor );
	void ConvertToSurface( RageSurface *pSurface ) const;

	int GetWidth() const { return m_TheoraInfo.frame_width; }
	int GetHeight() const { return m_TheoraInfo.frame_height; }
	float GetSourceAspectRatio() const;

	float GetTimestamp() const;
	float GetFrameDuration() const;
	bool SkippableFrame() const;

private:
	void Init();
	CString ProcessHeaders();
	int ReadPage( ogg_page *pOggPage );

	RageFile m_File;

	ogg_sync_state   m_OggSync;
	ogg_stream_state m_OggStream;
	theora_info      m_TheoraInfo;
	theora_comment   m_TheoraComment;
	theora_state     m_TheoraState;

	ogg_int64_t m_iGranulepos;

	avcodec::PixelFormat m_InputPixFmt; /* PixelFormat of YUV input surface */
	avcodec::PixelFormat m_OutputPixFmt; /* PixelFormat of RGB output surface */
};

MovieDecoder_Theora::MovieDecoder_Theora()
{
	avcodec::avcodec_init();

	memset( &m_TheoraInfo, 0, sizeof(m_TheoraInfo) );
	memset( &m_TheoraComment, 0, sizeof(m_TheoraComment) );
	memset( &m_OggStream, 0, sizeof(m_OggStream) );
	memset( &m_TheoraState, 0, sizeof(m_TheoraState) );
}

MovieDecoder_Theora::~MovieDecoder_Theora()
{
	Close();
}

void MovieDecoder_Theora::Init()
{
	theora_info_init( &m_TheoraInfo );
	theora_comment_init( &m_TheoraComment );
	ogg_sync_init( &m_OggSync );
}

int MovieDecoder_Theora::ReadPage( ogg_page *pOggPage )
{
	while(1)
	{
		int iRet = ogg_sync_pageout(&m_OggSync, pOggPage) > 0;
		if( iRet > 0 )
			return iRet;
		if( iRet < 0 )
		{
			LOG->Trace( "%s: hole in stream", m_File.GetPath().c_str() );
			continue;
		}

		if( iRet == 0 )
		{
			char *pBuffer = ogg_sync_buffer( &m_OggSync, 4096 );
			int iBytes = m_File.Read( pBuffer, 4096 );
			if( iBytes == 0 )
				return 0;
			if( iBytes == -1 )
				return -1;

			ogg_sync_wrote( &m_OggSync, iBytes );
		}
	}
}

CString MovieDecoder_Theora::ProcessHeaders()
{
	int iTheoraPacketsProcessed = 0;
	while(1)
	{
		ogg_page OggPage;
		int ret = ReadPage( &OggPage );
		if( ret == 0 )
			return ssprintf( "error opening %s: EOF while searching for codec headers", m_File.GetPath().c_str() );
		if( ret<0 )
			return ssprintf( "error reading %s: %s", m_File.GetPath().c_str(), m_File.GetError().c_str() );

		if( m_OggStream.body_data == NULL )
		{
			/* We don't have a stream yet.  If this packet doesn't start a stream,
			 * then there aren't any Theora streams. */
			if( !ogg_page_bos(&OggPage) )
				return ssprintf( "error opening %s: no Theora streams found", m_File.GetPath().c_str() );

			ogg_stream_init( &m_OggStream, ogg_page_serialno(&OggPage) );
			ogg_stream_pagein( &m_OggStream, &OggPage );
			ogg_packet op;
			ogg_stream_packetout( &m_OggStream, &op );

			if( theora_decode_header(&m_TheoraInfo, &m_TheoraComment, &op) < 0 )
			{
				ogg_stream_clear( &m_OggStream );
				continue;
			}

			/* We've found the Theora stream. */ 
			iTheoraPacketsProcessed++;
		}
		else
			ogg_stream_pagein( &m_OggStream, &OggPage );

		/* Look for more headers in this page. */
		while(1)
		{
			ogg_packet op;
			int ret = ogg_stream_packetpeek( &m_OggStream, &op );
			if( ret == 0 )
				break;
			if( ret < 0 )
				return ssprintf( "error opening %s: error parsing Theora stream headers", m_File.GetPath().c_str() );

			if( !theora_packet_isheader(&op) )
			{
				/* This is a body packet, not a header packet, so we're done processing
				 * headers.  We must have at least three header packets. */
				if( iTheoraPacketsProcessed < 3 )
					return ssprintf( "error opening %s: error parsing Theora stream headers", m_File.GetPath().c_str() );
				return CString();
			}

			ret = theora_decode_header( &m_TheoraInfo, &m_TheoraComment, &op);
			if( ret < 0 && ret != OC_NEWPACKET )
				return ssprintf( "error opening %s: error parsing Theora stream headers", m_File.GetPath().c_str() );

			ogg_stream_packetout( &m_OggStream, NULL );
			iTheoraPacketsProcessed++;
		}
	}
}

CString MovieDecoder_Theora::Open( CString sFile )
{
	if( !m_File.Open(sFile) )
		return ssprintf( "error opening %s: %s", sFile.c_str(), m_File.GetError().c_str() );

	Init();

	CString sError = ProcessHeaders();
	if( !sError.empty() )
		return sError;

	theora_decode_init( &m_TheoraState, &m_TheoraInfo );

	CString sOutput = ssprintf( "MovieDecoder_Theora: Opened \"%s\".  Serial: 0x%08lx, FPS: %.02f",
		m_File.GetPath().c_str(), m_OggStream.serialno, 1/GetFrameDuration() );

#if 1
	m_InputPixFmt = avcodec::PIX_FMT_YUV420P;
#else
	switch( m_TheoraInfo.pixelformat )
	{
	case OC_PF_420:
		m_InputPixFmt = avcodec::PIX_FMT_YUV420P;
		sOutput += ", Format: YUV420";
		break;
	case OC_PF_422:
		m_InputPixFmt = avcodec::PIX_FMT_YUV422P;
		sOutput += ", Format: YUV422";
		break;
	case OC_PF_444:
		m_InputPixFmt = avcodec::PIX_FMT_YUV444P;
		sOutput += ", Format: YUV444";
		break;
	default:
		return ssprintf( "error opening %s: unsupported pixel format %i", m_File.GetPath().c_str(), m_TheoraInfo.pixelformat );
	}
#endif
	LOG->Trace( "%s", sOutput.c_str() );

	return CString();
}

int MovieDecoder_Theora::GetFrame()
{
	while(1)
	{
		ogg_packet op;
		if( ogg_stream_packetout(&m_OggStream, &op) != 0 )
		{
			theora_decode_packetin( &m_TheoraState, &op );
			m_iGranulepos = m_TheoraState.granulepos;
			return 1;
		}

		/* Read more data. */
		ogg_page OggPage;
		int ret = ReadPage( &OggPage );
		if( ret == 0 )
			return 0;
		if( ret < 0 )
		{
			LOG->Warn( "error reading %s: %s", m_File.GetPath().c_str(), m_File.GetError().c_str() );
			return -1;
		}
		if( ret > 0 )
			ogg_stream_pagein( &m_OggStream, &OggPage );
	}
}

RageSurface *MovieDecoder_Theora::CreateCompatibleSurface( int iTextureWidth, int iTextureHeight, bool bPreferHighColor )
{
	return MovieTexture_FFMpeg::AVCodecCreateCompatibleSurface( iTextureWidth, iTextureHeight, bPreferHighColor, (int&) m_OutputPixFmt );
}

void MovieDecoder_Theora::ConvertToSurface( RageSurface *pSurface ) const
{
	yuv_buffer yuv;
	theora_decode_YUVout( (theora_state *) &m_TheoraState, &yuv );

	/* XXX: non-zero offset untested */
	int iUVOffsetX = m_TheoraInfo.offset_x;
	int iUVOffsetY = m_TheoraInfo.offset_y;
	if( m_InputPixFmt == avcodec::PIX_FMT_YUV420P || avcodec::PIX_FMT_YUV422P )
		iUVOffsetX /= 2;
	if( m_InputPixFmt == avcodec::PIX_FMT_YUV420P )
		iUVOffsetY /= 2;

	avcodec::AVPicture YUVIn;
	YUVIn.data[0] = yuv.y + m_TheoraInfo.offset_x + yuv.y_stride*m_TheoraInfo.offset_y;
	YUVIn.data[1] = yuv.u + iUVOffsetX + yuv.uv_stride*iUVOffsetY;
	YUVIn.data[2] = yuv.v + iUVOffsetX + yuv.uv_stride*iUVOffsetY;
	YUVIn.data[3] = NULL;
	YUVIn.linesize[0] = yuv.y_stride;
	YUVIn.linesize[1] = yuv.uv_stride;
	YUVIn.linesize[2] = yuv.uv_stride;
	YUVIn.linesize[3] = 0;

	avcodec::AVPicture RGBOut;
	RGBOut.data[0] = (unsigned char *) pSurface->pixels;
	RGBOut.linesize[0] = pSurface->pitch;

	avcodec::img_convert( &RGBOut, m_OutputPixFmt,
			&YUVIn, m_InputPixFmt,
			m_TheoraInfo.frame_width, m_TheoraInfo.frame_height );
}

float MovieDecoder_Theora::GetSourceAspectRatio() const
{
	return (float) m_TheoraInfo.aspect_numerator / m_TheoraInfo.aspect_denominator;
}

float MovieDecoder_Theora::GetTimestamp() const
{
	return (float) theora_granule_time( (theora_state *) &m_TheoraState, m_iGranulepos );
}

float MovieDecoder_Theora::GetFrameDuration() const
{
	return (float) m_TheoraInfo.fps_denominator / m_TheoraInfo.fps_numerator;
}

bool MovieDecoder_Theora::SkippableFrame() const
{
	ogg_int64_t iFrame = theora_granule_frame( (theora_state *) &m_TheoraState, m_iGranulepos );
	return (iFrame % 2) == 0;
}


void MovieDecoder_Theora::Close()
{
	ogg_stream_clear( &m_OggStream );
	theora_clear( &m_TheoraState );
	theora_comment_clear( &m_TheoraComment );
	theora_info_clear( &m_TheoraInfo );
	ogg_sync_clear( &m_OggSync );
}

MovieTexture_Theora::MovieTexture_Theora( RageTextureID ID ):
	MovieTexture_Generic( ID, new MovieDecoder_Theora )
{
}

/*
 * (c) 2005 Glenn Maynard
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
