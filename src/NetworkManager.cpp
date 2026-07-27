#include "global.h"
#include "NetworkManager.h"
#include "LuaManager.h"
#include "ProductInfo.h"
#include "RageFile.h"
#include "RageFileManager.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "SpecialFiles.h"
#include "StdString.h"
#include "ver.h"

#include <ixwebsocket/IXHttpClient.h>
#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXUrlParser.h>
#include <ixwebsocket/IXWebSocket.h>

#include <algorithm>
#include <climits>
#include <cstddef>
#include <memory>
#include <sstream>
#include <unordered_map>
#include <vector>


NetworkManager*	NETWORK = nullptr;	// global and accessible from anywhere in our program

Preference<bool> NetworkManager::httpEnabled("HttpEnabled", true, nullptr, PreferenceType::Immutable);
Preference<RString> NetworkManager::httpAllowHosts("HttpAllowHosts", "*.groovestats.com", nullptr, PreferenceType::Immutable);

static const char *HttpErrorCodeNames[] = {
	"Blocked",
	"UnknownError",
	"FileError",
	"CannotConnect",
	"Timeout",
	"Gzip",
	"UrlMalformed",
	"CannotCreateSocket",
	"SendError",
	"ReadError",
	"CannotReadStatusLine",
	"MissingStatus",
	"HeaderParsingError",
	"MissingLocation",
	"TooManyRedirects",
	"ChunkReadError",
	"CannotReadBody",
	"Cancelled",
};
XToString(HttpErrorCode);
StringToX(HttpErrorCode);
LuaXType(HttpErrorCode);

static const char *WebSocketMessageTypeNames[] = {
	"Message",
	"Open",
	"Close",
	"Error",
};
XToString(WebSocketMessageType);
StringToX(WebSocketMessageType);
LuaXType(WebSocketMessageType);

NetworkManager::NetworkManager() : httpClient(true), downloadClient(true)
{
	ix::initNetSystem();

	// Register with Lua.
	{
		Lua *L = LUA->Get();
		this->PushSelf(L);
		lua_setglobal(L, "NETWORK");
		LUA->Release(L);
	}

	RageFile f;
	if(f.Open(SpecialFiles::CA_BUNDLE_PATH))
	{
		RString data;
		f.Read(data);
		f.Close();

		this->tlsOptions.caFile = data;
	}
	else
	{
		LOG->Warn("Reading '%s' failed: %s", SpecialFiles::CA_BUNDLE_PATH.c_str(), f.GetError().c_str());
	}

	this->httpClient.setTLSOptions(this->tlsOptions);
	this->downloadClient.setTLSOptions(this->tlsOptions);

	this->ClearDownloads();
}

NetworkManager::~NetworkManager()
{
	// Unregister with Lua.
	LUA->UnsetGlobal("NETWORK");

	ix::uninitNetSystem();
}

bool NetworkManager::IsUrlAllowed(const std::string& url)
{
	if (!this->httpEnabled.Get())
	{
		return false;
	}

	std::string protocol;
	RString host;
	std::string path;
	std::string query;
	int port;

	bool valid = ix::UrlParser::parse(url, protocol, host, path, query, port);

	if (!valid)
	{
		return false;
	}

	if (protocol != "http" && protocol != "https" && protocol != "ws" && protocol != "wss")
	{
		return false;
	}

	host.MakeLower();

	RString allowedHostsStr = this->httpAllowHosts.Get();
	allowedHostsStr.MakeLower();

	std::vector<RString> allowedHosts;
	split(allowedHostsStr, ",", allowedHosts);

	for (const auto& allowedHost : allowedHosts)
	{
		// subdomain wildcards; ".domain" doesn't match "*.domain", but "a.domain" does
		if (allowedHost.substr(0, 2) == "*." && host.length() >= allowedHost.length())
		{
			std::size_t pos = host.length() - allowedHost.length() + 1;
			if (host.substr(pos) == allowedHost.substr(1))
				return true;
		}

		// literal match
		if (host == allowedHost)
			return true;
	}

	return false;
}

