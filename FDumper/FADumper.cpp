#include "pch.h"

#include "FADumper.h"
#include "FASubmission.h"

#include "../contrib/rapidjson/ostreamwrapper.h"
#include "../contrib/rapidjson/writer.h"
#include "../contrib/rapidjson/istreamwrapper.h"
#include "../contrib/rapidjson/document.h"

#include "control/Message.hpp"

#include "MainDlg.h"

#include "ctpl_stl.hpp"


int CFADumper::Action(const DownloadContext& ctx)
{
	cColors clrs{ RGB(67,181,129), m_item };
	SendMessage(CMainDlg::getInstance()->hwnd(), MSG_SETCOLOR, NULL, (LPARAM)&clrs);


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

	clrs.cf = RGB(100, 149, 237);
	SendMessage(CMainDlg::getInstance()->hwnd(), MSG_SETCOLOR, NULL, (LPARAM)&clrs);

	return 0;
}

int CFADumper::Download(const DownloadContext& ctx)
{
	xlog::Verbose("Dumping gallery '%s' to '%s'", ctx.username.c_str(), WstringToAnsi(ctx.path).c_str());

	CreateGalleryCfg(ctx.path, ctx.ratings, ctx.gallery);

	if (ctx.gallery & faGalleryFlags::SCRAPS)
	{
		std::wstring scrapsdir = ctx.path + L"\\" + std::wstring(L"Scraps");
		std::filesystem::create_directory(scrapsdir);

		auto scrapgallery = GetScrapGallery(ctx);

		if (scrapgallery.empty())
		{
			xlog::Normal("Gallery for '%s' was empty", ctx.username.c_str());
			return 0;
		}

		// download
		if (int failed = DownloadInternal(scrapgallery, scrapsdir, ctx))
		{
			// nonzero number of failed downloads, something went wrong, lets tell the user
			xlog::Warning("%d downloads failed for '%s'", failed, ctx.username.c_str());
			
			Message::ShowWarning(GetActiveWindow(), std::to_wstring(failed) + L" submissions failed to download, try updating the gallery to redownload the submissions");
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
		if (int failed = DownloadInternal(fullgallery, ctx.path, ctx))
		{
			// nonzero number of failed downloads, something went wrong, lets tell the user
			xlog::Warning("%d downloads failed for '%s'", failed, ctx.username.c_str());

			Message::ShowWarning(GetActiveWindow(), std::to_wstring(failed) + L" submissions failed to download, try updating the gallery to redownload the submissions");
			return -1;
		}
	}

	if (ctx.gallery & faGalleryFlags::FAVORITES)
	{
		std::wstring favesdir = ctx.path + L"\\" + std::wstring(L"favorites");
		std::filesystem::create_directory(favesdir);

		auto favorites = GetFavoritesGallery(ctx);

		if (favorites.empty())
		{
			xlog::Normal("User '%s' has no favorites!", ctx.username.c_str());
			return 0;
		}

		if (int failed = DownloadInternal(favorites, favesdir, ctx))
		{
			xlog::Warning("%d downloads failed for '%s'", failed, ctx.username.c_str());

			Message::ShowWarning(GetActiveWindow(), std::to_wstring(failed) + L" submissions failed to download, try updating the gallery to redownload the submissions");
			return -1;
		}
	}

	CMainDlg::getInstance()->m_userQueue.setText(L"Done!", m_item, 1);

	return 0;
}

std::vector<FASubmission> CFADumper::GetMainGallery(const DownloadContext& ctx)
{
	std::vector<FASubmission> gallery;
	std::vector<faData> tempIDs;

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

	xlog::Normal("Downloading submission pages for '%s'", ctx.username.c_str());

	CMainDlg::getInstance()->m_userQueue.setText(L"Downloading pages!", m_item, 1);

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

		std::string buffer;
		int httpcode = 0;

		if (CURLcode err = curlDownload(urlbuff, buffer, httpcode))
		{
			xlog::Warning("Download error while downloading pages for '%s', %s", ctx.username, curl_easy_strerror(err));
			continue;
		}

		if (httpcode != 200)
		{
			xlog::Warning("Download error while downloading pages for '%s', http code %d", ctx.username, httpcode);
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

			std::string buffer;
			int httpcode = 0;

			if (CURLcode err = curlDownload(urlbuff, buffer, httpcode))
			{
				xlog::Warning("Download error while downloading pages for '%s', %s", ctx.username, curl_easy_strerror(err));
				continue;
			}

			if (httpcode != 200)
			{
				xlog::Warning("Download error while downloading pages for '%s', http code %d", ctx.username, httpcode);
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

	CMainDlg::getInstance()->m_userQueue.setText(std::to_wstring(tempIDs.size()), m_item, 2);

	if (tempIDs.empty())
		return gallery;

	ctpl::thread_pool thumbLoader(1);

	auto thumbloaderThread = [&](int threadid, std::vector<faData> data, std::string user) -> void
	{
		wchar_t temp[MAX_PATH] = {};
		GetTempPath(MAX_PATH, temp);

		std::wstring workingpath(temp);
		workingpath.append(L"\\FDumper\\thumbs\\" + AnsiToWstring(user));

		std::filesystem::create_directory(workingpath);

		for (auto thumb : data)
		{
			std::string filename = thumb.thumbnailURL.substr(thumb.thumbnailURL.find_last_of("/") + 1);
			std::wstring path = workingpath + L"\\" + AnsiToWstring(filename);

			FILE* fp = nullptr;
			_wfopen_s(&fp, path.c_str(), L"wb");

			CURL* pCurl = curl_easy_init();

			curl_easy_setopt(pCurl, CURLOPT_URL, thumb.thumbnailURL.c_str());
			curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, writefilecallback);
			curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, fp);

			curl_easy_perform(pCurl);

			std::fclose(fp);
			curl_easy_cleanup(pCurl);

			thumbnailData thumbdata{ m_item, path, thumb };
			SendMessage(CMainDlg::getInstance()->hwnd(), MSG_LOADTHUMBNAIL, NULL, (LPARAM)&thumbdata);
		}
	};

	thumbLoader.push(thumbloaderThread, tempIDs, ctx.username);

	std::for_each(tempIDs.begin(), tempIDs.end(), [&](faData data) {gallery.push_back(FASubmission(data.id)); });


	return gallery;
}

std::vector<FASubmission> CFADumper::GetScrapGallery(const DownloadContext& ctx)
{
	std::vector<FASubmission> scraps;
	std::vector<faData> tempIDs;

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

	const auto& api = m_config.config().apiaddress;

	int curpage = 1;
	xlog::Normal("Downloading scrap submission pages for '%s'", ctx.username.c_str());

	CMainDlg::getInstance()->m_userQueue.setText(L"Downloading pages", m_item, 1);

	while (true)
	{
		char urlbuff[200] = {};

		if (ctx.ratings == SFW_ONLY) {
			sprintf_s(urlbuff, "%s/user/%s/scraps.json?sfw=1&page=%d", api.c_str(), ctx.username.c_str(), curpage);
		}
		else {
			sprintf_s(urlbuff, "%s/user/%s/scraps.json?page=%d", api.c_str(), ctx.username.c_str(), curpage);
		}

		std::string buffer;
		int httpcode = 0;

		if (CURLcode err = curlDownload(urlbuff, buffer, httpcode))
		{
			xlog::Warning("Download error while downloading pages for '%s', %s", ctx.username, curl_easy_strerror(err));
			continue;
		}

		if (httpcode != 200)
		{
			xlog::Warning("Download error while downloading pages for '%s', http code %d", ctx.username, httpcode);
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
			sprintf_s(urlbuff, "%s/user/%s/scraps.json?sfw=1&page=%d", api.c_str(), ctx.username.c_str(), curpage);

			std::string buffer;
			int httpcode = 0;

			if (CURLcode err = curlDownload(urlbuff, buffer, httpcode))
			{
				xlog::Warning("Download error while downloading pages for '%s', %s", ctx.username, curl_easy_strerror(err));
				continue;
			}

			if (httpcode != 200)
			{
				xlog::Warning("Download error while downloading pages for '%s', http code %d", ctx.username, httpcode);
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

	CMainDlg::getInstance()->m_userQueue.setText(std::to_wstring(tempIDs.size()), m_item, 2);

	xlog::Normal("%d total images for '%s'", tempIDs.size(), ctx.username.c_str());

	if (tempIDs.empty())
		return std::vector<FASubmission>();

	std::for_each(tempIDs.begin(), tempIDs.end(), [&](faData data) {scraps.push_back(FASubmission(data.id)); });
	
	return scraps;
}

std::vector<FASubmission> CFADumper::GetFavoritesGallery(const DownloadContext & ctx)
{
	std::vector<FASubmission> favorites;
	std::vector<faData> tempIDs;

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

	xlog::Normal("Downloading favorites pages...");

	CMainDlg::getInstance()->m_userQueue.setText(L"Downloading pages", m_item, 1);

	const auto& api = m_config.config().apiaddress;

	int favorite_id = 0;
	while (true)
	{
		char urlbuff[200] = {};

		if (ctx.ratings == SFW_ONLY)
			sprintf_s(urlbuff, "%s/user/%s/favorites.json?full=1&sfw=1&next=%d", api.c_str(), ctx.username.c_str(), favorite_id);
		else
			sprintf_s(urlbuff, "%s/user/%s/favorites.json?full=1&next=%d", api.c_str(), ctx.username.c_str(), favorite_id);

		std::string buffer;
		int httpcode = 0;

		if (CURLcode err = curlDownload(urlbuff, buffer, httpcode))
		{
			xlog::Warning("Download error while downloading pages for '%s', %s", ctx.username, curl_easy_strerror(err));
			continue;
		}

		if (httpcode != 200)
		{
			xlog::Warning("Download error while downloading pages for '%s', http code %d", ctx.username, httpcode);
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

			std::string buffer;
			int httpcode = 0;

			if (CURLcode err = curlDownload(urlbuff, buffer, httpcode))
			{
				xlog::Warning("Download error while downloading pages for '%s', %s", ctx.username, curl_easy_strerror(err));
				continue;
			}

			if (httpcode != 200)
			{
				xlog::Warning("Download error while downloading pages for '%s', http code %d", ctx.username, httpcode);
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

	CMainDlg::getInstance()->m_userQueue.setText(std::to_wstring(tempIDs.size()), m_item, 2);

	xlog::Normal("%d total images for '%s'", tempIDs.size(), ctx.username.c_str());

	if (tempIDs.empty())
		return std::vector<FASubmission>();

	if (tempIDs.size() > 300) // 5 minutes
	{
		// UGH ugly formatting (could've sworn time_t was for this)
		auto time = tempIDs.size();
		int days = time / (24 * 3600);
		time = time % (24 * 3600);
		int hours = time / 3600;
		time %= 3600;
		int minutes = time / 60;

		auto wusername = AnsiToWstring(ctx.username);
		wchar_t buff[256] = {};

		if (days > 0)
			std::swprintf(buff, sizeof(buff), L"Warning, dowloading %s's favorites will take approximately %d days, %d hours, and %d minutes. Continue anways?",
				wusername.c_str(), days, hours, minutes);
		else if (hours > 0)
			std::swprintf(buff, sizeof(buff), L"Warning, dowloading %s's favorites will take approximately %d hours and %d minutes. Continue anways?",
				wusername.c_str(), hours, minutes);
		else if (minutes > 0)
			std::swprintf(buff, sizeof(buff), L"Warning, dowloading %s's favorites will take approximately %d minutes. Continue anways?",
				wusername.c_str(), minutes);

		if (!Message::ShowQuestion(GetActiveWindow(), buff, L"Thats a LOT of favorites!"))
			return std::vector<FASubmission>();
	}

	std::for_each(tempIDs.begin(), tempIDs.end(), [&](faData data) {favorites.push_back(FASubmission(data.id)); });

	return favorites;
}

int CFADumper::DownloadInternal(std::vector<FASubmission> gallery, const std::wstring& path, const DownloadContext& ctx)
{
	CMainDlg::getInstance()->m_userQueue.setText(L"Downloading images", m_item, 1);

	ctpl::thread_pool downloadThreads(std::thread::hardware_concurrency());
	ctpl::thread_pool updatePool(1);
	std::vector<std::shared_future<CURLcode>> threadresults;

	int progress = 0;

	threadresults.resize(gallery.size());

	auto replacePreview = [&](int threadID, std::shared_future<CURLcode> future, FASubmission submission) -> void
	{
		future.wait();
		threadresults.push_back(future);
		if (future.get() != CURLE_OK)
			return;

		auto& targetvec = CMainDlg::getInstance()->m_previewData[m_item];

		auto it = std::find_if(targetvec.begin(), targetvec.end(), [&](faData data) {
			return data.title.compare(submission.GetSubmissionTitle()) == 0;
		});

		if (it == std::end(targetvec))
			return;

		auto link = submission.GetDownloadLink();
		auto filename = submission.GetFilename();

		std::string id = std::to_string(submission.GetSubmissionID()); id.append("-");
		filename.insert(0, id);

		std::wstring savedir = path + L"\\" + AnsiToWstring(filename);

		it->path = savedir;
		progress++;
	};

	for (auto submission : gallery)
	{
		submission.Setup(m_config.config().apiaddress);

		downloadListData dlData{ m_item, submission };
		SendMessage(CMainDlg::getInstance()->hwnd(), MSG_ADDTODOWNLOADLIST, NULL, (LPARAM)&dlData);

		int percent = ((float)progress / (float)threadresults.size()) * 100;
		std::wstring str = std::to_wstring(percent) + L"%";

		CMainDlg::getInstance()->m_userQueue.setText(str, m_item, 3);

		auto result = downloadThreads.push(DownloadImage, submission, path, ctx);
		updatePool.push(replacePreview, result.share(), submission);
	}

	while (downloadThreads.size() != downloadThreads.n_idle())
	{
		int percent = ((float)progress / (float)threadresults.size()) * 100;
		std::wstring str = std::to_wstring(percent) + L"%";

		CMainDlg::getInstance()->m_userQueue.setText(str, m_item, 3);
	}

	int faileddownloads = 0;
	for (auto& code : threadresults)
	{
		if (code.get() != CURLE_OK)
			++faileddownloads;
	}

	return faileddownloads;
}

CURLcode CFADumper::DownloadImage(int threadID, FASubmission submission, const std::wstring& path, const DownloadContext& ctx)
{
	auto curlDownload = [&](const std::string& url, const std::wstring& path) -> CURLcode
	{
		FILE* fp = nullptr;
		_wfopen_s(&fp, path.c_str(), L"wb");

		CURL* pCurl = curl_easy_init();

		curl_easy_setopt(pCurl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, writefilecallback);
		curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, fp);

		CURLcode res = curl_easy_perform(pCurl);

		return std::fclose(fp), res;
	};

	auto link = submission.GetDownloadLink();
	auto filename = submission.GetFilename();

	std::string id = std::to_string(submission.GetSubmissionID()); id.append("-");
	filename.insert(0, id);

	std::wstring savedir = path + L"\\" + AnsiToWstring(filename);

	CURLcode returncode = curlDownload(link, savedir);

	if (returncode != CURLE_OK)
	{
		xlog::Error("Failed to download '%s', %s", filename, curl_easy_strerror(returncode));

		// Delete the file so we can tell the user to update later
		std::filesystem::remove(savedir);
	}


	return returncode;
}