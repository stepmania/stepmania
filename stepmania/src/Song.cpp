#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: Song

 Desc: Holds metadata for a song and the song's step data.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Glenn Maynard
-----------------------------------------------------------------------------
*/

#include "Notes.h"
#include "RageUtil.h"
#include <math.h>	// for fmod
#include "RageLog.h"
#include "IniFile.h"
#include "Song.h"
#include "NoteData.h"
#include "MsdFile.h"
#include "RageSound.h"
#include "RageException.h"
#include "SongCacheIndex.h"
#include "GameManager.h"
#include "PrefsManager.h"
#include "StyleDef.h"
#include "Notes.h"
#include "GameState.h"
#include "FontCharAliases.h"

#include "NotesLoaderSM.h"
#include "NotesLoaderDWI.h"
#include "NotesLoaderBMS.h"
#include "NotesLoaderKSF.h"
#include "NotesWriterDWI.h"
#include "NotesWriterSM.h"

#include "SDL.h"
#include "SDL_image.h"


const int FILE_CACHE_VERSION = 107;	// increment this when Song or Notes changes to invalidate cache


static int CompareBPMSegments(const BPMSegment &seg1, const BPMSegment &seg2)
{
	return seg1.m_fStartBeat < seg2.m_fStartBeat;
}

void SortBPMSegmentsArray( vector<BPMSegment> &arrayBPMSegments )
{
	sort( arrayBPMSegments.begin(), arrayBPMSegments.end(), CompareBPMSegments );
}

static int CompareStopSegments(const StopSegment &seg1, const StopSegment &seg2)
{
	return seg1.m_fStartBeat < seg2.m_fStartBeat;
}

void SortStopSegmentsArray( vector<StopSegment> &arrayStopSegments )
{
	sort( arrayStopSegments.begin(), arrayStopSegments.end(), CompareStopSegments );
}

int CompareBackgroundChanges(const BackgroundChange &seg1, const BackgroundChange &seg2)
{
	return seg1.m_fStartBeat < seg2.m_fStartBeat;
}

void SortBackgroundChangesArray( vector<BackgroundChange> &arrayBackgroundChanges )
{
	sort( arrayBackgroundChanges.begin(), arrayBackgroundChanges.end(), CompareBackgroundChanges );
}


//////////////////////////////
// Song
//////////////////////////////
Song::Song()
{
	m_bChangedSinceSave = false;
	m_fBeat0OffsetInSeconds = 0;
	m_fMusicSampleStartSeconds = 0;
	m_fMusicSampleLengthSeconds = 12.0f;	// start fading out at m_fMusicSampleLengthSeconds-1 seconds
	m_iMusicBytes = 0;
	m_fMusicLengthSeconds = 0;
	m_fFirstBeat = -1;
	m_fLastBeat = -1;
	m_SelectionDisplay = SHOW_ALWAYS;
}

Song::~Song()
{
	for( unsigned i=0; i<m_apNotes.size(); i++ )
		SAFE_DELETE( m_apNotes[i] );

	m_apNotes.clear();
}


void Song::AddBPMSegment( BPMSegment seg )
{
	m_BPMSegments.push_back( seg );
	SortBPMSegmentsArray( m_BPMSegments );
}


void Song::AddStopSegment( StopSegment seg )
{
	m_StopSegments.push_back( seg );
	SortStopSegmentsArray( m_StopSegments );
}


void Song::AddBackgroundChange( BackgroundChange seg )
{
	m_BackgroundChanges.push_back( seg );
	SortBackgroundChangesArray( m_BackgroundChanges );
}

float Song::GetMusicStartBeat() const
{
	float fBPS = m_BPMSegments[0].m_fBPM / 60.0f; 
	return -m_fBeat0OffsetInSeconds*fBPS; 
};

void Song::GetBeatAndBPSFromElapsedTime( float fElapsedTime, float &fBeatOut, float &fBPSOut, bool &bFreezeOut ) const
{
//	LOG->Trace( "GetBeatAndBPSFromElapsedTime( fElapsedTime = %f )", fElapsedTime );
	// This function is a nightmare.  Don't even try to understand it. :-)

	fElapsedTime += m_fBeat0OffsetInSeconds;


	for( unsigned i=0; i<m_BPMSegments.size(); i++ ) // foreach BPMSegment
	{
		float fStartBeatThisSegment = m_BPMSegments[i].m_fStartBeat;
		bool bIsLastBPMSegment = i==m_BPMSegments.size()-1;
		float fStartBeatNextSegment = bIsLastBPMSegment ? 40000/*inf*/ : m_BPMSegments[i+1].m_fStartBeat; 
		float fBeatsInThisSegment = fStartBeatNextSegment - fStartBeatThisSegment;
		float fBPM = m_BPMSegments[i].m_fBPM;
		float fBPS = fBPM / 60.0f;

		// calculate the number of seconds in this segment
		float fSecondsInThisSegment =  fBeatsInThisSegment / fBPS;
		unsigned j;
		for( j=0; j<m_StopSegments.size(); j++ )	// foreach freeze
		{
			if( fStartBeatThisSegment <= m_StopSegments[j].m_fStartBeat  &&  m_StopSegments[j].m_fStartBeat < fStartBeatNextSegment )	
			{
				// this freeze lies within this BPMSegment
				fSecondsInThisSegment += m_StopSegments[j].m_fStopSeconds;
			}
		}


		if( fElapsedTime > fSecondsInThisSegment )
		{
			// this BPMSegement is NOT the current segment
			fElapsedTime -= fSecondsInThisSegment;
			continue;
		}

		// this BPMSegment IS the current segment

		float fBeatEstimate = fStartBeatThisSegment + fElapsedTime*fBPS;

		for( j=0; j<m_StopSegments.size(); j++ )	// foreach freeze
		{
			if( fStartBeatThisSegment > m_StopSegments[j].m_fStartBeat  ||
				m_StopSegments[j].m_fStartBeat > fStartBeatNextSegment )	
				continue;

			// this freeze lies within this BPMSegment

			if( m_StopSegments[j].m_fStartBeat > fBeatEstimate )
				break;

			fElapsedTime -= m_StopSegments[j].m_fStopSeconds;
			// re-estimate
			fBeatEstimate = fStartBeatThisSegment + fElapsedTime*fBPS;
			if( fBeatEstimate < m_StopSegments[j].m_fStartBeat )
			{
				fBeatOut = m_StopSegments[j].m_fStartBeat;
				fBPSOut = fBPS;
				bFreezeOut = true;
				return;
			}
		}

		fBeatOut = fBeatEstimate;
		fBPSOut = fBPS;
		bFreezeOut = false;
		return;
	}
}


