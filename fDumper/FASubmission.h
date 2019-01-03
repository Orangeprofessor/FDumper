#pragma once

#include "CBaseDumper.h"

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
				log_console(xlog::LogLevel::warning, "Download error! %s! retrying...\n", curl_easy_strerror(err));
				continue;
			}

			if (httpcode != 200)
			{
				log_console(xlog::LogLevel::warning, "download error! retrying...");
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

			break;
		}

	}

	inline std::string GetDownloadLink() { return downloadURL; }
	inline int GetSubmissionID() { return submissionID; }
	inline std::string GetSubmissionTitle() { return title; }
	inline int GetRating() { return rating; }
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
	int rating;
	int submissionID;
};