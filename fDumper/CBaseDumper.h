#pragma once

#include "pch.h"
#include "config.h"

#include "Log.h"

#include "configmgr.h"

#include "../contrib/curl/curl.h"
#include "../contrib/rapidjson/document.h"

#include <mutex>


enum faRatingFlags : int
{
	ALL_RATINGS = 0,
	NSFW_ONLY,
	SFW_ONLY
};

enum faGalleryFlags : int
{
	MAIN = (1 << 0),
	SCRAPS = (1 << 1),
	FAVORITES = (1 << 2)
};

struct DownloadContext
{
	DownloadContext() {
		memset(this, 0, sizeof(*this));
	}

	int gallery;
	faRatingFlags ratings;
	std::string	username;
	const std::wstring path;
};

#define MSG_SETCOLOR (WM_USER + 0x420)
#define MSG_LOADTHUMBNAILS (WM_USER + 0x666)
#define MSG_ADDTODOWNLOADLIST (WM_USER + 0x69)

struct cColors {
	COLORREF cf;
	int id;
};

struct faData {
	int id;
	std::string title;
	std::string thumbnailURL;
};

struct thumbnailData {
	std::wstring workingdir;
	std::vector<faData> data;
};

struct downloadListData {
	int index;
	FASubmission submission;
};

class CBaseDumper
{
public:
	CBaseDumper(ConfigMgr& config) : m_config(config) {}
	virtual ~CBaseDumper() {}

	void Main(const DownloadContext& ctx);
	virtual int Action(const DownloadContext& ctx) = 0;

	bool ValidUser(std::string user);
	std::pair<int, int> ParseGalleryCfg(const std::wstring& path);
	void CreateGalleryCfg(const std::wstring& path, int rating, int gallery);

public:
	ConfigMgr& m_config;
};

template<typename T>
class ThreadLock
{
public:
	ThreadLock() {}

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
	T GetType() const {
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_protected;
	}

private:
	ThreadLock(const ThreadLock&) = delete;
	ThreadLock& operator=(ThreadLock const&) = delete;
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