#include "global.h"
#include "MovieTexture_AVCodec.h"

#include "RageLog.h"
#include "RageTextureManager.h"
#include "RageUtil.h"
#include "SDL_utils.h"
#include "SDL_endian.h"

/* TODO: implement m_bLoop */

struct AVPixelFormat_t
{
	int bpp;
	int masks[4];
	avcodec::PixelFormat pf;
	bool HighColor;
	bool ByteSwapOnLittleEndian;
} AVPixelFormats[] = {
	{ 
		24,
		{ 0xFF0000,
		  0x00FF00,
		  0x0000FF,
		  0x000000 },
		avcodec::PIX_FMT_RGB24,
		true,
		true
	},
	{ 
		24,
		{ 0x0000FF,
		  0x00FF00,
		  0xFF0000,
		  0x000000 },
		avcodec::PIX_FMT_BGR24,
		true,
		true
	},
	{
		16,
		{ 0x7C00,
		  0x03E0,
		  0x001F,
		  0x8000 },
		avcodec::PIX_FMT_RGB555,
		false,
		false
	},
	{ 0, { 0,0,0,0 }, avcodec::PIX_FMT_NB, true }
};

static void FixLilEndian()
{
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	static bool Initialized = false;
	if( Initialized )
		return; 
	Initialized = true;

	for( int i = 0; i < AVPixelFormats[i].bpp; ++i )
	{
		AVPixelFormat_t &pf = AVPixelFormats[i];

		if( !pf.ByteSwapOnLittleEndian )
			continue;

		for( int mask = 0; mask < 4; ++mask)
		{
			int m = pf.masks[mask];
			switch( pf.bpp )
			{
				case 24: m = mySDL_Swap24(m); break;
				case 32: m = SDL_Swap32(m); break;
				default: ASSERT(0);
			}
			pf.masks[mask] = m;
		}
	}
#endif
}

static int FindCompatibleAVFormat( PixelFormat &pixfmt, bool HighColor )
{
	for( int i = 0; AVPixelFormats[i].bpp; ++i )
	{
		AVPixelFormat_t &fmt = AVPixelFormats[i];
		if( fmt.HighColor != HighColor )
			continue;

		pixfmt = DISPLAY->FindPixelFormat( fmt.bpp,
				fmt.masks[0],
				fmt.masks[1],
				fmt.masks[2],
				fmt.masks[3] );

		if( pixfmt == NUM_PIX_FORMATS )
			continue;

		return i;
	}

	return -1;
}
static avcodec::AVStream *FindVideoStream( avcodec::AVFormatContext *m_fctx )
{
    for( int stream = 0; stream < m_fctx->nb_streams; ++stream )
	{
		avcodec::AVStream *enc = m_fctx->streams[stream];
        if( enc->codec.codec_type == avcodec::CODEC_TYPE_VIDEO )
			return enc;
	}
	return NULL;
}

MovieTexture_AVCodec::MovieTexture_AVCodec( RageTextureID ID ):
	RageMovieTexture( ID )
{
	FixLilEndian();

	m_uTexHandle = 0;
	m_bLoop = true;
    m_State = DECODER_QUIT; /* it's quit until we call StartThread */
	m_bLoop = true;
	m_img = NULL;
	m_ImageWaiting = false;
	m_Rate = 1;
	m_Position = 0;

	m_BufferFinished = SDL_CreateSemaphore(0);
	m_OneFrameDecoded = SDL_CreateSemaphore(0);

	Create();
	CreateFrameRects();
}



