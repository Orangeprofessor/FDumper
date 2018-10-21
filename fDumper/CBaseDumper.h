#pragma once

#include "pch.h"

#include "Log.h"

#include "curl/curl.h"
#include "rapidjson/document.h"

struct arg_t
{
	const int c;
	const char** v;
	int i;
};

class CBaseDumper
{
public:
	virtual ~CBaseDumper() {}

	void Main(const int argc, const char* argv[]);
	virtual void Process(arg_t& arg);

	virtual void PrintDescription() = 0;

	bool ReadArgs(arg_t& arg);
	virtual bool Argument(arg_t& arg);

	virtual void Action(arg_t& arg) = 0;

public:
	const std::wstring m_path;
	std::string m_api;
};

static size_t writebuffercallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

static size_t writefilecallback(void* contents, size_t size, size_t nmemb, FILE* fp)
{
	return fwrite(contents, size, nmemb, fp);
}

static std::wstring AnsiToWstring(const std::string& input, DWORD locale = CP_ACP)
{
	wchar_t buf[2048] = { 0 };
	MultiByteToWideChar(locale, 0, input.c_str(), (int)input.length(), buf, ARRAYSIZE(buf));
	return buf;
}
static std::string WstringToAnsi(const std::wstring& input, DWORD locale = CP_ACP)
{
	char buf[2048] = { 0 };
	WideCharToMultiByte(locale, 0, input.c_str(), (int)input.length(), buf, ARRAYSIZE(buf), nullptr, nullptr);
	return buf;
}

static void console_print(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	char buff[1024] = {};
	vsprintf_s(buff, format, args);
	va_end(args);

	xlog::Verbose(buff);

	std::printf("[FDumper] ");
	std::printf(buff);
}

static void console_error(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	char buff[1024] = {};
	vsprintf_s(buff, format, args);
	va_end(args);

	xlog::Error(buff);

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 4);
	std::printf("[FDumper] ");
	std::printf(buff);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
}