// This is a super hack, but it's only called from ScreenEdit, so it's OK.
// Writing an inverse function of GetBeatAndBPSFromElapsedTime() uber difficult,
// so do a binary search to get close to the correct elapsed time.
float Song::GetElapsedTimeFromBeat( float fBeat ) const
{
	float fElapsedTimeBestGuess = this->m_fMusicLengthSeconds/2;	//  seconds
	float fSecondsToMove = fElapsedTimeBestGuess;	//  seconds
	float fBeatOut, fBPSOut;
	bool bFreezeOut;

	while( fSecondsToMove > 0.1f )
	{
		GetBeatAndBPSFromElapsedTime( fElapsedTimeBestGuess, fBeatOut, fBPSOut, bFreezeOut );
		if( fBeatOut > fBeat )
			fElapsedTimeBestGuess -= fSecondsToMove;
		else
			fElapsedTimeBestGuess += fSecondsToMove;

		fSecondsToMove /= 2;
	}

	return fElapsedTimeBestGuess;
}

CString Song::GetCacheFilePath() const
{
	return ssprintf( "Cache\\%u", GetHashForString(m_sSongDir) );
}

/* Get a path to the SM containing data for this song.  It might
 * be a cache file. */
const CString &Song::GetSongFilePath() const
{
	ASSERT ( m_sSongFileName.GetLength() != 0 );
	return m_sSongFileName;
}

NotesLoader *Song::MakeLoader( CString sDir ) const
{
	NotesLoader *ret;

	/* Actually, none of these have any persistant data, so we 
	 * could optimize this, but since they don't have any data,
	 * there's no real point ... */
	ret = new SMLoader;
	if(ret->Loadable( sDir )) return ret;
	delete ret;

	ret = new DWILoader;
	if(ret->Loadable( sDir )) return ret;
	delete ret;

	ret = new BMSLoader;
	if(ret->Loadable( sDir )) return ret;
	delete ret;

	ret = new KSFLoader;
	if(ret->Loadable( sDir )) return ret;
	delete ret;

	return NULL;
}

bool Song::LoadWithoutCache( CString sDir )
{
	//
	// There was no entry in the cache for this song.
	// Let's load it from a file, then write a cache entry.
	//
	
	NotesLoader *ld = MakeLoader( sDir );
	if(!ld)
	{
		LOG->Warn( "Couldn't find any SM, DWI, BMS, or KSF files in '%s'.  This is not a valid song directory.", sDir.GetString() );
		return false;
	}

	bool success = ld->LoadFromDir( sDir, *this );
	delete ld;

	if(!success)
		return false;

	TidyUpData();

	// save a cache file so we don't have to parse it all over again next time
	SaveToCacheFile();
	return true;
}

bool Song::LoadFromSongDir( CString sDir )
{
//	LOG->Trace( "Song::LoadFromSongDir(%s)", sDir.GetString() );

	sDir.Replace("\\", "/");
	// make sure there is a trailing '\\' at the end of sDir
	if( sDir.Right(1) != "/" )
		sDir += "/";

	// save song dir
	m_sSongDir = sDir;

	// save group name
	CStringArray sDirectoryParts;
	split( m_sSongDir, "/", sDirectoryParts, false );
	m_sGroupName = sDirectoryParts[sDirectoryParts.size()-3];	// second from last item

	//
	// First look in the cache for this song (without loading NoteData)
	//
	unsigned uDirHash = SONGINDEX->GetCacheHash(m_sSongDir);
	if( GetHashForDirectory(m_sSongDir) == uDirHash && // this cache is up to date 
		DoesFileExist(GetCacheFilePath()))	
	{
//		LOG->Trace( "Loading '%s' from cache file '%s'.", m_sSongDir.GetString(), GetCacheFilePath().GetString() );
		SMLoader ld;
		ld.LoadFromSMFile( GetCacheFilePath(), *this, true );
	}
	else
	{
		if(!LoadWithoutCache(m_sSongDir))
			return false;
	}

	{
		/* Generated filename; this doesn't always point to a loadable file,
		 * but instead points to the file we should write changed files to,
		 * and will always be an .SM.
		 *
		 * This is a little tricky.  We can't always use the song title directly,
		 * since it might contain characters we can't store in filenames.  Two
		 * easy options: we could manually filter out invalid characters, or we
		 * could use the name of the directory, which is always a valid filename
		 * and should always be the same as the song.  The former might not catch
		 * everything--filename restrictions are platform-specific; we might even
		 * be on an 8.3 filesystem, so let's do the latter.
		 *
		 * We can't rely on searching for other data filenames; it works for DWIs,
		 * but not KSFs and BMSs.
		 *
		 * So, let's do this (by priority):
		 * 1. If there's an .SM file, use that filename.  No reason to use anything
		 *    else; it's the filename in use.
		 * 2. If there's a .DWI, use it with a changed extension.
		 * 3. Otherwise, use the name of the directory, since it's definitely a valid
		 *    filename, and should always be the title of the song (unlike KSFs).
		 */
		m_sSongFileName = m_sSongDir;
		CStringArray asFileNames;
		GetDirListing( m_sSongDir+"*.sm", asFileNames );
		if( !asFileNames.empty() )
			m_sSongFileName += asFileNames[0];
		else {
			GetDirListing( m_sSongDir+"*.dwi", asFileNames );
			if( !asFileNames.empty() ) {
				m_sSongFileName += asFileNames[0];
				/* XXX: This would mess up "vote.for.dwight.d.eisenhower.dwi". */
				m_sSongFileName.Replace( ".dwi", ".sm" );
			} else {
				m_sSongFileName += sDirectoryParts[sDirectoryParts.size()-2];	// last item
				m_sSongFileName += ".sm";
			}
		}
	}

	/* Add AutoGen pointers.  (These aren't cached.) */
	AddAutoGenNotes();

	return true;
}


