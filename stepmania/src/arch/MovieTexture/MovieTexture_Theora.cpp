#include "global.h"
#include "MovieTexture_Theora.h"
#include "RageDisplay.h"
#include "RageFile.h"
#include "RageLog.h"
#include "RageSurface.h"
#include "RageUtil.h"
#include "MovieTexture_FFMpeg.h" /* for AVCodecCreateCompatibleSurface */

namespace avcodec
{
#include <ffmpeg/avcodec.h> /* for avcodec::img_convert */
};

// #define HAVE_THEORAEXP

#if !defined(HAVE_THEORAEXP)
#include <theora/theora.h>

#if defined(_MSC_VER)
#pragma comment(lib, OGG_LIB_DIR "ogg_static.lib")
#pragma comment(lib, OGG_LIB_DIR "theora_static.lib")
#endif

class MovieDecoder_Theora: public MovieDecoder
{
public:
	MovieDecoder_Theora();
	~MovieDecoder_Theora();

	RString Open( RString sFile );
	void Close();

	int GetFrame( RageSurface *pOut, float fTargetTime );
	RageSurface *CreateCompatibleSurface( int iTextureWidth, int iTextureHeight, bool bPreferHighColor );

	int GetWidth() const { return m_TheoraInfo.frame_width; }
	int GetHeight() const { return m_TheoraInfo.frame_height; }
	float GetSourceAspectRatio() const;

	float GetTimestamp() const;
	float GetFrameDuration() const;

private:
	void Init();
	RString ProcessHeaders();
	int ReadPage( ogg_page *pOggPage, RString &sError, bool bInitializing );
	void ConvertToSurface( RageSurface *pSurface ) const;

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

int MovieDecoder_Theora::ReadPage( ogg_page *pOggPage, RString &sError, bool bInitializing )
{
	while(1)
	{
		int iRet = ogg_sync_pageout( &m_OggSync, pOggPage );
		if( iRet > 0 )
			return iRet;
		if( iRet < 0 )
		{
			if( bInitializing )
			{
				sError = "not an Ogg file";
				return -1;
			}

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
			{
				sError = ssprintf( "error reading: %s", m_File.GetError().c_str() );
				return -1;
			}

			ogg_sync_wrote( &m_OggSync, iBytes );
		}
	}
}

RString MovieDecoder_Theora::ProcessHeaders()
{
	int iTheoraPacketsProcessed = 0;
	while(1)
	{
		ogg_page OggPage;
		RString sError;
		int ret = ReadPage( &OggPage, sError, true );
		if( ret == 0 )
			return ssprintf( "error opening %s: EOF while searching for codec headers", m_File.GetPath().c_str() );
		if( ret<0 )
			return ssprintf( "error reading %s: %s", m_File.GetPath().c_str(), sError.c_str() );

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
				return RString();
			}

			ret = theora_decode_header( &m_TheoraInfo, &m_TheoraComment, &op);
			if( ret < 0 && ret != OC_NEWPACKET )
				return ssprintf( "error opening %s: error parsing Theora stream headers", m_File.GetPath().c_str() );

			ogg_stream_packetout( &m_OggStream, NULL );
			iTheoraPacketsProcessed++;
		}
	}
}

