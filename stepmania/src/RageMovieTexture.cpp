#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: RageMovieTexture

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Adapted from Nehe tutorial 35
-----------------------------------------------------------------------------
*/

#include "RageMovieTexture.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageException.h"
#include "RageDisplay.h"
#include "RageTimer.h"
#include "sdl_opengl.h"
#include <vfw.h>

#pragma comment( lib, "vfw32.lib" )


static int power_of_two(int input)
{
    int value = 1;

	while ( value < input ) value <<= 1;

	return value;
}


struct AviRenderer
{
private:
	AVISTREAMINFO		psi;		// Pointer To A Structure Containing Stream Info
	PAVISTREAM			pavi;		// Handle To An Open Stream
	PGETFRAME			pgf;		// Pointer To A GetFrame Object
	BITMAPINFOHEADER	bmih;		// Header Information For DrawDibDraw Decoding
	int					videoWidth;
	int					videoHeight;
	int					frameWidth;
	int					frameHeight;
	char				*pdata;		// Pointer To Texture Data
	int					numFrames;	
	float				secsIntoPlay;	// Will Hold seconds since beginning of this loop
	float				streamSecs;	// Will Hold seconds since beginning of this loop
	HDRAWDIB			hdd;		// Handle For Our Dib
	HBITMAP				hBitmap;	// Handle To A Device Dependant Bitmap
	HDC					hdc;		// Creates A Compatible Device Context
	unsigned char*		data;		// Pointer To Our Resized Image
	int					last_frame;	// true if frame hasn't let been requested
	bool				frame_is_new;	// true if frame hasn't let been requested

public:
	int GetVideoWidth() { return videoWidth; }
	int GetVideoHeight() { return videoHeight; }
	int GetFrameWidth() { return frameWidth; }
	int GetFrameHeight() { return frameHeight; }
	void Play() {}
	void Pause() {}
	void Stop() {}
	void SetPosition( float fSeconds ) {}
	bool IsPlaying() { return true; }

	AviRenderer( CString sFile )
	{
		hdc = CreateCompatibleDC(0);								// Creates A Compatible Device Context
		hdd = DrawDibOpen();										// Grab A Device Context For Our Dib
	
		AVIFileInit();												// Opens The AVIFile Library

		// Opens The AVI Stream
		if (AVIStreamOpenFromFile(&pavi, sFile, streamtypeVIDEO, 0, OF_READ, NULL) !=0 )
			throw RageException( "Failed To Open The AVI Stream" );

		AVIStreamInfo(pavi, &psi, sizeof(psi));						// Reads Information About The Stream Into psi
		videoWidth=psi.rcFrame.right-psi.rcFrame.left;					// Width Is Right Side Of Frame Minus Left
		videoHeight=psi.rcFrame.bottom-psi.rcFrame.top;					// Height Is Bottom Of Frame Minus Top
		frameWidth = power_of_two( videoWidth );
		frameHeight = power_of_two( videoHeight );

		last_frame = -1;

		numFrames = AVIStreamLength(pavi);							// The Last Frame Of The Stream
		streamSecs = AVIStreamSampleToTime(pavi,numFrames)/1000.f;		// Calculate stream length
		secsIntoPlay = 0;

		bmih.biSize = sizeof (BITMAPINFOHEADER);					// Size Of The BitmapInfoHeader
		bmih.biPlanes = 1;											// Bitplanes	
		bmih.biBitCount = 24;										// Bits Format We Want (24 Bit, 3 Bytes)
		bmih.biWidth = frameWidth;											// Width We Want (256 Pixels)
		bmih.biHeight = frameHeight;										// Height We Want (256 Pixels)
		bmih.biCompression = BI_RGB;								// Requested Mode = RGB

		hBitmap = CreateDIBSection (hdc, (BITMAPINFO*)(&bmih), DIB_RGB_COLORS, (void**)(&data), NULL, NULL);
		SelectObject (hdc, hBitmap);								// Select hBitmap Into Our Device Context (hdc)

		pgf=AVIStreamGetFrameOpen(pavi, NULL);						// Create The PGETFRAME	Using Our Request Mode
		if (pgf==NULL)
			throw RageException( "Failed To Open The AVI Frame" );
	}