HttpRequestFuturePtr NetworkManager::HttpRequest(const HttpRequestArgs& args)
{
	auto &client = args.downloadFile.empty() ? this->httpClient : this->downloadClient;
	auto downloadFile = std::make_shared<RageFile>();
	std::string downloadFilename;

	ix::HttpRequestArgsPtr req = client.createRequest(args.url, args.method);
	req->body = args.body;
	req->multipartBoundary = args.multipartBoundary;

	req->extraHeaders["User-Agent"] = this->GetUserAgent();
	for (const auto& entry : args.headers)
	{
		req->extraHeaders[entry.first] = entry.second;
	}

	if (!args.downloadFile.empty())
	{
		downloadFilename = std::string("/Downloads/") + args.downloadFile;
		if (!downloadFile->Open(downloadFilename, RageFile::WRITE | RageFile::STREAMED))
		{
			req->cancel = true;
		}

		req->onChunkCallback = [args, downloadFile, req](const std::string& data) {
			if (downloadFile->Write(data.c_str(), data.size()) < 0)
			{
				req->cancel = true;
			}
		};

		// Don't timeout downloads by default. This value might be
		// overwritten below if a value is specified in the request.
		req->transferTimeout = INT_MAX;
	}

	if (args.connectTimeout > 0)
		req->connectTimeout = args.connectTimeout;

	if (args.transferTimeout > 0)
		req->transferTimeout = args.transferTimeout;

	if (args.onProgress)
		req->onProgressCallback = args.onProgress;

	client.performRequest(req, [args, downloadFile, downloadFilename](const ix::HttpResponsePtr& response) {
		if (!args.downloadFile.empty())
		{
			RString error = downloadFile->GetError();
			downloadFile->Close();

			if (!error.empty())
			{
				std::string errorMessage = "could not write to " + downloadFile->GetPath() + ": " + error;
				if (args.onFileError)
					args.onFileError(errorMessage);

				FILEMAN->Remove(downloadFilename);
				return;
			}
		}

		if (args.onResponse)
			args.onResponse(response);

		if (!args.downloadFile.empty())
		{
			FILEMAN->Remove(downloadFilename);
		}
	});

	return std::make_shared<HttpRequestFuture>(req);
}

WebSocketHandlePtr NetworkManager::WebSocket(const WebSocketArgs& args)
{
	auto handle = std::make_shared<WebSocketHandle>();
	handle->onClose = args.onClose;

	handle->webSocket.setUrl(args.url);
	handle->webSocket.setTLSOptions(this->tlsOptions);
	handle->webSocket.setOnMessageCallback(args.onMessage);

	ix::WebSocketHttpHeaders headers;
	headers["User-Agent"] = this->GetUserAgent();
	for (const auto& entry : args.headers)
	{
		headers[entry.first] = entry.second;
	}
	handle->webSocket.setExtraHeaders(headers);

	if (args.handshakeTimeout > -1)
		handle->webSocket.setHandshakeTimeout(args.handshakeTimeout);

	if (args.pingInterval > -1)
		handle->webSocket.setPingInterval(args.pingInterval);

	if (args.automaticReconnect)
	{
		handle->webSocket.enableAutomaticReconnection();
	}
	else
	{
		handle->webSocket.disableAutomaticReconnection();
	}

	handle->webSocket.start();

	return handle;
}

std::string NetworkManager::UrlEncode(const std::string& value)
{
	return this->httpClient.urlEncode(value);
}

std::string NetworkManager::EncodeQueryParameters(const std::unordered_map<std::string, std::string>& query)
{
	return this->httpClient.serializeHttpParameters(query);
}

std::string NetworkManager::GetUserAgent()
{
	std::stringstream ss;
	ss << PRODUCT_FAMILY << "/" << product_version;
	return ss.str();
}

