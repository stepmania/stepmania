#include "global.h"
#include "MovieTexture_FFMpeg.h"

#include "RageDisplay.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "RageFile.h"
#include "RageSurface.h"

#include <cerrno>

static void FixLilEndian()
{
#if defined(ENDIAN_LITTLE)
	static bool Initialized = false;
	if( Initialized )
		return;
	Initialized = true;

	for( int i = 0; i < AVPixelFormats[i].bpp; ++i )
	{
		AVPixelFormat_t &pf = AVPixelFormats[i];

		if( !pf.bByteSwapOnLittleEndian )
			continue;

		for( int mask = 0; mask < 4; ++mask)
		{
			int m = pf.masks[mask];
			switch( pf.bpp )
			{
				case 24: m = Swap24(m); break;
				case 32: m = Swap32(m); break;
				default:
					 FAIL_M(ssprintf("Unsupported BPP value: %i", pf.bpp));
			}
			pf.masks[mask] = m;
		}
	}
#endif
}

static int FindCompatibleAVFormat( bool bHighColor )
{
	for( int i = 0; AVPixelFormats[i].bpp; ++i )
	{
		AVPixelFormat_t &fmt = AVPixelFormats[i];
		if( fmt.YUV != PixelFormatYCbCr_Invalid )
		{
			EffectMode em = MovieTexture_Generic::GetEffectMode( fmt.YUV );
			if( !DISPLAY->IsEffectModeSupported(em) )
				continue;
		}
		else if( fmt.bHighColor != bHighColor )
		{
			continue;
		}

		RagePixelFormat pixfmt = DISPLAY->FindPixelFormat( fmt.bpp,
				fmt.masks[0],
				fmt.masks[1],
				fmt.masks[2],
				fmt.masks[3],
				true /* realtime */
				);

		if( pixfmt == RagePixelFormat_Invalid )
			continue;

		return i;
	}

	return -1;
}

RageSurface *RageMovieTextureDriver_FFMpeg::AVCodecCreateCompatibleSurface( int iTextureWidth, int iTextureHeight, bool bPreferHighColor, int &iAVTexfmt, MovieDecoderPixelFormatYCbCr &fmtout )
{
	FixLilEndian();

	int iAVTexfmtIndex = FindCompatibleAVFormat( bPreferHighColor );
	if( iAVTexfmtIndex == -1 )
		iAVTexfmtIndex = FindCompatibleAVFormat( !bPreferHighColor );

	if( iAVTexfmtIndex == -1 )
	{
		/* No dice.  Use the first avcodec format of the preferred bit depth,
		 * and let the display system convert. */
		for( iAVTexfmtIndex = 0; AVPixelFormats[iAVTexfmtIndex].bpp; ++iAVTexfmtIndex )
			if( AVPixelFormats[iAVTexfmtIndex].bHighColor == bPreferHighColor )
				break;
		ASSERT( AVPixelFormats[iAVTexfmtIndex].bpp != 0 );
	}

	const AVPixelFormat_t *pfd = &AVPixelFormats[iAVTexfmtIndex];
	iAVTexfmt = pfd->pf;
	fmtout = pfd->YUV;

	LOG->Trace( "Texture pixel format: %i %i (%ibpp, %08x %08x %08x %08x)", iAVTexfmt, fmtout,
		pfd->bpp, pfd->masks[0], pfd->masks[1], pfd->masks[2], pfd->masks[3] );

	if( pfd->YUV == PixelFormatYCbCr_YUYV422 )
		iTextureWidth /= 2;

	return CreateSurface( iTextureWidth, iTextureHeight, pfd->bpp,
		pfd->masks[0], pfd->masks[1], pfd->masks[2], pfd->masks[3] );
}

MovieDecoder_FFMpeg::MovieDecoder_FFMpeg()
{
	FixLilEndian();

	m_swsctx = NULL;
	m_avioContext = NULL;
	m_buffer = NULL;
	m_fctx = nullptr;
	m_pStream = nullptr;
	m_iCurrentPacketOffset = -1;
	m_Frame = avcodec::av_frame_alloc();

	Init();
}

