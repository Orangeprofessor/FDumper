#include "pch.h"

#include "curl/curl.h"
#include "fADumper.h"

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

void mainDump(const int argc, const char* argv[]);
void mainUpdate(const int argc, const char* argv[]);

int main(const int argc, const char* argv[])
{
	DestroyCmdLine();

	HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);

	CONSOLE_CURSOR_INFO cursorInfo;
	GetConsoleCursorInfo(out, &cursorInfo);
	cursorInfo.bVisible = false;
	SetConsoleCursorInfo(out, &cursorInfo);

	if (argc < 2)
	{
		console_print("FDumper-cli v1 by Orangeprofessor!\n");
		console_print("Usage: FDumper-cli.exe <mode> [args]\n");
		console_print("See fdumper.txt for more details\n");
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

	curl_global_cleanup();

	return 0;
}