#ifndef CONDITIONALBGA_H
#define CONDITIONALBGA_H

#include "Style.h"
#include "PlayerOptions.h"
//#include "StyleDef.h"

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
	vector<Style> styles;
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