void Song::TidyUpData()
{
	if( !HasMusic() )
	{
		CStringArray arrayPossibleMusic;
		GetDirListing( m_sSongDir + CString("*.mp3"), arrayPossibleMusic );
		GetDirListing( m_sSongDir + CString("*.ogg"), arrayPossibleMusic );
		GetDirListing( m_sSongDir + CString("*.wav"), arrayPossibleMusic );

		if( !arrayPossibleMusic.empty() )		// we found a match
			m_sMusicFile = arrayPossibleMusic[0];
//		Don't throw on missing music.  -Chris
//		else
//			RageException::Throw( "The song in '%s' is missing a music file.  You must place a music file in the song folder or remove the song", m_sSongDir.GetString() );
	}

	/* This must be done before radar calculation. */
	if( HasMusic() )
	{
		RageSound sound;
		sound.Load( GetMusicPath(), false ); /* don't pre-cache */

		m_fMusicLengthSeconds = sound.GetLengthSeconds();
		/* XXX: if(m_fMusicLengthSeconds == -1), warn and throw out the song */
		if(m_fMusicLengthSeconds == 0)
		{
			LOG->Warn("File %s is empty?", GetMusicPath().GetString());
		} else if(m_fMusicLengthSeconds == -1) {
			LOG->Warn("File %s: error getting length", GetMusicPath().GetString());
		}
	}
	else	// ! HasMusic()
	{
		m_fMusicLengthSeconds = 100;		// guess
		LOG->Warn("File %s has no music; guessing at %i seconds", m_fMusicLengthSeconds);
	}

	/* Generate these before we autogen notes, so the new notes can inherit
	 * their source's values. */
	ReCalculateRadarValuesAndLastBeat();

	TrimRight(m_sMainTitle);
	if( m_sMainTitle == "" )	m_sMainTitle = "Untitled song";
	TrimRight(m_sSubTitle);
	if( m_sArtist == "" )		m_sArtist = "Unknown artist";
	TranslateTitles();

	/* XXX don't throw due to broken songs */
	if( m_BPMSegments.empty() )
		RageException::Throw( "No #BPM specified in '%s%s.'", m_sSongDir.GetString(), m_sSongFileName.GetString() );

	// We're going to try and do something intelligent here...
	// The MusicSampleStart always seems to be about 100-120 beats into 
	// the song regardless of BPM.  Let's take a shot-in-the dark guess.
	if( m_fMusicSampleStartSeconds == 0 )
		m_fMusicSampleStartSeconds = this->GetElapsedTimeFromBeat( 100 );

	/* Some DWIs have lengths in ms when they meant seconds, eg. #SAMPLELENGTH:10;.
	 * If the sample length is way too short, change it. */
	if( m_fMusicSampleLengthSeconds < 3 )
		m_fMusicSampleLengthSeconds = 10;

	//
	// Here's the problem:  We have a directory full of images.  We want to determine which 
	// image is the banner, which is the background, and which is the CDTitle.
	//

	//
	// First, check the file name for hints.
	//
	if( !HasBanner() )
	{
		m_sBannerFile = "";

		// find an image with "banner" in the file name
		CStringArray arrayPossibleBanners;
		GetDirListing( m_sSongDir + CString("*banner*.png"), arrayPossibleBanners );
		GetDirListing( m_sSongDir + CString("*banner*.jpg"), arrayPossibleBanners );
		GetDirListing( m_sSongDir + CString("*banner*.bmp"), arrayPossibleBanners );
		GetDirListing( m_sSongDir + CString("*banner*.gif"), arrayPossibleBanners );
		if( !arrayPossibleBanners.empty() )
			m_sBannerFile = arrayPossibleBanners[0];
	}

	if( !HasBackground() )
	{
		m_sBackgroundFile = "";

		// find an image with "bg" or "background" in the file name
		CStringArray arrayPossibleBGs;
		GetDirListing( m_sSongDir + CString("*bg*.png"), arrayPossibleBGs );
		GetDirListing( m_sSongDir + CString("*bg*.jpg"), arrayPossibleBGs );
		GetDirListing( m_sSongDir + CString("*bg*.bmp"), arrayPossibleBGs );
		GetDirListing( m_sSongDir + CString("*bg*.gif"), arrayPossibleBGs );
		GetDirListing( m_sSongDir + CString("*background*.png"), arrayPossibleBGs );
		GetDirListing( m_sSongDir + CString("*background*.jpg"), arrayPossibleBGs );
		GetDirListing( m_sSongDir + CString("*background*.bmp"), arrayPossibleBGs );
		GetDirListing( m_sSongDir + CString("*background*.gif"), arrayPossibleBGs );
		if( !arrayPossibleBGs.empty() )
			m_sBackgroundFile = arrayPossibleBGs[0];
	}

	if( !HasCDTitle() )
	{
		m_sCDTitleFile = "";

		// find an image with "cdtitle" in the file name
		CStringArray arrayPossibleCDTitles;
		GetDirListing( m_sSongDir + CString("*cdtitle*.png"), arrayPossibleCDTitles );
		GetDirListing( m_sSongDir + CString("*cdtitle*.jpg"), arrayPossibleCDTitles );
		GetDirListing( m_sSongDir + CString("*cdtitle*.bmp"), arrayPossibleCDTitles );
		GetDirListing( m_sSongDir + CString("*cdtitle*.gif"), arrayPossibleCDTitles );
		if( !arrayPossibleCDTitles.empty() )
			m_sCDTitleFile = arrayPossibleCDTitles[0];
	}


	//
	// Now, For the images we still haven't found, look at the image dimensions of the remaining unclassified images.
	//
	CStringArray arrayImages;
	GetDirListing( m_sSongDir + CString("*.png"), arrayImages );
	GetDirListing( m_sSongDir + CString("*.jpg"), arrayImages );
	GetDirListing( m_sSongDir + CString("*.bmp"), arrayImages );
	GetDirListing( m_sSongDir + CString("*.gif"), arrayImages );
	unsigned i;
	for( i=0; i<arrayImages.size(); i++ )	// foreach image
	{
		// Skip any image that we've already classified

		if( HasBanner()  &&  stricmp(m_sBannerFile, arrayImages[i])==0 )
			continue;	// skip

		if( HasBackground()  &&  stricmp(m_sBackgroundFile, arrayImages[i])==0 )
			continue;	// skip

		if( HasCDTitle()  &&  stricmp(m_sCDTitleFile, arrayImages[i])==0 )
			continue;	// skip

		SDL_Surface *img = IMG_Load( m_sSongDir + arrayImages[i] );
		if( img )
		{
			int width = img->w;
			int height = img->h;
			SDL_FreeSurface( img );

			if( !HasBackground()  &&  width >= 320  &&  height >= 240 )
			{
				m_sBackgroundFile = arrayImages[i];
				continue;
			}
			if( !HasBanner()  &&  100<width  &&  width<320  &&  50<height  &&  height<240 )
			{
				m_sBannerFile = arrayImages[i];
				continue;
			}
			if( !HasCDTitle()  &&  width<=100  &&  height<=50 )
			{
				m_sCDTitleFile = arrayImages[i];
				continue;
			}
		}
	}


	// If no BGChanges are specified and there are movies in the song directory, then assume
	// they are DWI style where the movie begins at beat 0.
	if( !HasBGChanges() )
	{
		CStringArray arrayPossibleMovies;
		GetDirListing( m_sSongDir + CString("*movie*.avi"), arrayPossibleMovies );
		GetDirListing( m_sSongDir + CString("*movie*.mpg"), arrayPossibleMovies );
		GetDirListing( m_sSongDir + CString("*movie*.mpeg"), arrayPossibleMovies );
		GetDirListing( m_sSongDir + CString("*.avi"), arrayPossibleMovies );
		GetDirListing( m_sSongDir + CString("*.mpg"), arrayPossibleMovies );
		GetDirListing( m_sSongDir + CString("*.mpeg"), arrayPossibleMovies );
		if( arrayPossibleMovies.size() == 1 )
		{
			CString sBGMovieFile = arrayPossibleMovies[0];
			
			// calculate start beat of music
			float fMusicStartBeat, fBPS;
			bool bFreeze;
			this->GetBeatAndBPSFromElapsedTime( -this->m_fBeat0OffsetInSeconds, fMusicStartBeat, fBPS, bFreeze );

			this->AddBackgroundChange( BackgroundChange(fMusicStartBeat,sBGMovieFile) );
		}
	}

	for( i=0; i<m_apNotes.size(); i++ )
		m_apNotes[i]->Compress();
}

