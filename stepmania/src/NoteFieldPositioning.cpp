#include "global.h"
#include "NoteFieldPositioning.h"
#include "Actor.h"
#include "RageDisplay.h"
#include "RageUtil.h"
#include "RageMath.h"
#include "RageLog.h"


NoteFieldPositioning::NoteFieldPositioning()
{
	/* Note: while Actor class is usually the base class of something that
	 * renders something to screen, all Actor itself does is store location and
	 * effect state and handle tweening between them.  These actors do this
	 * without actually rendering anything; we only use them for their render
	 * state. */
	for( int t=0; t<MAX_NOTE_TRACKS; t++ )
	{
		m_Position[t] = new Actor;
		m_bPerspective[t] = false;
		m_PerspPosition[t] = new Actor;
	}

	Init();
}

NoteFieldPositioning::~NoteFieldPositioning()
{
	for( int t=0; t<MAX_NOTE_TRACKS; t++ )
	{
		delete m_Position[t];
		delete m_PerspPosition[t];
	}
}

void NoteFieldPositioning::Init()
{
	for( int t=0; t<MAX_NOTE_TRACKS; t++ )
	{
		m_Position[t]->Reset();
	}
}
/*
void NoteFieldPositioning::LoadFromFile(CString fn)
{
	Init();

}
*/
void NoteFieldPositioning::LoadFromStyleDef(const StyleDef *s, PlayerNumber pn)
{
	Init();

	for( int t=0; t<MAX_NOTE_TRACKS; t++ )
	{
		/* Set up the normal position of each track. */
		const float fPixelXOffsetFromCenter = s->m_ColumnInfo[pn][t].fXOffset;
		m_Position[t]->SetX(fPixelXOffsetFromCenter);
	}
}

void NoteFieldPositioning::Update(float fDeltaTime)
{
	for( int t=0; t<MAX_NOTE_TRACKS; t++ )
	{
		m_Position[t]->Update(fDeltaTime);
		m_PerspPosition[t]->Update(fDeltaTime);
	}
}

void NoteFieldPositioning::BeginDrawTrack(int tn)
{
	m_Position[tn]->BeginDraw();

	if(m_bPerspective[tn])
	{
		DISPLAY->EnterPerspective(45);
	}
	
//	m_PerspPosition[tn]->BeginDraw();
}

void NoteFieldPositioning::EndDrawTrack(int tn)
{
	m_PerspPosition[tn]->EndDraw();

	if(m_bPerspective[tn])
	{
		DISPLAY->ExitPerspective();
	}

//	m_Position[tn]->EndDraw();
}

