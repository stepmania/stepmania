#ifndef ActorCollision_H
#define ActorCollision_H

#include "Actor.h"
#include "GameConstantsAndTypes.h"


inline float GetOffScreenLeft(  Actor* pActor ) { return SCREEN_LEFT  - pActor->GetZoomedWidth()/2; }
inline float GetOffScreenRight( Actor* pActor ) { return SCREEN_RIGHT + pActor->GetZoomedWidth()/2; }
inline float GetOffScreenTop(   Actor* pActor ) { return SCREEN_TOP   - pActor->GetZoomedHeight()/2; }
inline float GetOffScreenBottom(Actor* pActor ) { return SCREEN_BOTTOM+ pActor->GetZoomedHeight()/2; }

inline bool IsOffScreenLeft(  Actor* pActor ) { return pActor->GetX() < GetOffScreenLeft(pActor); }
inline bool IsOffScreenRight( Actor* pActor ) { return pActor->GetX() > GetOffScreenRight(pActor); }
inline bool IsOffScreenTop(   Actor* pActor ) { return pActor->GetY() < GetOffScreenTop(pActor); }
inline bool IsOffScreenBottom(Actor* pActor ) { return pActor->GetY() > GetOffScreenBottom(pActor); }
inline bool IsOffScreen(      Actor* pActor ) { return IsOffScreenLeft(pActor) || IsOffScreenRight(pActor) || IsOffScreenTop(pActor) || IsOffScreenBottom(pActor); }

// guard rail is the area that keeps particles from going off screen
inline float GetGuardRailLeft(  Actor* pActor ) { return SCREEN_LEFT  + pActor->GetZoomedWidth()/2; }
inline float GetGuardRailRight( Actor* pActor ) { return SCREEN_RIGHT - pActor->GetZoomedWidth()/2; }
inline float GetGuardRailTop(   Actor* pActor ) { return SCREEN_TOP   + pActor->GetZoomedHeight()/2; }
inline float GetGuardRailBottom(Actor* pActor ) { return SCREEN_BOTTOM- pActor->GetZoomedHeight()/2; }

inline bool HitGuardRailLeft(  Actor* pActor ) { return pActor->GetX() < GetGuardRailLeft(pActor); }
inline bool HitGuardRailRight( Actor* pActor ) { return pActor->GetX() > GetGuardRailRight(pActor); }
inline bool HitGuardRailTop(   Actor* pActor ) { return pActor->GetY() < GetGuardRailTop(pActor); }
inline bool HitGuardRailBottom(Actor* pActor ) { return pActor->GetY() > GetGuardRailBottom(pActor); }


#endif

/*
 * (c) 2001-2002 Chris Danford
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
