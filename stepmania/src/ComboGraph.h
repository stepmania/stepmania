#ifndef COMBO_GRAPH_H
#define COMBO_GRAPH_H

#include "ActorFrame.h"
#include "PlayerNumber.h"
#include "Sprite.h"
#include "BitmapText.h"

struct StageStats;
class ComboGraph: public ActorFrame
{
public:
	~ComboGraph() { Unload(); }
	void Load( CString Path, const StageStats &s, PlayerNumber pn );
	void Unload();
	void TweenOffScreen();

private:
	vector<Actor*>		m_Sprites;
	vector<Actor*>		m_Numbers;
};

#endif