struct TitleTrans
{
	Regex TitleFrom, SubFrom, ArtistFrom;
	CString TitleTo, SubTo, ArtistTo;			/* plain text */

	TitleTrans(CString tf, CString sf, CString af, CString tt, CString st, CString at):
		TitleFrom(tf), SubFrom(sf), ArtistFrom(af),
			TitleTo(tt), SubTo(st), ArtistTo(at) { }

	bool Matches(CString title, CString sub, CString artist)
	{
		if(!TitleFrom.Compare(title)) return false; /* no match */
		if(!SubFrom.Compare(sub)) return false; /* no match */
		if(!ArtistFrom.Compare(artist)) return false; /* no match */

		return true;
	}
};

void Song::TranslateTitles()
{
	/* XXX make theme metrics for these or something.  Candy makes it
	 * a little annoying ...*/

	static vector<TitleTrans> ttab;
	/* These REs aren't completely tested. XXX */
	if(ttab.empty())
	{
		/* Ambiguous, so check artist.  Do this early; Riyu will be replaced later: */
		ttab.push_back(TitleTrans("^Candy$", "", "^Luv.*", "CANDY &whitestar;", "", "") );
		ttab.push_back(TitleTrans("^Candy$", "", ".*Riyu.*", "CANDY &whiteheart;", "", "") );

		/* Make sure this appears after the above "Riyu" match, so it doesn't
		 * break it.  I've seen both "Kosaku" and "Kosaka"; I think Kosaka is
		 * correct, but handle both. */
		ttab.push_back(TitleTrans("", "", "Riyu Kosak[au]", "", "", "&kosaka1;&kosaka2;&hri;&hyu;") );
		ttab.push_back(TitleTrans("", "", "Kosak[au] Riyu", "", "", "&kosaka1;&kosaka2;&hri;&hyu;") );

		/* Matsuri Japan is often just "Japan".  There may be other songs by
		 * this name, too, so match the artist, too. */
		ttab.push_back(TitleTrans("^Japan$", "", "(Re-Venge)|(RevenG)", "&matsuri; JAPAN", "", "") );

		/* Fix up hacked titles: */
		ttab.push_back(TitleTrans("^Max 300$", "", "%", "", "", "&omega;") );
		ttab.push_back(TitleTrans("^Bre=kdown$", "", "", "Bre&flipped-a;kdown", "", "") );
		ttab.push_back(TitleTrans("^Candy #$", "", "", "Candy &whitestar;", "", "") );
		ttab.push_back(TitleTrans("^Candy \\$$", "", "", "Candy &whiteheart;", "", "") );
		ttab.push_back(TitleTrans("^} JAPAN$", "", "", "&matsuri; Japan", "", "") );
		ttab.push_back(TitleTrans("^\\+\\{$", "", "", "&kakumei1;&kakumei2;", "", "") );
		ttab.push_back(TitleTrans("^Sweet Sweet \\$ Magic$", "", "", "Sweet Sweet &whiteheart; Magic", "", "") );

		/* Special stuff is done.  Titles: */
		ttab.push_back(TitleTrans("^Matsuri Japan$", "", "", "&matsuri; Japan", "", "") );
		ttab.push_back(TitleTrans("^Kakumei$", "", "", "&kakumei1;&kakumei2;", "", "") );
		ttab.push_back(TitleTrans("^Sweet Sweet (Love )?Magic$", "", "", "Sweet Sweet &whiteheart; Magic", "", "") );
		ttab.push_back(TitleTrans("^Break ?Down!?$", "", "", "BRE&flipped-a;K DOWN!", "", "") );
		/* サナ・モレッテ・ネ・エンテ 
		 * People can't decide how they want to spell this, so cope with
		 * both l or r, and one or two l/r and t. */
		ttab.push_back(TitleTrans("^Sana Mo((ll?)|(rr?))et(t?)e Ne Ente", "", "", 
			"&ksa;&kna;&kdot;&kmo;&kre;&kq;&kte;&kdot;&kne;&kdot;&ke;&kn;&kte;", "", ""));
		ttab.push_back(TitleTrans("^Freckles$", "", "", "&hso;&hba;&hka;&hsu;", "", "") );
		ttab.push_back(TitleTrans("^Sobakasu$", "", "", "&hso;&hba;&hka;&hsu;", "", "") );

		/* 夜空のムコウ */
		ttab.push_back(TitleTrans("^Yozora no Muko$", "", "", "&yozora1;&yozora2;&hno;&kmu;&kko;", "", "") );

		/* 17才 */
		ttab.push_back(TitleTrans("^17 ?(Sai)?$", "", "", "17&sai;", "", "") );

		/* Handle "Mobo Moga", "Mobo * Moga"; spaces optional. XXX blackstar or whitestar?  Is this supposed to be capped? */
		ttab.push_back(TitleTrans("^Mobo ?\\*? ?Moga$", "", "", "Mobo&whitestar;Moga", "", "") );

		/* XXX whiteheart or blackheart? */
		ttab.push_back(TitleTrans("^Love (Love )?Shine$", "", "", "LOVE &whiteheart; SHINE", "", "") );

		/* ロマンスの神様 */
		ttab.push_back(TitleTrans("^God of Romance$", "", "", "&kro;&kma;&kn;&ksu;&hno;&kami;&sama;", "", "") );

		/* ダンシン・オール・アローン */
		ttab.push_back(TitleTrans("^Dancing Pompokolin$", "", "", "&kda;&kn;&ksi;&kn;&kdot;&ko;&kdash;&kru;&kdot;&ka;&kro;&kdash;&kn;", "", "") );

		/* 青い振動 */
		ttab.push_back(TitleTrans("^Aoi Shoudou$", "", "", "&aoi;&hi;&shoudou1;&shoudou2;", "", "") );
		ttab.push_back(TitleTrans("^Blue Impulse$", "", "", "&aoi;&hi;&shoudou1;&shoudou2;", "", "") );

		/* 大見解 */
		ttab.push_back(TitleTrans("^Daikenkai$", "", "", "&ookii;&kenkai1;&kenkai2;", "", "") );

		/* ♡LOVE²シュガ→♡ */
		ttab.push_back(TitleTrans("^Love Love Sugar$", "", "", "&whiteheart;LOVE&squared; &ksi;&kyus;&kga;&rightarrow;&whiteheart;", "", "") );

		/* 三毛猫ロック */
		ttab.push_back(TitleTrans("^Mikeneko Rock$", "", "", "&num-san;&hair;&neko;&kro;&kq;&kku;", "", "") );
		
		/* 桜 */
		ttab.push_back(TitleTrans("^Sakura$", "", "", "&sakura;", "", "") );

		/* 魔法の扉, スペース★マコのテーマ */
        ttab.push_back(TitleTrans("^(Door of Magic)|(Mahou no Tobira)$", "", "", "&mahou1;&mahou2;&hno;&tobira;", "&ksu;&kpe;&kdash;&ksu;&whitestar;&kma;&kko;&hno;&kti;&kdash;&kmu;", "") ); 

		/* Subtitles: */
		/* それぞれの明日 (title is Graduation) */
		ttab.push_back(TitleTrans("", "^Each Tomorrow$", "", "", "&hso;&hre;&hzo;&hre;&hno;&aka;&nichi;", "") );

		/* Artists: */
		ttab.push_back(TitleTrans("", "", "^Omega$", "", "", "&omega;") );
		ttab.push_back(TitleTrans("", "", "^ZZ$", "", "", "&doublezeta;") );

		/* 亜熱帯マジ-SKA爆弾 (serious tropical ska bomb? ruh roh) */
		ttab.push_back(TitleTrans("", "", "^Anettai Maji.*Ska (Bakudan|Bukuden)", "", "", "&anettai1;&anettai2;&anettai3;&kma;&kji;-SKA&bakudan1;&bakudan2;") );

		/* メキシコ民謡 */
		ttab.push_back(TitleTrans("", "", "^Spanish Folk Music$", "", "", "&kme;&kki;&ksi;&kko;&minyou1;&minyou2;") );

		/* 新谷さなえ (Sanae or incorrect Sana) */
		ttab.push_back(TitleTrans("", "", "Sanae? Shintani", "", "", "&shintani1;&shintani2;&hsa;&hna;&he;") );

		ttab.push_back(TitleTrans("", "", "dj TAKA feat. ?Noria", "", "", "dj TAKA feat. &hno;&hri;&ha;") );

		/* くにたけみゆき */
		ttab.push_back(TitleTrans("", "", "Miyuki Kunitake", "", "", "&hku;&hni;&hta;&hke;&hmi;&hyu;&hki;") );
		ttab.push_back(TitleTrans("", "", "Kunitake Miyuki", "", "", "&hku;&hni;&hta;&hke;&hmi;&hyu;&hki;") );
	}

	for(unsigned i = 0; i < ttab.size(); ++i)
	{
		if(!ttab[i].Matches(m_sMainTitle, m_sSubTitle, m_sArtist))
			continue;

		/* The song matches.  Replace whichever strings aren't empty: */
		if(!ttab[i].TitleTo.empty())
		{
			m_sMainTitleTranslit = m_sMainTitle;
			m_sMainTitle = ttab[i].TitleTo;
			FontCharAliases::ReplaceMarkers( m_sMainTitle );
		}
		if(!ttab[i].SubTo.empty())
		{
			m_sSubTitleTranslit = m_sSubTitle;
			m_sSubTitle = ttab[i].SubTo;
			FontCharAliases::ReplaceMarkers( m_sSubTitle );
		}
		if(!ttab[i].ArtistTo.empty())
		{
			m_sArtistTranslit = m_sArtist;
			m_sArtist = ttab[i].ArtistTo;
			FontCharAliases::ReplaceMarkers( m_sArtist );
		}
	}
}

