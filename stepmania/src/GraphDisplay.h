#ifndef GRAPH_DISPLAY_H
#define GRAPH_DISPLAY_H

#include "ActorFrame.h"
#include "RageTexture.h"
#include "AutoActor.h"

struct StageStats;
struct PlayerStageStats;

class GraphDisplay: public ActorFrame
{
public:
	GraphDisplay();
	~GraphDisplay() { Unload(); }
	void Load( const CString &sTexturePath, float fInitialHeight, const CString &sJustBarelyPath );
	void Unload();

	void LoadFromStageStats( const StageStats &ss, const PlayerStageStats &s, const CString &sSongBoundaryPath );
	void Update( float fDeltaTime );
	void DrawPrimitives();

private:
	void UpdateVerts();

	enum { VALUE_RESOLUTION=200 };
	float m_CurValues[VALUE_RESOLUTION];
	float m_DestValues[VALUE_RESOLUTION];
	float m_LastValues[VALUE_RESOLUTION];
	float m_Position;

	RectF m_quadVertices;
	RageSpriteVertex m_Slices[4*(VALUE_RESOLUTION-1)];

	RageTexture	*m_pTexture;

	vector<AutoActor*> m_vpSongBoundaries;
	AutoActor m_sprJustBarely;
};

#endif

/*
 * (c) 2003 Glenn Maynard
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
