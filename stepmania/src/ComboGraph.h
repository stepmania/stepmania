#ifndef COMBO_GRAPH_H
#define COMBO_GRAPH_H

#include "ActorFrame.h"
#include "PlayerNumber.h"
#include "StageStats.h"
#include "Sprite.h"
#include "BitmapText.h"

class ComboGraph: public ActorFrame
{
public:
	~ComboGraph() { Unload(); }
	void Load( CString Path, const StageStats &s, PlayerNumber pn );
	void Unload();

private:
	vector<Actor*>		m_Actors;
};

#endif