MovieDecoder_FFMpeg::~MovieDecoder_FFMpeg()
{
	Init();
}

void MovieDecoder_FFMpeg::Init()
{
	if( m_iCurrentPacketOffset != -1 )
	{
		avcodec::av_free_packet( &m_Packet );
		m_iCurrentPacketOffset = -1;
	}

	m_iEOF = 0;
	m_fTimestamp = 0;
	m_fLastFrameDelay = 0;
	m_iFrameNumber = -1; /* decode one frame and you're on the 0th */
	m_fTimestampOffset = 0;
	m_fLastFrame = 0;
	if (m_swsctx)
	{
		avcodec::sws_freeContext(m_swsctx);
		m_swsctx = nullptr;
	}
	m_swsctx = NULL;
	// Note: m_avioContext->buffer refers to m_buffer, but ffmpeg sometimes
	// reallocates the buffer to change the size, and in that case our m_buffer
	// pointer is freed already, so we instead check m_avioContext->buffer
	// to free m_buffer
	if (m_avioContext && m_avioContext->buffer != NULL) {
		avcodec::av_free(m_avioContext->buffer);
	}
	m_buffer = NULL;
    if (m_avioContext != nullptr )
    {
        RageFile *file = (RageFile *)m_avioContext->opaque;
        file->Close();
        delete file;
        avcodec::av_free(m_avioContext);
    }
	m_avioContext = NULL;
#if LIBAVCODEC_VERSION_MAJOR >= 58
	if ( m_pStreamCodec != nullptr)
	{
		avcodec::avcodec_free_context(&m_pStreamCodec);
	}
#endif
}

/* Read until we get a frame, EOF or error.  Return -1 on error, 0 on EOF, 1 if we have a frame. */
int MovieDecoder_FFMpeg::DecodeFrame( float fTargetTime )
{
	//hack to filter out stuttering
	if(fTargetTime<m_fLastFrame)
	{
		fTargetTime=m_fLastFrame;
	}
	else
	{
		m_fLastFrame=fTargetTime;
	}

	for(;;)
	{
		int ret = DecodePacket( fTargetTime );

		if( ret == 1 )
		{
			return 1;
		}
		if( ret == -1 )
		{
			return -1;
		}
		if( ret == 0 && m_iEOF > 0 )
		{
			return 0; /* eof */
		}
		ASSERT( ret == 0 );
		ret = ReadPacket();
		if( ret < 0 )
		{
			return ret; /* error */
		}
	}
}

float MovieDecoder_FFMpeg::GetTimestamp() const
{
	return m_fTimestamp - m_fTimestampOffset;
}

float MovieDecoder_FFMpeg::GetFrameDuration() const
{
	return m_fLastFrameDelay;
}


/* Read a packet.  Return -1 on error, 0 on EOF, 1 on OK. */
int MovieDecoder_FFMpeg::ReadPacket()
{
	if( m_iEOF > 0 )
		return 0;

	for(;;)
	{
		if( m_iCurrentPacketOffset != -1 )
		{
			m_iCurrentPacketOffset = -1;
			avcodec::av_free_packet( &m_Packet );
		}

		int ret = avcodec::av_read_frame( m_fctx, &m_Packet );
		/* XXX: why is avformat returning AVERROR_NOMEM on EOF? */
		if( ret < 0 )
		{
			/* EOF. */
			m_iEOF = 1;
			m_Packet.size = 0;

			return 0;
		}

		if( m_Packet.stream_index == m_pStream->index )
		{
			m_iCurrentPacketOffset = 0;
			return 1;
		}

		/* It's not for the video stream; ignore it. */
		avcodec::av_free_packet( &m_Packet );
	}
}

/* Decode data from the current packet.  Return -1 on error, 0 if the packet is finished,
 * and 1 if we have a frame (we may have more data in the packet). */
