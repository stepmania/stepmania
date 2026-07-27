set(IXW_DIR "IXWebSocket/ixwebsocket")

set(IXW_SRC "${IXW_DIR}/IXBench.cpp"
            "${IXW_DIR}/IXCancellationRequest.cpp"
            "${IXW_DIR}/IXConnectionState.cpp"
            "${IXW_DIR}/IXDNSLookup.cpp"
            "${IXW_DIR}/IXExponentialBackoff.cpp"
            "${IXW_DIR}/IXGetFreePort.cpp"
            "${IXW_DIR}/IXGzipCodec.cpp"
            "${IXW_DIR}/IXHttp.cpp"
            "${IXW_DIR}/IXHttpClient.cpp"
            "${IXW_DIR}/IXHttpServer.cpp"
            "${IXW_DIR}/IXNetSystem.cpp"
            "${IXW_DIR}/IXSelectInterrupt.cpp"
            "${IXW_DIR}/IXSelectInterruptEvent.cpp"
            "${IXW_DIR}/IXSelectInterruptFactory.cpp"
            "${IXW_DIR}/IXSelectInterruptPipe.cpp"
            "${IXW_DIR}/IXSetThreadName.cpp"
            "${IXW_DIR}/IXSocket.cpp"
            "${IXW_DIR}/IXSocketConnect.cpp"
            "${IXW_DIR}/IXSocketFactory.cpp"
            "${IXW_DIR}/IXSocketServer.cpp"
            "${IXW_DIR}/IXSocketTLSOptions.cpp"
            "${IXW_DIR}/IXStrCaseCompare.cpp"
            "${IXW_DIR}/IXUdpSocket.cpp"
            "${IXW_DIR}/IXUrlParser.cpp"
            "${IXW_DIR}/IXUuid.cpp"
            "${IXW_DIR}/IXUserAgent.cpp"
            "${IXW_DIR}/IXWebSocket.cpp"
            "${IXW_DIR}/IXWebSocketCloseConstants.cpp"
            "${IXW_DIR}/IXWebSocketHandshake.cpp"
            "${IXW_DIR}/IXWebSocketHttpHeaders.cpp"
            "${IXW_DIR}/IXWebSocketPerMessageDeflate.cpp"
            "${IXW_DIR}/IXWebSocketPerMessageDeflateCodec.cpp"
            "${IXW_DIR}/IXWebSocketPerMessageDeflateOptions.cpp"
            "${IXW_DIR}/IXWebSocketProxyServer.cpp"
            "${IXW_DIR}/IXWebSocketServer.cpp"
            "${IXW_DIR}/IXWebSocketTransport.cpp")

