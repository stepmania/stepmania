#ifndef FOOTMODEL_H
#define FOOTMODEL_H
/*
-----------------------------------------------------------------------------
 File: FootModel.h

 Desc: Holds the equation for the statistical model for determining foot
	   ranking from Groove Radar and difficulty

 Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
	David Wilson
-----------------------------------------------------------------------------
*/
/*
Meter = 0.775 + 10.1 Stream + 5.27 Voltage - 0.905 Air - 1.10 Freeze
           + 2.86 Chaos + 0.722 Heavy - 0.877 Light - 6.35 SV Interaction
           - 2.58 CSQ
*/
class FootModel
{
private:
	
	//Model Coefficients
	#define BETA_ZERO	(0.775)
	#define STREAM		(10.1)
	#define VOLTAGE		(5.27)
	#define AIR			(-0.905)
	#define FREEZE		(-1.10)
	#define CHAOS		(2.86)
	#define HEAVY		(0.722)
	#define LIGHT		(-0.877)
	#define SXV			(-6.35) // Square/Voltage Interaction Varible
	#define CSQUARE		(-2.58)
	double m_predictedMeter;

public:
	FootModel(Steps* model);
	double GetPMeter();
};
#endif