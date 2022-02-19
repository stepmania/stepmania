#include "global.h"
#include "JsonUtil.h"
#include "RageFile.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "arch/Dialog/Dialog.h"
#include "json/json.h"

bool JsonUtil::LoadFromString(Json::Value &root, RString sData, RString &sErrorOut)
{
	Json::Reader reader;
	bool parsingSuccessful = reader.parse(sData, root);
	if (!parsingSuccessful)
	{
		RString err = reader.getFormattedErrorMessages();
		LOG->Warn("JSON: LoadFromFileShowErrors failed: %s", err.c_str());
		return false;
	}
	return true;
}

bool JsonUtil::LoadFromFileShowErrors(Json::Value &root, RageFileBasic &f)
{
	// Optimization opportunity: read this streaming instead of at once
	RString sData;
	f.Read(sData, f.GetFileSize());
	return LoadFromStringShowErrors(root, sData);
}

bool JsonUtil::LoadFromFileShowErrors(Json::Value &root, const RString &sFile)
{
	RageFile f;
	if(!f.Open(sFile, RageFile::READ))
	{
		LOG->Warn("Couldn't open %s for reading: %s", sFile.c_str(), f.GetError().c_str());
		return false;
	}

	return LoadFromFileShowErrors(root, f);
}

bool JsonUtil::LoadFromStringShowErrors(Json::Value &root, RString sData)
{
	RString sError;
	if(!LoadFromString(root, sData, sError))
	{
		Dialog::OK(sError, "JSON_PARSE_ERROR");
		return false;
	}
	return true;
}

bool JsonUtil::WriteFile(const Json::Value &root, const RString &sFile, bool bMinified)
{
	std::string s;
	if(!bMinified)
	{
		Json::StyledWriter writer;
		s = writer.write(root);
	}
	else 
	{
		Json::FastWriter writer;
		s = writer.write(root);
	}

	RageFile f;
	if(!f.Open(sFile, RageFile::WRITE))
	{
		LOG->Warn("Couldn't open %s for reading: %s", sFile.c_str(), f.GetError().c_str());
		return false;
	}
	f.Write(s);
	return true;
}

std::vector<RString> JsonUtil::DeserializeArrayStrings(const Json::Value &value)
{
	std::vector<RString> values;
	for(auto &&inner_value : value)
	{
		if(inner_value.isConvertibleTo(Json::stringValue))
		{
			values.push_back(inner_value.asString());
		}
	}
	return values;
}

/*
 * (c) 2001-2004 Chris Danford
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
