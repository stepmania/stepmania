#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: CompositedText

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "CompositedText.h"
#include "Font.h"
#include "FontManager.h"
#include "RageDisplay.h"
#include "Actor.h"
#include "SDL_video.h"
#include "SDL_utils.h"


SDL_Surface* CreateCompositedText( CString sFontFile, CString sText )
{
	// build a font texture
	Font* pFont = FONT->LoadFont( sFontFile );

	vector<wstring> asTextLines;
	split(CStringToWstring(sText), L"\n", asTextLines, false);

	/* calculate line lengths and widths */
	vector<int> LineWidths;
	int iWidestLineWidth = 0;
	for( unsigned l=0; l<asTextLines.size(); l++ )	// for each line
	{
		LineWidths.push_back(pFont->GetLineWidthInSourcePixels( asTextLines[l] ));
		iWidestLineWidth = max(iWidestLineWidth, LineWidths.back());
	}

	int TotalHeight = pFont->GetHeight() * asTextLines.size();
	unsigned i;
	int MinSpacing = 0;

	/* The height (from the origin to the baseline): */
	int Padding = max(pFont->GetLineSpacing(), MinSpacing) - pFont->GetHeight();

	/* There's padding between every line: */
	TotalHeight += Padding * (asTextLines.size()-1);


	/* Because of the "draw extra pixels" and the fact that frame height is often 
	 * greater than the line spacing, we need to add some padding around all edges. 
	 * Is there a more elegant way we could be handling this? */
	const int PADDING = 32;
	int imageWidth = iWidestLineWidth+PADDING;
	int imageHeight = TotalHeight+PADDING;

	/* Make sure the image size is even to maintain pixel/texel alignment. */
	if( imageWidth%2==1 )	imageWidth++;
	if( imageHeight%2==1 )	imageHeight++;

	/* Allocate surface */
	PixelFormat pixfmt = FMT_RGBA8;
	const PixelFormatDesc *pfd = DISPLAY->GetPixelFormatDesc(pixfmt);
	SDL_Surface *img = SDL_CreateRGBSurfaceSane(SDL_SWSURFACE, imageWidth, imageHeight,
		pfd->bpp, pfd->masks[0], pfd->masks[1], pfd->masks[2], pfd->masks[3]);
	SDL_FillRect( img, NULL, 0x00000000 );


	int iY = PADDING/2;

	for( i=0; i<asTextLines.size(); i++ )		// foreach line
	{
		iY += pFont->GetHeight();
		const wstring &sLine = asTextLines[i];
		const int iLineWidth = LineWidths[i];
		
		int iX = PADDING/2;

		for( unsigned j=0; j<sLine.size(); j++ )	// for each character in the line
		{
			const glyph &g = pFont->GetGlyph( sLine[j] );

			SDL_Rect blitSrc;
			blitSrc.x = g.blitSrc.left;
			blitSrc.w = g.blitSrc.right - g.blitSrc.left;
			blitSrc.y = g.blitSrc.top;
			blitSrc.h = g.blitSrc.bottom - g.blitSrc.top;

			SDL_Rect blitDst;
			blitDst.x = iX+g.hshift;
			blitDst.w = g.width;
			blitDst.y = iY+g.fp->vshift;
			blitDst.h = g.height;

			mySDL_BlitSurfaceSmartBlend( g.GetSurface(), &blitSrc, img, &blitDst );

//			SDL_SaveBMP( img, "testingText.bmp" );

			/* Advance the cursor. */
			iX += g.hadvance;
		}

		/* The amount of padding a line needs: */
		iY += Padding;
	}

	return img;
}