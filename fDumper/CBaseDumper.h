#pragma once

#include "pch.h"
#include "config.h"

#include "Log.h"

#include "../contrib/curl/curl.h"
#include "../contrib/rapidjson/document.h"

#include <mutex>

struct arg_t
{
	const int c;
	wchar_t** v;
	int i;
};

class CBaseDumper
{
public:
	virtual ~CBaseDumper() {}

	void Main(const int argc, wchar_t* argv[]);
	virtual void Process(arg_t& arg);

	virtual void PrintDescription() = 0;

	bool ReadArgs(arg_t& arg);
	virtual bool Argument(arg_t& arg);

	virtual void Action(arg_t& arg) = 0;
	bool ValidUser(std::string user);

public:
	const std::wstring m_path;
	std::string m_api;
};

template<typename T>
class ThreadLock
{
public:
	ThreadLock& operator=(const T& t) {
		std::lock_guard<std::mutex> lock(m_mutex);
		m_protected = t;
		return *this;
	}
	operator T() const {
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_protected;
	}
	void execute(std::function<void(T)> callback) {
		assert(callback != nullptr);
		std::lock_guard<std::mutex> lock(m_mutex);
		callback(m_protected);
	}
private:
	mutable std::mutex m_mutex;
	T m_protected{};
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

static void log_console(xlog::LogLevel::e level, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	xlog::Logger::Instance().DoLogV(level, format, args);
	va_end(args);

	va_start(args, format);
	char buff[1024] = {};
	vsprintf_s(buff, format, args);
	va_end(args);
	
	static const unsigned int colors[] = {
		4, 12, 2, 6, 7, 7 
	};

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), colors[level]);
	std::printf(buff);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
}
