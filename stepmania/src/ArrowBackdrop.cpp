#include "global.h"
#include "ArrowBackdrop.h"
#include "GameState.h"
#include "NoteFieldPositioning.h"

void ArrowBackdrop::BeginDraw()
{
	BGAnimation::BeginDraw();
	GAMESTATE->m_Position->BeginDrawBackdrop(m_PlayerNumber);
}

void ArrowBackdrop::EndDraw()
{
	GAMESTATE->m_Position->EndDrawBackdrop(m_PlayerNumber);
	BGAnimation::EndDraw();
}