void NetworkManager::ClearDownloads()
{
	std::vector<RString> files;
	FILEMAN->GetDirListing("/Downloads/*", files, false, true);

	for (const auto& file : files)
	{
		if (FILEMAN->IsADirectory(file))
		{
			FILEMAN->DeleteRecursive(file + "/");
		}
		else
		{
			FILEMAN->Remove(file);
		}
	}
}

int HttpRequestFuture::Collect(lua_State *L)
{
	void *udata = luaL_checkudata(L, 1, "HttpRequestFuture");
	auto futptr = static_cast<HttpRequestFuturePtr*>(udata);
	futptr->~shared_ptr();
	return 0;
}

int HttpRequestFuture::Cancel(lua_State *L)
{
	void *udata = luaL_checkudata(L, 1, "HttpRequestFuture");
	auto fut = *static_cast<HttpRequestFuturePtr*>(udata);
	fut->args->cancel = true;
	return 0;
}

int WebSocketHandle::Collect(lua_State *L)
{
	void *udata = luaL_checkudata(L, 1, "WebSocketHandle");
	auto handleptr = static_cast<WebSocketHandlePtr*>(udata);
	handleptr->~shared_ptr();
	return 0;
}

int WebSocketHandle::Close(lua_State *L)
{
	void *udata = luaL_checkudata(L, 1, "WebSocketHandle");
	auto handle = *static_cast<WebSocketHandlePtr*>(udata);
	LUA->YieldLua();
	handle->webSocket.stop();
	handle->onClose();
	LUA->UnyieldLua();
	return 0;
}

int WebSocketHandle::Send(lua_State *L)
{
	void *udata = luaL_checkudata(L, 1, "WebSocketHandle");
	auto handle = *static_cast<WebSocketHandlePtr*>(udata);

	std::size_t len;
	const char *s = luaL_checklstring(L, 2, &len);
	std::string data(s, len);

	bool binary = lua_toboolean(L, 3);

	auto info = handle->webSocket.send(data, binary);

	lua_pushboolean(L, info.success);
	return 1;
}


// lua start
#include "LuaBinding.h"

static void registerHttpRequestMetatable(lua_State *L)
{
	const luaL_Reg HttpRequest_meta[] = {
		{"__gc", HttpRequestFuture::Collect},
		{"Cancel", HttpRequestFuture::Cancel},
		{NULL, NULL},
	};

	luaL_newmetatable(L, "HttpRequestFuture");
	luaL_register(L, NULL, HttpRequest_meta);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	lua_pop(L, 1);
}

REGISTER_WITH_LUA_FUNCTION(registerHttpRequestMetatable)

static void registerWebSocketMetatable(lua_State *L)
{
	const luaL_Reg WebSocket_meta[] = {
		{"__gc", WebSocketHandle::Collect},
		{"Close", WebSocketHandle::Close},
		{"Send", WebSocketHandle::Send},
		{NULL, NULL},
	};

	luaL_newmetatable(L, "WebSocketHandle");
	luaL_register(L, NULL, WebSocket_meta);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	lua_pop(L, 1);
}

REGISTER_WITH_LUA_FUNCTION(registerWebSocketMetatable)

/** @brief Allow Lua to have access to the NetworkManager. */
class LunaNetworkManager: public Luna<NetworkManager>
{
public:
	static int IsUrlAllowed(T* p, lua_State *L)
	{
		std::string url = SArg(1);

		lua_pushboolean(L, p->IsUrlAllowed(url));
		return 1;
	}

