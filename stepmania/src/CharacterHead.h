#ifndef CharacterHead_H
#define CharacterHead_H
/*
-----------------------------------------------------------------------------
 Class: CharacterHead

 Desc: A little graphic to the left of the song's text banner in the MusicWheel.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "Sprite.h"
class Character;

class CharacterHead : public Sprite
{
public:
	CharacterHead();

	void LoadFromCharacter( Character* pCharacter );

	virtual void Update( float fDelta );

	enum Face { normal=0, taunt, attack, damage, defeated, reserved, NUM_FACES };
	void SetFace( Face face );

protected:
	float m_fSecondsUntilReturnToNormal;
};

#endif
