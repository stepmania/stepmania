#ifndef DIFFICULTY_LIST
#define DIFFICULTY_LIST

#include "ActorFrame.h"
#include "ActorUtil.h"
#include "PlayerNumber.h"
class DifficultyMeter;
class Song;
class Steps;
class BitmapText;

#define MAX_METERS 16

class DifficultyList: public ActorFrame
{
public:
	DifficultyList();
	~DifficultyList();

	void Load();
	void SetFromGameState();
	void TweenOnScreen();
	void TweenOffScreen();
	void Hide();
	void Show();

private:
	void PositionItems();

	DifficultyMeter *m_Meters;
	AutoActor		m_Cursors[NUM_PLAYERS];
	ActorFrame		m_CursorFrames[NUM_PLAYERS];
	BitmapText		*m_Descriptions;
	Song			*m_CurSong;
	vector<Steps*>	m_CurSteps;
};

#endif
