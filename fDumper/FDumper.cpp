#include "pch.h"

#include "config.h"

#include "../contrib/curl/curl.h"
#include "dumphandler.h"

#include "CBaseDumper.h"
#include "MainDlg.h"

#include <gdiplus.h>

int DumpNotifier(const wchar_t* path, void* context, PEXCEPTION_POINTERS expt, bool success)
{
	xlog::Fatal("Unhandled exception 0x%08x at 0x%08x", expt->ExceptionRecord->ExceptionCode, expt->ExceptionRecord->ExceptionAddress);
	std::wstring errmsg = L"Oopsie woopsie a widdle fuckie wuckie! Dump file saved at '" + std::wstring(path) + L"'";
	std::wstring title = L"Yikes! That wasn't supposed to happen!";
	MessageBoxW(GetActiveWindow(), errmsg.c_str(), title.c_str(), MB_OK | MB_ICONERROR);

	return 0;
}

void GetOSData()
{
	wchar_t winverbuff[100] = {};
	DWORD dwsize = 100;

	RegGetValue(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows NT\\CurrentVersion",
		L"BuildLabEx", RRF_RT_REG_SZ, NULL, winverbuff, &dwsize);

	xlog::Critical("Started on Windows %s", WstringToAnsi(winverbuff).c_str());
}

int APIENTRY wWinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, LPWSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
	wchar_t temp[MAX_PATH] = {};
	GetTempPath(MAX_PATH, temp);

	std::wstring workingpath(temp);
	workingpath.append(L"FDumper\\");

	std::filesystem::create_directory(workingpath);

	GetOSData();

	dump::DumpHandler::Instance().CreateWatchdog(workingpath, dump::CreateFullDump, &DumpNotifier);

	std::filesystem::create_directory(workingpath + L"thumbs");

	curl_global_init(CURL_GLOBAL_ALL);

	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	MainDlg dlg;
	dlg.RunModeless();

	Gdiplus::GdiplusShutdown(gdiplusToken);

	curl_global_cleanup();

	return 0;
}