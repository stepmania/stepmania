#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: FootModel

 Desc: See header.

 Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
	David Wilson
-----------------------------------------------------------------------------
*/
#include "Steps.h"
#include "FootModel.h"


FootModel::FootModel(Steps* model)
{	
	//Define and Init Non-Radar Values
	double SV;
	double ChaosSquare;
	SV = model->GetRadarValues()[RADAR_STREAM] * model->GetRadarValues()[RADAR_VOLTAGE];
	ChaosSquare = model->GetRadarValues()[RADAR_CHAOS] * model->GetRadarValues()[RADAR_CHAOS];

	m_predictedMeter = BETA_ZERO;
	m_predictedMeter += STREAM * model->GetRadarValues()[RADAR_STREAM];
	m_predictedMeter += VOLTAGE * model->GetRadarValues()[RADAR_VOLTAGE];
	m_predictedMeter += AIR * model->GetRadarValues()[RADAR_AIR];
	m_predictedMeter += FREEZE * model->GetRadarValues()[RADAR_FREEZE];
	m_predictedMeter += CHAOS * model->GetRadarValues()[RADAR_CHAOS];
	m_predictedMeter += SXV * SV;
	m_predictedMeter += CSQUARE * ChaosSquare;
	if (model->GetDifficulty() == DIFFICULTY_HARD) m_predictedMeter += HEAVY;
	if (model->GetDifficulty() == DIFFICULTY_EASY) m_predictedMeter += LIGHT;
}

double FootModel::GetPMeter()
{
	return m_predictedMeter;
}