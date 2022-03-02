#include "global.h"
#include "NotesWriterJson.h"
#include "TimingData.h"
#include "json/json.h"
#include "JsonUtil.h"
#include "Song.h"
#include "BackgroundUtil.h"
#include "Steps.h"
#include "NoteData.h"
#include "GameManager.h"

static void Serialize(const TimingSegment &seg, Json::Value &root)
{
	root["Beat"] = seg.GetBeat();
	if (seg.GetType() == SEGMENT_BPM)
	{
		root["BPM"] = static_cast<BPMSegment &>(const_cast<TimingSegment &>(seg)).GetBPM();
	}
	else
	{
		root["Seconds"] = static_cast<StopSegment &>(const_cast<TimingSegment &>(seg)).GetPause();
	}
}

static void Serialize(const TimingData &td, Json::Value &root)
{
	JsonUtil::SerializeVectorPointers( td.GetTimingSegments(SEGMENT_BPM), Serialize, root["BpmSegments"] );
	JsonUtil::SerializeVectorPointers( td.GetTimingSegments(SEGMENT_STOP), Serialize, root["StopSegments"] );
}

static void Serialize(const LyricSegment &o, Json::Value &root)
{
	root["StartTime"] = (float)o.m_fStartTime;
	root["Lyric"] = o.m_sLyric;
	root["Color"] = o.m_Color.ToString();
}

static void Serialize(const BackgroundDef &o, Json::Value &root)
{
	root["Effect"] = o.m_sEffect;
	root["File1"] = o.m_sFile1;
	root["File2"] = o.m_sFile2;
	root["Color1"] = o.m_sColor1;
}

static void Serialize(const BackgroundChange &o, Json::Value &root )
{
	Serialize( o.m_def, root["Def"] );
	root["StartBeat"] = o.m_fStartBeat;
	root["Rate"] = o.m_fRate;
	root["Transition"] = o.m_sTransition;
}

static void Serialize( const TapNote &o, Json::Value &root )
{
	root = Json::Value(Json::objectValue);

	if( o.type != TapNoteType_Tap )
		root["Type"] = (int)o.type;
	if( o.type == TapNoteType_HoldHead )
		root["SubType"] = (int)o.subType;
	//root["Source"] = (int)source;
	if( !o.sAttackModifiers.empty() )
		root["AttackModifiers"] = o.sAttackModifiers;
	if( o.fAttackDurationSeconds > 0 )
		root["AttackDurationSeconds"] = o.fAttackDurationSeconds;
	if( o.iKeysoundIndex != -1 )
		root["KeysoundIndex"] = o.iKeysoundIndex;
	if( o.iDuration > 0 )
		root["Duration"] = o.iDuration;
	if( o.pn != PLAYER_INVALID )
		root["PlayerNumber"] = (int)o.pn;
}

static void Serialize( const NoteData &o, Json::Value &root )
{
	root = Json::Value(Json::arrayValue);
	for(int t=0; t < o.GetNumTracks(); t++ )
	{
		NoteData::TrackMap::const_iterator begin, end;
		o.GetTapNoteRange( t, 0, MAX_NOTE_ROW, begin, end );
		for( ; begin != end; ++begin )
		{
			int iRow = begin->first;
			TapNote tn = begin->second;
			root.resize( root.size()+1 );
			Json::Value &root2 = root[ root.size()-1 ];
			root2 = Json::Value(Json::arrayValue);
			root2.resize(3);
			root2[(unsigned)0] = NoteRowToBeat(iRow);
			root2[1] = t;
			Serialize( tn, root2[2] );
		}
	}
}

static void Serialize( const RadarValues &o, Json::Value &root )
{
	FOREACH_ENUM( RadarCategory, rc )
	{
		root[ RadarCategoryToString(rc) ] = o[rc];
	}
}

