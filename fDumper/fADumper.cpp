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
	SendMessage(MainDlg::getInstance()->hwnd(), MSG_SETCOLOR, NULL, (LPARAM)&clrs);

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
	SendMessage(MainDlg::getInstance()->hwnd(), MSG_SETCOLOR, NULL, (LPARAM)&clrs);

	return 0;
}

int CFADumper::Download(const DownloadContext& ctx)
{
	xlog::Verbose("Dumping gallery '%s' to '%s'", ctx.username.c_str(), WstringToAnsi(ctx.path).c_str());

	if (ctx.gallery & faGalleryFlags::SCRAPS)
	{
		std::wstring scrapsdir = ctx.path + L"\\" + std::wstring(L"Scraps");
		std::filesystem::create_directory(scrapsdir);

		CreateGalleryCfg(scrapsdir, ctx.ratings, ctx.gallery);

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
		CreateGalleryCfg(ctx.path, ctx.ratings, ctx.gallery);

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

	MainDlg::getInstance()->m_userQueue.setText(L"Done!", m_item, 1);

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

	MainDlg::getInstance()->m_userQueue.setText(L"Downloading pages!", m_item, 1);

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

	MainDlg::getInstance()->m_userQueue.setText(std::to_wstring(tempIDs.size()), m_item, 2);

	if (tempIDs.empty())
		return std::vector<FASubmission>();

	int progress = 1;
	xlog::Normal("Downloading submission data for '%s'", ctx.username.c_str());

	MainDlg::getInstance()->m_userQueue.setText(L"Processing data", m_item, 1);

	//ctpl::thread_pool thumbLoader(1);

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
		}

		thumbnailData thumbdata{ workingpath, data };
		SendMessage(MainDlg::getInstance()->hwnd(), MSG_LOADTHUMBNAILS, NULL, (LPARAM)&thumbdata);
	};

	//thumbLoader.push(thumbloaderThread, tempIDs, ctx.username);


	MainDlg::getInstance()->m_downloadListData.GetType()[m_item].clear();


	for (auto IDs : tempIDs)
	{
		FASubmission submission(IDs.id);

		int perc = (int)std::ceil((((float)progress / (float)tempIDs.size()) * 100));

		std::wstring percstr = std::to_wstring(perc) + L"%";

		MainDlg::getInstance()->m_userQueue.setText(percstr, m_item, 3);

		submission.Setup(api);

		if (ListView_GetNextItem(MainDlg::getInstance()->m_downloadedList.hwnd(), -1, LVNI_SELECTED) == m_item)
		{
			auto title = AnsiToWstring(submission.GetSubmissionTitle());
			auto date = AnsiToWstring(submission.GetDatePosted());
			auto res = AnsiToWstring(submission.GetResolution());

			MainDlg::getInstance()->m_downloadedList.AddItem(title, NULL, { date, res, std::to_wstring(submission.GetSubmissionID()) });
		}

		MainDlg::getInstance()->m_downloadListData.GetType()[m_item].push_back(submission);

		++progress;

		gallery.push_back(submission);
	}

	return gallery;
}

std::vector<FASubmission> CFADumper::GetScrapGallery(const DownloadContext& ctx)
{
	std::vector<FASubmission> scraps;
	std::vector<int> tempIDs;

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

	MainDlg::getInstance()->m_userQueue.setText(L"Downloading pages", m_item, 1);

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
			std::string jsonElm = itr->GetString();
			int subID = std::stoi(jsonElm);

			tempIDs.push_back(subID);
		}

		auto docsize = doc.Size();
		if (docsize < 72)
			break;

		++curpage;
	}

	if (ctx.ratings == NSFW_ONLY)
	{
		curpage = 1;
		std::vector<int> sfwIDs;

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
				std::string jsonElm = itr->GetString();
				int subID = std::stoi(jsonElm);

				sfwIDs.push_back(subID);
			}

			auto docsize = doc.Size();
			if (docsize < 72)
				break;

			++curpage;
		}

		tempIDs.erase(std::remove_if(tempIDs.begin(), tempIDs.end(), [&](int id)
		{
			return std::find(sfwIDs.begin(), sfwIDs.end(), id) != sfwIDs.end();
		}), tempIDs.end());
	}

	MainDlg::getInstance()->m_userQueue.setText(std::to_wstring(tempIDs.size()), m_item, 2);

	xlog::Normal("%d total images for '%s'", tempIDs.size(), ctx.username.c_str());

	if (tempIDs.empty())
		return std::vector<FASubmission>();

	int progress = 1;

	xlog::Normal("Downloading scrap submission data for '%s'", ctx.username.c_str());

	MainDlg::getInstance()->m_userQueue.setText(L"Processing data", m_item, 1);

	for (auto IDs : tempIDs)
	{
		FASubmission submission(IDs);

		int perc = (int)std::ceil((((float)progress / (float)tempIDs.size()) * 100));

		std::wstring percstr = std::to_wstring(perc) + L"%";

		MainDlg::getInstance()->m_userQueue.setText(percstr, m_item, 3);

		submission.Setup(api);

		++progress;

		scraps.push_back(submission);
	}

	return scraps;
}

int CFADumper::DownloadInternal(std::vector<FASubmission> gallery, const std::wstring& path, const DownloadContext& ctx)
{
	ThreadLock<int> progress;
	ThreadLock<int> genericmut;
	progress.operator=(1);

	ctpl::thread_pool threads(std::thread::hardware_concurrency());
	std::vector<std::shared_future<CURLcode>> threadresults;

	MainDlg::getInstance()->m_userQueue.setText(L"Downloading images", m_item, 1);

	for (auto submission : gallery) {
		auto result = threads.push(ThreadedImageDownload, submission, path, &progress, &genericmut);
		threadresults.push_back(result.share());
	}

	int perc = 0;
	while (threads.size() != threads.n_idle() || perc < 100)
	{
		if (perc < 100)
		{
			xlog::Normal("Downloading submissions for '%s'", ctx.username.c_str());

			float size = (float)gallery.size();
			float progsize = (float)progress.operator int();
			perc = (int)std::ceil(((progsize / size) * 100));

			std::wstring percstr = std::to_wstring(perc) + L"%";

			MainDlg::getInstance()->m_userQueue.setText(percstr, m_item, 1);
		}
	}

	int faileddownloads = 0;

	for (auto& code : threadresults)
	{
		if (code.get() != CURLE_OK)
			++faileddownloads;
	}

	return faileddownloads;
}

CURLcode CFADumper::ThreadedImageDownload(int threadID, FASubmission submission, std::wstring path, ThreadLock<int>* progress, ThreadLock<int>* consolelock)
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

	// temporarily unlock & lock mutex for progress bar
	int curprogress = progress->operator int();

	// increment that shit
	curprogress++;

	// unlock & lock mutex for progress bar again and assign the new value
	progress->operator=(curprogress);

	return returncode;
}
