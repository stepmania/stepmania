#include "global.h"
#include "MovieTexture.h"

#if defined(WIN32)
#include "MovieTexture_DShow.h"
#endif

/* Try drivers in order of preference until we find one that works. */
/* Well, eventually; for now it's DShow or bust.w */
RageMovieTexture *MakeRageMovieTexture(RageTextureID ID)
{
#if defined(WIN32)
	return new MovieTexture_DShow(ID);
#else
	/* XXX: need a simple null movie texture object */
	RageException::Throw("xxx");
#endif
}

