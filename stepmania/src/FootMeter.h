/*
-----------------------------------------------------------------------------
 File: FootMeter.h

 Desc: A graphic displayed in the FootMeter during Dancing.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/


#ifndef _FootMeter_H_
#define _FootMeter_H_


#include "Sprite.h"
#include "Song.h"
#include "BitmapText.h"
#include "ThemeManager.h"


class FootMeter : public BitmapText
{
public:
	FootMeter()
	{
		Load( THEME->GetPathTo(FONT_FEET) );

		SetNumFeet( 0, "" );
	};

	void SetFromSteps( Steps* pSteps )
	{
		if( pSteps != NULL )
		{
			SetDiffuseColor( D3DXCOLOR(1,1,1,1) );
			SetNumFeet( pSteps->m_iNumFeet, pSteps->m_sDescription );
		}
		else
		{
			SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
			SetNumFeet( 0, "" );
		}
	};

private:

	void SetNumFeet( int iNumFeet, CString sDescription )
	{
		CString sNewText;
		for( int f=0; f<=9; f++ )
			sNewText += (f<iNumFeet) ? "1" : "0";
		for( f=10; f<=12; f++ )
			if( f<iNumFeet )
				sNewText += "1";

		SetText( sNewText );


		sDescription.MakeLower();
		if(	sDescription.Find( "basic" ) != -1 )
			SetDiffuseColor( D3DXCOLOR(1,1,0,1) );
		else if( sDescription.Find( "trick" ) != -1 )
			SetDiffuseColor( D3DXCOLOR(1,0,0,1) );
		else if( sDescription.Find( "another" ) != -1 )
			SetDiffuseColor( D3DXCOLOR(1,0,0,1) );
		else if( sDescription.Find( "maniac" ) != -1 )
			SetDiffuseColor( D3DXCOLOR(0,1,0,1) );
		else if( sDescription.Find( "ssr" ) != -1 )
			SetDiffuseColor( D3DXCOLOR(0,1,0,1) );
		else if( sDescription.Find( "battle" ) != -1 )
			SetDiffuseColor( D3DXCOLOR(1,1,1,1) );
		else if( sDescription.Find( "couple" ) != -1 )
			SetDiffuseColor( D3DXCOLOR(1,1,1,1) );
		else
			SetDiffuseColor( D3DXCOLOR(0.8f,0.8f,0.8f,1) );

	};
};

#endif