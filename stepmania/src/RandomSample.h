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

class RageSound;

class RandomSample
{
public:
	RandomSample();
	virtual ~RandomSample();

	bool Load( CString sFilePath, int iMaxToLoad = 1000 /*load all*/ );
	void PlayRandom();
	void Stop();

private:
	bool LoadSoundDir( CString sDir, int iMaxToLoad  );
	bool LoadSound( CString sSoundFilePath );


	vector<RageSound*> m_pSamples;
	int m_iIndexLastPlayed;
};


#endif
