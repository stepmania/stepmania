#ifndef GRAPH_DISPLAY_H
#define GRAPH_DISPLAY_H

#include "ActorFrame.h"
#include "PlayerNumber.h"
#include "RageTexture.h"

struct StageStats;
class GraphDisplay: public ActorFrame
{
public:
	GraphDisplay();
	~GraphDisplay() { Unload(); }
	void Load( CString TexturePath, float height );
	void Unload();

	void LoadFromStageStats( const StageStats &s, PlayerNumber pn );
	void Update( float fDeltaTime );
	void DrawPrimitives();

private:
	void UpdateVerts();

	enum { VALUE_RESOLUTION=100 };
	float m_CurValues[VALUE_RESOLUTION];
	float m_DestValues[VALUE_RESOLUTION];
	float m_LastValues[VALUE_RESOLUTION];
	float m_Position;

	RageSpriteVertex Slices[4*(VALUE_RESOLUTION-1)];

	RageTexture	*m_pTexture;
};

#endif
