#pragma once


#if defined(_MSC_VER)

#define CURL_STATICLIB

#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "comctl32.lib")

#if defined(_DEBUG)

#pragma comment(lib, "../contrib/curl/lib/libcurl_a_debug.lib")

#else

#pragma comment(lib, "../contrib/curl/lib/libcurl_a.lib")

#endif

#else
#error "Unsupported compiler"
#endif