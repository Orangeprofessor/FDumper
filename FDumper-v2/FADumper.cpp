#include "stdafx.h"

#include "FADumper.h"

#include "../contrib/rapidjson/ostreamwrapper.h"
#include "../contrib/rapidjson/writer.h"
#include "../contrib/rapidjson/istreamwrapper.h"
#include "../contrib/rapidjson/document.h"


#include "ctpl_stl.hpp"

int CFADumper::Action(const DownloadContext& ctx)
{

	if (!ValidUser(ctx.username))
	{
		xlog::Error("User '%s' doesn't exist!", ctx.username.c_str());
		return -1;
	}

	std::filesystem::create_directory(ctx.path);

	xlog::Normal("Processing user gallery '%s'", ctx.username.c_str());

	if (Download(ctx))
	{
		xlog::Error("Errors while processing user gallery '%s'", ctx.username.c_str());
		return -1;
	}

	xlog::Normal("Finished processing user gallery '%s'\n", ctx.username.c_str());


	return 0;
}

int CFADumper::Download(const DownloadContext& ctx)
{
	auto workingdir = ctx.path;
	auto userdata = workingdir; userdata.operator/=(L"userdata");

	xlog::Verbose("Dumping gallery '%s' to '%s'", ctx.username.c_str(), workingdir.string().c_str());

	CreateGalleryCfg(ctx);

	const auto& api = m_config.config().apiaddress;

	char urlbuff[200] = {};
	sprintf_s(urlbuff, "%s/user/%s.json", api.c_str(), ctx.username.c_str());

	std::string buffer;
	curl_ptr curl;
	curl.SetUrl(urlbuff);
	if (curl.DownloadToBuffer(buffer) != CURLE_OK || curl.GetHTTPCode() != 200) {
		// uhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh
	}

	rapidjson::Document tempdoc;
	tempdoc.Parse(buffer.c_str());

	std::string pfplink = tempdoc.FindMember("avatar")->value.GetString();
	curl.SetUrl(pfplink);

	if (curl.DownloadToDisk(userdata.operator/=(L"pfp.png")) || curl.GetHTTPCode() != 200) {
		//uhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh
	}

	userdata.remove_filename();

	if (ctx.gallery & faGalleryFlags::SCRAPS)
	{
		auto scrapspath = workingdir; scrapspath.operator/=(L"Scraps");
		std::filesystem::create_directory(scrapspath);

		auto scrapgallery = GetScrapGallery(ctx);

		if (scrapgallery.empty())
		{
			xlog::Normal("Gallery for '%s' was empty", ctx.username.c_str());
			return 0;
		}

		// download
		if (int failed = DownloadInternal(scrapgallery, scrapspath, ctx))
		{
			// nonzero number of failed downloads, something went wrong, lets tell the user
			xlog::Warning("%d downloads failed for '%s'", failed, ctx.username.c_str());

			//Message::ShowWarning(GetActiveWindow(), std::to_wstring(failed) + L" submissions failed to download, try updating the gallery to redownload the submissions");
			return -1;
		}
		return 0;
	}

	if (ctx.gallery & faGalleryFlags::MAIN)
	{
		auto fullgallery = GetMainGallery(ctx);

		if (fullgallery.empty())
		{
			xlog::Normal("Gallery for '%s' was empty", ctx.username.c_str());
			return 0;
		}

		// download
		if (int failed = DownloadInternal(fullgallery, workingdir, ctx))
		{
			// nonzero number of failed downloads, something went wrong, lets tell the user
			xlog::Warning("%d downloads failed for '%s'", failed, ctx.username.c_str());

			//Message::ShowWarning(GetActiveWindow(), std::to_wstring(failed) + L" submissions failed to download, try updating the gallery to redownload the submissions");
			return -1;
		}
	}

	if (ctx.gallery & faGalleryFlags::FAVORITES)
	{
		//std::wstring favesdir = ctx.path + L"\\" + std::wstring(L"favorites");
		auto favoritespath = workingdir; favoritespath.operator/=(L"favorites");
		std::filesystem::create_directory(favoritespath);

		auto favorites = GetFavoritesGallery(ctx);

		if (favorites.empty())
		{
			xlog::Normal("User '%s' has no favorites!", ctx.username.c_str());
			return 0;
		}

		if (int failed = DownloadInternal(favorites, favoritespath, ctx))
		{
			xlog::Warning("%d downloads failed for '%s'", failed, ctx.username.c_str());

			//Message::ShowWarning(GetActiveWindow(), std::to_wstring(failed) + L" submissions failed to download, try updating the gallery to redownload the submissions");
			return -1;
		}
	}

	return 0;
}

