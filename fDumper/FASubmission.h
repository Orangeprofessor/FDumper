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

			auto curlDownload = [&](const std::string& url, std::string& buffer) -> CURLcode
			{
				CURL* pCurl = curl_easy_init();

				curl_easy_setopt(pCurl, CURLOPT_URL, url.c_str());
				curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, writebuffercallback);
				curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &buffer);
				CURLcode code = curl_easy_perform(pCurl);

				curl_easy_cleanup(pCurl);
				return code;
			};

			std::string buffer;

			if (auto err = curlDownload(urlbuff, buffer)) {
				console_error("Download error! %s! retrying...\n", curl_easy_strerror(err));
				continue;
			}

			if (buffer == "FAExport encounter an internal error") {
				std::printf("\n");
				console_error("download error! retrying...\n");
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

private:
	std::string downloadURL;
	std::string title;
	std::string ratingstr;
	int rating;
	int submissionID;
};