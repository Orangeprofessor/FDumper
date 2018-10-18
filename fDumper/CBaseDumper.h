#pragma once

#include "pch.h"

#include "Log.h"

#include "curl/curl.h"
#include "rapidjson/document.h"

class CBaseDumper
{
public:
	virtual ~CBaseDumper() {}
	virtual int Download() = 0;
};

enum faRatingFlags : int
{
	ALL_RATINGS = 0,
	NSFW_ONLY,
	SFW_ONLY
};

enum faGalleryFlags : int
{
	ALL_GALLERIES = 0,
	NO_SCRAPS,
	SCRAPS_ONLY,
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

struct FASubmission
{
	FASubmission(int submission) : submissionID(submission) {}

	void Setup(std::string api)
	{
		char urlbuff[200] = {};

		sprintf_s(urlbuff, "%s/submission/%d.json", api.c_str(), submissionID);

		auto curlDownload = [&](const std::string& url, std::string& buffer) -> int
		{
			CURL* pCurl = curl_easy_init();

			curl_easy_setopt(pCurl, CURLOPT_URL, url.c_str());
			curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, writebuffercallback);
			curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, buffer);
			CURLcode code = curl_easy_perform(pCurl);

			curl_easy_cleanup(pCurl);
			return code;
		};

		std::string buffer;

		rapidjson::Document doc;
		doc.Parse(buffer.c_str());

		auto fullimage = doc.FindMember("download");
		downloadURL = fullimage->value.GetString();

		auto imagetitle = doc.FindMember("title");
		title = imagetitle->value.GetString();

		auto imagerating = doc.FindMember("rating");
		imagerating->value.GetString() == std::string("General") ? rating = 1 : rating = 2;
		ratingstr = imagerating->value.GetString();
	}

	inline std::string GetDownloadLink() { return downloadURL; }
	inline int GetSubmissionID() { return submissionID; }
	inline std::string GetSubmissionTitle() { return title; }
	inline int GetRating() { return rating; }
	inline std::string GetRatingText() { return ratingstr; }

private:
	std::string downloadURL;
	std::string title;
	std::string ratingstr;
	int rating;
	int submissionID;
};