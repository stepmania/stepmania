#ifndef CONDITIONALBGA_H
#define CONDITIONALBGA_H

#include "PlayerOptions.h"
#include "GameConstantsAndTypes.h"
#include "BGAnimation.h"
//#include "Style.h"

enum CBGACLEAREDSTATES
{
	CBGA_CSUNUSED = 0, // unused is if the condition doesnt matter
	CBGA_CSCLEARED, // if the player must have cleared the stage
	CBGA_CSFAILED, // if the player must have failed the stage
	CBGA_CSMAXCOMBO, // if the player cleared the song with full combo
	CBGA_CSBROKECOMBO // if the player cleared the song with a broken combo
};

struct BgaCondInfo
{
//	char bganame[512];
	CString bganame;
//	char songtitle[512];
	CString songtitle;
//	char songartist[512];
	CString songartist;
	int cleared;
	vector<Difficulty> difficulties; // heavy, light e.t.c.
	vector<int> songmeters; // footmeter
	vector<int> songdays;
	vector<int> songdows;
	vector<int> songmonths;
	vector<int> grades;
	vector<const Style*> styles;
	PlayerOptions disallowedpo;
	bool dpoused; // indicate if disallowed po has been set
};


class ConditionalBGA
{
public:
	ConditionalBGA();
	~ConditionalBGA();
	void Load(CString szScreenName);
	void Update( float fDeltaTime );
	void DrawPrimitives();
private:
	void ClearINFO(int iEntry);
	void CheckBgaRequirements(BgaCondInfo info);
	vector<BgaCondInfo> m_bgainfo;
	//	BgaCondInfo bestmatch;
	BGAnimation bganim;
	CString bganimtouse;
};


#endif

/*
 * (c) 2004 Andrew Livy
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