void Song::ReCalculateRadarValuesAndLastBeat()
{
	//
	// calculate radar values and first/last beat
	//
	for( unsigned i=0; i<m_apNotes.size(); i++ )
	{
		Notes* pNotes = m_apNotes[i];
		NoteData tempNoteData;
		pNotes->GetNoteData( &tempNoteData );

		for( int r=0; r<NUM_RADAR_VALUES; r++ )
		{
			/* If it's autogen, radar vals come from the parent. */
			if(pNotes->IsAutogen())
				continue;

			pNotes->SetRadarValue(r, NoteDataUtil::GetRadarValue( tempNoteData, (RadarCategory)r, m_fMusicLengthSeconds ));
		}

		float fFirstBeat = tempNoteData.GetFirstBeat();
		float fLastBeat = tempNoteData.GetLastBeat();
		if( m_fFirstBeat == -1 )
			m_fFirstBeat = fFirstBeat;
		else
			m_fFirstBeat = min( m_fFirstBeat, fFirstBeat );
		if( m_fLastBeat == -1 )
			m_fLastBeat = fLastBeat;
		else
			m_fLastBeat = max( m_fLastBeat, fLastBeat );
	}
}

void Song::GetNotesThatMatch( NotesType nt, vector<Notes*>& arrayAddTo ) const
{
	for( unsigned i=0; i<m_apNotes.size(); i++ )	// for each of the Song's Notes
	{
		if( m_apNotes[i]->m_NotesType == nt )
			arrayAddTo.push_back( m_apNotes[i] );
	}
}