int MovieDecoder_FFMpeg::DecodePacket( float fTargetTime )
{
	if( m_iEOF == 0 && m_iCurrentPacketOffset == -1 )
		return 0; /* no packet */

	while( m_iEOF == 1 || (m_iEOF == 0 && m_iCurrentPacketOffset < m_Packet.size) )
	{
		/* If we have no data on the first frame, just return EOF; passing an empty packet
		 * to avcodec_decode_video in this case is crashing it.  However, passing an empty
		 * packet is normal with B-frames, to flush.  This may be unnecessary in newer
		 * versions of avcodec, but I'm waiting until a new stable release to upgrade. */
		if( m_Packet.size == 0 && m_iFrameNumber == -1 )
			return 0; /* eof */

		bool bSkipThisFrame =
			fTargetTime != -1 &&
			GetTimestamp() + GetFrameDuration() < fTargetTime &&
			(m_pStreamCodec->frame_number % 2) == 0;

		int iGotFrame;
		int len;
		/* Hack: we need to send size = 0 to flush frames at the end, but we have
		 * to give it a buffer to read from since it tries to read anyway. */
		m_Packet.data = m_Packet.size ? m_Packet.data : nullptr;
#if LIBAVCODEC_VERSION_MAJOR < 58
		len = avcodec::avcodec_decode_video2(
				m_pStreamCodec,
				m_Frame, &iGotFrame,
				&m_Packet );
#else
		len = m_Packet.size;
		avcodec::avcodec_send_packet(m_pStreamCodec, &m_Packet);
		iGotFrame = !avcodec::avcodec_receive_frame(m_pStreamCodec, m_Frame);
#endif

		if( len < 0 )
		{
			LOG->Warn("avcodec_decode_video2: %i", len);
			return -1; // XXX
		}

		m_iCurrentPacketOffset += len;

		if( !iGotFrame )
		{
			if( m_iEOF == 1 )
				m_iEOF = 2;
			continue;
		}

		if( m_Frame->pkt_dts != AV_NOPTS_VALUE )
		{
			m_fTimestamp = (float) (m_Frame->pkt_dts * av_q2d(m_pStream->time_base));
		}
		else
		{
			/* If the timestamp is zero, this frame is to be played at the
			 * time of the last frame plus the length of the last frame. */
			m_fTimestamp += m_fLastFrameDelay;
		}

		/* Length of this frame: */
		m_fLastFrameDelay = (float) av_q2d(m_pStream->time_base);
		m_fLastFrameDelay += m_Frame->repeat_pict * (m_fLastFrameDelay * 0.5f);

		++m_iFrameNumber;

		if( m_iFrameNumber == 0 )
		{
			/* Some videos start with a timestamp other than 0.  I think this is used
			 * when audio starts before the video.  We don't want to honor that, since
			 * the DShow renderer doesn't and we don't want to break sync compatibility. */
			const float expect = 0;
			const float actual = m_fTimestamp;
			if( actual - expect > 0 )
			{
				LOG->Trace("Expect %f, got %f -> %f", expect, actual, actual - expect );
				m_fTimestampOffset = actual - expect;
			}
		}

		if( bSkipThisFrame )
			continue;

		return 1;
	}

	return 0; /* packet done */
}

void MovieDecoder_FFMpeg::GetFrame( RageSurface *pSurface )
{
#if LIBAVCODEC_VERSION_MAJOR < 58
	avcodec::AVPicture pict;
	pict.data[0] = (unsigned char *) pSurface->pixels;
	pict.linesize[0] = pSurface->pitch;
#else
	avcodec::AVFrame pict;
	pict.data[0] = (unsigned char *) pSurface->pixels;
	pict.linesize[0] = pSurface->pitch;
#endif

	/* XXX 1: Do this in one of the Open() methods instead?
	 * XXX 2: The problem of doing this in Open() is that m_AVTexfmt is not
	 * already initialized with its correct value.
	 */
	if( m_swsctx == nullptr )
	{
		m_swsctx = avcodec::sws_getCachedContext( m_swsctx,
				GetWidth(), GetHeight(), m_pStreamCodec->pix_fmt,
				GetWidth(), GetHeight(), m_AVTexfmt,
				sws_flags, nullptr, nullptr, nullptr );
		if( m_swsctx == nullptr )
		{
			LOG->Warn("Cannot initialize sws conversion context for (%d,%d) %d->%d", GetWidth(), GetHeight(), m_pStreamCodec->pix_fmt, m_AVTexfmt);
			return;
		}
	}

	avcodec::sws_scale( m_swsctx,
			m_Frame->data, m_Frame->linesize, 0, GetHeight(),
			pict.data, pict.linesize );
}