std::vector<FASubmission> CFADumper::GetMainGallery(const DownloadContext& ctx)
{
	std::vector<FASubmission> gallery;
	std::vector<faData> tempIDs;

	xlog::Normal("Downloading submission pages for '%s'", ctx.username.c_str());

	const auto& api = m_config.config().apiaddress;

	int curpage = 1;
	while (true)
	{
		char urlbuff[200] = {};
		if (ctx.ratings == SFW_ONLY) {
			sprintf_s(urlbuff, "%s/user/%s/gallery.json?full=1&sfw=1&page=%d", api.c_str(), ctx.username.c_str(), curpage);
		}
		else {
			sprintf_s(urlbuff, "%s/user/%s/gallery.json?full=1&page=%d", api.c_str(), ctx.username.c_str(), curpage);
		}

		curl_ptr curl;
		std::string buffer;
		curl.SetUrl(urlbuff);

		if (CURLcode err = curl.DownloadToBuffer(buffer))
		{
			xlog::Warning("Download error while downloading pages for '%s', %s", ctx.username, curl.GetError(err).c_str());
			continue;
		}

		if (int code = curl.GetHTTPCode() != 200)
		{
			xlog::Warning("Download error while downloading pages for '%s', http code %d", ctx.username, code);
			continue;
		}

		rapidjson::Document doc;
		doc.Parse(buffer.c_str());

		for (auto itr = doc.Begin(); itr != doc.End(); ++itr)
		{
			std::string jID = itr->FindMember("id")->value.GetString();
			std::string jTitle = itr->FindMember("title")->value.GetString();
			std::string jThumbnail = itr->FindMember("thumbnail")->value.GetString();

			tempIDs.push_back({ std::stoi(jID), jTitle, jThumbnail });
		}

		auto docsize = doc.Size();
		if (docsize < 72)
			break;

		++curpage;
	}

	if (ctx.ratings == NSFW_ONLY)
	{
		curpage = 1;
		std::vector<faData> sfwIDs;

		while (true)
		{
			char urlbuff[200] = {};
			sprintf_s(urlbuff, "%s/user/%s/gallery.json?full=1&sfw=1&page=%d", api.c_str(), ctx.username.c_str(), curpage);

			curl_ptr curl;
			std::string buffer;

			curl.SetUrl(urlbuff);

			if (CURLcode err = curl.DownloadToBuffer(buffer))
			{
				xlog::Warning("Download error while downloading pages for '%s', %s", ctx.username, curl.GetError(err).c_str());
				continue;
			}

			if (int code = curl.GetHTTPCode() != 200)
			{
				xlog::Warning("Download error while downloading pages for '%s', http code %d", ctx.username, code);
				continue;
			}

			rapidjson::Document doc;
			doc.Parse(buffer.c_str());

			for (auto itr = doc.Begin(); itr != doc.End(); ++itr)
			{
				std::string jID = itr->FindMember("id")->value.GetString();
				std::string jTitle = itr->FindMember("title")->value.GetString();
				std::string jThumbnail = itr->FindMember("thumbnail")->value.GetString();

				sfwIDs.push_back({ std::stoi(jID), jTitle, jThumbnail });
			}

			auto docsize = doc.Size();
			if (docsize < 72)
				break;
			++curpage;
		}

		tempIDs.erase(std::remove_if(tempIDs.begin(), tempIDs.end(), [&](faData data)
		{
			return std::find_if(sfwIDs.begin(), sfwIDs.end(), [&](faData sfw) {return sfw.id == data.id; }) != sfwIDs.end();
		}), tempIDs.end());
	}

	xlog::Normal("%d total images for '%s'", tempIDs.size(), ctx.username.c_str());

	if (tempIDs.empty())
		return std::vector<FASubmission>();

	std::for_each(tempIDs.begin(), tempIDs.end(), [&](faData data) {gallery.push_back(FASubmission(data.id)); });

	return gallery;
}

