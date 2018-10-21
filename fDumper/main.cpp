#include "pch.h"

#include "curl/curl.h"
#include "fADumper.h"
#include "dumphandler.h"

#include <winternl.h>

void DestroyCmdLine()
{
	srand(static_cast<unsigned int>(__rdtsc()));
	wchar_t* pszCmdLine = ::GetCommandLineW();
	size_t cbLen = wcslen(pszCmdLine);
	size_t cbCutOff = static_cast<size_t>(rand()) % cbLen;
	for (size_t i = 0; i < cbLen; ++i) {
		pszCmdLine[i] = (i == cbCutOff) ? 0 : pszCmdLine[i] ^ rand();
	}
}

int DumpNotifier(const wchar_t* path, void* context, EXCEPTION_POINTERS* expt, bool success)
{
	std::wstring errmsg = L"Oopsie woopsie a widdle fuckie wuckie! Dump file saved at '" + std::wstring(path) + L"'";
	std::wstring title = L"Ohhhhhhhhhhhhh Nooooooooo";
	MessageBoxW(GetActiveWindow(), errmsg.c_str(), title.c_str(), MB_OK | MB_ICONERROR);
	return 0;
}


void mainDump(const int argc, const char* argv[]);
void mainUpdate(const int argc, const char* argv[]);

void prtdesc()
{
	console_print("FDumper-cli v1 by Orangeprofessor!\n");
	console_print("Usage: FDumper-cli.exe <mode> [args]\n");
	console_print("See fdumper.txt for more details\n");
}

void loginfo()
{
	uintptr_t peb = (uintptr_t)NtCurrentTeb()->ProcessEnvironmentBlock;

	int majorver = *reinterpret_cast<int*>(peb + 0x118);
	int minorver = *reinterpret_cast<int*>(peb + 0x11c);
	int oscsd = *reinterpret_cast<int*>(peb + 0x122);
	int buildno = *reinterpret_cast<int*>(peb + 0x120);

	xlog::Normal(
		"Started on Windows %d.%d.%d.%d",
		majorver,
		minorver,
		(oscsd >> 8) & 0xFF,
		buildno
	);
}

int main(const int argc, const char* argv[])
{
	loginfo();

	DestroyCmdLine();

	wchar_t exepath[MAX_PATH] = {};
	GetModuleFileNameW(NULL, exepath, MAX_PATH);

	dump::DumpHandler::Instance().CreateWatchdog(exepath, dump::CreateFullDump, &DumpNotifier);

	HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);

	CONSOLE_CURSOR_INFO cursorInfo;
	GetConsoleCursorInfo(out, &cursorInfo);
	cursorInfo.bVisible = false;
	SetConsoleCursorInfo(out, &cursorInfo);

	if (argc < 2)
	{
		prtdesc();
		return 0;
	}

	curl_global_init(CURL_GLOBAL_ALL);

	std::string mode = argv[1];

	if (!mode.compare("dump"))
	{
		mainDump(argc, argv);
	}
	else if (!mode.compare("update"))
	{
		mainUpdate(argc, argv);
	}
	else
	{
		prtdesc();
	}

	curl_global_cleanup();

	GetConsoleCursorInfo(out, &cursorInfo);
	cursorInfo.bVisible = true;
	SetConsoleCursorInfo(out, &cursorInfo);

	return 0;
}