#include "global.h"
#include <ctime>

#include "GameState.h"
#include "GameManager.h"
#include "song.h"
#include "GameConstantsAndTypes.h"
#include "RageLog.h"
#include "BGAnimation.h"
#include "ConditionalBGA.h"
#include "ThemeManager.h"
#include "RageUtil.h"
#include "RageFile.h"
#include "StageStats.h"
#include "Steps.h"

ConditionalBGA::ConditionalBGA()
{

}

ConditionalBGA::~ConditionalBGA()
{

}

void ConditionalBGA::Load(CString szScreenName)
{
	RageFile file;

	CString szThemeDir = THEME->GetCurThemeDir();
	CString szConditionalBGAFile = szThemeDir += szScreenName += " ConditionalBGA.ini";


//	char filepath[512];
//	strcpy(filepath,""); // empty the path first
//	strcpy(filepath,szConditionalBGAFile.c_str());
	
	LOG->Trace("ConditionalBGA Load:%s",szConditionalBGAFile.c_str());

	bool loaded = file.Open(szConditionalBGAFile,RageFile::READ);
//	FILE* fp = NULL;
//	fp = fopen(filepath,"r");
	if(!loaded)
	{
		LOG->Trace("ConditionalBGA File Not Found");
		return;
	}
	else
	{
		CString currentline;
		int bgano=0;

		while(!file.AtEOF())
		{
			file.GetLine(currentline); // get the current line
	
			// kill any possible comments
			CStringArray asKillComments;
			asKillComments.clear(); // get rid of anything in there
			split(currentline, "#",asKillComments); // A comment starting with #
			if(!asKillComments.empty())
			{
				currentline = asKillComments[0]; // there was some commentstuff here, take the first bit to be the actual data
			}
			asKillComments.clear(); // get rid of anything in there
			split(currentline, "/",asKillComments); // A comment starting with // or /*
			if(!asKillComments.empty())
			{
				currentline = asKillComments[0]; // there was some commentstuff here, take the first bit to be the actual data
			}
			TrimRight(currentline); // nuke trailing whitespace

			// start parsing the data
			if(currentline.c_str()[0] == '[') // we found a new bganimation
			{
				if(!m_bgainfo.empty()) // last one wasnt empty
				{
					CheckBgaRequirements(m_bgainfo[bgano]);
					bgano++;
				}
				BgaCondInfo temp;
				m_bgainfo.push_back(temp);
				ClearINFO(bgano); // wipe out the old info structure.

				CStringArray asSplitLine;
				split(currentline,"[",asSplitLine);
				split(asSplitLine[0],"]",asSplitLine);
				if(!asSplitLine.empty() && asSplitLine.size() >= 1)
					m_bgainfo[bgano].bganame = asSplitLine[asSplitLine.size() - 1];
			}
			else
			{
				CStringArray asSplitLine;
				split(currentline,":",asSplitLine);
				if(asSplitLine.empty()) continue;

				if(!asSplitLine[0].CompareNoCase("clear") && asSplitLine.size() > 1)
				{
					if(!asSplitLine[1].CompareNoCase("true") || !asSplitLine[1].CompareNoCase("cleared") || !asSplitLine[1].CompareNoCase("clear")) // true / clear (any clear condition)
						m_bgainfo[bgano].cleared = CBGA_CSCLEARED;
					else if(!asSplitLine[1].CompareNoCase("false") || !asSplitLine[1].CompareNoCase("failed")) // false / failed 
						m_bgainfo[bgano].cleared = CBGA_CSFAILED;
					else if(!asSplitLine[1].CompareNoCase("maxcombo") || !asSplitLine[1].CompareNoCase("fullcombo")) // passed with maxcombo 
						m_bgainfo[bgano].cleared = CBGA_CSMAXCOMBO;
					else if(!asSplitLine[1].CompareNoCase("brokencombo")) // passed with a broken combo 
						m_bgainfo[bgano].cleared = CBGA_CSBROKECOMBO;	

			//		LOG->Trace("Clear Conditon: %d",info.cleared);
				}
				if(!asSplitLine[0].CompareNoCase("songtitle") && asSplitLine.size() > 1)
				{
					m_bgainfo[bgano].songtitle = asSplitLine[1];
				//	LOG->Trace("SongTitle: %s",info.songtitle.c_str());
				}
				if(!asSplitLine[0].CompareNoCase("songartist") && asSplitLine.size() > 1)
				{
					m_bgainfo[bgano].songartist = asSplitLine[1];
				//	LOG->Trace("SongArtist: %s",info.songartist.c_str());
				}
				if(!asSplitLine[0].CompareNoCase("songday") && asSplitLine.size() > 1)
				{
					CStringArray asDays;
					split( asSplitLine[1], ",", asDays );
					for( unsigned d=0; d<asDays.size(); d++ )
					{
						int dn = atoi(asDays[d].c_str());
						if(!(dn < 1 || dn > 32)) // ignore if date is out of range
						{
							m_bgainfo[bgano].songdays.push_back(dn);
						}
					}
			//		for(d=0; d<info.songdays.size(); d++)
			//		{
			//			LOG->Trace("SongDay: %d",info.songdays[d]);
			//		}
				}
				if(!asSplitLine[0].CompareNoCase("songmonth") && asSplitLine.size() > 1)
				{
					CStringArray asMonths;
					split( asSplitLine[1], ",", asMonths );
					for( unsigned d=0; d<asMonths.size(); d++ )
					{
						int dn = atoi(asMonths[d].c_str());
						if(!(dn < 1 || dn > 12)) // ignore if date is out of range
						{
							m_bgainfo[bgano].songmonths.push_back(dn);
						}
					}
		//			for(d=0; d<info.songmonths.size(); d++)
		//			{
		//				LOG->Trace("SongMonth: %d",info.songmonths[d]);
		//			}
				}

				// foot meter ratings
				if(!asSplitLine[0].CompareNoCase("songdifficulty") && asSplitLine.size() > 1)
				{
					CStringArray asDifficulties;
					split( asSplitLine[1], ",", asDifficulties );
					
					for(unsigned d=0;d<asDifficulties.size();d++)
					{
						// check to see if the last character is a +
						bool bHandled = false;
						if(asDifficulties[d].c_str()[strlen(asDifficulties[d].c_str())-1] == '+')
						{
							bHandled = true;
							CStringArray asVal;
							split(asDifficulties[d],"+",asVal);
							int temp=0;
							temp = 0 - atoi(asVal[0].c_str()); // negative numbers will indicate 'greater than' for this system
							m_bgainfo[bgano].songmeters.push_back(temp);
						}

						if(!bHandled) // didnt find the + (gt) so find a - (range)
						{
							bool isarange=false;
							for(unsigned b=0; b<strlen(asDifficulties[d].c_str());b++)
							{
								if(asDifficulties[d].c_str()[b] == '-')
								{
									bHandled = isarange = true;
									break;
								}
							}
							if(isarange)
							{
								CStringArray asVal;
								split(asDifficulties[d],"-",asVal);
								int imin=0,imax=0,itmp=0;
								imin=atoi(asVal[0].c_str());
								imax=atoi(asVal[1].c_str());
								itmp=imin;
								while(itmp<=imax) // fill in the values between the min and max range inclusive
								{
									m_bgainfo[bgano].songmeters.push_back(itmp);
									itmp++;
								}
							}
						}

						if(!bHandled) // its not a range so must be a value on its own
						{
							int tmp = atoi(asDifficulties[d].c_str());
							m_bgainfo[bgano].songmeters.push_back(tmp);
						}

					}
				}

				// mods that mustn't be present
				if(!asSplitLine[0].CompareNoCase("moddisallow") && asSplitLine.size() > 1)
				{
					m_bgainfo[bgano].dpoused = true;
					m_bgainfo[bgano].disallowedpo.FromString(asSplitLine[1]);

				}
				
				// heavy, light e.t.c.
				if(!asSplitLine[0].CompareNoCase("songrating") && asSplitLine.size() > 1)
				{
					CStringArray asDifficulties;
					split( asSplitLine[1], ",", asDifficulties );
					for( unsigned d=0; d<asDifficulties.size(); d++ )
					{
						m_bgainfo[bgano].difficulties.push_back(StringToDifficulty(asDifficulties[d]));
					}
		//			for(d=0; d<info.difficulties.size(); d++)
		//			{
		//				LOG->Trace("Difficulty: %d",info.difficulties[d]);
		//			}
				}
				if(!asSplitLine[0].CompareNoCase("grade") && asSplitLine.size() > 1)
				{
					CStringArray asGrades;
					split( asSplitLine[1], ",", asGrades );
					for( unsigned d=0; d<asGrades.size(); d++ )
					{
						m_bgainfo[bgano].grades.push_back(StringToGrade(asGrades[d]));
					}

				}
				if(!asSplitLine[0].CompareNoCase("style") && asSplitLine.size() > 1)
				{
					LOG->Info("Comparing Styles");
					CStringArray asStyles;
					split( asSplitLine[1], ",", asStyles );
					for( unsigned d=0; d<asStyles.size(); d++ )
					{
						LOG->Info( "Style:%s", asStyles[d].c_str() );

						m_bgainfo[bgano].styles.push_back(GAMEMAN->GameAndStringToStyle(GAMESTATE->m_pCurGame,asStyles[d]));
					}

				}


			}

		}
		if(bganimtouse.CompareNoCase("")!=0)
		{
			LOG->Info("Best Match BGA Was: %s",bganimtouse.c_str());
			bganim.LoadFromAniDir( THEME->GetPathToB(bganimtouse) );
		}

	}
	file.Close(); 
}

