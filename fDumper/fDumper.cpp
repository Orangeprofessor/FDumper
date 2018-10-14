// fDumper.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

#include "curl/curl.h"

#include "fDumper.h"
#include "Menu.h"

ctpl::thread_pool g_threads = ctpl::thread_pool(10);

int APIENTRY wWinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, LPWSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
	curl_global_init(CURL_GLOBAL_ALL);

	CMainDialog dlg;
	dlg.RunModeless();

	g_threads.kill();

	curl_global_cleanup();

	return 0;
}

