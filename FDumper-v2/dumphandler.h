#pragma once

#include "stdafx.h"

// idk where tf i pasted this from but it work so

namespace dump
{
	/// <summary>
	/// Dump flags
	/// </summary>
	enum eDumpFlags
	{
		NoFlags = 0,
		CreateMinidump = 1,    // Create mini dump
		CreateFullDump = 2,    // Create full memory dump

		CreateBoth = CreateMinidump | CreateFullDump
	};

	/// <summary>
	/// Crash dump handler
	/// </summary>
	class DumpHandler
	{
	public:
		typedef int(*fnCallback)(const wchar_t*, void*, EXCEPTION_POINTERS*, bool);

	public:
		~DumpHandler(void);

		static DumpHandler& Instance();

		bool CreateWatchdog(
			const std::wstring& dumpRoot,
			eDumpFlags flags,
			fnCallback pDump = nullptr,
			fnCallback pFilter = nullptr,
			void* pContext = nullptr
		);

		void DisableWatchdog();

	private:
		DumpHandler(void);
		static LONG CALLBACK UnhandledExceptionFilter(PEXCEPTION_POINTERS ExceptionInfo);
		void CreateDump(const std::wstring& strFilePath, int flags);
		static DWORD CALLBACK WatchdogProc(LPVOID lpParam);
		void ExceptionHandler();
		bool GenDumpFilenames(std::wstring& strFullDump, std::wstring& strMiniDump);

	private:
		void* _pPrevFilter = nullptr;                   // Previous unhanded exception filter
		bool _active = true;                            // Watchdog activity flag
		eDumpFlags _flags = NoFlags;                    // Dump creation flags
		DWORD _FailedThread = 0;                        // Crashed thread ID
		HANDLE _hWatchThd = NULL;                       // Watchdog thread handle
		PEXCEPTION_POINTERS _ExceptionInfo = nullptr;   // Crash exception info
		void* _pUserContext = nullptr;                  // User context for callback
		std::wstring _dumpRoot = L".";                  // Root folder for dump files
		fnCallback _pDumpProc = nullptr;                // Crash callback
	};
}

