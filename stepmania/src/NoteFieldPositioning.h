#ifndef NOTEFIELD_POSITIONING_H
#define NOTEFIELD_POSITIONING_H

#include "NoteTypes.h"
#include "PlayerNumber.h"
#include "StyleDef.h"
#include "PlayerNumber.h"

class Actor;

class NoteFieldPositioning
{
	Actor *m_Position[MAX_NOTE_TRACKS];
	bool m_bPerspective[MAX_NOTE_TRACKS];
//	float 
	Actor *m_PerspPosition[MAX_NOTE_TRACKS];

public:
	NoteFieldPositioning();
	~NoteFieldPositioning();
	void Init();
	void Update(float fDeltaTime);

	void LoadFromFile(CString fn);
	void LoadFromStyleDef(const StyleDef *s, PlayerNumber pn);

	void BeginDrawTrack(int tn);
	void EndDrawTrack(int tn);
};

#endif
