#ifndef SCORE_DISPLAY_PERCENTAGE_H
#define SCORE_DISPLAY_PERCENTAGE_H

#include "ScoreDisplay.h"
#include "PercentageDisplay.h"

class ScoreDisplayPercentage : public ScoreDisplay
{
public:
	ScoreDisplayPercentage();
	void Init( PlayerNumber pn );

private:
	PercentageDisplay	m_Percent;
	Sprite		m_sprFrame;
};

#endif
