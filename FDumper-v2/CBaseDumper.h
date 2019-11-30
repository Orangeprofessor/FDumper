#pragma once

#include "stdafx.h"
#include "config.h"

#include "Log.h"
#include "configmgr.h"

#include "../contrib/curl/curl.h"
#include "../contrib/rapidjson/document.h"

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
	DownloadContext() {}

	int gallery;
	faRatingFlags ratings;
	std::string	username;
	const std::filesystem::path path;
};


class curl_ptr
{
	curl_ptr(const curl_ptr&) = delete;
	curl_ptr& operator=(curl_ptr const&) = delete;

public:
	curl_ptr() : m_curl(curl_easy_init(), curl_easy_cleanup) {}

	inline void SetUrl(const std::string& url) {
		SetOpt(CURLOPT_URL, url.c_str());
	}

	inline std::string GetError(CURLcode errcode) {
		return curl_easy_strerror(errcode);
	}

	inline int GetHTTPCode() {
		return m_httpcode;
	}

	CURLcode DownloadToBuffer(std::string& buffer)
	{
		SetOpt(CURLOPT_WRITEFUNCTION, writebuffer);
		SetOpt(CURLOPT_WRITEDATA, &buffer);

		CURLcode code = curl_easy_perform(m_curl.get());
		curl_easy_getinfo(m_curl.get(), CURLINFO_RESPONSE_CODE, &m_httpcode);

		return code;
	}

	CURLcode DownloadToDisk(const std::wstring& dir)
	{
		FILE* fp = nullptr;
		_wfopen_s(&fp, dir.c_str(), L"wb");

		if (!fp)
			return CURLE_BAD_FUNCTION_ARGUMENT;

		SetOpt(CURLOPT_WRITEFUNCTION, writefile);
		SetOpt(CURLOPT_WRITEDATA, fp);

		CURLcode code = curl_easy_perform(m_curl.get());
		curl_easy_getinfo(m_curl.get(), CURLINFO_RESPONSE_CODE, &m_httpcode);

		return std::fclose(fp), code;
	}

	template<typename T>
	inline CURLcode SetOpt(CURLoption option, T type) {
		return curl_easy_setopt(m_curl.get(), option, type);
	}

private:
	static size_t writebuffer(void* contents, size_t size, size_t nmemb, void* userp) {
		((std::string*)userp)->append((char*)contents, size * nmemb);
		return size * nmemb;
	}

	static size_t writefile(void* contents, size_t size, size_t nmemb, FILE* fp) {
		return fwrite(contents, size, nmemb, fp);
	}

private:
	std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> m_curl;
	int m_httpcode;
};


struct FASubmission
{
	FASubmission(int submission) : submissionID(submission) {}

	void Setup(std::string api)
	{
		while (true)
		{
			char urlbuff[200] = {};
			sprintf_s(urlbuff, "%s/submission/%d.json", api.c_str(), submissionID);

			curl_ptr curl;
			std::string buffer;
			int httpcode = 0;

			curl.SetUrl(urlbuff);

			if (auto err = curl.DownloadToBuffer(buffer)) {
				xlog::Warning("Download error! %s!", curl.GetError(err).c_str());
				continue;
			}

			if (int code = curl.GetHTTPCode() != 200)
			{
				xlog::Warning("Download error! http code %d", code);
				continue;
			}

			rapidjson::Document doc;
			doc.Parse(buffer.c_str());

			auto fullimage = doc.FindMember("download");
			downloadURL = fullimage->value.GetString();

			auto imagetitle = doc.FindMember("title");
			title = imagetitle->value.GetString();

			auto imagerating = doc.FindMember("rating");
			imagerating->value.GetString() == std::string("General") ? rating = 1 : rating = 2;
			ratingstr = imagerating->value.GetString();

			auto dateposted = doc.FindMember("posted");
			date = dateposted->value.GetString();

			auto res = doc.FindMember("resolution");
			resolution = res->value.GetString();

			break;
		}
	}

	inline std::string GetDownloadLink() { return downloadURL; }
	inline int GetSubmissionID() { return submissionID; }
	inline std::string GetSubmissionTitle() { return title; }
	inline int GetRating() { return rating; }
	inline std::string GetResolution() { return resolution; }
	inline std::string GetDatePosted() { return date; }
	inline std::string GetRatingText() { return ratingstr; }
	inline std::string GetFilename() {
		return downloadURL.substr(downloadURL.find_last_of("/") + 1);
	}
	inline std::string GetCDNFilename() {
		return GetFilename().substr(GetFilename().find_first_of(".") + 1);
	}

private:
	std::string downloadURL;
	std::string title;
	std::string ratingstr;
	std::string resolution;
	std::string date;
	int rating;
	int submissionID;
};

struct cColors {
	COLORREF cf;
	int id;
};

struct faData {
	faData(int id, std::string title, std::string thumbnailURL) : id(id), title(title), thumbnailURL(thumbnailURL) {}
	int id;
	std::string title;
	std::string thumbnailURL;
	std::wstring path;
};

struct thumbnailData {
	int index;
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
	void CreateGalleryCfg(const DownloadContext& ctx);

public:
	ConfigMgr& m_config;
};

template<typename T>
class ThreadLock
{
	ThreadLock(const ThreadLock&) = delete;
	ThreadLock& operator=(ThreadLock const&) = delete;

public:
	ThreadLock() = default;

	ThreadLock& operator=(const T& t) {
		std::lock_guard<std::mutex> lock(m_mutex);
		m_protected = t;
		return *this;
	}
	operator T() const {
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_protected;
	}

	T GetType() const {
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_protected;
	}

private:
	mutable std::mutex m_mutex;
	T m_protected{};
};

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