#include "global.h"
#include "ArrowBackdrop.h"
#include "GameState.h"
#include "NoteFieldPositioning.h"

void ArrowBackdrop::BeginDraw()
{
	BGAnimation::BeginDraw();
	g_NoteFieldMode[m_PlayerNumber].BeginDrawTrack(-1);
}

void ArrowBackdrop::EndDraw()
{
	g_NoteFieldMode[m_PlayerNumber].EndDrawTrack(-1);
	BGAnimation::EndDraw();
}
