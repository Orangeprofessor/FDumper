#include "pch.h"

#include "config.h"

#include "../contrib/curl/curl.h"
#include "fADumper.h"
#include "dumphandler.h"

#include "FDumper.h"


int DumpNotifier(const wchar_t* path, void* context, PEXCEPTION_POINTERS expt, bool success)
{
	log_console(xlog::LogLevel::fatal, "Unhandled exception 0x%08x at 0x%08x", expt->ExceptionRecord->ExceptionCode, expt->ExceptionRecord->ExceptionAddress);
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


int APIENTRY wWinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, LPWSTR lpCmdLine, int /*nCmdShow*/)
{
	GetOSData();

	wchar_t temp[MAX_PATH] = {};
	GetModuleFileNameW(NULL, temp, MAX_PATH);

	std::wstring exepath(temp);
	exepath = exepath.substr(0, exepath.find_last_of(L"\\/")); exepath.append(L"\\");

	dump::DumpHandler::Instance().CreateWatchdog(exepath, dump::CreateFullDump, &DumpNotifier);

	AllocConsole();

	FILE* fp = nullptr;
	freopen_s(&fp, "CONIN$", "r", stdin);
	freopen_s(&fp, "CONOUT$", "w", stdout);
	freopen_s(&fp, "CONOUT$", "w", stderr);

	xlog::Critical("Starting cURL instance...");
	if (auto code = curl_global_init(CURL_GLOBAL_ALL)) 
	{
		log_console(xlog::LogLevel::fatal, "Failed to instance libcurl! CURLcode: %d! Quiting...\n", code);
		return 0;
	}
	xlog::Critical("libcurl successfully instanced!");

	log_console(xlog::LogLevel::critical, "FDumper v2.1.1 by Orangeprofessor!\n");
	log_console(xlog::LogLevel::normal, "See the readme for more info\n");

	FDumper st;
	st.Start(lpCmdLine);

	FreeConsole();

	curl_global_cleanup();

	return 0;
}