/* Return whether the song is playable in the given style. */
bool Song::SongCompleteForStyle( const StyleDef *st ) const
{
	if(!SongHasNotesType(st->m_NotesType))
		return false;
	return true;
}

bool Song::SongHasNotesType( NotesType nt ) const
{
	for( unsigned i=0; i < m_apNotes.size(); i++ ) // foreach Notes
		if( m_apNotes[i]->m_NotesType == nt )
			return true;
	return false;
}

bool Song::SongHasNotesTypeAndDifficulty( NotesType nt, Difficulty dc ) const
{
	for( unsigned i=0; i < m_apNotes.size(); i++ ) // foreach Notes
		if( m_apNotes[i]->m_NotesType == nt  &&  m_apNotes[i]->GetDifficulty() == dc )
			return true;
	return false;
}

void Song::SaveToCacheFile()
{
	LOG->Trace( "Song::SaveToCacheFile()" );

	SONGINDEX->AddCacheIndex(m_sSongDir, GetHashForDirectory(m_sSongDir));
	SaveToSMFile( GetCacheFilePath(), true );
}

void Song::Save()
{
	LOG->Trace( "Song::SaveToSongFile()" );

	/* rename all old files to avoid confusion.
	 *
	 * This also serves as a backup, so rename .sm's, too.  If we crash when
	 * saving the .sm, we don't want to lose what we had.  But, what we really
	 * should be doing is saving to another file (eg. foo.sm.new), then once we 
	 * know we havn't crashed, move the old .sm to .sm.old and the new one to
	 * the real filename.  That way, if we crash, we don't leave the song in an
	 * unplayable state where the user has to manually un-rename stuff.  XXX -glenn 
	 */
	CStringArray arrayOldFileNames;
	GetDirListing( m_sSongDir + "*.bms", arrayOldFileNames );
	GetDirListing( m_sSongDir + "*.dwi", arrayOldFileNames );
	GetDirListing( m_sSongDir + "*.ksf", arrayOldFileNames );
	GetDirListing( m_sSongDir + "*.sm", arrayOldFileNames );
	
	for( unsigned i=0; i<arrayOldFileNames.size(); i++ )
	{
		CString sOldPath = m_sSongDir + arrayOldFileNames[i];
		CString sNewPath = sOldPath + ".old";
		MoveFile( sOldPath, sNewPath );
	}

	ReCalculateRadarValuesAndLastBeat();
	TranslateTitles();

	SaveToSMFile( GetSongFilePath(), false );
	SaveToDWIFile();
}


void Song::SaveToSMFile( CString sPath, bool bSavingCache )
{
	LOG->Trace( "Song::SaveToSMFile('%s')", sPath.GetString() );

	NotesWriterSM wr;
	wr.Write(sPath, *this, bSavingCache);
}

void Song::SaveToDWIFile()
{
	LOG->Trace( "Song::SaveToSongFileAndDWI()" );

	CString sPath = GetSongFilePath();
	sPath.Replace( ".sm", ".dwi" );

	NotesWriterDWI wr;
	wr.Write(sPath, *this);
}


void Song::AddAutoGenNotes()
{
	for( NotesType ntMissing=(NotesType)0; ntMissing<NUM_NOTES_TYPES; ntMissing=(NotesType)(ntMissing+1) )
	{
		if( SongHasNotesType(ntMissing) )
			continue;


		// missing Notes of this type
		int iNumTracksOfMissing = GAMEMAN->NotesTypeToNumTracks(ntMissing);

		// look for closest match
		NotesType	ntBestMatch = (NotesType)-1;
		int			iBestTrackDifference = 10000;	// inf

		for( NotesType nt=(NotesType)0; nt<NUM_NOTES_TYPES; nt=(NotesType)(nt+1) )
		{
			vector<Notes*> apNotes;
			this->GetNotesThatMatch( nt, apNotes );

			if(apNotes.empty() || apNotes[0]->IsAutogen())
				continue; /* can't autogen from other autogen */

			int iNumTracks = GAMEMAN->NotesTypeToNumTracks(nt);
			int iTrackDifference = abs(iNumTracks-iNumTracksOfMissing);
			if( iTrackDifference < iBestTrackDifference )
			{
				ntBestMatch = nt;
				iBestTrackDifference = iTrackDifference;
			}
		}

		if( ntBestMatch != -1 )
			AutoGen( ntMissing, ntBestMatch );
	}
}

