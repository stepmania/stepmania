#include "global.h"
#include "MovieTexture.h"


#include "MovieTexture_DShow.h"

/* Try drivers in order of preference until we find one that works. */
/* Well, eventually; for now it's DShow or bust.w */
RageMovieTexture *MakeRageMovieTexture(RageTextureID ID)
{
#if defined(WIN32)
	return new MovieTexture_DShow(ID);
#else
	RageException::Throw("xxx");
#endif
}

