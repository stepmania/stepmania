#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenStage

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenStage.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "GameConstantsAndTypes.h"
#include "BitmapText.h"
#include "TransitionFadeWipe.h"
#include "TransitionFade.h"
#include "SongManager.h"
#include "Sprite.h"
#include "AnnouncerManager.h"
#include "GameState.h"
#include "RageMusic.h"


#define NEXT_SCREEN			THEME->GetMetric("ScreenStage","NextScreen")
#define STAGE_TYPE			THEME->GetMetricI("ScreenStage","StageType")
enum StageType		// for use with the metric above
{
	STAGE_TYPE_MAX = 0,
	STAGE_TYPE_PUMP,
	STAGE_TYPE_EZ2
};

StageType g_StageType;	// cache for STAGE_TYPE because it's slow to look up


const ScreenMessage SM_StartFadingOut	=	ScreenMessage(SM_User + 1);
const ScreenMessage SM_DoneFadingIn		=	ScreenMessage(SM_User + 2);
const ScreenMessage SM_GoToNextScreen	=	ScreenMessage(SM_User + 3);


enum StageMode
{
	MODE_NORMAL,
	MODE_FINAL,
	MODE_EXTRA1,
	MODE_EXTRA2,
	MODE_ONI,
	MODE_ENDLESS
};

