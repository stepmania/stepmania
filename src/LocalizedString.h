#ifndef LocalizedString_H
#define LocalizedString_H

#include <string>

class ILocalizedStringImpl
{
public:
	virtual ~ILocalizedStringImpl() { }
	virtual void Load( std::string const &sGroup, std::string const &sName ) = 0;
	virtual std::string const GetLocalized() const = 0;
};
/** @brief Get a String based on the user's natural language. */
class LocalizedString
{
public:
	LocalizedString( std::string const & sGroup = "", std::string const & sName = "" );
	LocalizedString(LocalizedString const& other);
	~LocalizedString();
	void Load( std::string const &sGroup, std::string const &sName );
	operator std::string const () const { return GetValue(); }
	std::string const GetValue() const;
	std::string const& GetGroup() const { return m_sGroup; }
	std::string const& GetName() const { return m_sName; }

	typedef ILocalizedStringImpl *(*MakeLocalizer)();
	static void RegisterLocalizer( MakeLocalizer pFunc );

private:
	void CreateImpl();
	std::string m_sGroup;
	std::string m_sName;
	ILocalizedStringImpl *m_pImpl;
	// Swallow up warnings. If they must be used, define them.
	LocalizedString& operator=(const LocalizedString&);
};

#endif

/**
 * @file
 * @author Chris Danford, Glenn Maynard (c) 2001-2005
 * @section LICENSE
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
