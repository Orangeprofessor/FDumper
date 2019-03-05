#pragma once

#include "../contrib/curl/curl.h"
#include "../contrib/rapidjson/document.h"

static size_t writebuffercallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

static size_t writefilecallback(void* contents, size_t size, size_t nmemb, FILE* fp)
{
	return fwrite(contents, size, nmemb, fp);
}

struct FASubmission
{
	FASubmission(int submission) : submissionID(submission) {}

	void Setup(std::string api)
	{
		while (true)
		{
			char urlbuff[200] = {};

			sprintf_s(urlbuff, "%s/submission/%d.json", api.c_str(), submissionID);

			auto curlDownload = [&](const std::string& url, std::string& buffer, int& httpcode) -> CURLcode
			{
				CURL* pCurl = curl_easy_init();

				curl_easy_setopt(pCurl, CURLOPT_URL, url.c_str());
				curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, writebuffercallback);
				curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &buffer);
				CURLcode code = curl_easy_perform(pCurl);

				if (code == CURLE_OK)
					curl_easy_getinfo(pCurl, CURLINFO_RESPONSE_CODE, &httpcode);

				curl_easy_cleanup(pCurl);
				return code;
			};

			std::string buffer;
			int httpcode = 0;

			if (auto err = curlDownload(urlbuff, buffer, httpcode)) {
				xlog::Warning("Download error! %s!", curl_easy_strerror(err));
				continue;
			}

			if (httpcode != 200)
			{
				xlog::Warning("Download error! http code %d", httpcode);
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