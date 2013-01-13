#ifndef ScreenPackages_H
#define ScreenPackages_H

#include "ScreenWithMenuElements.h"
#include "BitmapText.h"
#include "ezsockets.h"
#include "RageFileManager.h"
#include "RageFile.h"
#include "Sprite.h"
#include "ThemeMetric.h"

#if !defined(WITHOUT_NETWORKING)

class ScreenPackages : public ScreenWithMenuElements
{
public:
	virtual void Init();

	virtual void HandleScreenMessage( const ScreenMessage SM );

	virtual bool MenuStart( const InputEventPlus &input );
	virtual bool MenuUp( const InputEventPlus &input );
	virtual bool MenuDown( const InputEventPlus &input );
	virtual bool MenuLeft( const InputEventPlus &input );
	virtual bool MenuRight( const InputEventPlus &input );
	virtual bool MenuBack( const InputEventPlus &input );

	virtual void TweenOffScreen( );
	virtual void Update(float f);

protected:
	ThemeMetric<float> EXISTINGBG_WIDTH; // "PackagesBGWidth"
	ThemeMetric<float> WEBBG_WIDTH; // "WebBGWidth"
	ThemeMetric<int> NUM_PACKAGES_SHOW; // "NumPackagesShow"
	ThemeMetric<int> NUM_LINKS_SHOW; // "NumLinksShow"
	ThemeMetric<RString> DEFAULT_URL; // "DefaultUrl"

private:
	void UpdatePackagesList();
	void UpdateLinksList();
	void RefreshPackages();

	void HTMLParse();

	RString StripOutContainers( const RString & In );	//Strip off "'s and ''s

	AutoActor	m_sprExistingBG;
	AutoActor	m_sprWebBG;

	Sprite	m_sprWebSel;

	BitmapText	m_textPackages;
	BitmapText	m_textWeb;

	vector<RString>		m_Packages;

	vector <RString>	m_Links;
	vector <RString>	m_LinkTitles;
	BitmapText	m_textURL;

	int m_iPackagesPos;
	int m_iLinksPos;

	int m_iDLorLST;
	int m_bCanDL;

	// HTTP portion
	void CancelDownload( );
	void EnterURL( const RString & sURL );
	void HTTPUpdate( );

	//True if proper string, false if improper
	bool ParseHTTPAddress( const RString & URL, RString & Proto, RString & Server, int & Port, RString & Addy );

	Sprite	m_sprDL;
	Sprite	m_sprDLBG;
	void	UpdateProgress();

	bool	m_bIsDownloading;
	float	m_fLastUpdate;
	long	m_bytesLastUpdate;

	RString	m_sStatus;
	BitmapText	m_textStatus;

	EzSockets m_wSocket;

	bool	m_bGotHeader;

	RageFile	m_fOutputFile;
	RString	m_sEndName;
	bool	m_bIsPackage;

	RString m_sBaseAddress;
	//HTTP Header information responce
	long	m_iTotalBytes;
	long	m_iDownloaded;

	long	m_iResponseCode;
	RString	m_sResponseName;

	//Raw HTTP Buffer
	RString m_sBUFFER;
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
