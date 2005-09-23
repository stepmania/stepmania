#ifndef ScreenPackages_H
#define ScreenPackages_H

#include "ScreenWithMenuElements.h"
#include "BitmapText.h"
#include "ezsockets.h"
#include "RageFileManager.h"
#include "RageFile.h"

#if !defined(WITHOUT_NETWORKING)

class ScreenPackages : public ScreenWithMenuElements
{
public:
	ScreenPackages( CString sName );
	virtual void Init();

	virtual void Input( const InputEventPlus &input );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	virtual void MenuStart( PlayerNumber pn );
	virtual void MenuUp( const InputEventPlus &input );
	virtual void MenuDown( const InputEventPlus &input );
	virtual void MenuLeft( const InputEventPlus &input );
	virtual void MenuRight( const InputEventPlus &input );
	virtual void MenuBack( PlayerNumber pn );

	virtual void TweenOffScreen( );
	virtual void Update(float f);

private:
	void UpdatePackagesList();
	void UpdateLinksList();
	void RefreshPackages();

	void HTMLParse();

	CString URLEncode( const CString &URL );			//Encode any string in URL-style
	CString StripOutContainers( const CString & In );	//Strip off "'s and ''s

	Sprite	m_sprExistingBG;
	Sprite	m_sprWebBG;

	Sprite	m_sprWebSel;

	BitmapText	m_textPackages;
	BitmapText	m_textWeb;

	CStringArray		m_Packages;

	vector <CString>	m_Links;
	vector <CString>	m_LinkTitles;
	BitmapText	m_textURL;

	int m_iPackagesPos;
	int m_iLinksPos;

	int m_iDLorLST;
	int m_bCanDL;

	//HTTP portion
	void CancelDownload( );
	void EnterURL( const CString & sURL );
	void HTTPUpdate( );

	//True if proper string, false if improper
	bool ParseHTTPAddress( const CString & URL, CString & Proto, CString & Server, int & Port, CString & Addy );

	Sprite	m_sprDL;
	Sprite	m_sprDLBG;
	void	UpdateProgress();

	bool	m_bIsDownloading;
	float	m_fLastUpdate;
	long	m_bytesLastUpdate;

	CString	m_sStatus;
	BitmapText	m_textStatus;

	EzSockets m_wSocket;

	bool	m_bGotHeader;

	RageFile	m_fOutputFile;
	CString	m_sEndName;
	bool	m_bIsPackage;

	CString m_sBaseAddress;
	//HTTP Header information responce
	long	m_iTotalBytes;
	long	m_iDownloaded;

	long	m_iResponseCode;
	CString	m_sResponseName;

	//Raw HTTP Buffer
	CString m_sBUFFER;
};

#endif

#endif 
/*
 * (c) 2004 Charles Lohr
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