void MovieTexture_AVCodec::Create()
{
	m_Timer = 0;

    ASSERT( m_State == DECODER_QUIT );

	RageTextureID actualID = GetID();

	actualID.iAlphaBits = 0;

	avcodec::av_register_all();
	int ret = av_open_input_file( &m_fctx, actualID.filename, NULL, 0, NULL );
	if( ret < 0 )
		RageException::Throw("AVCodec: Couldn't open \"%s\" (%i)", ret); /* XXX */

	ret = av_find_stream_info( m_fctx );
	if ( ret < 0 )
		RageException::Throw("AVCodec: Couldn't find codec parameters"); /* XXX */
	
	m_stream = FindVideoStream( m_fctx );
	if ( m_stream == NULL )
		RageException::Throw("AVCodec: Couldn't find video stream"); /* XXX */

	m_codec = avcodec_find_decoder( m_stream->codec.codec_id );
	if( m_codec == NULL )
		RageException::Throw("AVCodec: Couldn't find decoder"); /* XXX */

	ret = avcodec_open( &m_stream->codec, m_codec );
	if ( ret < 0 )
		RageException::Throw("AVCodec: Couldn't open decoder (%i)", ret ); /* XXX */

	/* I think avcodec wants a special case for this (no decoding); I don't
	 * want to bother.  XXX: test and see if this is really needed */
	if (m_stream->codec.codec_id == avcodec::CODEC_ID_RAWVIDEO)
		RageException::Throw("AVCodec: Can't handle raw video" );

	/* Cap the max texture size to the hardware max. */
	actualID.iMaxSize = min( actualID.iMaxSize, DISPLAY->GetMaxTextureSize() );

	m_iSourceWidth  = m_stream->codec.width;
	m_iSourceHeight = m_stream->codec.height;

	/* image size cannot exceed max size */
	m_iImageWidth = min( m_iSourceWidth, actualID.iMaxSize );
	m_iImageHeight = min( m_iSourceHeight, actualID.iMaxSize );

	/* Texture dimensions need to be a power of two; jump to the next. */
	m_iTextureWidth = power_of_two(m_iImageWidth);
	m_iTextureHeight = power_of_two(m_iImageHeight);

	LOG->Trace("Codec: %s", m_codec->name );
	LOG->Trace("Resolution: %ix%i (%ix%i, %ix%i)",
			m_iSourceWidth, m_iSourceHeight,
			m_iImageWidth, m_iImageHeight, m_iTextureWidth, m_iTextureHeight);
	LOG->Trace("Bitrate: %i", m_stream->codec.bit_rate );

	/* We've set up the movie, so we know the dimensions we need.  Set
	 * up the texture. */
	CreateTexture();

	StartThread();

	/* Wait until we decode one frame, to guarantee that the texture is
	 * drawn when this function returns. */
    ASSERT( m_State == PAUSE_DECODER );
	CHECKPOINT;
    m_State = PLAYING_ONE;
	SDL_SemWait( m_OneFrameDecoded );
	CHECKPOINT;
	m_State = PAUSE_DECODER;
    CheckFrame();
	CHECKPOINT;

    Play();
}

void MovieTexture_AVCodec::Destroy()
{
	StopThread();

	avcodec_close( &m_stream->codec );
	av_close_input_file( m_fctx ); m_fctx = NULL;
	if( m_img )
	{
		SDL_FreeSurface( m_img );
		m_img=NULL;
	}
	if(m_uTexHandle)
	{
		DISPLAY->DeleteTexture( m_uTexHandle );
		m_uTexHandle = 0;
	}
}


void MovieTexture_AVCodec::CreateTexture()
{
    if( m_uTexHandle )
        return;

	CHECKPOINT;

	/* If the movie is coming in RGB, we can potentially save a lot
	 * of time by simply sending frame updates to RageDisplay in the
	 * format we're receiving data.  However, I think just about all
	 * formats output data in some form of YUV, so I havn't bothered
	 * with this yet.
	 *
	 * TODO: We could get a big speed bonus by doing the above if the
	 * hardware renderer can handle YUV textures.  I think D3D can do
	 * this, as well as OpenGL on the Mac, but we don't have any infrastructure
	 * for this right now. */
    PixelFormat pixfmt;
	bool PreferHighColor = (TEXTUREMAN->GetMovieColorDepth() == 32);
	m_AVTexfmt = FindCompatibleAVFormat( pixfmt, PreferHighColor );

	if( m_AVTexfmt == -1 )
		m_AVTexfmt = FindCompatibleAVFormat( pixfmt, !PreferHighColor );

	if( m_AVTexfmt == -1 )
	{
		/* No dice.  Use the first avcodec format of the preferred bit depth,
		 * and let the display system convert. */
		for( m_AVTexfmt = 0; AVPixelFormats[m_AVTexfmt].bpp; ++m_AVTexfmt )
			if( AVPixelFormats[m_AVTexfmt].HighColor == PreferHighColor )
				break;
		ASSERT( AVPixelFormats[m_AVTexfmt].bpp );

		switch( TEXTUREMAN->GetMovieColorDepth() )
		{
		default:
			ASSERT(0);
		case 16:
			if( DISPLAY->SupportsTextureFormat(FMT_RGB5) )
				pixfmt = FMT_RGB5;
			else
				pixfmt = FMT_RGBA4; // everything supports RGBA4

			break;

		case 32:
			if( DISPLAY->SupportsTextureFormat(FMT_RGB8) )
				pixfmt = FMT_RGB8;
			else if( DISPLAY->SupportsTextureFormat(FMT_RGBA8) )
				pixfmt = FMT_RGBA8;
			else if( DISPLAY->SupportsTextureFormat(FMT_RGB5) )
				pixfmt = FMT_RGB5;
			else
				pixfmt = FMT_RGBA4; // everything supports RGBA4
			break;
		}
	}
	
	if( !m_img )
	{
		const AVPixelFormat_t *pfd = &AVPixelFormats[m_AVTexfmt];

		LOG->Trace("format %i, %08x %08x %08x %08x",
			pfd->bpp, pfd->masks[0], pfd->masks[1], pfd->masks[2], pfd->masks[3]);

		m_img = SDL_CreateRGBSurfaceSane(SDL_SWSURFACE, m_iTextureWidth, m_iTextureHeight,
			pfd->bpp, pfd->masks[0], pfd->masks[1], pfd->masks[2], pfd->masks[3]);
	}

    m_uTexHandle = DISPLAY->CreateTexture( pixfmt, m_img );
}



