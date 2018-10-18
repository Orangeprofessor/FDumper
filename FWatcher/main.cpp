#include "pch.h"
#include "watcher.h"
#include "toaster.hpp"

#include "curl/curl.h"


__declspec(dllexport) CWatcher* __stdcall InitWatcher(const std::wstring& path, const std::string& api)
{
	// init the toaster 
	using namespace WinToastLib;

	WinToast::instance()->setAppName(L"FDumper");
	WinToast::instance()->setAppUserModelId(L"FDumper");

	if (!WinToast::instance()->initialize()) {
		return nullptr;
	}

	const_cast<std::string&>(g_API) = api;

	auto watcher = new CWatcher(path);
	if (!watcher->SetupWatchList(path)) {
		delete watcher;
		return nullptr;
	}

	return watcher;
}

__declspec(dllexport) void __stdcall ShutdownWatcher(CWatcher* pWatcher)
{
	delete pWatcher;
}

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID reserved)
{
	return TRUE;
}

