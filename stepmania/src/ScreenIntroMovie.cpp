#include "stdafx.h"
#include "ScreenIntroMovie.h"

void ScreenIntroMovie::Update( float fDelta )
{
	ScreenAttract::Update(fDelta);

/*	
	RageTexture *tex = sprite->GetTexture();
	if(tex->IsAMovie() && !tex->IsPlaying())
		this->SendScreenMessage( SM_BeginFadingOut,0 );
if( BackgroundImage->GetTexture()->RageMovieTexture::IsFinishedPlaying() == true )
		{
			this->SendScreenMessage( SM_BeginFadingOut,0 );
		}
*/
}

