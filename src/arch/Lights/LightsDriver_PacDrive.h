/*
 * LightsDriver_PacDrive
 */

#ifndef LightsDriver_PacDrive_H
#define LightsDriver_PacDrive_H

#include "arch/Lights/LightsDriver.h"

#define BIT(i) (1<<(i))


class LightsDriver_PacDrive : public LightsDriver
{
	public:
	LightsDriver_PacDrive();
	virtual ~LightsDriver_PacDrive();

	virtual void Set( const LightsState *ls );
};

#endif

/* Modified 2015 - Dave Barribeau for StepMania 5.09
* 2014 - twistedsymphony
* Created for use with Beware's StepMania  3.9 DDR Extreme Simulation
* feel free to reuse and distribute this code
*/