void Song::AutoGen( NotesType ntTo, NotesType ntFrom )
{
//	int iNumTracksOfTo = GAMEMAN->NotesTypeToNumTracks(ntTo);

	for( unsigned int j=0; j<m_apNotes.size(); j++ )
	{
		Notes* pOriginalNotes = m_apNotes[j];
		if( pOriginalNotes->m_NotesType == ntFrom )
		{
			Notes* pNewNotes = new Notes;
			pNewNotes->AutogenFrom(pOriginalNotes, ntTo);
			this->m_apNotes.push_back( pNewNotes );
		}
	}
}

Grade Song::GetGradeForDifficulty( const StyleDef *st, PlayerNumber pn, Difficulty dc ) const
{
	// return max grade of notes in difficulty class
	vector<Notes*> aNotes;
	this->GetNotesThatMatch( st->m_NotesType, aNotes );
	SortNotesArrayByDifficulty( aNotes );

	Grade grade = GRADE_NO_DATA;

	for( unsigned i=0; i<aNotes.size(); i++ )
	{
		const Notes* pNotes = aNotes[i];
		if( pNotes->GetDifficulty() == dc )
			grade = max( grade, pNotes->m_MemCardScores[pn].grade );
	}
	return grade;
}


bool Song::IsNew() const
{
	return GetNumTimesPlayed()==0;
}

bool Song::IsEasy( NotesType nt ) const
{
	for( unsigned i=0; i<m_apNotes.size(); i++ )
	{
		Notes* pNotes = m_apNotes[i];
		if( pNotes->m_NotesType != nt )
			continue;
		if( pNotes->GetMeter() <= 2 )
			return true;
	}
	return false;
}

/////////////////////////////////////
// Sorting
/////////////////////////////////////

int CompareSongPointersByTitle(const Song *pSong1, const Song *pSong2)
{
	//Prefer transliterations to full titles
	CString sTitle1 = pSong1->GetSortTitle();
	CString sTitle2 = pSong2->GetSortTitle();

	int ret = sTitle1.CompareNoCase(sTitle2);
	if(ret < 0) return true;
	if(ret > 0) return false;

	/* The titles are the same.  Ensure we get a consistent ordering
	 * by comparing the unique SongFilePaths. */
	return pSong1->GetSongFilePath().CompareNoCase(pSong2->GetSongFilePath()) < 0;
}

void SortSongPointerArrayByTitle( vector<Song*> &arraySongPointers )
{
	sort( arraySongPointers.begin(), arraySongPointers.end(), CompareSongPointersByTitle );
}

int CompareSongPointersByDifficulty(const Song *pSong1, const Song *pSong2)
{
	vector<Notes*> aNotes1;
	vector<Notes*> aNotes2;

	pSong1->GetNotesThatMatch( GAMESTATE->GetCurrentStyleDef()->m_NotesType, aNotes1 );
	pSong2->GetNotesThatMatch( GAMESTATE->GetCurrentStyleDef()->m_NotesType, aNotes2 );

	int iEasiestMeter1 = 1000;	// infinity
	int iEasiestMeter2 = 1000;	// infinity

	unsigned i;
	for( i=0; i<aNotes1.size(); i++ )
		iEasiestMeter1 = min( iEasiestMeter1, aNotes1[i]->GetMeter() );
	for( i=0; i<aNotes2.size(); i++ )
		iEasiestMeter2 = min( iEasiestMeter2, aNotes2[i]->GetMeter() );

	if( iEasiestMeter1 < iEasiestMeter2 )
		return true;
	if( iEasiestMeter1 > iEasiestMeter2 )
		return false;
	return CompareSongPointersByTitle( pSong1, pSong2 );
}

void SortSongPointerArrayByDifficulty( vector<Song*> &arraySongPointers )
{
	sort( arraySongPointers.begin(), arraySongPointers.end(), CompareSongPointersByDifficulty );
}

bool CompareSongPointersByBPM(const Song *pSong1, const Song *pSong2)
{
	float fMinBPM1, fMaxBPM1, fMinBPM2, fMaxBPM2;
	pSong1->GetMinMaxBPM( fMinBPM1, fMaxBPM1 );
	pSong2->GetMinMaxBPM( fMinBPM2, fMaxBPM2 );

	if( fMaxBPM1 < fMaxBPM2 )
		return true;
	if( fMaxBPM1 > fMaxBPM2 )
		return false;
	
	return CompareCStringsAsc( pSong1->GetSongFilePath(), pSong2->GetSongFilePath() );
}

void SortSongPointerArrayByBPM( vector<Song*> &arraySongPointers )
{
	sort( arraySongPointers.begin(), arraySongPointers.end(), CompareSongPointersByBPM );
}


int CompareSongPointersByArtist(const Song *pSong1, const Song *pSong2)
{
	CString sArtist1 = pSong1->m_sArtist;
	CString sArtist2 = pSong2->m_sArtist;

	if( sArtist1 < sArtist2 )
		return true;
	if( sArtist1 > sArtist2 )
		return false;
	return CompareSongPointersByTitle( pSong1, pSong2 );
}

void SortSongPointerArrayByArtist( vector<Song*> &arraySongPointers )
{
	sort( arraySongPointers.begin(), arraySongPointers.end(), CompareSongPointersByArtist );
}

int CompareSongPointersByGroup(const Song *pSong1, const Song *pSong2)
{
	const CString &sGroup1 = pSong1->m_sGroupName;
	const CString &sGroup2 = pSong2->m_sGroupName;

	if( sGroup1 < sGroup2 )
		return true;
	if( sGroup1 > sGroup2 )
		return false;

	/* Same group; compare by difficulty. */
	return CompareSongPointersByDifficulty( pSong1, pSong2 );
}

void SortSongPointerArrayByGroup( vector<Song*> &arraySongPointers )
{
	sort( arraySongPointers.begin(), arraySongPointers.end(), CompareSongPointersByGroup );
}