	static int HttpRequest(T* p, lua_State *L)
	{
		luaL_checktype(L, 1, LUA_TTABLE);

		HttpRequestArgs args;
		int onProgressRef = LUA_NOREF;
		int onResponseRef = LUA_NOREF;

		lua_getfield(L, 1, "url");
		if (lua_isnil(L, -1))
		{
			luaL_error(L, "url is required");
		}
		else if (lua_isstring(L, -1))
		{
			args.url = lua_tostring(L, -1);
		}
		else
		{
			luaL_error(L, "url must be a string");
		}
		lua_pop(L, 1);

		lua_getfield(L, 1, "method");
		if (!lua_isnil(L, -1))
		{
			if (lua_isstring(L, -1))
			{
				std::string method = lua_tostring(L, -1);

				std::vector<std::string> supported_methods{"GET", "POST", "PUT", "PATCH", "DELETE", "HEAD"};
				if (std::find(supported_methods.begin(), supported_methods.end(), method) == supported_methods.end())
				{
					luaL_error(L, "unknown method: '%s'", method.c_str());
				}

				args.method = method;
			}
			else
			{
				luaL_error(L, "method must be a string");
			}
		}
		lua_pop(L, 1);

		lua_getfield(L, 1, "body");
		if (!lua_isnil(L, -1))
		{
			if (lua_isstring(L, -1))
			{
				std::size_t len;
				const char *s = lua_tolstring(L, -1, &len);
				args.body = std::string(s, len);
			}
			else
			{
				luaL_error(L, "body must be a string");
			}
		}
		lua_pop(L, 1);

		lua_getfield(L, 1, "multipartBoundary");
		if (!lua_isnil(L, -1))
		{
			if (lua_isstring(L, -1))
			{
				std::size_t len;
				const char *s = lua_tolstring(L, -1, &len);
				args.multipartBoundary = std::string(s, len);
			}
			else
			{
				luaL_error(L, "multipartBoundary must be a string");
			}
		}
		lua_pop(L, 1);

		lua_getfield(L, 1, "headers");
		if (!lua_isnil(L, -1)) {
			if (lua_istable(L, -1))
			{
				lua_pushnil(L);
				while(lua_next(L, -2) != 0)
				{
					if (!lua_isstring(L, -2))
					{
						luaL_error(L, "header keys must be strings");
					}

					if (!lua_isstring(L, -1))
					{
						luaL_error(L, "header values must be strings");
					}

					std::string key = lua_tostring(L, -2);
					std::string value = lua_tostring(L, -1);

					args.headers[key] = value;

					lua_pop(L, 1);
				}
			}
			else
			{
				luaL_error(L, "headers must be a table");
			}
		}
		lua_pop(L, 1);

		lua_getfield(L, 1, "connectTimeout");
		if (!lua_isnil(L, -1)) {
			if (lua_isnumber(L, -1))
			{
				args.connectTimeout = lua_tointeger(L, -1);
			}
			else
			{
				luaL_error(L, "connectTimeout must be an integer");
			}
		}
		lua_pop(L, 1);

		lua_getfield(L, 1, "transferTimeout");
		if (!lua_isnil(L, -1))
		{
			if (lua_isnumber(L, -1))
			{
				args.transferTimeout = lua_tointeger(L, -1);
			}
			else
			{
				luaL_error(L, "transferTimeout must be an integer");
			}
		}
		lua_pop(L, 1);

		lua_getfield(L, 1, "downloadFile");
		if (!lua_isnil(L, -1))
		{
			if (lua_isstring(L, -1))
			{
				args.downloadFile = lua_tostring(L, -1);
			}
			else
			{
				luaL_error(L, "downloadFile must be a string");
			}
		}
		lua_pop(L, 1);

		lua_getfield(L, 1, "onProgress");
		if (!lua_isnil(L, -1))
		{
			if (lua_isfunction(L, -1))
			{
				lua_pushvalue(L, -1);
				onProgressRef = luaL_ref(L, LUA_REGISTRYINDEX);

				args.onProgress = [onProgressRef](int current, int total)
				{
					Lua *L = LUA->Get();
					handleProgress(L, current, total, onProgressRef);
					LUA->Release(L);

					return true;
				};
			}
			else
			{
				luaL_error(L, "onProgress must be a function");
			}
		}
		lua_pop(L, 1);

		lua_getfield(L, 1, "onResponse");
		if (!lua_isnil(L, -1))
		{
			if (lua_isfunction(L, -1))
			{
				lua_pushvalue(L, -1);
				onResponseRef = luaL_ref(L, LUA_REGISTRYINDEX);
			}
			else
			{
				luaL_unref(L, LUA_REGISTRYINDEX, onProgressRef);
				luaL_error(L, "onResponse must be a function");
			}
		}
		lua_pop(L, 1);

		args.onFileError = [onProgressRef, onResponseRef](const std::string& errorMessage)
		{
			Lua *L = LUA->Get();

			luaL_unref(L, LUA_REGISTRYINDEX, onProgressRef);

			if (onResponseRef != LUA_NOREF)
				handleFileError(L, errorMessage, onResponseRef);

			LUA->Release(L);
		};

		args.onResponse = [onProgressRef, onResponseRef](const ix::HttpResponsePtr& response)
		{
			Lua *L = LUA->Get();

			luaL_unref(L, LUA_REGISTRYINDEX, onProgressRef);

			if (onResponseRef != LUA_NOREF)
				handleHttpResponse(L, response, onResponseRef);

			LUA->Release(L);
		};

		if (p->IsUrlAllowed(args.url))
		{
			auto fut = p->HttpRequest(args);

			void *vp = lua_newuserdata(L, sizeof(std::shared_ptr<HttpRequestFuture>));
			new(vp) std::shared_ptr<::HttpRequestFuture>(fut);
			luaL_getmetatable(L, "HttpRequestFuture");
			lua_setmetatable(L, -2);
			return 1;
		}
		else
		{
			LOG->Warn("blocked access to %s", args.url.c_str());
			luaL_unref(L, LUA_REGISTRYINDEX, onProgressRef);
			if (onResponseRef != LUA_NOREF)
			{
				handleUrlForbidden(L, args.url, onResponseRef);
			}
			return 0;
		}
	}

