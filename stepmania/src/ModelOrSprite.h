#ifndef ModelOrSprite_H
#define ModelOrSprite_H
/*
-----------------------------------------------------------------------------
 Class: ModelOrSprite

 Desc: A 3D ModelOrSprite.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Actor.h"


class ModelOrSprite
{
public:
	ModelOrSprite () { m_pModel = NULL; m_pSprite = NULL; }
	virtual ~ModelOrSprite () { delete m_pModel; delete m_pSprite; }

	virtual void	Update( float fDelta ) { ASSERT(m_pModel||m_pSprite); if(m_pModel) m_pModel->Update(fDelta); else m_pModel->Update(fDelta); } 
	virtual void	DrawPrimitives() { ASSERT(m_pModel||m_pSprite); if(m_pModel) m_pModel->Update(fDelta); else m_pModel->Update(fDelta); } 

	void Load( CString sFile ) 
	{
		ASSERT( !m_pModel && !m_pSprite );
		if( sFile.Right(3)=="txt" )
		{
			m_pModel = new Model();
			m_pModel->Load( sFile );
		}
		else
		{
			m_pSprite = new Sprite();
			m_pSprite->Load( sFile );
		}
	}

private:
	Model* m_pModel;
	Sprite* m_pSprite;
};



#endif