ScreenStage::ScreenStage()
{
	MUSIC->Stop();

	g_StageType = (StageType)STAGE_TYPE; 


	StageMode		stage_mode;
	if( GAMESTATE->m_PlayMode == PLAY_MODE_ONI )			stage_mode = MODE_ONI;
	else if( GAMESTATE->m_PlayMode == PLAY_MODE_ENDLESS )	stage_mode = MODE_ENDLESS;
	else if( GAMESTATE->IsExtraStage() )					stage_mode = MODE_EXTRA1;
	else if( GAMESTATE->IsExtraStage2() )					stage_mode = MODE_EXTRA2;
	else if( GAMESTATE->IsFinalStage() )					stage_mode = MODE_FINAL;
	else													stage_mode = MODE_NORMAL;


	//
	// Play announcer
	//
	switch( stage_mode )
	{
	case MODE_NORMAL:
		{
			const int iStageNo = GAMESTATE->GetStageIndex()+1;
			switch( iStageNo )
			{
			case 1:	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo("stage 1") );	break;
			case 2:	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo("stage 2") );	break;
			case 3:	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo("stage 3") );	break;
			case 4:	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo("stage 4") );	break;
			case 5:	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo("stage 5") );	break;
			default:	;	break;	// play nothing
			}
		}
		break;
	case MODE_FINAL:	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo("stage final") );		break;
	case MODE_EXTRA1:	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo("stage extra1") );		break;
	case MODE_EXTRA2:	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo("stage extra2") );		break;
	case MODE_ONI:		SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo("stage oni") );		break;
	case MODE_ENDLESS:	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo("stage endless") );	break;
	default:		ASSERT(0);
	}



	//
	// Init common graphics
	//
	this->AddChild( &m_sprSongBackground );	// add background first so it draws bottom-most
	//this->AddChild( &m_quadMask );	// add quad mask before stage so that it will block out the stage sprites
	
	for( int i=0; i<4; i++ )
	{
		m_sprNumbers[i].Load( THEME->GetPathTo("Graphics","stage parts 5x3") );

		if( g_StageType != STAGE_TYPE_EZ2 ) // DONT DIFFUSE COLORS FOR EZ2. Coz we actually set the color in the .png
			m_sprNumbers[i].SetDiffuse( GAMESTATE->GetStageColor() );

		m_sprNumbers[i].StopAnimating();
	}
	m_sprStage.Load( THEME->GetPathTo("Graphics","stage stage") );	// we may load a different graphic into here later
	m_sprStage.SetDiffuse( GAMESTATE->GetStageColor() );



	switch( stage_mode )
	{
	case MODE_NORMAL:
		{
			const int iStageNo = GAMESTATE->GetStageIndex()+1;

			CString sStageNo = ssprintf("%d", iStageNo);

			// Set frame of numbers
			int i;
			for( i=0; i<sStageNo.GetLength(); i++ )
				m_sprNumbers[i].SetState( sStageNo[i]-'0' );

			// Set frame of suffix
			{
				int iSuffix;
				if( ( (iStageNo/10) % 10 ) == 1 )	// in the teens (e.g. 19, 213)
					iSuffix = 13;	// th
				else	// not in the teens
				{
					switch( iStageNo%10 )
					{
					case 1:		iSuffix = 10;	break;	// st
					case 2:		iSuffix = 11;	break;	// nd
					case 3:		iSuffix = 12;	break;	// rd
					default:	iSuffix = 13;	break;	// th
					}
				}

				int iIndexOfSuffix = sStageNo.GetLength();
				m_sprNumbers[iIndexOfSuffix].SetState( iSuffix );
			}
			
			// Set X positions
			const float fFrameWidth = m_sprNumbers[0].GetUnzoomedWidth();
			const int iNumChars = sStageNo.GetLength()+1;
			const float fTotalCharsWidth = m_sprNumbers[0].GetUnzoomedWidth();
			const float fSpaceBetweenNumsAndStage = fFrameWidth;
			const float fStageWidth = m_sprStage.GetUnzoomedWidth();
			const float fTotalWidth = fTotalCharsWidth + fSpaceBetweenNumsAndStage + fStageWidth;
			const float fCharsCenterX = -fTotalWidth/2.0f + fTotalCharsWidth/2.0f;
			const float fStageCenterX = +fTotalWidth/2.0f - fStageWidth/2.0f;

			for( i=0; i<iNumChars; i++ )
			{
				float fOffsetX = SCALE((float)i, 0,
						iNumChars-1, -(iNumChars-1)/2.0f*fFrameWidth,
						(iNumChars-1)/2.0f*fFrameWidth);
				m_sprNumbers[i].SetX( fCharsCenterX + fOffsetX );
			}
			m_sprStage.SetX( fStageCenterX );

			for( i=0; i<iNumChars; i++ )
				m_frameStage.AddChild( &m_sprNumbers[i] );
		}
		break;
	case MODE_FINAL:
		m_sprStage.Load( THEME->GetPathTo("Graphics","stage final") );
		break;
	case MODE_EXTRA1:
		m_sprStage.Load( THEME->GetPathTo("Graphics","stage extra1") );
		break;
	case MODE_EXTRA2:
		m_sprStage.Load( THEME->GetPathTo("Graphics","stage extra2") );
		break;
	case MODE_ONI:
		m_sprStage.Load( THEME->GetPathTo("Graphics","stage oni") );
		break;
	case MODE_ENDLESS:
		m_sprStage.Load( THEME->GetPathTo("Graphics","stage endless") );
		break;
	default:
		ASSERT(0);
	}
	m_frameStage.AddChild( &m_sprStage );



	//
	// Init dance-specific graphics
	//
	if( g_StageType == STAGE_TYPE_MAX )
	{
		const float fStageHeight = m_sprStage.GetUnzoomedHeight();

		const float fStageOffScreenY = CENTER_Y+fStageHeight;

		/* The quadMask masks out draws via Z; it doesn't actually erase
		 * anything, so make it transparent. */
		m_quadMask.SetDiffuse( D3DXCOLOR(0,0,0,0) );
		m_quadMask.StretchTo( CRect(SCREEN_LEFT, int(roundf(fStageOffScreenY-fStageHeight/2)), 
							        SCREEN_RIGHT, int(roundf(fStageOffScreenY+fStageHeight/2))) );
		/* Put the quad mask on top, so draws to the Stage will be "under" it. */
		m_quadMask.SetZ( -1 );

		m_frameStage.SetXY( CENTER_X, fStageOffScreenY );
		m_frameStage.BeginTweening(0.8f, Actor::TWEEN_BIAS_BEGIN );
		m_frameStage.SetTweenY( CENTER_Y );

		this->AddChild( &m_quadMask );	// add quad mask before stage so that it will block out the stage sprites
		this->AddChild( &m_frameStage ); 
	}


	//
	// Init pump-sepecific graphics
	//
	if( g_StageType == STAGE_TYPE_PUMP )
	{
		const Song* pSong = GAMESTATE->m_pCurSong;
		m_sprSongBackground.Load( (pSong && pSong->HasBackground()) ? pSong->GetBackgroundPath() : THEME->GetPathTo("Graphics","fallback background") );
		m_sprSongBackground.StretchTo( CRect(SCREEN_LEFT,SCREEN_TOP,SCREEN_RIGHT,SCREEN_BOTTOM) );

		/* Move the stage numbers downward, so they don't overlay the
		 * center of the image. */
		m_frameStage.SetXY( CENTER_X, CENTER_Y+160 );
		m_frameStage.SetZoom( 0.5f );

		/* Why are we adding this in three different places?  It's added at the
		 * end of each case ... */
		this->AddChild( &m_frameStage ); 
	}


	//
	// Init ez2-specific graphics
	//
	if( g_StageType == STAGE_TYPE_EZ2 )
	{
		/////////////////////////////////////////////////
		// START moving the numbers how ez2 wants them //
		/////////////////////////////////////////////////

		for( i=0; i<4; i++ )
		{
		//	float fOffsetX = SCALE(i, 0, iNumChars-1, -(iNumChars-1)/2.0f*fFrameWidth, (iNumChars-1)/2.0f*fFrameWidth);
		
			m_sprNumbers[i].SetXY( CENTER_X - 170 + (90 * i), CENTER_Y );
			m_sprNumbers[i].SetY( CENTER_Y );
			m_sprNumbers[i].ZoomToWidth( 200.0f ); // make the numbers that appear really big
			m_sprNumbers[i].ZoomToHeight( 200.0f); // so they can 'shrink' onto the screen
			m_sprNumbers[i].SetZoom( 20.0f ); // make it really really big on-screen
			m_sprNumbers[i].SetRotation( 10 ); // make this thing rotated

			m_sprNumbers[i].BeginTweening(0.8f, Actor::TWEEN_BIAS_BEGIN );
			m_sprNumbers[i].SetTweenZoom( 3.0f ); // shrink it down to it's proper size
			m_sprNumbers[i].SetTweenRotationZ( 0 ); // make it rotate into place
		}
	
	//	m_frameStage.SetXY( CENTER_X, CENTER_Y );

		///////////////////////////
		// Start Ez2 Backgrounds //
		///////////////////////////
	
		// Background Blocks rotate their way in

		int bg_modeoffset=0; // used to shuffle some graphics if we're in FINAL rather than normal stage
		float element_y_offsets=0.0f; // used to shuffle some graphics if we're in FINAL rather than nornal stage
		if (stage_mode == MODE_FINAL) // if we're in final redefine those offsets
		{
			element_y_offsets = 25.0f;
			bg_modeoffset = 4; // shuffle graphics +4 in the elements file for FINAL graphics.
		}

		for (i=0; i<3; i++) // Load In The Background Graphics
		{
			m_sprbg[i].Load( THEME->GetPathTo("Graphics","stage elements") );
			m_sprbg[i].StopAnimating();
			m_sprbg[i].SetState( i+bg_modeoffset ); // We wanna use i to load in an element from the graphicmap OR the offset will
													// give us alternative locations if we're in final
		}

		// Final mode is a little more special than our regular normal stage screen
		// it has FOUR background elements.
		// so if we should actually be on final stage
		// setup this extra background element
		
		if (stage_mode == MODE_FINAL) 
		{
			m_sprbgxtra.Load( THEME->GetPathTo("Graphics","stage elements") ); // get the graphic
			m_sprbgxtra.StopAnimating(); 
			m_sprbgxtra.SetState( bg_modeoffset ); // use the first element of the offset for this graphic
			m_sprbgxtra.SetXY( CENTER_X-30, CENTER_Y+180); // set it's initial XY coordinates
			m_sprbgxtra.ZoomToHeight( 30 ); // set it's height and width. As we're only dealing with solid color
			m_sprbgxtra.ZoomToWidth( SCREEN_WIDTH + 50 ); // stretching shouldn't be a concern.
			m_sprbgxtra.SetRotation( -20 ); // rotate this graphic
			m_sprbgxtra.BeginTweening(0.3f); // start tweening
			m_sprbgxtra.SetTweenRotationZ( 0 ); // set the rotation we want it to finally appear as
		}


		if (stage_mode == MODE_FINAL)
			m_sprbg[0].SetXY( CENTER_X-30, CENTER_Y-160);
		else
			m_sprbg[0].SetXY( CENTER_X, CENTER_Y+150);
		// Fairly high in normal, "squashed" in final.
		if (stage_mode == MODE_FINAL)
			m_sprbg[0].ZoomToHeight( 30 );
		else
			m_sprbg[0].ZoomToHeight( 100 ); 

		m_sprbg[0].ZoomToWidth( SCREEN_WIDTH + 50 ); // no matter what... it's this wide
		m_sprbg[0].SetRotation( -20 ); // and is initially this rotation
		m_sprbg[0].BeginTweening(0.3f); // start tweening
		m_sprbg[0].SetTweenRotationZ( 0 ); // and set the rotation to where we want it to end up
		
		m_sprbg[1].SetXY( CENTER_X-(SCREEN_WIDTH/2)-20, CENTER_Y+element_y_offsets);
		m_sprbg[1].ZoomToHeight( SCREEN_HEIGHT - 140 );
		m_sprbg[1].ZoomToWidth( 130 );
		m_sprbg[1].SetRotation( -20 );
		m_sprbg[1].BeginTweening(0.3f);
		m_sprbg[1].SetTweenRotationZ( 0 );

		m_sprbg[2].SetXY( CENTER_X+430, CENTER_Y+element_y_offsets);
		m_sprbg[2].ZoomToHeight( SCREEN_HEIGHT - 140 );
		m_sprbg[2].ZoomToWidth( SCREEN_WIDTH + 50 );
		m_sprbg[2].SetRotation( -20 );
		m_sprbg[2].BeginTweening(0.3f);
		m_sprbg[2].SetTweenX( CENTER_X );
		m_sprbg[2].SetTweenRotationZ( 0 );

		for (i = 3; i >= 0; i--) // add them backwards
			AddChild( &m_sprbg[i] ); 

		if (stage_mode == MODE_FINAL) // if we're in FINAL add that extra background element mentioned earlier.
			AddChild( &m_sprbgxtra );


		/////////////////////////
		// END Ez2 Backgrounds //
		/////////////////////////


		//////////////////////////////
		// START stage description  //
		// and gamename tag display //
		//////////////////////////////

		// scroll in the name of the game ("ez2dancer ukmove" on arcade) across the top of the screen

		for (i=0; i<2; i++) // specify the font file.
		{
			m_ez2ukm[i].LoadFromFont( THEME->GetPathTo("Fonts","Stage ez2") );
			m_ez2ukm[i].TurnShadowOff();
			m_stagedesc[i].LoadFromFont( THEME->GetPathTo("Fonts","Stage ez2") );
			m_stagedesc[i].TurnShadowOff();
		}

		m_ez2ukm[0].SetXY( CENTER_X-400, CENTER_Y-220 ); // set the intiial UKMOVE positions
		m_ez2ukm[1].SetXY( CENTER_X+400, CENTER_Y+220 );
		

		m_stagedesc[0].SetXY( CENTER_X-400, CENTER_Y-150+element_y_offsets ); // set the intiial desc positions
		m_stagedesc[1].SetXY( CENTER_X+400, CENTER_Y+70+element_y_offsets ); 
		
		if (stage_mode == MODE_FINAL)
		{
			m_stagedesc[1].SetY( CENTER_Y+140.0f ); // description text is in a different Y location on final stage
		}
		
		for (i=0; i<2; i++) // initialize the UK MOVE text and positions
		{
			m_ez2ukm[i].SetText( "STEPMANIA EZ2 MOVE" ); // choose something better if you like ;)
			m_ez2ukm[i].SetDiffuse( D3DXCOLOR(1,1,1,1) ); // it's white
			m_ez2ukm[i].BeginTweening(0.5f); // start it tweening
			if (stage_mode == MODE_FINAL)
			{
				m_stagedesc[i].SetText( "FINAL FINAL FINAL FINAL FINAL FINAL FINAL FINAL FINAL FINAL" ); // this is the desc text for final stage
				m_stagedesc[i].SetDiffuse( D3DXCOLOR(1.0f/225.0f*227.0f,1.0f/225.0f*228.0f,1/225.0f*255.0f,1) ); // it's blueish
			}
			else
			{
				m_stagedesc[i].SetText( "NEXT NEXT NEXT NEXT NEXT NEXT NEXT NEXT NEXT NEXT NEXT" ); // normal stages use this text
				m_stagedesc[i].SetDiffuse( D3DXCOLOR(1.0f/225.0f*166.0f,1.0f/225.0f*83.0f,1/225.0f*16.0f,1) ); // it's orangey
			}
			m_stagedesc[i].BeginTweening(0.5f); // start tweening the descriptions

		}
		
		m_ez2ukm[0].SetTweenX(CENTER_X+90); // set their new locations
		m_ez2ukm[1].SetTweenX(CENTER_X-160);
		m_stagedesc[0].SetTweenX(CENTER_X+10); // set their new locations
		m_stagedesc[1].SetTweenX(CENTER_X-100);

		for (i=0; i<2; i++) // add the actors
		{
			AddChild( &m_ez2ukm[i] );
			AddChild( &m_stagedesc[i] );
		}
		
		//////////////////////////////
		// END stage description    //
		// and gamename tag display //
		//////////////////////////////

		//////////////////////////////
		// START scrolling blobs    //
		//////////////////////////////


		// 20 Scrolling Blobs come in one at a time from either side at the bottom in the following code:

		// there are two sets of blobs, each with 20 elements in them
		// they don't just scroll all together at the same time.
		// theres an effect as if the'res a long line of them and they 'bump' into each other when the first one stops
		// so take a careful look at the starting position and the final tween positions of these critters.

		for( int j=0; j<2; j++) // 2 sets.....
		{
			for (i=0; i<20; i++) // 20 blobs...
			{
				m_sprScrollingBlobs[j][i].Load( THEME->GetPathTo("Graphics","stage elements") ); // load the graphics
				m_sprScrollingBlobs[j][i].StopAnimating();
				m_sprScrollingBlobs[j][i].SetState( 3+bg_modeoffset ); // shuffle the state according to offset
																		// (final uses different blobs)
				if (j == 0) // if it's the first set
				{
					m_sprScrollingBlobs[j][i].SetXY( CENTER_X-(SCREEN_WIDTH/2)-500-((i*i)*4), CENTER_Y + 135 ); // starting position
					if (stage_mode == MODE_FINAL)
					{
						m_sprScrollingBlobs[j][i].SetY( CENTER_Y-160); // different Y position for FINAL stage
					}
				}
				else // if it's the second set
				{
					m_sprScrollingBlobs[j][i].SetXY( CENTER_X+(SCREEN_WIDTH/2)+500-((i*i)*4), CENTER_Y+170 ); // starting position
					if (stage_mode == MODE_FINAL)
					{
						m_sprScrollingBlobs[j][i].SetY( CENTER_Y+180 ); // different Y position for FINAL stage
					}
				}

				m_sprScrollingBlobs[j][i].BeginTweening(0.2f * i / 7); // start them tweening, different delay for each blob
				if (j == 0) // set 1
					m_sprScrollingBlobs[j][i].SetTweenX(CENTER_X-(SCREEN_WIDTH/2)+30+(i*30.0f)); // keep them at equal distance when they arrive
				else // set 2
					m_sprScrollingBlobs[j][i].SetTweenX(CENTER_X+(SCREEN_WIDTH/2)+30-70-(i*30.0f)); 
	
				AddChild( &m_sprScrollingBlobs[j][i] );
			}
		}

		//////////////////////////////
		// END scrolling blobs    //
		//////////////////////////////
		
		//////////////////////////////
		// START stage name         //
		//////////////////////////////

		// in this bit we write the stage name onto the screen
		// basically instead of 1st 2nd 3rd e.t.c.
		// ez2dancer spells it fully 
		// The First Stage.... The Second Stage
		// this is what this bit implements


		// write the stage name
		m_stagename.LoadFromFont( THEME->GetPathTo("Fonts","Stage ez2") );
		m_stagename.TurnShadowOff();

		m_stagename.SetXY( CENTER_X+400, CENTER_Y-30+element_y_offsets );  // set initial position			

		const int iStageNo = GAMESTATE->GetStageIndex()+1;	
		switch (iStageNo) // from the stage number figure out which text to use
		{
			case 0: m_stagename.SetText( "THE NEXT STAGE" ); break;
			case 1: m_stagename.SetText( "THE FIRST STAGE" ); break;
			case 2: m_stagename.SetText( "THE SECOND STAGE" ); break;
			case 3: m_stagename.SetText( "THE THIRD STAGE" ); break;
			case 4: m_stagename.SetText( "THE FOURTH STAGE" ); break;
			case 5: m_stagename.SetText( "THE FIFTH STAGE" ); break;
			case 6: m_stagename.SetText( "THE SIXTH STAGE" ); break;
			case 7: m_stagename.SetText( "THE SEVENTH STAGE" ); break;
			case 8: case 9: m_stagename.SetText("THE NEXT STAGE"); break;
			default: m_stagename.SetText( "" ); break; // make this text disappear.
		}

		m_stagename.SetDiffuse( D3DXCOLOR(1.0f/225.0f*166.0f,1.0f/225.0f*83.0f,1/225.0f*16.0f,1) ); // orangey colour

		if (stage_mode == MODE_FINAL) // if we're final stage 
		{
			m_stagename.SetDiffuse( D3DXCOLOR(1.0f/225.0f*227.0f,1.0f/225.0f*228.0f,1/225.0f*255.0f,1) ); // blueish colour
			m_stagename.SetText( "THE FINAL STAGE" );
		}

		m_stagename.BeginTweening(0.5f); // start tweening them to their new home
		
		m_stagename.SetTweenX(CENTER_X+70); // set their new locations
		AddChild( &m_stagename ); //add the actor
	
		this->AddChild( &m_frameStage ); 

		//////////////////////////////
		// END stage name         //
		//////////////////////////////

		m_sprStage.SetZoom( 0 ); // hide this element for Ez2 :)
	}

	this->AddChild( &m_Fade );	// fade should draw last, on top of everything else
	m_Fade.SetOpened();

	this->SendScreenMessage( SM_DoneFadingIn, 1.0f );
	this->SendScreenMessage( SM_StartFadingOut, 4.0f );
}

void ScreenStage::Update( float fDeltaTime )
{
	Screen::Update( fDeltaTime );
}

void ScreenStage::DrawPrimitives()
{
	// Enable Zbuffering for the mask in the dance mode
	if( g_StageType == STAGE_TYPE_MAX )
		DISPLAY->EnableZBuffer();

	Screen::DrawPrimitives();

	if( g_StageType == STAGE_TYPE_MAX )
		DISPLAY->DisableZBuffer();
}

void ScreenStage::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_StartFadingOut:
		m_Fade.CloseWipingRight( SM_GoToNextScreen );
		break;
	case SM_DoneFadingIn:
		SCREENMAN->PrepNewScreen( "ScreenGameplay" );
		break;
	case SM_GoToNextScreen:
		SCREENMAN->LoadPreppedScreen(); /* use prepped */
		break;
	}
}
