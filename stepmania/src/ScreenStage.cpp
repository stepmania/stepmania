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

const ScreenMessage SM_StartFadingOut	=	ScreenMessage(SM_User + 1);
const ScreenMessage SM_DoneFadingIn		=	ScreenMessage(SM_User + 2);
const ScreenMessage SM_GoToNextState	=	ScreenMessage(SM_User + 3);


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

	for( int i=0; i<4; i++ )
	{
		m_sprNumbers[i].Load( THEME->GetPathTo("Graphics","stage numbers") );
		m_sprNumbers[i].StopAnimating();
	}
	m_sprStage.Load( THEME->GetPathTo("Graphics","stage stage") );	// we may load a different graphic into here later
	const float fStageHeight = m_sprStage.GetUnzoomedHeight();

	const float fStageOffScreenY = CENTER_Y+fStageHeight;

	m_quadMask.SetDiffuseColor( D3DXCOLOR(0,0,0,0) );
	m_quadMask.StretchTo( CRect(SCREEN_LEFT, roundf(fStageOffScreenY-fStageHeight/2), SCREEN_RIGHT, roundf(fStageOffScreenY+fStageHeight/2)) );
	m_quadMask.SetZ( -1 );		// important: fill Z buffer with values that will cause subsequent draws to fail the Z test

	m_frameStage.SetXY( CENTER_X, fStageOffScreenY );
	m_frameStage.BeginTweening(0.8f, Actor::TWEEN_BIAS_BEGIN );
	m_frameStage.SetTweenY( CENTER_Y );

	
	StageMode		stage_mode;
	if( GAMESTATE->m_PlayMode == PLAY_MODE_ONI )			stage_mode = MODE_ONI;
	else if( GAMESTATE->m_PlayMode == PLAY_MODE_ENDLESS )	stage_mode = MODE_ENDLESS;
	else if( GAMESTATE->IsExtraStage() )					stage_mode = MODE_EXTRA1;
	else if( GAMESTATE->IsExtraStage2() )					stage_mode = MODE_EXTRA2;
	else if( GAMESTATE->IsFinalStage() )					stage_mode = MODE_FINAL;
	else													stage_mode = MODE_NORMAL;


	switch( stage_mode )
	{
	case MODE_NORMAL:
		{
			const int iStageNo = GAMESTATE->GetStageIndex()+1;
			switch( iStageNo )
			{
			case 1:	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_STAGE_1) );	break;
			case 2:	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_STAGE_2) );	break;
			case 3:	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_STAGE_3) );	break;
			case 4:	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_STAGE_4) );	break;
			case 5:	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_STAGE_5) );	break;
			default:	;	break;	// play nothing
			}
		}
		break;
	case MODE_FINAL:
		SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_STAGE_FINAL) );
		break;
	case MODE_EXTRA1:
		SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_STAGE_EXTRA1) );
		break;
	case MODE_EXTRA2:
		SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_STAGE_EXTRA2) );
		break;
	case MODE_ONI:
		SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_STAGE_ONI) );
		break;
	case MODE_ENDLESS:
		SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_STAGE_ENDLESS) );
		break;
	default:
		ASSERT(0);
	}

	int ez2Final=0; // if we're 0 it's not a final stage. if we're 1 it is.
	// why do this? Ez2dancer uses NORMAL for it's final stage but just re-arranges
	// the elements. so the following takes us into NORMAL and just re-arranges stuff
	// the ez2Final is so that when we're in normal we can go and see if we're really
	// FINAL or not. at the end of normal, if we WERE FINAL, we set it back to final
	// hacky or what? =)
	if( STAGE_TYPE == STAGE_TYPE_EZ2  &&  stage_mode == MODE_FINAL )
	{
		for( int i=0; i<4; i++ )
		{
			m_sprNumbers[i].Load( THEME->GetPathTo("Graphics","stage numbers final") );
			m_sprNumbers[i].StopAnimating();
		}
		ez2Final=1;
		stage_mode = MODE_NORMAL;
	}

	switch( stage_mode )
	{
	case MODE_NORMAL:
		{
			const int iStageNo = GAMESTATE->GetStageIndex()+1;

			CString sStageNo = ssprintf("%d", iStageNo);

			// Set frame of numbers
			int i;
			for( i=0; i<sStageNo.GetLength(); i++ )
				m_sprNumbers[i].SetState( atoi(CString(sStageNo[i])) );

			// Set frame of suffix
			int iIndexOfSuffix = sStageNo.GetLength();
			if( ( (iStageNo/10) % 10 ) == 1 )	// in the teens (e.g. 19, 213)
				m_sprNumbers[iIndexOfSuffix].SetState( 13 );	// th
			else	// not in the teens
			{
				const int iLastDigit = iStageNo%10;
				switch( iLastDigit )
				{
				case 1:		m_sprNumbers[iIndexOfSuffix].SetState( 10 );	break;	// st
				case 2:		m_sprNumbers[iIndexOfSuffix].SetState( 11 );	break;	// nd
				case 3:		m_sprNumbers[iIndexOfSuffix].SetState( 12 );	break;	// rd
				default:	m_sprNumbers[iIndexOfSuffix].SetState( 13 );	break;	// th
				}
			}
			
			// Set X positions
			const float fFrameWidth = m_sprNumbers[0].GetUnzoomedWidth();
			const int iNumChars = iIndexOfSuffix+1;
			const float fTotalCharsWidth = m_sprNumbers[0].GetUnzoomedWidth();
			const float fSpaceBetweenNumsAndStage = fFrameWidth;
			const float fStageWidth = m_sprStage.GetUnzoomedWidth();
			const float fTotalWidth = fTotalCharsWidth + fSpaceBetweenNumsAndStage + fStageWidth;
			const float fCharsCenterX = -fTotalWidth/2.0f + fTotalCharsWidth/2.0f;
			const float fStageCenterX = +fTotalWidth/2.0f - fStageWidth/2.0f;

			for( i=0; i<iNumChars; i++ )
			{
				float fOffsetX = SCALE((float)i, 0, iNumChars-1, -(iNumChars-1)/2.0f*fFrameWidth, (iNumChars-1)/2.0f*fFrameWidth);
				m_sprNumbers[i].SetX( fCharsCenterX + fOffsetX );
			}
			m_sprStage.SetX( fStageCenterX );

			/////////////////////////
			// EZ2 TYPE DEFINITION //
			/////////////////////////

			if ( STAGE_TYPE == STAGE_TYPE_EZ2) // Initialize and manipulate existing graphics for Ez2dancer Screen Type
			{
				for( i=0; i<iNumChars; i++ )
				{
					float fOffsetX = SCALE(i, 0, iNumChars-1, -(iNumChars-1)/2.0f*fFrameWidth, (iNumChars-1)/2.0f*fFrameWidth);
					m_sprNumbers[i].SetX( fCharsCenterX + fOffsetX + (150 * i) );
				}


				for( i=0; i<iNumChars; i++ ) // redefine the size of the numbers
				{
					m_sprNumbers[i].SetWidth( 200.0f );
					m_sprNumbers[i].SetHeight( 200.0f);
				}
			
				m_frameStage.SetXY( CENTER_X - 140, CENTER_Y ); // align the big number
				
				for( i=0; i<iNumChars; i++)
				{
					m_sprNumbers[i].SetZoom( 20.0f ); // make it really really big
					m_sprNumbers[i].SetRotation( 10 ); // make this thing rotated
					m_sprNumbers[i].BeginTweening(0.8f, Actor::TWEEN_BIAS_BEGIN );
					m_sprNumbers[i].SetTweenZoom( 1.0f ); // shrink it down to it's proper size
					m_sprNumbers[i].SetTweenRotationZ( 0 ); // make it rotate into place
				}
			
			
				// Background Blocks rotate their way in

				int bg_modeoffset=0;
				float element_y_offsets=0.0f;
				if (ez2Final == 1)
				{
					element_y_offsets = 25.0f;
					bg_modeoffset = 4; // shuffle graphics +4 in the elements file for FINAL graphics.
				}

				for (i=0; i<3; i++)
				{
					m_sprbg[i].Load( THEME->GetPathTo("Graphics","stage elements") );
				    m_sprbg[i].StopAnimating();
					m_sprbg[i].SetState( i+bg_modeoffset );
				}
				if (ez2Final == 1)
				{
					m_sprbgxtra.Load( THEME->GetPathTo("Graphics","stage elements") );
					m_sprbgxtra.StopAnimating();
					m_sprbgxtra.SetState( bg_modeoffset );
			  		m_sprbgxtra.SetXY( fStageCenterX-30, 0+180);
					m_sprbgxtra.SetHeight( 30 );
					m_sprbgxtra.SetWidth( SCREEN_WIDTH + 50 );
					m_sprbgxtra.SetRotation( -20 );
					m_sprbgxtra.BeginTweening(0.3f);
					m_sprbgxtra.SetTweenRotationZ( 0 );
				}


				m_sprbg[0].SetXY( fStageCenterX-30, 0+150);
				if (ez2Final == 1)
					m_sprbg[0].SetXY( fStageCenterX-30, 0-160);
				m_sprbg[0].SetHeight( 100 );
				if (ez2Final == 1)
					m_sprbg[0].SetHeight( 30 );
				m_sprbg[0].SetWidth( SCREEN_WIDTH + 50 );
				m_sprbg[0].SetRotation( -20 );
				m_sprbg[0].BeginTweening(0.3f);
				m_sprbg[0].SetTweenRotationZ( 0 );
				
				m_sprbg[1].SetXY( fStageCenterX-(SCREEN_WIDTH/2)-20, 0+element_y_offsets);
				m_sprbg[1].SetHeight( SCREEN_HEIGHT - 140 );
				m_sprbg[1].SetWidth( 130 );
				m_sprbg[1].SetRotation( -20 );
				m_sprbg[1].BeginTweening(0.3f);
				m_sprbg[1].SetTweenRotationZ( 0 );

				m_sprbg[2].SetXY( fStageCenterX+430, 0+element_y_offsets);
				m_sprbg[2].SetHeight( SCREEN_HEIGHT - 140 );
				m_sprbg[2].SetWidth( SCREEN_WIDTH + 50 );
				m_sprbg[2].SetRotation( -20 );
				m_sprbg[2].BeginTweening(0.3f);
				m_sprbg[2].SetTweenX( fStageCenterX - 30 );
				m_sprbg[2].SetTweenRotationZ( 0 );

				for (i=3; i>=0; i--) // work backwards as we wanna add em in reverse 
				{
					m_frameStage.AddSubActor( &m_sprbg[i] );
				}
				if (ez2Final == 1)
				{
					m_frameStage.AddSubActor( &m_sprbgxtra );
				}

				// scroll in the name of the game (ez2dancer ukmove) across the top of the screen

				for (i=0; i<2; i++) // specify the font file.
				{
					m_ez2ukm[i].LoadFromFont( THEME->GetPathTo("Fonts","stage") );
					m_ez2ukm[i].TurnShadowOff();
					m_stagedesc[i].LoadFromFont( THEME->GetPathTo("Fonts","stage") );
					m_stagedesc[i].TurnShadowOff();
				}

				m_ez2ukm[0].SetXY( fStageCenterX-400, 0-220 ); // set the intiial UKMOVE positions
				m_ez2ukm[1].SetXY( fStageCenterX+400, 0+220 ); 
				m_stagedesc[0].SetXY( fStageCenterX-400, 0-150+element_y_offsets ); // set the intiial desc positions
				m_stagedesc[1].SetXY( fStageCenterX+400, 0+70+element_y_offsets ); 
				
				if (ez2Final == 1)
				{
					m_stagedesc[1].SetY( 140.0f );
				}
				
				for (i=0; i<2; i++) // initialize the UK MOVE text and positions
				{
					m_ez2ukm[i].SetText( "STEPMANIA EZ2 MOVE" );
					m_ez2ukm[i].SetDiffuseColor( D3DXCOLOR(1,1,1,1) );
					m_ez2ukm[i].BeginTweening(0.5f);
					if (ez2Final == 1)
					{
						m_stagedesc[i].SetText( "FINAL FINAL FINAL FINAL FINAL FINAL FINAL FINAL FINAL FINAL" );
						m_stagedesc[i].SetDiffuseColor( D3DXCOLOR(1.0f/225.0f*227.0f,1.0f/225.0f*228.0f,1/225.0f*255.0f,1) );
					}
					else
					{
						m_stagedesc[i].SetText( "NEXT NEXT NEXT NEXT NEXT NEXT NEXT NEXT NEXT NEXT NEXT" );
						m_stagedesc[i].SetDiffuseColor( D3DXCOLOR(1.0f/225.0f*166.0f,1.0f/225.0f*83.0f,1/225.0f*16.0f,1) );
					}
					m_stagedesc[i].BeginTweening(0.5f);

				}
				
				m_ez2ukm[0].SetTweenX(fStageCenterX+90); // set their new locations
				m_ez2ukm[1].SetTweenX(fStageCenterX-170);
				m_stagedesc[0].SetTweenX(fStageCenterX+10); // set their new locations
				m_stagedesc[1].SetTweenX(fStageCenterX-100);

				for (i=0; i<2; i++)
				{
					m_frameStage.AddSubActor( &m_ez2ukm[i] );
					m_frameStage.AddSubActor( &m_stagedesc[i] );
				}




				// 20 Scrolling Blobs come in one at a time from either side at the bottom in the following code:
				for( int j=0; j<2; j++)
				{
					for (i=0; i<20; i++)
					{
						m_sprScrollingBlobs[j][i].Load( THEME->GetPathTo("Graphics","stage elements") );
						m_sprScrollingBlobs[j][i].StopAnimating();
						m_sprScrollingBlobs[j][i].SetState( 3+bg_modeoffset );
					}
				}

				for( j=0; j<2; j++)
				{
					for (i=0; i<20; i++)
					{
						if (j == 0)
						{
							m_sprScrollingBlobs[j][i].SetXY( fStageCenterX-(SCREEN_WIDTH/2)-500-((i*i)*4), 135 );
							if (ez2Final == 1)
							{
								m_sprScrollingBlobs[j][i].SetY( 0-160);
							}
						}
						else
						{
							m_sprScrollingBlobs[j][i].SetXY( fStageCenterX+(SCREEN_WIDTH/2)+500-((i*i)*4), 170 );
							if (ez2Final == 1)
							{
								m_sprScrollingBlobs[j][i].SetY( 180 );
							}
						}
					}
				}

				for( j=0; j<2; j++)
				{
					for (i=0; i<20; i++)
					{
						m_sprScrollingBlobs[j][i].BeginTweening(0.2f * i / 7);
						if (j == 0)
							m_sprScrollingBlobs[j][i].SetTweenX(fStageCenterX-(SCREEN_WIDTH/2)+(i*30.0f));
						else
							m_sprScrollingBlobs[j][i].SetTweenX(fStageCenterX+(SCREEN_WIDTH/2)-70-(i*30.0f));
			
						m_frameStage.AddSubActor( &m_sprScrollingBlobs[j][i] );
					}
				}

				// write the stage name
				m_stagename.LoadFromFont( THEME->GetPathTo("Fonts","stage") );
				m_stagename.TurnShadowOff();

				m_stagename.SetXY( fStageCenterX+400, 0-30+element_y_offsets ); 			

				switch (iStageNo)
				{
					case 0: m_stagename.SetText( "THE NEXT STAGE" ); break;
					case 1: m_stagename.SetText( "THE FIRST STAGE" ); break;
					case 2: m_stagename.SetText( "THE SECOND STAGE" ); break;
					case 3: m_stagename.SetText( "THE THIRD STAGE" ); break;
					case 4: m_stagename.SetText( "THE FOURTH STAGE" ); break;
					case 5: m_stagename.SetText( "THE FIFTH STAGE" ); break;
					case 6: m_stagename.SetText( "THE SIXTH STAGE" ); break;
					case 7: m_stagename.SetText( "THE SEVENTH STAGE" ); break;
					default: m_stagename.SetText("THE NEXT STAGE"); break;
				}

				if (iStageNo > 9) // if we're in two digits or more
				{
					m_stagename.SetText( "" ); // make this text disappear.
				}

				m_stagename.SetDiffuseColor( D3DXCOLOR(1.0f/225.0f*166.0f,1.0f/225.0f*83.0f,1/225.0f*16.0f,1) );

				if (ez2Final == 1)
				{
					m_stagename.SetDiffuseColor( D3DXCOLOR(1.0f/225.0f*227.0f,1.0f/225.0f*228.0f,1/225.0f*255.0f,1) );
					m_stagename.SetText( "THE FINAL STAGE" );
					stage_mode = MODE_FINAL; // set back to final again.
					ez2Final = 0;
				}

				m_stagename.BeginTweening(0.5f);
				
				m_stagename.SetTweenX(fStageCenterX+70); // set their new locations
				m_frameStage.AddSubActor( &m_stagename );

			}
			
			////////////////////////////
			// END EZ2 TYPE DEFINITION//
			////////////////////////////

			////////////////
			// PUMP STUFF //
			////////////////
			m_pSong = GAMESTATE->m_pCurSong;
			m_sprSongBackground.Load( m_pSong->HasBackground() ? m_pSong->GetBackgroundPath() : THEME->GetPathTo("Graphics","fallback background"), true, 4, 0, true );
			m_sprSongBackground.StretchTo( CRect(SCREEN_LEFT,SCREEN_TOP,SCREEN_RIGHT,SCREEN_BOTTOM) );

			for( i=0; i<iNumChars; i++ )
				m_frameStage.AddSubActor( &m_sprNumbers[i] );
			m_frameStage.AddSubActor( &m_sprStage );
		}
		break;
	case MODE_FINAL:
		m_sprStage.Load( THEME->GetPathTo("Graphics","stage final") );
		m_frameStage.AddSubActor( &m_sprStage );
		break;
	case MODE_EXTRA1:
		m_sprStage.Load( THEME->GetPathTo("Graphics","stage extra1") );
		m_frameStage.AddSubActor( &m_sprStage );
		break;
	case MODE_EXTRA2:
		m_sprStage.Load( THEME->GetPathTo("Graphics","stage extra2") );
		m_frameStage.AddSubActor( &m_sprStage );
		break;
	case MODE_ONI:
		m_sprStage.Load( THEME->GetPathTo("Graphics","stage oni") );
		m_frameStage.AddSubActor( &m_sprStage );
		break;
	case MODE_ENDLESS:
		m_sprStage.Load( THEME->GetPathTo("Graphics","stage endless") );
		m_frameStage.AddSubActor( &m_sprStage );
		break;
	default:
		ASSERT(0);
	}

	m_Fade.SetOpened();

	this->SendScreenMessage( SM_DoneFadingIn, 1.0f );
	this->SendScreenMessage( SM_StartFadingOut, 4.0f );
}

void ScreenStage::Update( float fDeltaTime )
{
	Screen::Update( fDeltaTime );


	m_quadMask.Update( fDeltaTime );
	m_frameStage.Update( fDeltaTime );
	m_Fade.Update( fDeltaTime );
	m_sprSongBackground.Update( fDeltaTime );
}

void ScreenStage::DrawPrimitives()
{
	DISPLAY->EnableZBuffer();

	if ( STAGE_TYPE == STAGE_TYPE_MAX) // only DANCE uses the Z mask
		m_quadMask.Draw();
	m_frameStage.Draw();

	DISPLAY->DisableZBuffer();

	m_Fade.Draw();

	if ( STAGE_TYPE == STAGE_TYPE_PUMP) // only PUMP uses the song background on the stage screen
		m_sprSongBackground.Draw();
}

void ScreenStage::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_StartFadingOut:
		m_Fade.CloseWipingRight( SM_GoToNextState );
		break;
	case SM_DoneFadingIn:
		break;
	case SM_GoToNextState:
		SCREENMAN->SetNewScreen( "ScreenGameplay" );
		break;
	}
}