RString MovieDecoder_Theora::Open( RString sFile )
{
	if( !m_File.Open(sFile) )
		return ssprintf( "error opening %s: %s", sFile.c_str(), m_File.GetError().c_str() );

	Init();

	RString sError = ProcessHeaders();
	if( !sError.empty() )
		return sError;

	theora_decode_init( &m_TheoraState, &m_TheoraInfo );

	RString sOutput = ssprintf( "MovieDecoder_Theora: Opened \"%s\".  Serial: 0x%08lx, FPS: %.02f",
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

	return RString();
}

int MovieDecoder_Theora::GetFrame( RageSurface *pOut, float fTargetTime )
{
	while(1)
	{
		ogg_packet op;
		if( ogg_stream_packetout(&m_OggStream, &op) != 0 )
		{
			theora_decode_packetin( &m_TheoraState, &op );

			if( fTargetTime != -1 &&
				GetTimestamp() + GetFrameDuration() <= fTargetTime )
			{
				ogg_int64_t iFrame = theora_granule_frame( (theora_state *) &m_TheoraState, m_iGranulepos );
				if( (iFrame % 2) == 0 )
					continue;
			}

			ConvertToSurface( pOut );
			m_iGranulepos = m_TheoraState.granulepos;
			return 1;
		}

		/* Read more data. */
		ogg_page OggPage;
		RString sError;
		int ret = ReadPage( &OggPage, sError, false );
		if( ret == 0 )
			return 0;
		if( ret == -1 )
		{
			LOG->Warn( "error reading %s: %s", m_File.GetPath().c_str(), sError.c_str() );
			return -1;
		}
		if( ret > 0 )
			ogg_stream_pagein( &m_OggStream, &OggPage );
	}
}

RageSurface *MovieDecoder_Theora::CreateCompatibleSurface( int iTextureWidth, int iTextureHeight, bool bPreferHighColor )
{
	return MovieTexture_FFMpeg::AVCodecCreateCompatibleSurface( iTextureWidth, iTextureHeight, bPreferHighColor, UnionCast(m_OutputPixFmt) );
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

void MovieDecoder_Theora::Close()
{
	ogg_stream_clear( &m_OggStream );
	theora_clear( &m_TheoraState );
	theora_comment_clear( &m_TheoraComment );
	theora_info_clear( &m_TheoraInfo );
	ogg_sync_clear( &m_OggSync );
}

#else

#include <theora/codec.h>
#include <theora/theoradec.h>

#if defined(_MSC_VER)
#pragma comment(lib, OGG_LIB_DIR "ogg_static.lib")
#pragma comment(lib, OGG_LIB_DIR "theorabase_static.lib")
#pragma comment(lib, OGG_LIB_DIR "theoradec_static.lib")
#endif

class MovieDecoder_Theora: public MovieDecoder
{
public:
	MovieDecoder_Theora();
	~MovieDecoder_Theora();

	RString Open( RString sFile );
	void Close();

	int GetFrame( RageSurface *pOut, float fTargetTime );
	RageSurface *CreateCompatibleSurface( int iTextureWidth, int iTextureHeight, bool bPreferHighColor );

	int GetWidth() const { return m_TheoraInfo.frame_width; }
	int GetHeight() const { return m_TheoraInfo.frame_height; }
	float GetSourceAspectRatio() const;

	float GetTimestamp() const;
	float GetFrameDuration() const;

private:
	void Init();
	RString ProcessHeaders( theora_setup_info **pTheoraSetupInfo );
	int ReadPage( ogg_page *pOggPage, RString &sError, bool bInitializing );
	static void DecodeStripeStub( void *pCtx, theora_ycbcr_buffer yuv, int yfrag0, int yfrag_end );
	void DecodeStripe( theora_ycbcr_buffer yuv, int yfrag0, int yfrag_end );

	RageFile m_File;

	ogg_sync_state   m_OggSync;
	ogg_stream_state m_OggStream;
	theora_info      m_TheoraInfo;
	theora_comment   m_TheoraComment;
	theora_dec_ctx   *m_TheoraState;

	ogg_int64_t m_iGranulepos;

	avcodec::PixelFormat m_InputPixFmt; /* PixelFormat of YUV input surface */
	avcodec::PixelFormat m_OutputPixFmt; /* PixelFormat of RGB output surface */

	/* This is only set during GetFrame, for DecodeStripe. */
	RageSurface *m_pSurface;
};

MovieDecoder_Theora::MovieDecoder_Theora()
{
	avcodec::avcodec_init();

	memset( &m_TheoraInfo, 0, sizeof(m_TheoraInfo) );
	memset( &m_TheoraComment, 0, sizeof(m_TheoraComment) );
	memset( &m_OggStream, 0, sizeof(m_OggStream) );
	m_TheoraState = NULL;
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

int MovieDecoder_Theora::ReadPage( ogg_page *pOggPage, RString &sError, bool bInitializing )
{
	while(1)
	{
		int iRet = ogg_sync_pageout( &m_OggSync, pOggPage );
		if( iRet > 0 )
			return iRet;
		if( iRet < 0 )
		{
			if( bInitializing )
			{
				sError = "not an Ogg file";
				return -1;
			}

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
			{
				sError = ssprintf( "error reading: %s", m_File.GetError().c_str() );
				return -1;
			}

			ogg_sync_wrote( &m_OggSync, iBytes );
		}
	}
}

RString MovieDecoder_Theora::ProcessHeaders( theora_setup_info **pTheoraSetupInfo )
{
	int iTheoraPacketsProcessed = 0;
	while(1)
	{
		ogg_page OggPage;
		RString sError;
		int ret = ReadPage( &OggPage, sError, true );
		if( ret == 0 )
			return ssprintf( "error opening %s: EOF while searching for codec headers", m_File.GetPath().c_str() );
		if( ret<0 )
			return ssprintf( "error reading %s: %s", m_File.GetPath().c_str(), sError.c_str() );

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

			if( theora_decode_headerin(&m_TheoraInfo, &m_TheoraComment, pTheoraSetupInfo, &op) < 0 )
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
				return RString();
			}

			ret = theora_decode_headerin( &m_TheoraInfo, &m_TheoraComment, pTheoraSetupInfo, &op );
			if( ret < 0 )
				return ssprintf( "error opening %s: error parsing Theora stream headers", m_File.GetPath().c_str() );

			ogg_stream_packetout( &m_OggStream, NULL );
			iTheoraPacketsProcessed++;
		}
	}
}

RString MovieDecoder_Theora::Open( RString sFile )
{
	if( !m_File.Open(sFile) )
		return ssprintf( "error opening %s: %s", sFile.c_str(), m_File.GetError().c_str() );

	Init();

	theora_setup_info *pTheoraSetupInfo = NULL;
	RString sError = ProcessHeaders( &pTheoraSetupInfo );
	if( !sError.empty() )
	{
		theora_setup_free( pTheoraSetupInfo );
		return sError;
	}

	m_TheoraState = theora_decode_alloc( &m_TheoraInfo, pTheoraSetupInfo );
	theora_setup_free( pTheoraSetupInfo );

	RString sOutput = ssprintf( "MovieDecoder_Theora: Opened \"%s\".  Serial: 0x%08lx, FPS: %.02f",
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

	{
		theora_stripe_callback cb;
		cb.ctx = this;
		cb.stripe_decoded = DecodeStripeStub;
		theora_decode_ctl( m_TheoraState, OC_DECCTL_SET_STRIPE_CB, &cb, sizeof(cb) );
	}

	return RString();
}

int MovieDecoder_Theora::GetFrame( RageSurface *pOut, float fTargetTime )
{
	while(1)
	{
		ogg_packet op;
		if( ogg_stream_packetout(&m_OggStream, &op) != 0 )
		{
			bool bSkipThisFrame = false;
			if( fTargetTime != -1 &&
				GetTimestamp() + GetFrameDuration() <= fTargetTime )
			{
				ogg_int64_t iFrame = theora_granule_frame( m_TheoraState, op.granulepos );
				if( (iFrame % 2) == 0 )
					bSkipThisFrame = true;
			}

			if( bSkipThisFrame )
				m_pSurface = NULL;
			else
				m_pSurface = pOut;

			theora_decode_packetin( m_TheoraState, &op, &m_iGranulepos );

			m_pSurface = NULL;

			return 1;
		}

		/* Read more data. */
		ogg_page OggPage;
		RString sError;
		int ret = ReadPage( &OggPage, sError, false );
		if( ret == 0 )
			return 0;
		if( ret == -1 )
		{
			LOG->Warn( "error reading %s: %s", m_File.GetPath().c_str(), sError.c_str() );
			return -1;
		}
		if( ret > 0 )
			ogg_stream_pagein( &m_OggStream, &OggPage );
	}
}

RageSurface *MovieDecoder_Theora::CreateCompatibleSurface( int iTextureWidth, int iTextureHeight, bool bPreferHighColor )
{
	return MovieTexture_FFMpeg::AVCodecCreateCompatibleSurface( iTextureWidth, iTextureHeight, bPreferHighColor, UnionCast(m_OutputPixFmt) );
}

void MovieDecoder_Theora::DecodeStripeStub( void *pCtx, theora_ycbcr_buffer yuv, int yfrag0, int yfrag_end )
{
	((MovieDecoder_Theora *) pCtx)->DecodeStripe( yuv, yfrag0, yfrag_end );
}

void MovieDecoder_Theora::DecodeStripe( theora_ycbcr_buffer yuv, int yfrag0, int yfrag_end )
{
	yfrag0 *= 8;
	yfrag_end *= 8;

	/* If m_pSurface is NULL, we're skipping this frame. */
	if( m_pSurface != NULL )
	{
		/* XXX: non-zero offset untested */
		int iYOffsetY = m_TheoraInfo.pic_y;
		int iUVOffsetX = m_TheoraInfo.pic_x;
		int iUVOffsetY = m_TheoraInfo.pic_y;

		iYOffsetY += yfrag0;
		iUVOffsetY += yfrag0;

		if( m_InputPixFmt == avcodec::PIX_FMT_YUV420P || avcodec::PIX_FMT_YUV422P )
			iUVOffsetX /= 2;
		if( m_InputPixFmt == avcodec::PIX_FMT_YUV420P )
			iUVOffsetY /= 2;

		avcodec::AVPicture YUVIn;
		YUVIn.data[0] = yuv[0].data + m_TheoraInfo.pic_x + yuv[0].ystride*iYOffsetY;
		YUVIn.data[1] = yuv[1].data + iUVOffsetX + yuv[1].ystride*iUVOffsetY;
		YUVIn.data[2] = yuv[2].data + iUVOffsetX + yuv[2].ystride*iUVOffsetY;
		YUVIn.data[3] = NULL;
		YUVIn.linesize[0] = yuv[0].ystride;
		YUVIn.linesize[1] = yuv[1].ystride;
		YUVIn.linesize[2] = yuv[2].ystride;
		YUVIn.linesize[3] = 0;

		avcodec::AVPicture RGBOut;
		RGBOut.data[0] = (unsigned char *) m_pSurface->pixels;
		RGBOut.linesize[0] = m_pSurface->pitch;
		RGBOut.data[0] += RGBOut.linesize[0] * yfrag0;

		int iRows = (yfrag_end-yfrag0) + 1;
		avcodec::img_convert( &RGBOut, m_OutputPixFmt,
				&YUVIn, m_InputPixFmt,
				m_TheoraInfo.frame_width, iRows );
	}

	{
		/* yfrag0 = 0 ... frame_height-1 */
		int iHalf = m_TheoraInfo.frame_height / 2;
		if( yfrag0 < iHalf && yfrag_end+1 >= iHalf )
		{
			float fFPS = (float)m_TheoraInfo.fps_numerator / m_TheoraInfo.fps_denominator;
			if( fFPS <= 30.0001f )
			{
//				LOG->Trace( "..." );
				float fSecondsPerFrame = 1/fFPS;
				int iUsecPerFrame = int(fSecondsPerFrame * 1000000);
//				usleep( iUsecPerFrame/2 );
			}
		}
	}
}

float MovieDecoder_Theora::GetSourceAspectRatio() const
{
	return (float) m_TheoraInfo.aspect_numerator / m_TheoraInfo.aspect_denominator;
}

float MovieDecoder_Theora::GetTimestamp() const
{
	return (float) theora_granule_time( const_cast<theora_dec_ctx *>(m_TheoraState), m_iGranulepos );
}

float MovieDecoder_Theora::GetFrameDuration() const
{
	return (float) m_TheoraInfo.fps_denominator / m_TheoraInfo.fps_numerator;
}

void MovieDecoder_Theora::Close()
{
	ogg_stream_clear( &m_OggStream );
	theora_decode_free( m_TheoraState );
	m_TheoraState = NULL;
	theora_comment_clear( &m_TheoraComment );
	theora_info_clear( &m_TheoraInfo );
	ogg_sync_clear( &m_OggSync );
}
#endif

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