std::vector<FASubmission> CFADumper::GetScrapGallery(const DownloadContext& ctx)
{
	std::vector<FASubmission> scraps;
	std::vector<faData> tempIDs;

	const auto& api = m_config.config().apiaddress;

	int curpage = 1;
	xlog::Normal("Downloading scrap submission pages for '%s'", ctx.username.c_str());

	while (true)
	{
		char urlbuff[200] = {};

		if (ctx.ratings == SFW_ONLY) {
			sprintf_s(urlbuff, "%s/user/%s/scraps.json?full=1&sfw=1&page=%d", api.c_str(), ctx.username.c_str(), curpage);
		}
		else {
			sprintf_s(urlbuff, "%s/user/%s/scraps.json?full=1&page=%d", api.c_str(), ctx.username.c_str(), curpage);
		}

		curl_ptr curl;
		std::string buffer;

		curl.SetUrl(urlbuff);

		if (CURLcode err = curl.DownloadToBuffer(buffer))
		{
			xlog::Warning("Download error while downloading pages for '%s', %s", ctx.username, curl.GetError(err).c_str());
			continue;
		}

		if (int code = curl.GetHTTPCode() != 200)
		{
			xlog::Warning("Download error while downloading pages for '%s', http code %d", ctx.username, code);
			continue;
		}

		rapidjson::Document doc;
		doc.Parse(buffer.c_str());

		for (auto itr = doc.Begin(); itr != doc.End(); ++itr)
		{
			std::string jID = itr->FindMember("id")->value.GetString();
			std::string jTitle = itr->FindMember("title")->value.GetString();
			std::string jThumbnail = itr->FindMember("thumbnail")->value.GetString();

			tempIDs.push_back({ std::stoi(jID), jTitle, jThumbnail });
		}

		auto docsize = doc.Size();
		if (docsize < 72)
			break;

		++curpage;
	}

	if (ctx.ratings == NSFW_ONLY)
	{
		curpage = 1;
		std::vector<faData> sfwIDs;

		while (true)
		{
			char urlbuff[200] = {};
			sprintf_s(urlbuff, "%s/user/%s/scraps.json?full=1&sfw=1&page=%d", api.c_str(), ctx.username.c_str(), curpage);

			curl_ptr curl;
			std::string buffer;
			curl.SetUrl(urlbuff);

			if (CURLcode err = curl.DownloadToBuffer(buffer))
			{
				xlog::Warning("Download error while downloading pages for '%s', %s", ctx.username, curl.GetError(err).c_str());
				continue;
			}

			if (int code = curl.GetHTTPCode() != 200)
			{
				xlog::Warning("Download error while downloading pages for '%s', http code %d", ctx.username, code);
				continue;
			}

			rapidjson::Document doc;
			doc.Parse(buffer.c_str());

			for (auto itr = doc.Begin(); itr != doc.End(); ++itr)
			{
				std::string jID = itr->FindMember("id")->value.GetString();
				std::string jTitle = itr->FindMember("title")->value.GetString();
				std::string jThumbnail = itr->FindMember("thumbnail")->value.GetString();

				sfwIDs.push_back({ std::stoi(jID), jTitle, jThumbnail });
			}

			auto docsize = doc.Size();
			if (docsize < 72)
				break;

			++curpage;
		}

		tempIDs.erase(std::remove_if(tempIDs.begin(), tempIDs.end(), [&](faData data)
		{
			return std::find_if(sfwIDs.begin(), sfwIDs.end(), [&](faData sfw) {return sfw.id == data.id; }) != sfwIDs.end();
		}), tempIDs.end());
	}

	xlog::Normal("%d total images for '%s'", tempIDs.size(), ctx.username.c_str());

	if (tempIDs.empty())
		return std::vector<FASubmission>();

	std::for_each(tempIDs.begin(), tempIDs.end(), [&](faData data) {scraps.push_back(FASubmission(data.id)); });

	return scraps;
}

