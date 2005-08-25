/* Model - A 3D model. */

#ifndef MODEL_H
#define MODEL_H

#include "Actor.h"
#include "RageTypes.h"
#include "ModelTypes.h"
#include <vector>
#include <map>

class RageModelGeometry;
class RageCompiledGeometry;

class Model : public Actor
{
public:
	Model();
	virtual ~Model();
	virtual Actor *Copy() const;

	void	Clear();
	void	Load( CString sFile );

	void	LoadPieces( CString sMeshesPath, CString sMaterialsPath, CString sBomesPath );
	void	LoadMilkshapeAscii( CString sFile );
	void 	LoadMaterialsFromMilkshapeAscii( CString sPath );
	bool	LoadMilkshapeAsciiBones( CString sAniName, CString sPath );

	void LoadFromNode( const CString& sDir, const XNode* pNode );

	void	PlayAnimation( CString sAniName, float fPlayRate = 1 );

	virtual void	Update( float fDelta );
	virtual bool	EarlyAbortDraw() const;
	virtual void	DrawPrimitives();

	void	DrawCelShaded();

	virtual int GetNumStates() const;
	virtual void SetState( int iNewState );
	virtual float GetAnimationLengthSeconds() const;
	virtual void SetSecondsIntoAnimation( float fSeconds );

	CString		GetDefaultAnimation() const { return m_sDefaultAnimation; };
	void		SetDefaultAnimation( CString sAnimation, float fPlayRate = 1 );
	bool		m_bRevertToDefaultAnimation;

	bool	MaterialsNeedNormals() const;

	//
	// Lua
	//
	virtual void PushSelf( lua_State *L );

private:
    RageModelGeometry	*m_pGeometry;

    vector<msMaterial>			m_Materials;
	map<CString,msAnimation>	m_mapNameToAnimation;
	const msAnimation*				m_pCurAnimation;

	static void SetBones( const msAnimation* pAnimation, float fFrame, vector<myBone_t> &vpBones );
	vector<myBone_t>	m_vpBones;

	// If any vertex has a bone weight, then then render from m_pTempGeometry.  
	// Otherwise, render directly from m_pGeometry.
	RageCompiledGeometry* m_pTempGeometry;
	void UpdateTempGeometry();
	
	/* Keep a copy of the mesh data only if m_pTempGeometry is in use.  The normal and
	 * position data will be changed; the rest is static and kept only to prevent making
	 * a complete copy. */
	vector<msMesh>	m_vTempMeshes;

	void DrawMesh( int i ) const;
	void AdvanceFrame( float fDeltaTime );

	float		m_fCurFrame;
	CString		m_sDefaultAnimation;
	float		m_fDefaultAnimationRate;
	float		m_fCurAnimationRate;

};

#endif

/*
 * (c) 2003-2004 Chris Danford
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
