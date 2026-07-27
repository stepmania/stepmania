#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include "Preference.h"
#include "StdString.h"

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include <ixwebsocket/IXHttp.h>
#include <ixwebsocket/IXHttpClient.h>
#include <ixwebsocket/IXSocketTLSOptions.h>
#include <ixwebsocket/IXWebSocket.h>

#include "EnumHelper.h"
#include "LuaManager.h"

struct lua_State;

enum HttpErrorCode
{
	HttpErrorCode_Blocked,
	HttpErrorCode_UnknownError,
	HttpErrorCode_FileError,

	// from IXWebSocket
	HttpErrorCode_CannotConnect,
	HttpErrorCode_Timeout,
	HttpErrorCode_Gzip,
	HttpErrorCode_UrlMalformed,
	HttpErrorCode_CannotCreateSocket,
	HttpErrorCode_SendError,
	HttpErrorCode_ReadError,
	HttpErrorCode_CannotReadStatusLine,
	HttpErrorCode_MissingStatus,
	HttpErrorCode_HeaderParsingError,
	HttpErrorCode_MissingLocation,
	HttpErrorCode_TooManyRedirects,
	HttpErrorCode_ChunkReadError,
	HttpErrorCode_CannotReadBody,
	HttpErrorCode_Cancelled,

	NUM_HttpErrorCode,
	HttpErrorCode_Invalid,
};
const RString& HttpErrorCodeToString(HttpErrorCode dc);
HttpErrorCode StringToHttpErrorCode(const RString& sDC);
LuaDeclareType(HttpErrorCode);

enum WebSocketMessageType
{
	// from IXWebSocket
	WebSocketMessageType_Message,
	WebSocketMessageType_Open,
	WebSocketMessageType_Close,
	WebSocketMessageType_Error,

	NUM_WebSocketMessageType,
	WebSocketMessageType_Invalid,
};
const RString& WebSocketMessageTypeToString(WebSocketMessageType dc);
WebSocketMessageType StringToWebSocketMessageType(const RString& sDC);
LuaDeclareType(WebSocketMessageType);

struct HttpRequestArgs
{
	std::string url;
	std::string method = ix::HttpClient::kGet;
	std::string body;
	std::string multipartBoundary;
	std::unordered_map<std::string, std::string> headers;
	int connectTimeout = -1;
	int transferTimeout = -1;
	std::string downloadFile;
	std::function<bool(int current, int total)> onProgress;
	std::function<void(const ix::HttpResponsePtr& response)> onResponse;
	std::function<void(const std::string& errorMessage)> onFileError;
};

class HttpRequestFuture
{
public:
	HttpRequestFuture(ix::HttpRequestArgsPtr& args) : args(args) {};

	static int Collect(lua_State *L);
	static int Cancel(lua_State *L);

private:
	ix::HttpRequestArgsPtr args;
};

typedef std::shared_ptr<HttpRequestFuture> HttpRequestFuturePtr;

struct WebSocketArgs
{
	std::string url;
	std::unordered_map<std::string, std::string> headers;
	int handshakeTimeout = -1;
	int pingInterval = -1;
	bool automaticReconnect = true;
	std::function<void(const ix::WebSocketMessagePtr& response)> onMessage;
	std::function<void()> onClose;
};

class WebSocketHandle
{
public:
	WebSocketHandle() {};

	static int Collect(lua_State *L);
	static int Close(lua_State *L);
	static int Send(lua_State *L);

	ix::WebSocket webSocket;
	std::function<void()> onClose;
};

typedef std::shared_ptr<WebSocketHandle> WebSocketHandlePtr;

class NetworkManager
{
public:
	NetworkManager();
	~NetworkManager();

	bool IsUrlAllowed(const std::string& url);
	HttpRequestFuturePtr HttpRequest(const HttpRequestArgs& args);
	WebSocketHandlePtr WebSocket(const WebSocketArgs& args);
	std::string UrlEncode(const std::string& value);
	std::string EncodeQueryParameters(const std::unordered_map<std::string, std::string>& query);

	// Lua
	void PushSelf(lua_State *L);

private:
	std::string GetUserAgent();
	void ClearDownloads();

	ix::HttpClient httpClient;
	ix::HttpClient downloadClient;
	ix::SocketTLSOptions tlsOptions;

	static Preference<bool> httpEnabled;
	static Preference<RString> httpAllowHosts;
};

extern NetworkManager*	NETWORK;

#endif

/*
 * (c) 2021 Martin Natano
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