	static int WebSocket(T* p, lua_State *L)
	{
		luaL_checktype(L, 1, LUA_TTABLE);

		WebSocketArgs args;
		int onMessageRef = LUA_NOREF;

		lua_getfield(L, 1, "url");
		if (lua_isnil(L, -1))
		{
			luaL_error(L, "url is required");
		}
		else if (lua_isstring(L, -1))
		{
			args.url = lua_tostring(L, -1);
		}
		else
		{
			luaL_error(L, "url must be a string");
		}
		lua_pop(L, 1);

		lua_getfield(L, 1, "headers");
		if (!lua_isnil(L, -1)) {
			if (lua_istable(L, -1))
			{
				lua_pushnil(L);
				while(lua_next(L, -2) != 0)
				{
					if (!lua_isstring(L, -2))
					{
						luaL_error(L, "header keys must be strings");
					}

					if (!lua_isstring(L, -1))
					{
						luaL_error(L, "header values must be strings");
					}

					std::string key = lua_tostring(L, -2);
					std::string value = lua_tostring(L, -1);

					args.headers[key] = value;

					lua_pop(L, 1);
				}
			}
			else
			{
				luaL_error(L, "headers must be a table");
			}
		}
		lua_pop(L, 1);

		lua_getfield(L, 1, "handshakeTimeout");
		if (!lua_isnil(L, -1)) {
			if (lua_isnumber(L, -1))
			{
				args.handshakeTimeout = lua_tointeger(L, -1);
			}
			else
			{
				luaL_error(L, "handshakeTimeout must be an integer");
			}
		}
		lua_pop(L, 1);

		lua_getfield(L, 1, "pingInterval");
		if (!lua_isnil(L, -1))
		{
			if (lua_isnumber(L, -1))
			{
				args.pingInterval = lua_tointeger(L, -1);
			}
			else
			{
				luaL_error(L, "pingInterval must be an integer");
			}
		}
		lua_pop(L, 1);

		lua_getfield(L, 1, "automaticReconnect");
		if (!lua_isnil(L, -1))
		{
			if (lua_isboolean(L, -1))
			{
				args.automaticReconnect = lua_toboolean(L, -1);
			}
			else
			{
				luaL_error(L, "automaticReconnect must be a boolean");
			}
		}
		lua_pop(L, 1);

		lua_getfield(L, 1, "onMessage");
		if (!lua_isnil(L, -1))
		{
			if (lua_isfunction(L, -1))
			{
				lua_pushvalue(L, -1);
				onMessageRef = luaL_ref(L, LUA_REGISTRYINDEX);
			}
			else
			{
				luaL_error(L, "onMessage must be a function");
			}
		}
		lua_pop(L, 1);

		args.onMessage = [onMessageRef](const ix::WebSocketMessagePtr& msg)
		{
			Lua *L = LUA->Get();

			if (onMessageRef != LUA_NOREF)
				handleMessage(L, msg, onMessageRef);

			LUA->Release(L);
		};

		args.onClose = [onMessageRef]()
		{
			Lua *L = LUA->Get();

			if (onMessageRef != LUA_NOREF)
				luaL_unref(L, LUA_REGISTRYINDEX, onMessageRef);

			LUA->Release(L);
		};

		if (p->IsUrlAllowed(args.url))
		{
			auto handle = p->WebSocket(args);

			void *vp = lua_newuserdata(L, sizeof(std::shared_ptr<WebSocketHandle>));
			new(vp) std::shared_ptr<::WebSocketHandle>(handle);
			luaL_getmetatable(L, "WebSocketHandle");
			lua_setmetatable(L, -2);
			return 1;
		}
		else
		{
			LOG->Warn("blocked access to %s", args.url.c_str());
			if (onMessageRef != LUA_NOREF)
			{
				handleWebSocketUrlForbidden(L, args.url, onMessageRef);
			}
			return 0;
		}
	}