void MovieTexture_AVCodec::DecoderThread()
{
	bool GetNextTimestamp = true;
	float CurrentTimestamp = 0, LastGoodTimestamp = 0, Last_IP_Timestamp=0;

	CHECKPOINT;
	while( m_State != DECODER_QUIT )
	{
		if( m_State == PAUSE_DECODER )
		{
			SDL_Delay( 5 );
			continue;
		}

		CHECKPOINT;

		/* We're playing.  Update the play timer. */
		float TimePassed = m_RunningTimer.GetDeltaTime();
		m_Timer += TimePassed * m_Rate;
		
		/* Read a packet. */
		avcodec::AVPacket pkt;
		while( 1 )
		{
			CHECKPOINT;
			int ret = avcodec::av_read_packet(m_fctx, &pkt);
			if( ret < 0 )
			{
				/* XXX ? */
				LOG->Warn("AVCodec: Error decoding %s: %i",
						GetID().filename.c_str(), ret );
				return;
			}
			
			if( pkt.stream_index != m_stream->index )
			{
				/* It's not for the video stream; ignore it. */
				av_free_packet( &pkt );
				continue;
			}

			break;
		}
		CHECKPOINT;

		/* Decode the packet. */
		int current_packet_offset = 0;
		while( m_State != DECODER_QUIT && current_packet_offset < pkt.size )
		{
			if ( GetNextTimestamp )
			{
				int64_t ts = pkt.pts;
				if (ts == AV_NOPTS_VALUE)
					ts = 0;
				CurrentTimestamp = (float)ts * m_fctx->pts_num / m_fctx->pts_den;
				if( CurrentTimestamp != 0 )
					LastGoodTimestamp = CurrentTimestamp;
				GetNextTimestamp = false;
			}

			avcodec::AVFrame frame;
			int got_frame;
			int len = avcodec::avcodec_decode_video(
					&m_stream->codec, 
					&frame, &got_frame,
					pkt.data + current_packet_offset,
					pkt.size - current_packet_offset );

			if (len < 0)
			{
				LOG->Warn("avcodec_decode_video: %i", len);
				return; // XXX
			}

			current_packet_offset += len;

			if (!got_frame)
				continue;

			/* We got a frame.  Convert it. */
			avcodec::AVPicture pict;
			pict.data[0] = (unsigned char *)m_img->pixels;
			pict.linesize[0] = m_img->pitch;

			img_convert(&pict, AVPixelFormats[m_AVTexfmt].pf,
					(avcodec::AVPicture *) &frame, m_stream->codec.pix_fmt, 
					m_stream->codec.width, m_stream->codec.height);

			if ( m_stream->codec.has_b_frames &&
				 frame.pict_type != FF_B_TYPE )
				swap( CurrentTimestamp, Last_IP_Timestamp );

			if( CurrentTimestamp != 0 )
				LastGoodTimestamp = CurrentTimestamp;
			else {
				/* If the timestamp is zero, guess what the timestamp
				 * would be. */
				CurrentTimestamp = LastGoodTimestamp;
				const float frame_delay = (float)m_stream->codec.frame_rate_base /
					(float)m_stream->codec.frame_rate;

				CurrentTimestamp += frame_delay;
				CurrentTimestamp += frame.repeat_pict * (frame_delay * 0.5);
			}

			{
				const float Offset = CurrentTimestamp - m_Timer;
			
				/* If we're ahead, we're decoding too fast; delay. */
				if( Offset > 0 )
					SDL_Delay( int(1000*(CurrentTimestamp - m_Timer)) );

				/* XXX: If we're behind, Offset will be negative and we
				 * simply won't delay, so the video will play fast until
				 * it catches up.  However, if we're too far behind, we
				 * can take some steps to catch up.  1: We can stop updating
				 * the texture; 2: we can play with hurry_up. */
			}

			m_Position = CurrentTimestamp;

			/* Signal the main thread to update the image on the next Update. */
			m_ImageWaiting=true;

			if( m_State == PLAYING_ONE )
				SDL_SemPost( m_OneFrameDecoded );

			CHECKPOINT;
			SDL_SemWait( m_BufferFinished );
			CHECKPOINT;
			/* If the frame wasn't used, then we must be shutting down. */
			ASSERT( !m_ImageWaiting || m_State == DECODER_QUIT );

			GetNextTimestamp = true;
		}
		av_free_packet( &pkt );
	}
	CHECKPOINT;
}