static void Serialize( const Steps &o, Json::Value &root )
{
	root["StepsType"] = StringConversion::ToString(o.m_StepsType);

	o.Decompress();

	NoteData nd;
	o.GetNoteData( nd );
	Serialize( nd, root["NoteData"] );
	root["Hash"] = o.GetHash();
	root["Description"] = o.GetDescription();
	root["Difficulty"] = DifficultyToString(o.GetDifficulty());
	root["Meter"] = o.GetMeter();
	Serialize( o.GetRadarValues( PLAYER_1 ), root["RadarValues"] );
}


bool NotesWriterJson::WriteSong( const RString &sFile, const Song &out, bool bWriteSteps )
{
	Json::Value root;
	root["SongDir"] = out.GetSongDir();
	root["GroupName"] = out.m_sGroupName;
	root["Title"] = out.m_sMainTitle;
	root["SubTitle"] = out.m_sSubTitle;
	root["Artist"] = out.m_sArtist;
	root["TitleTranslit"] = out.m_sMainTitleTranslit;
	root["SubTitleTranslit"] = out.m_sSubTitleTranslit;
	root["Genre"] = out.m_sGenre;
	root["Credit"] = out.m_sCredit;
	root["Banner"] = out.m_sBannerFile;
	root["Background"] = out.m_sBackgroundFile;
	root["LyricsFile"] = out.m_sLyricsFile;
	root["CDTitle"] = out.m_sCDTitleFile;
	root["Music"] = out.m_sMusicFile;
	root["Offset"] = out.m_SongTiming.m_fBeat0OffsetInSeconds;
	root["SampleStart"] = out.m_fMusicSampleStartSeconds;
	root["SampleLength"] = out.m_fMusicSampleLengthSeconds;
	if( out.m_SelectionDisplay == Song::SHOW_ALWAYS )
		root["Selectable"] = "YES";
	else if( out.m_SelectionDisplay == Song::SHOW_NEVER )
		root["Selectable"] = "NO";
	else
		root["Selectable"] = "YES";

	root["FirstBeat"] = out.GetFirstBeat();
	root["LastBeat"] = out.GetLastBeat();
	root["SongFileName"] = out.m_sSongFileName;
	root["HasMusic"] = out.m_bHasMusic;
	root["HasBanner"] = out.m_bHasBanner;
	root["MusicLengthSeconds"] = out.m_fMusicLengthSeconds;

	root["DisplayBpmType"] = StringConversion::ToString(out.m_DisplayBPMType);
	if( out.m_DisplayBPMType == DISPLAY_BPM_SPECIFIED )
	{
		root["SpecifiedBpmMin"] = out.m_fSpecifiedBPMMin;
		root["SpecifiedBpmMax"] = out.m_fSpecifiedBPMMax;
	}

	Serialize( out.m_SongTiming, root["TimingData"] );
	JsonUtil::SerializeVectorObjects( out.m_LyricSegments, Serialize, root["LyricSegments"] );

	{
		Json::Value &root2 = root["BackgroundChanges"];
		FOREACH_BackgroundLayer( bl )
		{
			Json::Value &root3 = root2[bl];
			const vector<BackgroundChange> &vBgc = out.GetBackgroundChanges(bl);
			JsonUtil::SerializeVectorObjects( vBgc, Serialize, root3 );
		}
	}

	{
		const vector<BackgroundChange> &vBgc = out.GetForegroundChanges();
		JsonUtil::SerializeVectorObjects( vBgc, Serialize, root["ForegroundChanges"] );
	}

	JsonUtil::SerializeArrayValues( out.m_vsKeysoundFile, root["KeySounds"] );

	if( bWriteSteps )
	{
		vector<const Steps*> vpSteps;
		for (Steps * iter : out.GetAllSteps())
		{
			if( iter->IsAutogen() )
				continue;
			vpSteps.push_back( iter );
		}
		JsonUtil::SerializeVectorPointers<Steps>( vpSteps, Serialize, root["Charts"] );
	}

	return JsonUtil::WriteFile( root, sFile, false );
}

bool NotesWriterJson::WriteSteps( const RString &sFile, const Steps &out )
{
	Json::Value root;
	Serialize( out, root );
	return JsonUtil::WriteFile( root, sFile, false );
}

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
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
