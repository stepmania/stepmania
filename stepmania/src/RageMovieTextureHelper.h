#ifndef RAGEMOVIETEXTURE_HELPER_H
#define RAGEMOVIETEXTURE_HELPER_H


// #include <windows.h>
// #include <atlbase.h>

//#include "baseclasses/streams.h"
#include "RageMovieTexture.h"

//-----------------------------------------------------------------------------
// CTextureRenderer Class Declarations
//
//	Usage: 1) CheckMediaType is called by the graph
//         2) SetMediaType is called by the graph
//         3) call GetVidWidth and GetVidHeight to get texture information
//         4) call SetRenderTarget
//         5) Do RenderSample is called by the graph
//-----------------------------------------------------------------------------
class CTextureRenderer : public CBaseVideoRenderer
{
public:
    CTextureRenderer();
    ~CTextureRenderer();

	// overridden methods
    HRESULT CheckMediaType(const CMediaType *pmt );     // Format acceptable?
    HRESULT SetMediaType(const CMediaType *pmt );       // Video format notification
    HRESULT DoRenderSample(IMediaSample *pMediaSample); // New video sample

	// new methods
	long GetVidWidth() const { return m_lVidWidth; }
	long GetVidHeight() const { return m_lVidHeight; }
	void SetRenderTarget( RageMovieTexture* pTexture );

protected:
	// Video width, height, and pitch.
	long m_lVidWidth, m_lVidHeight, m_lVidPitch;

	char *output;

	RageMovieTexture*	m_pTexture;	// the video surface we will copy new frames to
//	D3DFORMAT			m_TextureFormat; // Texture format
};

#endif