set(IXW_HPP "${IXW_DIR}/IXBench.h"
            "${IXW_DIR}/IXCancellationRequest.h"
            "${IXW_DIR}/IXConnectionState.h"
            "${IXW_DIR}/IXDNSLookup.h"
            "${IXW_DIR}/IXExponentialBackoff.h"
            "${IXW_DIR}/IXGetFreePort.h"
            "${IXW_DIR}/IXGzipCodec.h"
            "${IXW_DIR}/IXHttp.h"
            "${IXW_DIR}/IXHttpClient.h"
            "${IXW_DIR}/IXHttpServer.h"
            "${IXW_DIR}/IXNetSystem.h"
            "${IXW_DIR}/IXProgressCallback.h"
            "${IXW_DIR}/IXSelectInterrupt.h"
            "${IXW_DIR}/IXSelectInterruptEvent.h"
            "${IXW_DIR}/IXSelectInterruptFactory.h"
            "${IXW_DIR}/IXSelectInterruptPipe.h"
            "${IXW_DIR}/IXSetThreadName.h"
            "${IXW_DIR}/IXSocket.h"
            "${IXW_DIR}/IXSocketConnect.h"
            "${IXW_DIR}/IXSocketFactory.h"
            "${IXW_DIR}/IXSocketServer.h"
            "${IXW_DIR}/IXSocketTLSOptions.h"
            "${IXW_DIR}/IXStrCaseCompare.h"
            "${IXW_DIR}/IXUdpSocket.h"
            "${IXW_DIR}/IXUniquePtr.h"
            "${IXW_DIR}/IXUrlParser.h"
            "${IXW_DIR}/IXUuid.h"
            "${IXW_DIR}/IXUtf8Validator.h"
            "${IXW_DIR}/IXUserAgent.h"
            "${IXW_DIR}/IXWebSocket.h"
            "${IXW_DIR}/IXWebSocketCloseConstants.h"
            "${IXW_DIR}/IXWebSocketCloseInfo.h"
            "${IXW_DIR}/IXWebSocketErrorInfo.h"
            "${IXW_DIR}/IXWebSocketHandshake.h"
            "${IXW_DIR}/IXWebSocketHandshakeKeyGen.h"
            "${IXW_DIR}/IXWebSocketHttpHeaders.h"
            "${IXW_DIR}/IXWebSocketInitResult.h"
            "${IXW_DIR}/IXWebSocketMessage.h"
            "${IXW_DIR}/IXWebSocketMessageType.h"
            "${IXW_DIR}/IXWebSocketOpenInfo.h"
            "${IXW_DIR}/IXWebSocketPerMessageDeflate.h"
            "${IXW_DIR}/IXWebSocketPerMessageDeflateCodec.h"
            "${IXW_DIR}/IXWebSocketPerMessageDeflateOptions.h"
            "${IXW_DIR}/IXWebSocketProxyServer.h"
            "${IXW_DIR}/IXWebSocketSendInfo.h"
            "${IXW_DIR}/IXWebSocketServer.h"
            "${IXW_DIR}/IXWebSocketTransport.h"
            "${IXW_DIR}/IXWebSocketVersion.h")

list(APPEND IXW_SRC "${IXW_DIR}/IXSocketMbedTLS.cpp")
list(APPEND IXW_HPP "${IXW_DIR}/IXSocketMbedTLS.h")

source_group("" FILES ${IXW_SRC} ${IXW_HPP})

add_library("ixwebsocket" STATIC ${IXW_SRC} ${IXW_HPP})

set_property(TARGET "ixwebsocket" PROPERTY FOLDER "External Libraries")
set_property(TARGET "ixwebsocket" PROPERTY CXX_STANDARD 11)
set_property(TARGET "ixwebsocket" PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET "ixwebsocket" PROPERTY CXX_EXTENSIONS OFF)

disable_project_warnings("ixwebsocket")

target_compile_definitions("ixwebsocket" PRIVATE IXWEBSOCKET_USE_TLS)
target_compile_definitions("ixwebsocket" PRIVATE IXWEBSOCKET_USE_MBED_TLS)
target_compile_definitions("ixwebsocket" PRIVATE IXWEBSOCKET_USE_MBED_TLS_MIN_VERSION_3)
set(ENABLE_TESTING OFF CACHE INTERNAL "Don't build tests")
set(GEN_FILES OFF CACHE INTERNAL "Don't generate files (requires perl and python)")
add_subdirectory("mbedtls" EXCLUDE_FROM_ALL)
set_property(TARGET "mbedtls" PROPERTY FOLDER "External Libraries")
set_property(TARGET "mbedcrypto" PROPERTY FOLDER "External Libraries")
set_property(TARGET "mbedx509" PROPERTY FOLDER "External Libraries")
target_link_libraries("ixwebsocket" mbedtls mbedcrypto mbedx509)

target_compile_definitions("ixwebsocket" PRIVATE IXWEBSOCKET_USE_ZLIB)

if(WIN32)
  target_link_libraries(ixwebsocket wsock32 ws2_32 shlwapi Crypt32)
  target_compile_definitions(ixwebsocket PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

if(UNIX)
  find_package(Threads)
  target_link_libraries(ixwebsocket ${CMAKE_THREAD_LIBS_INIT})
endif()

target_include_directories(ixwebsocket PUBLIC
  "zlib"
  "IXWebSocket"
)