static RString averr_ssprintf( int err, const char *fmt, ... )
{
	ASSERT( err < 0 );

	va_list     va;
	va_start(va, fmt);
	RString s = vssprintf( fmt, va );
	va_end(va);

	size_t errbuf_size = 512;
	char* errbuf = new char[errbuf_size];
	avcodec::av_strerror(err, errbuf, errbuf_size);
	RString Error = ssprintf("%i: %s", err, errbuf);
	delete[] errbuf;

	return s + " (" + Error + ")";
}

void MovieTexture_FFMpeg::RegisterProtocols()
{
	static bool Done = false;
	if( Done )
		return;
	Done = true;

#if !FF_API_NEXT
	avcodec::avcodec_register_all();
	avcodec::av_register_all();
#endif
}

static int AVIORageFile_ReadPacket( void *opaque, uint8_t *buf, int buf_size )
{
    RageFile *f = (RageFile *)opaque;
    return f->Read( buf, buf_size );
}

static int64_t AVIORageFile_Seek( void *opaque, int64_t offset, int whence )
{
    RageFile *f = (RageFile *)opaque;
    if( whence == AVSEEK_SIZE )
		return f->GetFileSize();

	if( whence != SEEK_SET && whence != SEEK_CUR && whence != SEEK_END )
	{
		LOG->Trace("Error: unsupported seek whence: %d", whence);
		return -1;
	}

	return f->Seek( (int) offset, whence );
}

RString MovieDecoder_FFMpeg::Open( RString sFile )
{
	MovieTexture_FFMpeg::RegisterProtocols();

	Close();

	m_fctx = avcodec::avformat_alloc_context();
	if( !m_fctx )
		return "AVCodec: Couldn't allocate context";

	RageFile *f = new RageFile;

	if( !f->Open(sFile, RageFile::READ) )
	{
		RString errorMessage = f->GetError();
		RString error = ssprintf("MovieDecoder_FFMpeg: Error opening \"%s\": %s", sFile.c_str(), errorMessage.c_str() );
		delete f;
		return error;
	}

	m_buffer = (unsigned char *)avcodec::av_malloc(STEPMANIA_FFMPEG_BUFFER_SIZE);
	m_avioContext = avcodec::avio_alloc_context(m_buffer, STEPMANIA_FFMPEG_BUFFER_SIZE, 0, f, AVIORageFile_ReadPacket, nullptr, AVIORageFile_Seek);
	m_fctx->pb = m_avioContext;
	int ret = avcodec::avformat_open_input( &m_fctx, sFile.c_str(), nullptr, nullptr );
	if( ret < 0 )
		return RString( averr_ssprintf(ret, "AVCodec: Couldn't open \"%s\"", sFile.c_str()) );

	ret = avcodec::avformat_find_stream_info( m_fctx, nullptr );
	if( ret < 0 )
		return RString( averr_ssprintf(ret, "AVCodec (%s): Couldn't find codec parameters", sFile.c_str()) );

	int stream_idx = avcodec::av_find_best_stream( m_fctx, avcodec::AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0 );
	if ( stream_idx < 0 ||
		static_cast<unsigned int>(stream_idx) >= m_fctx->nb_streams ||
		m_fctx->streams[stream_idx] == nullptr )
		return "Couldn't find any video streams";
	m_pStream = m_fctx->streams[stream_idx];
#if LIBAVCODEC_VERSION_MAJOR >= 58
	m_pStreamCodec = avcodec::avcodec_alloc_context3(nullptr);
	if (avcodec::avcodec_parameters_to_context(m_pStreamCodec, m_pStream->codecpar) < 0)
		return ssprintf("Could not get context from parameters");
#else
	m_pStreamCodec = m_pStream->codec;
#endif

	if( m_pStreamCodec->codec_id == avcodec::CODEC_ID_NONE )
		return ssprintf( "Unsupported codec %08x", m_pStreamCodec->codec_tag );

	RString sError = OpenCodec();
	if( !sError.empty() )
		return ssprintf( "AVCodec (%s): %s", sFile.c_str(), sError.c_str() );

	LOG->Trace( "Bitrate: %i", static_cast<int>(m_pStreamCodec->bit_rate) );
	LOG->Trace( "Codec pixel format: %s", avcodec::av_get_pix_fmt_name(m_pStreamCodec->pix_fmt) );

	return RString();
}

