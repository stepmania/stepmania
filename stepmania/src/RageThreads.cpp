/*
 * If you're going to use threads, remember this: 
 *
 * Threads suck.
 *
 * If there's any way to avoid them, take it!  Threaded code an order of
 * magnitude more complicated, harder to debug and harder to make robust.
 *
 * That said, here are a few helpers for when they're unavoidable.  (Use
 * SDL for the rest.)
 */

#include "stdafx.h"

#include "RageThreads.h"

RageMutex::RageMutex()
{
	Locked = 0;
	mut = SDL_CreateMutex();
	mutwait = SDL_CreateMutex();
}

RageMutex::~RageMutex()
{
	SDL_DestroyMutex(mut);
	SDL_DestroyMutex(mutwait);
}

void RageMutex::Lock()
{
	while(1)
	{
		SDL_LockMutex(mut);
		if(!Locked || LockedBy == SDL_ThreadID())
		{
			if(!Locked)
			{
				/* This mutex is now locked. */
				SDL_LockMutex(mutwait);
				LockedBy = SDL_ThreadID();
			} /* (else it was already locked and we're just increasing the counter) */
			Locked++;
			SDL_UnlockMutex(mut);
			return;
		}

		SDL_UnlockMutex(mut);

		/* Someone else is locking it.  Wait until it's available and try again. */
		SDL_LockMutex(mutwait);
		SDL_UnlockMutex(mutwait);
	}
}

void RageMutex::Unlock()
{
	SDL_LockMutex(mut);
	ASSERT(Locked);
	ASSERT(LockedBy == SDL_ThreadID());

	Locked--;
	if(!Locked)
	{
		LockedBy = 0;
		SDL_UnlockMutex(mutwait);
	}

	SDL_UnlockMutex(mut);
}

/*
-----------------------------------------------------------------------------
 File: RageThreads

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
    Glenn Maynard
-----------------------------------------------------------------------------
*/