	~AviRenderer(void)							// Properly Closes The Avi File
	{
		DeleteObject(hBitmap);					// Delete The Device Dependant Bitmap Object
		DrawDibClose(hdd);						// Closes The DrawDib Device Context
		AVIStreamGetFrameClose(pgf);			// Deallocates The GetFrame Resources
		AVIStreamRelease(pavi);					// Release The Stream
		AVIFileExit();							// Release The File
	}

	void Update( float fDeltaTime ) 
	{
		secsIntoPlay += fDeltaTime;
		while( secsIntoPlay > streamSecs )
		{
			secsIntoPlay -= streamSecs;
		}
		
		int this_frame = secsIntoPlay/streamSecs*numFrames;

		if( last_frame != this_frame )
		{
			frame_is_new = true;
			GetFrame( this_frame );
			last_frame = this_frame;
		}
	}

	void flipIt(void* buffer)						// Flips The Red And Blue Bytes (256x256)
	{
		void* b = buffer;						// Pointer To The Buffer
		unsigned int numFips = frameWidth*frameHeight;
		__asm								// Assembler Code To Follow
		{
			mov ecx, numFips					// Set Up A Counter (Dimensions Of Memory Block)
			mov ebx, b						// Points ebx To Our Data (b)
			label:							// Label Used For Looping
				mov al,[ebx+0]					// Loads Value At ebx Into al
				mov ah,[ebx+2]					// Loads Value At ebx+2 Into ah
				mov [ebx+2],al					// Stores Value In al At ebx+2
				mov [ebx+0],ah					// Stores Value In ah At ebx
				
				add ebx,3					// Moves Through The Data By 3 Bytes
				dec ecx						// Decreases Our Loop Counter
				jnz label					// If Not Zero Jump Back To Label
		}
	}

	bool IsNewFrameReady()	// returns true only once when a new frame has been taken from the stream
	{
		bool bNew = frame_is_new;
		frame_is_new = false;
		return bNew;
	}

	unsigned char* GetData()			// Grabs current frame from the stream
	{
		Update( 1/60.f );
		return data;
	}
	
	void GetFrame(int frame)		// Grabs A Frame From The Stream
	{
		LPBITMAPINFOHEADER lpbi;					// Holds The Bitmap Header Information
		lpbi = (LPBITMAPINFOHEADER)AVIStreamGetFrame(pgf, frame);	// Grab Data From The AVI Stream
		pdata=(char *)lpbi+lpbi->biSize+lpbi->biClrUsed * sizeof(RGBQUAD);	// Pointer To Data Returned By AVIStreamGetFrame

		SelectObject (hdc, hBitmap);								// Select hBitmap Into Our Device Context (hdc)

		// (Skip The Header Info To Get To The Data)
		// 1:1 copy into the new surface
		DrawDibDraw (hdd, hdc, 0, 0, videoWidth, videoHeight, lpbi, pdata, 0, 0, videoWidth, videoHeight, 0);

		flipIt(data);							// Swap The Red And Blue Bytes (GL Compatability)
	}
};




    // Copy the bits    
    // OPTIMIZATION OPPORTUNITY: Use a video and texture
    // format that allows a simpler copy than this one.
//    switch( m_TextureFormat )
//	{
//	case D3DFMT_A8R8G8B8:
//		{
//			for(int y = 0; y < m_pTexture->GetImageHeight(); y++ ) {
//				BYTE *pBmpBufferOld = pBmpBuffer;
//				BYTE *pTxtBufferOld = pTxtBuffer;   
//				for (int x = 0; x < m_pTexture->GetImageWidth(); x++) {
//					pTxtBuffer[0] = pBmpBuffer[0];
//					pTxtBuffer[1] = pBmpBuffer[1];
//					pTxtBuffer[2] = pBmpBuffer[2];
//					pTxtBuffer[3] = 0xff;
//					pTxtBuffer += 4;
//					pBmpBuffer += 3;
//				}
//				pBmpBuffer = pBmpBufferOld + m_lVidPitch;
//				pTxtBuffer = pTxtBufferOld + lTxtPitch;
//			}
//		}
//		break;
//	case D3DFMT_A1R5G5B5:
//		{
//			for(int y = 0; y < m_pTexture->GetImageHeight(); y++ ) {
//				BYTE *pBmpBufferOld = pBmpBuffer;
//				BYTE *pTxtBufferOld = pTxtBuffer;   
//				for (int x = 0; x < m_pTexture->GetImageWidth(); x++) {
//					*(WORD *)pTxtBuffer =
//						0x8000 +
//						((pBmpBuffer[2] & 0xF8) << 7) +
//						((pBmpBuffer[1] & 0xF8) << 2) +
//						(pBmpBuffer[0] >> 3);
//					pTxtBuffer += 2;
//					pBmpBuffer += 3;
//				}
//				pBmpBuffer = pBmpBufferOld + m_lVidPitch;
//				pTxtBuffer = pTxtBufferOld + lTxtPitch;
//			}
//		}
//		break;




