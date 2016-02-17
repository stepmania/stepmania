/* LightsDriver_LinuxMinimaid - Minimaid based lights with libmmmagic */

#ifndef LightsDriver_LinuxMinimaid_H
#define LightsDriver_LinuxMinimaid_H

#include "LightsDriver.h"

#define DDR_DOUBLE_BASS_LIGHTS			0 //unknown but guessed
#define DDR_DOUBLE_PLAYER1_PANEL		2
#define DDR_DOUBLE_PLAYER2_PANEL		3
#define DDR_DOUBLE_MARQUEE_LOWER_RIGHT	4
#define DDR_DOUBLE_MARQUEE_UPPER_RIGHT	5
#define DDR_DOUBLE_MARQUEE_LOWER_LEFT	6
#define DDR_DOUBLE_MARQUEE_UPPER_LEFT	7

//PADX_LIGHTS
#define DDR_DOUBLE_PAD_UP		0
#define DDR_DOUBLE_PAD_DOWN		1
#define DDR_DOUBLE_PAD_LEFT		2
#define DDR_DOUBLE_PAD_RIGHT	3
#define DDR_DOUBLE_PAD_RESET	4 

#define BIT(i) (1<<(i))
#define BIT_IS_SET(v,i) ((v&BIT(i))!=0)

static bool _mmmagic_loaded=false;

class LightsDriver_LinuxMinimaid : public LightsDriver
{
public:
	LightsDriver_LinuxMinimaid();

	virtual ~LightsDriver_LinuxMinimaid();
	virtual void Set( const LightsState *ls );
};

#endif
