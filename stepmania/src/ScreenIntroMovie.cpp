#include "stdafx.h"
#include "ScreenIntroMovie.h"

void ScreenIntroMovie::Update( float fDelta )
{
	ScreenAttract::Update(fDelta);

/*	
	RageTexture *tex = sprite->GetTexture();
	if(tex->IsAMovie() && !tex->IsPlaying())
		this->SendScreenMessage( SM_BeginFadingOut,0 );

	and when initting the sprite in ScreenIntroMovie::ScreenIntroMovie:
	if(sprite->GetTexture->IsAMovie())
	{
		tex->SetLooping(false);
		ClearMessageQueue(SM_BeginFadingOut);
	}

	Don't use RageMovieTexture directly.

*/
}

