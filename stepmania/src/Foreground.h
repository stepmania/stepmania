#ifndef FOREGROUND_H
#define FOREGROUND_H

#include "ActorFrame.h"
#include "song.h"
class BGAnimation;

class Foreground: public ActorFrame
{
public:
	~Foreground();

	void Unload();

	void LoadFromSong( const Song *pSong );

	virtual void Update( float fDeltaTime );

protected:
	void LoadFromAniDir( CString sAniDir );
	struct LoadedBGA
	{
		BGAnimation		*m_bga;
		float			m_fStartBeat;
		float			m_fStopBeat;
	};

	vector<LoadedBGA>	m_BGAnimations;
	float m_fLastMusicSeconds;
	const Song *m_pSong;
};

#endif