	static int UrlEncode(T* p, lua_State *L)
	{
		std::string url = SArg(1);

		std::string encoded = p->UrlEncode(url);

		lua_pushstring(L, encoded.c_str());
		return 1;
	}

	static int EncodeQueryParameters(T* p, lua_State *L)
	{
		std::unordered_map<std::string, std::string> query;

		if (lua_istable(L, 1))
		{
			lua_pushnil(L);
			while(lua_next(L, -2) != 0)
			{
				if (!lua_isstring(L, -2))
				{
					luaL_error(L, "parameter keys must be strings");
				}

				if (!lua_isstring(L, -1))
				{
					luaL_error(L, "parameter values must be strings");
				}

				std::string key = lua_tostring(L, -2);
				std::string value = lua_tostring(L, -1);

				query[key] = value;

				lua_pop(L, 1);
			}
		}
		else
		{
			luaL_error(L, "query must be a table");
		}

		std::string encoded = p->EncodeQueryParameters(query);

		lua_pushstring(L, encoded.c_str());
		return 1;
	}

	LunaNetworkManager()
	{
		ADD_METHOD(IsUrlAllowed);
		ADD_METHOD(HttpRequest);
		ADD_METHOD(WebSocket);
		ADD_METHOD(UrlEncode);
		ADD_METHOD(EncodeQueryParameters);
	}

private:
	static void handleUrlForbidden(Lua *L, std::string& url, int onResponseRef)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, onResponseRef);
		luaL_unref(L, LUA_REGISTRYINDEX, onResponseRef);

		lua_newtable(L);

		LuaHelpers::Push(L, HttpErrorCode_Blocked);
		lua_setfield(L, -2, "error");

		lua_pushfstring(L, "access to %s is not allowed", url.c_str());
		lua_setfield(L, -2, "errorMessage");

		RString error = "Lua error in HTTP response handler: ";
		LuaHelpers::RunScriptOnStack(L, error, 1, 0, true);
	}

	static void handleFileError(Lua *L, const std::string& errorMessage, int onResponseRef)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, onResponseRef);
		luaL_unref(L, LUA_REGISTRYINDEX, onResponseRef);

		lua_newtable(L);

		LuaHelpers::Push(L, HttpErrorCode_FileError);
		lua_setfield(L, -2, "error");

		lua_pushstring(L, errorMessage.c_str());
		lua_setfield(L, -2, "errorMessage");

		RString error = "Lua error in HTTP response handler: ";
		LuaHelpers::RunScriptOnStack(L, error, 1, 0, true);
	}

	static void handleHttpResponse(Lua *L, const ix::HttpResponsePtr& response, int onResponseRef)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, onResponseRef);
		luaL_unref(L, LUA_REGISTRYINDEX, onResponseRef);

		lua_newtable(L);

		switch (response->errorCode)
		{
			case ix::HttpErrorCode::Ok:			lua_pushnil(L); break;
			case ix::HttpErrorCode::CannotConnect:		LuaHelpers::Push(L, HttpErrorCode_CannotConnect); break;
			case ix::HttpErrorCode::Timeout:		LuaHelpers::Push(L, HttpErrorCode_Timeout); break;
			case ix::HttpErrorCode::Gzip:			LuaHelpers::Push(L, HttpErrorCode_Gzip); break;
			case ix::HttpErrorCode::UrlMalformed:		LuaHelpers::Push(L, HttpErrorCode_UrlMalformed); break;
			case ix::HttpErrorCode::CannotCreateSocket:	LuaHelpers::Push(L, HttpErrorCode_CannotCreateSocket); break;
			case ix::HttpErrorCode::SendError:		LuaHelpers::Push(L, HttpErrorCode_SendError); break;
			case ix::HttpErrorCode::ReadError:		LuaHelpers::Push(L, HttpErrorCode_ReadError); break;
			case ix::HttpErrorCode::CannotReadStatusLine:	LuaHelpers::Push(L, HttpErrorCode_CannotReadStatusLine); break;
			case ix::HttpErrorCode::MissingStatus:		LuaHelpers::Push(L, HttpErrorCode_MissingStatus); break;
			case ix::HttpErrorCode::HeaderParsingError:	LuaHelpers::Push(L, HttpErrorCode_HeaderParsingError); break;
			case ix::HttpErrorCode::MissingLocation:	LuaHelpers::Push(L, HttpErrorCode_MissingLocation); break;
			case ix::HttpErrorCode::TooManyRedirects:	LuaHelpers::Push(L, HttpErrorCode_TooManyRedirects); break;
			case ix::HttpErrorCode::ChunkReadError:		LuaHelpers::Push(L, HttpErrorCode_ChunkReadError); break;
			case ix::HttpErrorCode::CannotReadBody:		LuaHelpers::Push(L, HttpErrorCode_CannotReadBody); break;
			case ix::HttpErrorCode::Cancelled:		LuaHelpers::Push(L, HttpErrorCode_Cancelled); break;
			default:					LuaHelpers::Push(L, HttpErrorCode_UnknownError); break;
		}
		lua_setfield(L, -2, "error");

		if (response->errorMsg == "")
		{
			lua_pushnil(L);
		}
		else
		{
			lua_pushstring(L, response->errorMsg.c_str());
		}
		lua_setfield(L, -2, "errorMessage");

		lua_pushinteger(L, response->statusCode);
		lua_setfield(L, -2, "statusCode");

		lua_newtable(L);
		for (const auto& entry : response->headers)
		{
			lua_pushstring(L, entry.second.c_str());
			lua_setfield(L, -2, entry.first.c_str());
		}
		lua_setfield(L, -2, "headers");

		lua_pushlstring(L, response->body.c_str(), response->body.length());
		lua_setfield(L, -2, "body");

		lua_pushnumber(L, response->uploadSize);
		lua_setfield(L, -2, "uploadSize");

		lua_pushnumber(L, response->downloadSize);
		lua_setfield(L, -2, "downloadSize");

		RString error = "Lua error in HTTP response handler: ";
		LuaHelpers::RunScriptOnStack(L, error, 1, 0, true);
	}

	static void handleProgress(Lua *L, int current, int total, int onProgressRef)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, onProgressRef);
		lua_pushinteger(L, current);
		lua_pushinteger(L, total);

		RString error = "Lua error in HTTP progress handler: ";
		LuaHelpers::RunScriptOnStack(L, error, 2, 0, true);
	}

	static void handleWebSocketUrlForbidden(Lua *L, std::string& url, int onMessageRef)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, onMessageRef);
		luaL_unref(L, LUA_REGISTRYINDEX, onMessageRef);

		lua_newtable(L);

		LuaHelpers::Push(L, WebSocketMessageType_Error);
		lua_setfield(L, -2, "type");

		lua_pushfstring(L, "access to %s is not allowed", url.c_str());
		lua_setfield(L, -2, "reason");

		RString error = "Lua error in WebSocket message handler: ";
		LuaHelpers::RunScriptOnStack(L, error, 1, 0, true);
	}

	static void handleMessage(Lua *L, const ix::WebSocketMessagePtr& msg, int onMessageRef)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, onMessageRef);

		lua_newtable(L);

		switch (msg->type)
		{
			case ix::WebSocketMessageType::Message:
				LuaHelpers::Push(L, WebSocketMessageType_Message);
				lua_setfield(L, -2, "type");

				lua_pushlstring(L, msg->str.c_str(), msg->str.length());
				lua_setfield(L, -2, "data");

				lua_pushboolean(L, msg->binary);
				lua_setfield(L, -2, "binary");
				break;
			case ix::WebSocketMessageType::Open:
				LuaHelpers::Push(L, WebSocketMessageType_Open);
				lua_setfield(L, -2, "type");

				lua_pushstring(L, msg->openInfo.uri.c_str());
				lua_setfield(L, -2, "uri");

				lua_newtable(L);
				for (const auto& entry : msg->openInfo.headers)
				{
					lua_pushstring(L, entry.second.c_str());
					lua_setfield(L, -2, entry.first.c_str());
				}
				lua_setfield(L, -2, "headers");

				lua_pushstring(L, msg->openInfo.protocol.c_str());
				lua_setfield(L, -2, "protocol");
				break;
			case ix::WebSocketMessageType::Close:
				LuaHelpers::Push(L, WebSocketMessageType_Close);
				lua_setfield(L, -2, "type");

				lua_pushstring(L, msg->closeInfo.reason.c_str());
				lua_setfield(L, -2, "reason");

				lua_pushboolean(L, msg->closeInfo.remote);
				lua_setfield(L, -2, "remote");
				break;
			case ix::WebSocketMessageType::Error:
				LuaHelpers::Push(L, WebSocketMessageType_Error);
				lua_setfield(L, -2, "type");

				lua_pushinteger(L, msg->errorInfo.retries);
				lua_setfield(L, -2, "retries");

				lua_pushnumber(L, msg->errorInfo.wait_time);
				lua_setfield(L, -2, "waitTime");

				if (msg->errorInfo.http_status > 0)
				{
					lua_pushinteger(L, msg->errorInfo.http_status);
				}
				else
				{
					lua_pushnil(L);
				}
				lua_setfield(L, -2, "httpStatusCode");

				lua_pushstring(L, msg->errorInfo.reason.c_str());
				lua_setfield(L, -2, "reason");

				lua_pushboolean(L, msg->errorInfo.decompressionError);
				lua_setfield(L, -2, "decompressionError");
				break;
			case ix::WebSocketMessageType::Ping:
			case ix::WebSocketMessageType::Pong:
			case ix::WebSocketMessageType::Fragment:
			default:
				lua_pop(L, 2);
				return;
		}

		RString error = "Lua error in WebSocket message handler: ";
		LuaHelpers::RunScriptOnStack(L, error, 1, 0, true);
	}

};

LUA_REGISTER_CLASS(NetworkManager)
// lua end

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