void ConditionalBGA::Update( float fDeltaTime )
{
	bganim.Update(fDeltaTime);
}

void ConditionalBGA::DrawPrimitives()
{
	bganim.Draw();
}

void ConditionalBGA::ClearINFO(int iEntry)
{
	m_bgainfo[iEntry].bganame = "";
	m_bgainfo[iEntry].songtitle = "";
	m_bgainfo[iEntry].songartist = "";
	m_bgainfo[iEntry].songdays.clear();
	m_bgainfo[iEntry].songdows.clear();
	m_bgainfo[iEntry].songmeters.clear();
	m_bgainfo[iEntry].difficulties.clear();
	m_bgainfo[iEntry].songmonths.clear();
	m_bgainfo[iEntry].styles.clear();
	m_bgainfo[iEntry].cleared = CBGA_CSUNUSED;
	m_bgainfo[iEntry].dpoused = false;
}

void ConditionalBGA::CheckBgaRequirements(BgaCondInfo info)
{
	LOG->Trace("Checking Conditions For BGA: %s",info.bganame.c_str());				


/*
	for(unsigned d=0; d<info.difficulties.size(); d++)
	{
		LOG->Trace("Difficulty: %d",info.difficulties[d]);
	}
	for(d=0; d<info.songmonths.size(); d++)
	{
		LOG->Trace("SongMonth: %d",info.songmonths[d]);
	}	
	for(d=0; d<info.songdays.size(); d++)
	{
		LOG->Trace("SongDay: %d",info.songdays[d]);
	}

	LOG->Trace("Clear Conditon: %d",info.cleared);
	LOG->Trace("SongTitle: %s",info.songtitle.c_str());
	LOG->Trace("SongArtist: %s",info.songartist.c_str());

*/
	bool valid = true; // valid until proven otherwise.
	bool hasconditions = false; // if there is a bga with no conditions for display
								// then we dont want it to appear

	if(info.songtitle.CompareNoCase("")!=0) // not equal
	{
	//	LOG->Info("COMP: %s",info.songtitle.c_str());

		hasconditions = true;
		if(GAMESTATE->m_pCurSong != NULL)
		{
			if(!GAMESTATE->m_pCurSong->m_sMainTitle.CompareNoCase(info.songtitle))
			{
				LOG->Info("SongTitle Matches");
				// valid
			}
			else
				valid = false; // different song
		}
		else // song not being played at the time
		{
			valid = false;
		}
	}

	if(info.songartist.CompareNoCase("")!=0 && valid) // not equal
	{
	//	LOG->Info("COMP: %s",info.songtitle.c_str());

		hasconditions = true;
		if(GAMESTATE->m_pCurSong != NULL)
		{
			if(!GAMESTATE->m_pCurSong->m_sArtist.CompareNoCase(info.songartist))
			{
				LOG->Info("SongArtist Matches");
				// valid
			}
			else
				valid = false; // different song
		}
		else // song not being played at the time
		{
			valid = false;
		}
	}


	if(!info.difficulties.empty() && valid) // dont bother checking any more if its already been invalidated.
	{
	//	LOG->Info("Checking Difficulties");
		hasconditions = true;
		bool foundmatchingdiff=false;
		for(unsigned d=0;d<info.difficulties.size();d++)
		{
			FOREACH_EnabledPlayer( pn )
			{
				if(GAMESTATE->m_pCurSteps[pn] != NULL)
				{
					if(GAMESTATE->m_pCurSteps[pn]->GetDifficulty() == info.difficulties[d])
					{
						foundmatchingdiff = true;
						LOG->Info("Found Valid Difficulty");
				
					}
				}
			}
		}
		valid = foundmatchingdiff;
	}

	if(info.dpoused)
	{
		PlayerOptions po = info.disallowedpo;
		bool bModsValid = true;
		FOREACH_EnabledPlayer( pn )
		{	
			unsigned md;
			for(md=0;md<PlayerOptions::NUM_ACCELS;md++)
			{
				if(po.m_fAccels[md] != 0.0f && GAMESTATE->m_PlayerOptions[pn].m_fAccels[md] != 0.0f)
				{
					bModsValid=false;
					LOG->Info("Found Invalid Accel Mod");
				}
			}
			for(md=0;md<PlayerOptions::NUM_EFFECTS;md++)
			{
				if(po.m_fEffects[md] != 0.0f && GAMESTATE->m_PlayerOptions[pn].m_fEffects[md] != 0.0f)
				{
					bModsValid=false;
					LOG->Info("Found Invalid Effect Mod");
				}
			}
			for(md=0;md<PlayerOptions::NUM_APPEARANCES;md++)
			{
				if(po.m_fAppearances[md] != 0.0f && GAMESTATE->m_PlayerOptions[pn].m_fAppearances[md] != 0.0f)
				{
					bModsValid=false;
					LOG->Info("Found Invalid Appearance Mod");
				}
			}
			for(md=0;md<PlayerOptions::NUM_TURNS;md++)
			{
				if(po.m_bTurns[md] != 0.0f && GAMESTATE->m_PlayerOptions[pn].m_bTurns[md] != 0.0f)
				{
					bModsValid=false;
					LOG->Info("Found Invalid Turn Mod");
				}
			}
			for(md=0;md<PlayerOptions::NUM_TRANSFORMS;md++)
			{
				if(po.m_bTransforms[md] != 0.0f && GAMESTATE->m_PlayerOptions[pn].m_bTransforms[md] != 0.0f)
				{
					bModsValid=false;
					LOG->Info("Found Invalid Transform Mod");
				}
			}
		}
		valid = bModsValid;
	}

	if(!info.songmeters.empty() && valid) // dont bother checking any more if its already been invalidated.
	{
		hasconditions = true;
		bool foundmatchingmeter=false;
		for(unsigned d=0;d<info.songmeters.size();d++)
		{
			LOG->Info("MeterRating: %d",info.songmeters[d]);
			FOREACH_EnabledPlayer( pn )
			{
				if(GAMESTATE->m_pCurSteps[pn] != NULL)
				{			
					if(info.songmeters[d] < 0) // negative values stored mean we want to check a value greaterthan or equal to its negative
					{
						// first make it positive then check to see if the footrating is >= to it
						int tmp = 0 - info.songmeters[d];
						if(GAMESTATE->m_pCurSteps[pn]->GetMeter() >= tmp)
						{
							LOG->Info("Found Valid MeterRating");
							foundmatchingmeter = true;
						}
					}
					else if(GAMESTATE->m_pCurSteps[pn]->GetMeter() == info.songmeters[d])
					{
						LOG->Info("Found Valid MeterRating");
						foundmatchingmeter = true;
					}
				}
			}
		}
		valid = foundmatchingmeter;
	}


	if(!info.styles.empty() && valid)
	{
		hasconditions = true;
		bool foundmatchingstyle=false;
		for(unsigned d=0;d<info.styles.size();d++)
		{
			//LOG->Info("info.styles = %d m_CurStyle = %d",info.styles[d],GAMESTATE->m_pCurStyle);
			if(info.styles[d] == GAMESTATE->m_pCurStyle)
			{
				foundmatchingstyle = true;
				LOG->Info("Found Valid Style");
			}
		
		}
		valid = foundmatchingstyle;
	}

	if(!info.grades.empty() && valid) // dont bother checking any more if its already been invalidated.
	{
	//	LOG->Info("Checking Difficulties");
		hasconditions = true;
		bool foundmatchinggrades=true; // assume true until proven otherwise
			if(info.grades.size() == 1) // only looking for a single grade
			{
				LOG->Info("Checking Single Grade");
				bool foundaplayerwithgrade = false;
				FOREACH_EnabledPlayer( pn )
				{
					if(g_CurStageStats.GetGrade(pn) == info.grades[0])
					{
						LOG->Info("Found Valid Grade");
						foundaplayerwithgrade = true;
					}
				}
				foundmatchinggrades = foundaplayerwithgrade;
			}
			else if(g_vPlayedStageStats.size() < info.grades.size()) // we've not played enough stages to achieve a grade history condition
			{
				LOG->Info("Not Enough Stages Played To Compare Grade History");
				foundmatchinggrades = false;
			}
			else // we have to check the history of grades to ensure they met the spec asked
			{
				LOG->Info("Checking Grade History");
				LOG->Info("Stage Stats Size: %u NumConditionalGrades: %u",
                          unsigned(g_vPlayedStageStats.size()), unsigned(info.grades.size()));
				bool foundavalidgradeforstage = false;
				for(unsigned d=info.grades.size()-1,g=g_vPlayedStageStats.size()-1;d>0;d--,g--)
				{
					if(d>g) ASSERT(0); // this should never happen.

					for(unsigned pn=0; pn<NUM_PLAYERS;pn++)
					{
						if(GAMESTATE->IsPlayerEnabled((PlayerNumber)pn))
						{
							LOG->Info("Player%d Grade: %d :: Expected Grade: %d",pn,g_vPlayedStageStats[g].GetGrade((PlayerNumber)pn),info.grades[d]);
							if(g_vPlayedStageStats[g].GetGrade((PlayerNumber)pn) == info.grades[d])
							{
								LOG->Info("One Valid Grade");
								foundavalidgradeforstage = true;
							}
						}
					}
				}
				foundmatchinggrades = foundavalidgradeforstage;
			}
	
		valid = foundmatchinggrades;
	}


	if(!info.songmonths.empty() && valid)
	{
		time_t rawtime;
		time(&rawtime);
		struct tm ptm;
		localtime_r( &rawtime, &ptm );
		int month = ptm.tm_mon; /* Month 0..11 */
		LOG->Info("Month: %d",month);
		bool foundvalidmonth = false;
		for(unsigned d=0;d<info.songmonths.size();d++)
		{	
			LOG->Info("Month: %d, THIS Month: %d",info.songmonths[d],month);
			if(info.songmonths[d] == month)
			{
				foundvalidmonth = true;
				LOG->Info("Found Valid Month");
			}
		}
		valid = foundvalidmonth;
	}


	if(!info.songdays.empty() && valid)
	{
		time_t rawtime;
		time(&rawtime);
		struct tm ptm;
		localtime_r( &rawtime, &ptm );
		int day = ptm.tm_mday; /* day in the month */
		LOG->Info("DAy: %d",day);
		bool foundvalidday = false;
		for(unsigned d=0;d<info.songdays.size();d++)
		{	
			LOG->Info("Day: %d, THIS Day: %d",info.songdays[d],day);
			if(info.songdays[d] == day)
			{
				foundvalidday = true;
				LOG->Info("Found Valid Day");
			}
		}
		valid = foundvalidday;
	}


	if(info.cleared != CBGA_CSUNUSED && valid) // already found invalid? forget it :>
	{
		bool foundclearcond = false;
		if(info.cleared == CBGA_CSFAILED)
		{
			if(GAMESTATE->AllAreDead() || g_CurStageStats.AllFailed()) // they met the fail condition
			{
				LOG->Info("Failed Condition");
				foundclearcond = true;
			}
		}
		else if(info.cleared == CBGA_CSCLEARED)
		{
			if(!g_CurStageStats.AllFailed() || !GAMESTATE->AllAreDead() ) // stage was cleared
			{
				LOG->Info("Cleared Condition");
				foundclearcond = true;
			}
		}
		else if(info.cleared == CBGA_CSMAXCOMBO)
		{
			FOREACH_EnabledPlayer( pn )
				if(g_CurStageStats.FullCombo((PlayerNumber)pn))
				{
					foundclearcond = true;
					LOG->Info("MaxCombo Condition");
				}
		}
		else if(info.cleared == CBGA_CSBROKECOMBO)
		{
			FOREACH_EnabledPlayer( pn )
				if(!g_CurStageStats.FullCombo((PlayerNumber)pn))
				{
					LOG->Info("BrokenCombo Condition");
					foundclearcond = true;
				}
		}

		valid = foundclearcond;
	}

	if(valid && hasconditions)
	{
		LOG->Info("Valid");
		bganimtouse = info.bganame;
	}
}

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
