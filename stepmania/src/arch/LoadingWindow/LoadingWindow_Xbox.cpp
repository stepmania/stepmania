#include "global.h"
#include "LoadingWindow_Xbox.h"
#include "RageLog.h"

#include "ProductInfo.h"

LPDIRECT3D8 g_pD3D = NULL; // DirectX Object
LPDIRECT3DDEVICE8 g_pD3DDevice = NULL; // Screen Object
LPDIRECT3DTEXTURE8 splash = NULL; // splash texture
LPD3DXSPRITE g_sprite = NULL; // sprite object

LoadingWindow_Xbox::LoadingWindow_Xbox()
{
	// Initialise Direct3D
	g_pD3D = Direct3DCreate8(D3D_SDK_VERSION);
	
    // Create a structure to hold the settings for our device
    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory(&d3dpp, sizeof(d3dpp));

    // Fill the structure.
    // Set fullscreen 640x480x32 mode
    d3dpp.BackBufferWidth = 640;
    d3dpp.BackBufferHeight = 480;
    d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;

    // Create one backbuffer and a zbuffer
    d3dpp.BackBufferCount = 1;

	// Set up how the backbuffer is "presented" to the frontbuffer each time
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;

    //Create a Direct3D device.
    g_pD3D->CreateDevice(0, D3DDEVTYPE_HAL, NULL,
                            D3DCREATE_HARDWARE_VERTEXPROCESSING,
                            &d3dpp, &g_pD3DDevice);
    g_pD3DDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

	// Create the sprite object (for painting the image)
	D3DXCreateSprite(g_pD3DDevice, &g_sprite);

	// Load the default font
	XFONT_OpenDefaultFont(&font);
	font->SetTextColor(D3DCOLOR_XRGB(255,255,255));
	font->SetTextAlignment(XFONT_CENTER);

	// Load the splash.png

	HRESULT result = D3DXCreateTextureFromFileA(g_pD3DDevice, "D:\\Data\\splash.png", &splash);
	useImage = (result == D3D_OK);

	if(!useImage)
		LOG->Trace("Error loading splash.png - %i", result);
	
	SetText(RString("Loading songs"));
}

LoadingWindow_Xbox::~LoadingWindow_Xbox()
{
	g_pD3DDevice->Release();
    g_pD3D->Release();
	font->Release();
}

void LoadingWindow_Xbox::Paint()
{
	LPDIRECT3DSURFACE8 g_pFrontBuffer;

	if(text == "")
		return;

	g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

	g_pD3DDevice->BeginScene();
	g_pD3DDevice->GetBackBuffer(0,D3DBACKBUFFER_TYPE_MONO,&g_pFrontBuffer);

	if(useImage)
	{
		// Draw the splash image
		// Only draw if the splash texture is successfully loaded
		D3DXVECTOR2 pos;
		pos.x = 70.0f;
		pos.y = 30.0f;
		
		g_sprite->Begin();
		g_sprite->Draw(splash, NULL, NULL, NULL, NULL, &pos, 0xFFFFFFFF);
		g_sprite->End();
	}
	else
	{
		// Lo-fi version: print the product name and version at the top of the screen
		font->SetTextColor(D3DCOLOR_XRGB(255, 0, 0));
		RString title = "Version ";
		title = title + PRODUCT_VER;
		WCHAR wc_title[200] = {0};
		swprintf(wc_title, L"%S", title.c_str());

		font->TextOut(g_pFrontBuffer, L"StepMania", -1, 320, 30);
		font->SetTextColor(D3DCOLOR_XRGB(255, 255, 0));
		font->TextOut(g_pFrontBuffer, wc_title, -1, 320, 40 + font->GetTextHeight());
	}

	// Draw the text on the screen
	font->SetTextColor(D3DCOLOR_XRGB(255, 255, 255));
	basic_string <char>::size_type newLineIndex = text.find("\n", 0);
	int y = 240;

	if(newLineIndex == RString.npos)
	{
		WCHAR wc_text[200] = {0};
		swprintf(wc_text, L"%S", text.c_str());
	
		font->TextOut(g_pFrontBuffer, wc_text, wcslen(wc_text), 320, y);
	}
	else
	{
		int start = 0;

		while(start != RString.npos)
		{
			RString toPrint;
			if(newLineIndex != RString.npos)
				toPrint = text.substr(start, newLineIndex - start);
			else
				toPrint = text.substr(start);

			if(toPrint != "")
			{
				WCHAR wc_text[200] = {0};
				swprintf(wc_text, L"%S", toPrint.c_str());
		
				font->TextOut(g_pFrontBuffer, wc_text, wcslen(wc_text), 320, y);
			}
			y = y + font->GetTextHeight() + 10;

			if(newLineIndex != RString.npos)
				start = newLineIndex + 1;
			else
				start = RString.npos;

			newLineIndex = text.find("\n", start);
		}
	}

	g_pFrontBuffer->Release();

	g_pD3DDevice->EndScene();
	g_pD3DDevice->Present(NULL, NULL, NULL, NULL);
}

void LoadingWindow_Xbox::SetText(RString str)
{
	text = str ;
}

/*
 * (c) 2004 Ryan Dortmans
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