RString MovieDecoder_FFMpeg::OpenCodec()
{
	if (m_iCurrentPacketOffset != -1) {
		avcodec::av_free_packet(&m_Packet);
		m_iCurrentPacketOffset = -1;
	}

	m_iEOF = 0;
	m_fTimestamp = 0;
	m_fLastFrameDelay = 0;
	m_iFrameNumber = -1; /* decode one frame and you're on the 0th */
	m_fTimestampOffset = 0;
	m_fLastFrame = 0;

	ASSERT( m_pStream != nullptr );
	if( m_pStreamCodec->codec )
		avcodec::avcodec_close( m_pStreamCodec );

	avcodec::AVCodec *pCodec = avcodec::avcodec_find_decoder( m_pStreamCodec->codec_id );
	if( pCodec == nullptr )
		return ssprintf( "Couldn't find decoder %i", m_pStreamCodec->codec_id );

	m_pStreamCodec->workaround_bugs   = 1;
	m_pStreamCodec->idct_algo         = FF_IDCT_AUTO;
	m_pStreamCodec->error_concealment = 3;

#if LIBAVCODEC_VERSION_MAJOR < 58
	if( pCodec->capabilities & CODEC_CAP_DR1 )
		m_pStreamCodec->flags |= CODEC_FLAG_EMU_EDGE;
#endif

	LOG->Trace("Opening codec %s", pCodec->name );

	int ret = avcodec::avcodec_open2( m_pStreamCodec, pCodec, nullptr );
	if( ret < 0 )
		return RString( averr_ssprintf(ret, "Couldn't open codec \"%s\"", pCodec->name) );
	ASSERT( m_pStreamCodec->codec != nullptr );

	return RString();
}

void MovieDecoder_FFMpeg::Close()
{
	if( m_pStream && m_pStreamCodec->codec )
	{
		avcodec::avcodec_close( m_pStreamCodec );
		m_pStream = nullptr;
	}

	if( m_fctx )
	{
		avcodec::avformat_close_input( &m_fctx );
		m_fctx = nullptr;
	}

	Init();
}

void MovieDecoder_FFMpeg::Rewind()
{
	avcodec::av_seek_frame( m_fctx, -1, 0, 0 );
	OpenCodec();
}

RageSurface *MovieDecoder_FFMpeg::CreateCompatibleSurface( int iTextureWidth, int iTextureHeight, bool bPreferHighColor, MovieDecoderPixelFormatYCbCr &fmtout )
{
	return RageMovieTextureDriver_FFMpeg::AVCodecCreateCompatibleSurface( iTextureWidth, iTextureHeight, bPreferHighColor, *ConvertValue<int>(&m_AVTexfmt), fmtout );
}

MovieTexture_FFMpeg::MovieTexture_FFMpeg( RageTextureID ID ):
	MovieTexture_Generic( ID, new MovieDecoder_FFMpeg )
{
}

RageMovieTexture *RageMovieTextureDriver_FFMpeg::Create( RageTextureID ID, RString &sError )
{
	MovieTexture_FFMpeg *pRet = new MovieTexture_FFMpeg( ID );
	sError = pRet->Init();
	if( !sError.empty() )
		SAFE_DELETE( pRet );
	return pRet;
}

REGISTER_MOVIE_TEXTURE_CLASS( FFMpeg );

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
