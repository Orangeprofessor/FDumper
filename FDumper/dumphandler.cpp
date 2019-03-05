#include "pch.h"

#include "DumpHandler.h"

#pragma warning(disable : 4091)
#include <DbgHelp.h>
#pragma warning(default : 4091)

namespace dump
{

	DumpHandler::DumpHandler(void)
	{
		memset(&_ExceptionInfo, 0x00, sizeof(_ExceptionInfo));

		// Create new thread to ensure dump creation in case of stack overflow exception
		_hWatchThd = CreateThread(NULL, 0, &DumpHandler::WatchdogProc, this, 0, NULL);
	}

	DumpHandler::~DumpHandler(void)
	{
		DisableWatchdog();

		_active = false;
		if (_hWatchThd)
		{
			WaitForSingleObject(_hWatchThd, INFINITE);
			_hWatchThd = NULL;
		}
	}

	DumpHandler& DumpHandler::Instance()
	{
		static DumpHandler instance;
		return instance;
	}

	bool DumpHandler::CreateWatchdog(
		const std::wstring& dumpRoot,
		eDumpFlags flags,
		fnCallback pDump /*= nullptr*/,
		fnCallback pFilter /*= nullptr*/,
		void* pContext /*= nullptr*/
	)
	{
		// Prevent multiple assignment
		if (_pPrevFilter == nullptr)
		{
			_flags = flags;
			_pUserContext = pContext;
			_dumpRoot = dumpRoot;
			_pDumpProc = pDump;

			_pPrevFilter = SetUnhandledExceptionFilter(&DumpHandler::UnhandledExceptionFilter);

			return true;
		}

		return false;
	}

	void DumpHandler::DisableWatchdog()
	{
		if (_pPrevFilter)
		{
			SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)_pPrevFilter);
			_pPrevFilter = nullptr;
		}
	}

	LONG CALLBACK DumpHandler::UnhandledExceptionFilter(PEXCEPTION_POINTERS ExceptionInfo)
	{
		auto& inst = Instance();

		inst._ExceptionInfo = ExceptionInfo;
		inst._FailedThread = GetCurrentThreadId();

		if (Instance()._hWatchThd == NULL)
			ExitProcess(0x1338);

		// Wait for dump generation reasonable amount of time (3 min)
		WaitForSingleObject(inst._hWatchThd, 3 * 60 * 1000);

		return EXCEPTION_CONTINUE_SEARCH;
	}

	DWORD CALLBACK DumpHandler::WatchdogProc(LPVOID lpParam)
	{
		DumpHandler* pClass = reinterpret_cast<DumpHandler*>(lpParam);

		// Wait for crashed thread id
		while (pClass && pClass->_FailedThread == 0)
		{
			if (pClass->_active)
				Sleep(5);
			else
				return ERROR_SUCCESS;
		}

		if (pClass)
			pClass->ExceptionHandler();

		return ERROR_SUCCESS;
	}

	void DumpHandler::ExceptionHandler()
	{
		std::wstring strFullDump, strMiniDump;

		int dumpFlags = MiniDumpWithIndirectlyReferencedMemory |
			MiniDumpWithDataSegs |
			MiniDumpWithHandleData |
			MiniDumpWithFullMemory;

		GenDumpFilenames(strFullDump, strMiniDump);

		// Create full memory dump
		if (_flags & CreateFullDump)
			CreateDump(strFullDump, dumpFlags);

		// Create mini dump
		if (_flags & CreateMinidump)
			CreateDump(strMiniDump, MiniDumpNormal);

		// Call user routine
		if (_pDumpProc)
			_pDumpProc(strFullDump.c_str(), _pUserContext, _ExceptionInfo, true);

		// Prevent dtor deadlock
		_hWatchThd = NULL;
		ExitProcess(0x1337);
	}


	void DumpHandler::CreateDump(const std::wstring& strFilePath, int flags)
	{
		MINIDUMP_EXCEPTION_INFORMATION aMiniDumpInfo = { 0 };

		HANDLE hFile = CreateFileW(strFilePath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);

		aMiniDumpInfo.ThreadId = _FailedThread;
		aMiniDumpInfo.ExceptionPointers = _ExceptionInfo;
		aMiniDumpInfo.ClientPointers = TRUE;

		if (hFile != INVALID_HANDLE_VALUE)
		{
			MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, (MINIDUMP_TYPE)flags, &aMiniDumpInfo, NULL, NULL);

			FlushFileBuffers(hFile);
			CloseHandle(hFile);
		}
	}

	bool DumpHandler::GenDumpFilenames(std::wstring& strFullDump, std::wstring& strMiniDump)
	{
		wchar_t tFullName[MAX_PATH] = { 0 };
		wchar_t tMiniName[MAX_PATH] = { 0 };
		wchar_t tImageName[MAX_PATH] = { 0 };

		SYSTEMTIME time = { 0 };

		// Current time
		GetLocalTime(&time);

		// Image name
		GetModuleFileNameW(NULL, tImageName, ARRAYSIZE(tImageName));
		std::wstring modulename(tImageName);
		modulename = modulename.substr(modulename.rfind(L"\\") + 1);

		// File name based on current time
		swprintf_s(tFullName, sizeof(tFullName) / sizeof(wchar_t), L"%s\\%s_%d.%.2d.%.2d.%.2d.%.2d.%.2d.dmp",
			_dumpRoot.c_str(), modulename.c_str(), time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);

		swprintf_s(tMiniName, sizeof(tFullName) / sizeof(wchar_t), L"%s\\%s_%d.%.2d.%.2d.%.2d.%.2d.%.2d.mdmp",
			_dumpRoot.c_str(), modulename.c_str(), time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);

		strFullDump = tFullName;
		strMiniDump = tMiniName;

		return true;
	}
}
