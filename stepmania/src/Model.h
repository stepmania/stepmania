#ifndef Model_H
#define Model_H
/*
-----------------------------------------------------------------------------
 Class: Model

 Desc: A 3D model.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Actor.h"
#include "RageTypes.h"
#include <vector>

struct msModel;

typedef float matrix_t[3][4];
typedef struct
{
	matrix_t	mRelative;
	matrix_t	mAbsolute;
	matrix_t	mRelativeFinal;
	matrix_t	mFinal;
} myBone_t;


class Model : public Actor
{
public:
	Model ();
	virtual ~Model ();

public:
	void	Clear ();
	void	Load( CString sPath )
	{
		if( sPath == "" ) return;
		if( sPath.Right(6) == ".model" )
			LoadFromModelFile( sPath );
		else 
			LoadMilkshapeAscii( sPath );
	};

	bool	LoadFromModelFile( CString sPath );
	bool	LoadMilkshapeAscii( CString sPath );
	bool	LoadMilkshapeAsciiBones( CString sPath );

	virtual void	Update( float fDelta );
	virtual void	DrawPrimitives();

	float	CalcDistance () const;
	void	SetupBones ();
	void	AdvanceFrame (float dt);

	virtual void SetState( int iNewState );
	virtual int GetNumStates();


private:
	msModel		*m_pModel;
	RageVector3	m_vMins, m_vMaxs;
	myBone_t	*m_pBones;
	typedef vector<RageVertex> RageVertexVector;
	vector<RageVertexVector>	m_vTempVerticesByBone;
	float		m_fCurrFrame;
};



#endif