int CompareSongPointersByMostPlayed(const Song *pSong1, const Song *pSong2)
{
	int iNumTimesPlayed1 = pSong1->GetNumTimesPlayed();
	int iNumTimesPlayed2 = pSong2->GetNumTimesPlayed();

	if( iNumTimesPlayed1 > iNumTimesPlayed2 )
		return true;
	if( iNumTimesPlayed1 < iNumTimesPlayed2 )
		return false;
	return CompareSongPointersByTitle( pSong1, pSong2 );
}

void SortSongPointerArrayByMostPlayed( vector<Song*> &arraySongPointers )
{
	sort( arraySongPointers.begin(), arraySongPointers.end(), CompareSongPointersByMostPlayed );
}

bool Song::NormallyDisplayed() const 
{
	if(!PREFSMAN->m_bHiddenSongs) return true;
	return m_SelectionDisplay == SHOW_ALWAYS;
}

bool Song::RouletteDisplayed() const
{
	if(!PREFSMAN->m_bHiddenSongs) return true;
	return m_SelectionDisplay != SHOW_NEVER;
}

bool Song::HasMusic() const 		{return m_sMusicFile != ""			&&	IsAFile(GetMusicPath()); }
bool Song::HasBanner() const 		{return m_sBannerFile != ""			&&  IsAFile(GetBannerPath()); }
bool Song::HasBackground() const 	{return m_sBackgroundFile != ""		&&  IsAFile(GetBackgroundPath()); }
bool Song::HasCDTitle() const 		{return m_sCDTitleFile != ""		&&  IsAFile(GetCDTitlePath()); }
bool Song::HasBGChanges() const 	{return !m_BackgroundChanges.empty(); }

int Song::GetNumTimesPlayed() const
{
	int iTotalNumTimesPlayed = 0;
	for( unsigned i=0; i<m_apNotes.size(); i++ )
	{
		iTotalNumTimesPlayed += m_apNotes[i]->m_iNumTimesPlayed;
	}
	return iTotalNumTimesPlayed;
}

/* Search semantics for all song files:
 *
 * If the path doesn't have any directory separators, it's a filename in the
 * song directory.
 *
 * If it does, it's relative to the top directory of the tree it was loaded
 * from, eg ".\music\stuff\song.ogg".  Most of the time, that's the SM tree,
 * and that's the PWD, so just return it directly.  If the file was originally
 * loaded from the DWIPath, it's relative to that, so prepend it.
 *
 * Some DWI's do have relative paths that don't start with ".\".
 */

/* We only follow this for song files and cdtitles; it's for compatibility
 * with DWI.  We prefer paths relative to the song directory; only support
 * that for all other paths. */

/* Note: Prepending the dwipath is ugly.  The first impression might be to
 * add a Song::TopFilePath, but don't do that--we have too much stuff in there
 * already.  We don't really need it; this is the only place it'd be used.  What
 * we *should* be doing is prepending this path when we first load the song.
 * However, dwipaths are usually like "c:\games\dwi", and we can't store colons
 * in SM's; they'll get interpreted as delimiters.  So:
 * XXX: Add some kind of escape character to SM's.
 *
 * -glenn */
 
CString Song::GetMusicPath() const
{
	/* If there's no path in the music file, the file is in the same directory
	 * as the song.  (This is the preferred configuration.) */
	if( m_sMusicFile.Find('\\') == -1)
		return m_sSongDir+m_sMusicFile;

	/* The file has a path.  If it was loaded from the m_DWIPath, it's relative
	 * to that. */
	if( PREFSMAN->m_DWIPath!="" && m_sSongDir.Left(PREFSMAN->m_DWIPath.GetLength()) == PREFSMAN->m_DWIPath )
		return PREFSMAN->m_DWIPath+"\\"+m_sMusicFile;

	/* Otherwise, it's relative to the top of the SM directory (the CWD), so
	 * return it directly. */
	return m_sMusicFile;
}

CString Song::GetBannerPath() const
{
	return m_sSongDir+m_sBannerFile;
}

CString Song::GetCDTitlePath() const
{
	if( m_sCDTitleFile.Find('\\') == -1)
		return m_sSongDir+m_sCDTitleFile;

	if( PREFSMAN->m_DWIPath!="" && m_sSongDir.Left(PREFSMAN->m_DWIPath.GetLength()) == PREFSMAN->m_DWIPath )
		return PREFSMAN->m_DWIPath+"\\"+m_sCDTitleFile;

	return m_sCDTitleFile;
}

CString Song::GetBackgroundPath() const
{
	return m_sSongDir+m_sBackgroundFile;
}

CString Song::GetSortTitle() const
{
	CString Title = m_sMainTitleTranslit.empty()?
			m_sMainTitle: m_sMainTitleTranslit;

	CString SubTitle = m_sSubTitleTranslit.empty()?
		m_sSubTitle:m_sSubTitleTranslit;

	if(!SubTitle.empty())
	{
		Title += " " + SubTitle;
	}

	return Title;
}

/* Get the first/last beat of any currently active note pattern.  If two
 * players are active, they often have the same start beat, but they don't
 * have to. 
 *
 * This is currently slow (notedata can't cache the return, and getnotedata
 * is slow). */
#if 0 /* XXX not finished/tested/used yet -glenn */
float Song::GetFirstBeat() const
{
	float first = MAX_BEATS;
	for( int pn = 0; pn < NUM_PLAYERS; ++pn) {
		if(!GAMESTATE->IsPlayerEnabled(pn)) continue;

		NoteData tempNoteData;
		GAMESTATE->m_pCurNotes[pn]->GetNoteData( &tempNoteData );

		first = min(first, tempNoteData.GetFirstBeat());
	}
	return first;
}

float Song::GetLastBeat() const
{
	float last = MAX_BEATS;
	for( int pn = 0; pn < NUM_PLAYERS; ++pn) {
		if(!GAMESTATE->IsPlayerEnabled(pn)) continue;

		NoteData tempNoteData;
		GAMESTATE->m_pCurNotes[pn]->GetNoteData( &tempNoteData );

		last = max(last, tempNoteData.GetLastBeat());
	}
	return last;
}
#endif