//-----------------------------------------------------------------------------
// RageMovieTexture constructor
//-----------------------------------------------------------------------------
RageMovieTexture::RageMovieTexture( CString sFilePath, RageTexturePrefs prefs ) : 
	RageTexture( sFilePath, prefs )
{
	LOG->Trace( "RageBitmapTexture::RageBitmapTexture()" );

	m_uGLTextureID = 0;
	pAviRenderer = new AviRenderer( m_sFilePath );
    pAviRenderer->Play();

	Create();
}

RageMovieTexture::~RageMovieTexture()
{
	LOG->Trace("RageMovieTexture::~RageMovieTexture");

	delete pAviRenderer;
}

void RageMovieTexture::Reload( RageTexturePrefs prefs )
{
	DISPLAY->SetTexture(0);

	if(m_uGLTextureID) 
	{
		glDeleteTextures(1, &m_uGLTextureID);
		m_uGLTextureID = 0;
	}

	Create();
}

void RageMovieTexture::Create()
{
	if(!m_uGLTextureID)
		glGenTextures(1, &m_uGLTextureID);

	/* Cap the max texture size to the hardware max. */
	m_prefs.iMaxSize = min( m_prefs.iMaxSize, DISPLAY->GetMaxTextureSize() );

	/* Save information about the source. */
	m_iSourceWidth = pAviRenderer->GetVideoWidth();
	m_iSourceHeight = pAviRenderer->GetVideoHeight();

	/* image size cannot exceed max size */
	m_iImageWidth = min( m_iSourceWidth, m_prefs.iMaxSize );
	m_iImageHeight = min( m_iSourceHeight, m_prefs.iMaxSize );

	/* Texture dimensions need to be a power of two; jump to the next. */
	m_iTextureWidth = power_of_two( m_iSourceWidth );
	m_iTextureHeight = power_of_two( m_iSourceHeight );


	glBindTexture( GL_TEXTURE_2D, m_uGLTextureID );

	// Create The Texture
	unsigned char* data = pAviRenderer->GetData();
	glPixelStorei(GL_UNPACK_ROW_LENGTH, m_iTextureWidth);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, m_iTextureWidth, m_iTextureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);


	CreateFrameRects();

	// invert text coor rect
	m_TextureCoordRects[0].top = 1 - m_TextureCoordRects[0].top;
	m_TextureCoordRects[0].bottom = 1 - m_TextureCoordRects[0].bottom;

}

unsigned int RageMovieTexture::GetGLTextureID()
{
	if( pAviRenderer->IsNewFrameReady() )
	{
		unsigned char* data = pAviRenderer->GetData();

		// Update The Texture
		glBindTexture( GL_TEXTURE_2D, m_uGLTextureID );
		glPixelStorei(GL_UNPACK_ROW_LENGTH, m_iTextureWidth);
		glTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, m_iTextureWidth, m_iTextureHeight, GL_RGB, GL_UNSIGNED_BYTE, data);
	}

	return m_uGLTextureID; 
}

void RageMovieTexture::Update( float fDeltaTime )
{
	pAviRenderer->Update( fDeltaTime );	
}

void RageMovieTexture::Play()
{
	pAviRenderer->Play();
}

void RageMovieTexture::Pause()
{
	pAviRenderer->Pause();
}

void RageMovieTexture::Stop()
{
	pAviRenderer->Stop();
}

void RageMovieTexture::SetPosition( float fSeconds )
{
	pAviRenderer->SetPosition( fSeconds );
}

bool RageMovieTexture::IsPlaying() const
{
	return pAviRenderer->IsPlaying();
}