std::vector<FASubmission> CFADumper::GetFavoritesGallery(const DownloadContext& ctx)
{
	std::vector<FASubmission> favorites;
	std::vector<faData> tempIDs;

	xlog::Normal("Downloading favorites pages...");

	const auto& api = m_config.config().apiaddress;

	int favorite_id = 0;
	while (true)
	{
		char urlbuff[200] = {};

		if (ctx.ratings == SFW_ONLY)
			sprintf_s(urlbuff, "%s/user/%s/favorites.json?full=1&sfw=1&next=%d", api.c_str(), ctx.username.c_str(), favorite_id);
		else
			sprintf_s(urlbuff, "%s/user/%s/favorites.json?full=1&next=%d", api.c_str(), ctx.username.c_str(), favorite_id);

		curl_ptr curl;
		std::string buffer;

		curl.SetUrl(urlbuff);

		if (CURLcode err = curl.DownloadToBuffer(buffer))
		{
			xlog::Warning("Download error while downloading pages for '%s', %s", ctx.username, curl.GetError(err).c_str());
			continue;
		}

		if (int code = curl.GetHTTPCode() != 200)
		{
			xlog::Warning("Download error while downloading pages for '%s', http code %d", ctx.username, code);
			continue;
		}

		rapidjson::Document doc;
		doc.Parse(buffer.c_str());

		for (auto itr = doc.Begin(); itr != doc.End(); ++itr)
		{
			favorite_id = std::stoi(itr->FindMember("fav_id")->value.GetString());

			std::string jID = itr->FindMember("id")->value.GetString();
			std::string jTitle = itr->FindMember("title")->value.GetString();
			std::string jThumbnail = itr->FindMember("thumbnail")->value.GetString();

			tempIDs.push_back({ std::stoi(jID), jTitle, jThumbnail });
		}

		auto docsize = doc.Size();
		if (docsize < 72)
			break;
	}

	if (ctx.ratings == NSFW_ONLY)
	{
		std::vector<faData> sfwIDs;

		favorite_id = 0;
		while (true)
		{
			char urlbuff[200] = {};
			sprintf_s(urlbuff, "%s/user/%s/favorites.json?full=1&sfw=1&next=%d", api.c_str(), ctx.username.c_str(), favorite_id);

			curl_ptr curl;
			std::string buffer;

			curl.SetUrl(urlbuff);

			if (CURLcode err = curl.DownloadToBuffer(buffer))
			{
				xlog::Warning("Download error while downloading pages for '%s', %s", ctx.username, curl.GetError(err).c_str());
				continue;
			}

			if (int code = curl.GetHTTPCode() != 200)
			{
				xlog::Warning("Download error while downloading pages for '%s', http code %d", ctx.username, code);
				continue;
			}

			rapidjson::Document doc;
			doc.Parse(buffer.c_str());

			for (auto itr = doc.Begin(); itr != doc.End(); ++itr)
			{
				favorite_id = std::stoi(itr->FindMember("fav_id")->value.GetString());

				std::string jID = itr->FindMember("id")->value.GetString();
				std::string jTitle = itr->FindMember("title")->value.GetString();
				std::string jThumbnail = itr->FindMember("thumbnail")->value.GetString();

				sfwIDs.push_back({ std::stoi(jID), jTitle, jThumbnail });
			}

			auto docsize = doc.Size();
			if (docsize < 72)
				break;
		}

		tempIDs.erase(std::remove_if(tempIDs.begin(), tempIDs.end(), [&](faData data)
		{
			return std::find_if(sfwIDs.begin(), sfwIDs.end(), [&](faData sfw) {return sfw.id == data.id; }) != sfwIDs.end();
		}), tempIDs.end());
	}

	xlog::Normal("%d total images for '%s'", tempIDs.size(), ctx.username.c_str());

	if (tempIDs.empty())
		return std::vector<FASubmission>();

	// !!!!!! FIND A SOLUTION TO THIS P L E A S E
	// i think i did
	if (tempIDs.size() > 300) // 5 minutes
	{
		namespace chrono = std::chrono;

		chrono::seconds time(tempIDs.size());
	
		auto hours = chrono::duration_cast<chrono::hours>(time); time -= hours;
		auto minutes = chrono::duration_cast<chrono::minutes>(time); time -= minutes;

		wchar_t buff[256] = {};

		if (hours.count())
			std::swprintf(buff, sizeof(buff), L"Warning, dowloading %s's favorites will take approximately %d hours and %d minutes. Continue anways?",
				ctx.username.c_str(), hours.count(), minutes.count());
		else if (minutes.count())
			std::swprintf(buff, sizeof(buff), L"Warning, dowloading %s's favorites will take approximately %d minutes. Continue anways?",
				ctx.username.c_str(), minutes.count());

		//if (!Message::ShowQuestion(GetActiveWindow(), buff, L"Thats a LOT of favorites!"))
		//	return std::vector<FASubmission>();
	}

	std::for_each(tempIDs.begin(), tempIDs.end(), [&](faData data) {favorites.push_back(FASubmission(data.id)); });

	return favorites;
}

int CFADumper::DownloadInternal(std::vector<FASubmission> gallery, const std::wstring& path, const DownloadContext& ctx)
{
	ctpl::thread_pool downloadThreads(std::thread::hardware_concurrency());
	ctpl::thread_pool updatePool(1);
	std::vector<CURLcode> threadresults;

	std::atomic<int> progress = 1;

	threadresults.resize(gallery.size());


	while (downloadThreads.size() != downloadThreads.n_idle())
	{
		int percent = ((float)progress.operator int() / (float)gallery.size()) * 100;
		
	}

	int faileddownloads = 0;
	for (auto& code : threadresults)
	{
		if (code != CURLE_OK)
			++faileddownloads;
	}
	return faileddownloads;
}

CURLcode CFADumper::DownloadImage(int threadID, FASubmission submission, std::filesystem::path path, const DownloadContext& ctx)
{
	auto link = submission.GetDownloadLink();
	auto filename = submission.GetFilename();

	std::string id = std::to_string(submission.GetSubmissionID()); id.append("-");
	filename.insert(0, id);

	path.operator/=(filename);

	curl_ptr curl;
	curl.SetUrl(link);
	auto returncode = curl.DownloadToDisk(path);

	if (returncode != CURLE_OK)
	{
		xlog::Error("Failed to download '%s', %s", filename, curl_easy_strerror(returncode));

		// Delete the file so we can tell the user to update later
		std::filesystem::remove(path);
	}

	return returncode;
}