MovieTexture_AVCodec::~MovieTexture_AVCodec()
{
	Destroy();

	SDL_DestroySemaphore( m_BufferFinished );
	SDL_DestroySemaphore( m_OneFrameDecoded );
}

void MovieTexture_AVCodec::Update(float fDeltaTime)
{
	CHECKPOINT;

	if( m_State == PLAYING )
		CheckFrame();
}

void MovieTexture_AVCodec::CheckFrame()
{
	if( !m_ImageWaiting )
		return;

    /* Just in case we were invalidated: */
    CreateTexture();

	CHECKPOINT;
	DISPLAY->UpdateTexture(
        m_uTexHandle,
        m_img,
        0, 0,
        m_iImageWidth, m_iImageHeight );
    CHECKPOINT;

	m_ImageWaiting = false;
	SDL_SemPost(m_BufferFinished);
}

void MovieTexture_AVCodec::Reload()
{
}

void MovieTexture_AVCodec::StartThread()
{
	ASSERT( m_State == DECODER_QUIT );
	m_State = PAUSE_DECODER;
	m_DecoderThread.SetName( ssprintf("MovieTexture_AVCodec(%s)", GetID().filename.c_str()) );
	m_DecoderThread.Create( DecoderThread_start, this );
}

void MovieTexture_AVCodec::StopThread()
{
	LOG->Trace("Shutting down decoder thread ...");

	if( m_State == DECODER_QUIT )
		return;

	m_State = DECODER_QUIT;

	/* Make sure we don't deadlock waiting for m_BufferFinished. */
	SDL_SemPost(m_BufferFinished);

	CHECKPOINT;
	m_DecoderThread.Wait();
	CHECKPOINT;
	
	/* Clear the above post, if the thread didn't. */
	SDL_SemTryWait(m_BufferFinished);

	LOG->Trace("Decoder thread shut down.");
}

void MovieTexture_AVCodec::Play()
{
    m_State = PLAYING;
}

void MovieTexture_AVCodec::Pause()
{
    m_State = PAUSE_DECODER;
}

void MovieTexture_AVCodec::SetPosition( float fSeconds )
{
    ASSERT( m_State != DECODER_QUIT );

	/* We can reset to 0, but I don't think this API supports fast seeking
	 * yet.  I don't think we ever actually seek except to 0 right now,
	 * anyway. XXX */
	if( fSeconds == m_Position )
		return;

	if( fSeconds != 0 )
	{
		LOG->Warn( "MovieTexture_AVCodec::SetPosition(%f): non-0 seeking unsupported; ignored", fSeconds );
		return;
	}

	LOG->Trace( "Seek to %f (from %f)", fSeconds, m_Position );
    State OldState = m_State;

	Destroy();
	Create();

	m_State = OldState;
}

