#ifndef RANDOMSAMPLE_H
#define RANDOMSAMPLE_H
/*
-----------------------------------------------------------------------------
 Class: RandomSample

 Desc: Holds multiple sounds samples and can play a random sound easily.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "RageSound.h"
#include "RageUtil.h"

class RandomSample
{
public:
	RandomSample();
	virtual ~RandomSample();

	virtual bool Load( CString sFilePath )
	{
		CString sDir, sFName, sExt;
		splitrelpath( sFilePath, sDir, sFName, sExt );

		sExt.MakeLower();

		if( sExt == "" )	return LoadSoundDir( sFilePath );
		else				return LoadSound( sFilePath );
	};

	void PlayRandom();
	void Stop();

private:
	bool LoadSoundDir( CString sDir );
	bool LoadSound( CString sSoundFilePath );


	vector<RageSound*> m_pSamples;
	int m_iIndexLastPlayed;
};


#endif
