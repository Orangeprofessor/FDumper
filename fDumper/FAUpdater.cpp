#include "pch.h"

#include "FAUpdater.h"
#include "FASubmission.h"

#include <filesystem>

#include "../contrib/rapidjson/istreamwrapper.h"

#include "ctpl_stl.hpp"

void CFAUpdater::PrintDescription()
{
	log_console(xlog::LogLevel::normal,
		"FurAffinity user gallery updater\n"
		"Usage: update [flags] [user1] [user2] ...\n\n"
		"Content filtering:\n"
		" --all-users		Update all galleries in folder (don't specify a user for this one)\n\n"
		" --no-favorites	Don't update favorites for gallery"
		" --favorites-only	Only update favorites for gallery");

	BaseClass::PrintDescription();
}

bool CFAUpdater::Argument(arg_t & arg)
{
	std::wstring s = arg.v[arg.i];

	if (!s.compare(L"--all-users"))
	{
		m_allusers = true;
		return ++arg.i <= arg.c;
	}
	return BaseClass::Argument(arg);
}

void CFAUpdater::Action(arg_t & arg)
{
	if (m_allusers)
	{
		UpdateAll();
	}
	else
	{
		for (; arg.i < arg.c; ++arg.i)
		{
			std::string name = WstringToAnsi(arg.v[arg.i]);

			if (Update(name)) {
				log_console(xlog::LogLevel::error, "Couldn't update gallery for '%s'\n", name.c_str());
			}
		}
	}
}

int CFAUpdater::Update(std::string name)
{
	log_console(xlog::LogLevel::critical, "Processing gallery '%s'\n", name.c_str());

	std::wstring dir = m_path + L"\\" + AnsiToWstring(name);

	std::ifstream is(dir + L"\\" + std::wstring(L"config.json"));
	if (!is.good())
	{
		log_console(xlog::LogLevel::error, "Config not found for %s!\n", name.c_str());
		return -1;
	}


	rapidjson::Document doc;
	rapidjson::IStreamWrapper isw(is);

	doc.ParseStream(isw);

	int rating = std::stoi(doc[0].GetString());
	int gallery = std::stoi(doc[1].GetString());

	namespace fs = std::filesystem;

	if (gallery == SCRAPS_ONLY)
	{
		dir += L"\\" + std::wstring(L"Scraps");

		auto scraps = GetUserScrapGalleryPages(name, rating);

		std::vector<int> currentImgs;

		log_console(xlog::LogLevel::normal, "Scanning images in scrap folder for %s...", name.c_str());

		for (auto& p : fs::directory_iterator(dir))
		{
			if (!p.is_regular_file())
				continue;

			auto filename = p.path().filename().wstring();
			auto ext = filename.substr(filename.find_last_of(L".") + 1);

			if (!(!ext.compare(L"png") || !ext.compare(L"jpg") || !ext.compare(L"jpeg")
				|| !ext.compare(L"gif") || !ext.compare(L"swf")))
				continue;

			auto faidstr = filename.substr(0, filename.find_first_of(L"-"));
			if (faidstr.empty())
				continue;
			auto faID = std::stoi(faidstr);

			currentImgs.push_back(faID);
		}

		std::printf("Done!\n");

		log_console(xlog::LogLevel::normal, "Comparing new and legacy submission lists...");

		scraps.erase(std::remove_if(scraps.begin(), scraps.end(), [&](int id)
		{
			return std::find(currentImgs.begin(), currentImgs.end(), id) != currentImgs.end();
		}), scraps.end());

		std::printf("Done!\n");

		log_console(xlog::LogLevel::warning, "%d submission(s) not found!\n", scraps.size());

		if (!scraps.empty())
		{
			std::vector<FASubmission> subvec; int progress = 1;
			for (auto id : scraps)
			{
				FASubmission sub(id);

				log_console(xlog::LogLevel::warning, "Downloading new scrap submission data...");

				int barWidth = 45;
				std::cout << "[";
				int pos = ((float)progress / (float)scraps.size()) * barWidth;
				for (int i = 0; i < barWidth; ++i) {
					if (i < pos) std::cout << "=";
					else if (i == pos) std::cout << ">";
					else std::cout << " ";
				}
				std::cout << "] " << int(((float)progress / (float)scraps.size()) * 100) << "%\r";
				std::cout.flush();

				sub.Setup(m_api);

				++progress;

				subvec.push_back(sub);
			}
			std::printf("\n");

			DownloadInternal(subvec, dir);
		}

		log_console(xlog::LogLevel::critical, "Finished processing gallery '%s'\n", name.c_str());
		log_console(xlog::LogLevel::verbose, "======================================\n");
		return 0;
	}

	{
		auto maingallery = GetUserMainGalleryPages(name, rating);

		std::vector<int> currentImgs;

		log_console(xlog::LogLevel::normal, "Scanning images in main folder for '%s'...", name.c_str());

		for (auto& p : fs::directory_iterator(dir))
		{
			if (!p.is_regular_file())
				continue;

			auto filename = p.path().filename().wstring();
			auto ext = filename.substr(filename.find_last_of(L".") + 1);

			if (!(!ext.compare(L"png") || !ext.compare(L"jpg") || !ext.compare(L"jpeg")
				|| !ext.compare(L"gif") || !ext.compare(L"swf")))
				continue;

			auto faidstr = filename.substr(0, filename.find_first_of(L"-"));
			if (faidstr.empty())
				continue;
			auto faID = std::stoi(faidstr);

			currentImgs.push_back(faID);
		}
		std::printf("Done!\n");

		log_console(xlog::LogLevel::normal, "Comparing new and legacy submission lists...");

		maingallery.erase(std::remove_if(maingallery.begin(), maingallery.end(), [&](int id)
		{
			return std::find(currentImgs.begin(), currentImgs.end(), id) != currentImgs.end();
		}), maingallery.end());

		std::printf("Done!\n");

		log_console(xlog::LogLevel::normal, "%d submission(s) not found!\n", maingallery.size());

		if (!maingallery.empty())
		{
			std::vector<FASubmission> subvec; int progress = 1;
			for (auto id : maingallery)
			{
				FASubmission sub(id);

				log_console(xlog::LogLevel::normal, "Downloading new submission data...");

				int barWidth = 45;
				std::cout << "[";
				int pos = ((float)progress / (float)maingallery.size()) * barWidth;
				for (int i = 0; i < barWidth; ++i) {
					if (i < pos) std::cout << "=";
					else if (i == pos) std::cout << ">";
					else std::cout << " ";
				}
				std::cout << "] " << int(((float)progress / (float)maingallery.size()) * 100) << "%\r";
				std::cout.flush();

				sub.Setup(m_api);

				++progress;

				subvec.push_back(sub);
			}
			std::printf("\n");

			DownloadInternal(subvec, dir);
		}
	}



	if (gallery != NO_SCRAPS)
	{
		dir += L"\\" + std::wstring(L"Scraps");

		auto scraps = GetUserScrapGalleryPages(name, rating);

		std::vector<int> currentImgs;

		log_console(xlog::LogLevel::normal, "Scanning images in scrap folder for '%s'...", name.c_str());

		for (auto& p : fs::directory_iterator(dir))
		{
			if (!p.is_regular_file())
				continue;

			auto filename = p.path().filename().wstring();
			auto ext = filename.substr(filename.find_last_of(L".") + 1);

			if (!(!ext.compare(L"png") || !ext.compare(L"jpg") || !ext.compare(L"jpeg")
				|| !ext.compare(L"gif") || !ext.compare(L"swf")))
				continue;

			auto faidstr = filename.substr(0, filename.find_first_of(L"-"));
			if (faidstr.empty())
				continue;
			auto faID = std::stoi(faidstr);

			currentImgs.push_back(faID);
		}

		std::printf("Done!\n");

		log_console(xlog::LogLevel::normal, "Comparing new and legacy submission lists...");

		scraps.erase(std::remove_if(scraps.begin(), scraps.end(), [&](int id)
		{
			return std::find(currentImgs.begin(), currentImgs.end(), id) != currentImgs.end();
		}), scraps.end());

		std::printf("Done!\n");

		log_console(xlog::LogLevel::normal, "%d submission(s) not found!\n", scraps.size());

		if (!scraps.empty())
		{
			std::vector<FASubmission> subvec; int progress = 1;
			for (auto id : scraps)
			{
				FASubmission sub(id);

				log_console(xlog::LogLevel::normal, "Downloading new scrap submission data...");

				int barWidth = 45;
				std::cout << "[";
				int pos = ((float)progress / (float)scraps.size()) * barWidth;
				for (int i = 0; i < barWidth; ++i) {
					if (i < pos) std::cout << "=";
					else if (i == pos) std::cout << ">";
					else std::cout << " ";
				}
				std::cout << "] " << int(((float)progress / (float)scraps.size()) * 100) << "%\r";
				std::cout.flush();

				sub.Setup(m_api);

				++progress;

				subvec.push_back(sub);
			}
			std::printf("\n");

			DownloadInternal(subvec, dir);
		}
	}

	log_console(xlog::LogLevel::critical, "Finished processing gallery '%s'\n", name.c_str());
	log_console(xlog::LogLevel::verbose, "======================================\n");
	return 0;
}

int CFAUpdater::UpdateAll()
{
	namespace fs = std::filesystem;

	std::vector<std::string> validusers;

	for (auto& p : fs::directory_iterator(m_path))
	{
		if (!p.is_directory())
			continue;

		auto dir = p.path().wstring();
		auto user = dir.substr(dir.find_last_of(L"\\") + 1);
		std::ifstream ifs(dir + L"\\" + std::wstring(L"config.json"));
		if (ifs.good()) {
			validusers.push_back(WstringToAnsi(user));
		}
		ifs.close();

	}

	std::for_each(validusers.begin(), validusers.end(), [&](std::string user) {
		Update(user);
	});

	return 0;
}


std::vector<int> CFAUpdater::GetUserMainGalleryPages(std::string user, int rating)
{
	std::vector<int> gallery;

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


	log_console(xlog::LogLevel::normal, "Downloading new submission pages...");

	int curpage = 1;
	while (true)
	{
		char urlbuff[200] = {};
		if (rating == SFW_ONLY) {
			sprintf_s(urlbuff, "%s/user/%s/gallery.json?sfw=1&page=%d", m_api.c_str(), user.c_str(), curpage);
		}
		else {
			sprintf_s(urlbuff, "%s/user/%s/gallery.json?page=%d", m_api.c_str(), user.c_str(), curpage);
		}

		std::string buffer;

		if (CURLcode err = curlDownload(urlbuff, buffer))
		{
			log_console(xlog::LogLevel::warning, "Couldn't download page %d!, %s, retrying...\n", curpage, curl_easy_strerror(err));
			continue;
		}

		rapidjson::Document doc;
		doc.Parse(buffer.c_str());

		for (auto itr = doc.Begin(); itr != doc.End(); ++itr)
		{
			std::string jsonElm = itr->GetString();
			int subID = std::stoi(jsonElm);

			gallery.push_back(subID);
		}

		++curpage;
		if (doc.Size() < 72)
			break;
	}
	std::printf("Done!\n");

	if (rating == NSFW_ONLY)
	{
		curpage = 1;
		std::vector<int> sfwIDs;

		while (true)
		{
			char urlbuff[200] = {};
			sprintf_s(urlbuff, "%s/user/%s/gallery.json?sfw=1&page=%d", m_api.c_str(), user.c_str(), curpage);

			std::string buffer;

			if (CURLcode err = curlDownload(urlbuff, buffer))
			{
				log_console(xlog::LogLevel::warning, "Couldn't download page %d!, %s, retrying...\n", curpage, curl_easy_strerror(err));
				continue;
			}

			if (buffer == "FAExport encounter an internal error") {
				std::printf("\n");
				log_console(xlog::LogLevel::warning, "download error! retrying...\n");
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



		gallery.erase(std::remove_if(gallery.begin(), gallery.end(), [&](int sub)
		{
			return std::find(sfwIDs.begin(), sfwIDs.end(), sub) != sfwIDs.end();
		}), gallery.end());
	}

	log_console(xlog::LogLevel::normal, "%d total images\n", gallery.size());

	return gallery;
}

std::vector<int> CFAUpdater::GetUserScrapGalleryPages(std::string user, int rating)
{
	std::vector<int> scraps;
	log_console(xlog::LogLevel::normal, "Downloading new scrap submission pages...");

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

	int curpage = 1;
	while (true)
	{
		char urlbuff[200] = {};
		if (rating == SFW_ONLY) {
			sprintf_s(urlbuff, "%s/user/%s/scraps.json?sfw=1&page=%d", m_api.c_str(), user.c_str(), curpage);
		}
		else {
			sprintf_s(urlbuff, "%s/user/%s/scraps.json?page=%d", m_api.c_str(), user.c_str(), curpage);
		}

		std::string buffer;

		if (CURLcode err = curlDownload(urlbuff, buffer))
		{
			log_console(xlog::LogLevel::warning, "Couldn't download page %d!, %s, retrying...\n", curpage, curl_easy_strerror(err));
			continue;
		}

		rapidjson::Document doc;
		doc.Parse(buffer.c_str());

		for (auto itr = doc.Begin(); itr != doc.End(); ++itr)
		{
			std::string jsonElm = itr->GetString();
			int subID = std::stoi(jsonElm);

			scraps.push_back(subID);
		}

		++curpage;
		if (doc.Size() < 72)
			break;
	}
	std::printf("Done!\n");

	if (rating == NSFW_ONLY)
	{
		curpage = 1;
		std::vector<int> sfwIDs;

		while (true)
		{
			char urlbuff[200] = {};
			sprintf_s(urlbuff, "%s/user/%s/scraps.json?sfw=1&page=%d", m_api.c_str(), user.c_str(), curpage);

			std::string buffer;

			if (CURLcode err = curlDownload(urlbuff, buffer))
			{
				log_console(xlog::LogLevel::warning, "Couldn't download page %d!, %s, retrying...\n", curpage, curl_easy_strerror(err));
				continue;
			}

			if (buffer == "FAExport encounter an internal error") {
				std::printf("\n");
				log_console(xlog::LogLevel::warning, "download error! retrying...\n");
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

		scraps.erase(std::remove_if(scraps.begin(), scraps.end(), [&](int sub)
		{
			return std::find(sfwIDs.begin(), sfwIDs.end(), sub) != sfwIDs.end();
		}), scraps.end());

	}

	log_console(xlog::LogLevel::normal, "%d total images\n", scraps.size());

	return scraps;
}

int CFAUpdater::DownloadInternal(std::vector<FASubmission> gallery, std::wstring folder)
{
	ThreadLock<int> progress;
	progress.operator=(1);

	ctpl::thread_pool threads(std::thread::hardware_concurrency());

	for (auto submission : gallery)
		threads.push(ThreadedImageDownload, submission, folder, &progress);

	do
	{
		log_console(xlog::LogLevel::normal, "Downloading submissions...");
		int barWidth = 45;
		std::cout << "[";
		int pos = ((float)progress.operator int() / (float)gallery.size()) * barWidth;
		for (int i = 0; i < barWidth; ++i) {
			if (i < pos) std::cout << "=";
			else if (i == pos) std::cout << ">";
			else std::cout << " ";
		}
		std::cout << "] " << int(((float)progress.operator int() / (float)gallery.size()) * 100) << "%\r";
		std::cout.flush();

	} while (threads.size() != threads.n_idle());

	std::printf("\n");

	std::printf("\n");

	return 0;
}

CURLcode CFAUpdater::ThreadedImageDownload(int threadID, FASubmission submission, std::wstring path, ThreadLock<int>* progress)
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

	auto truncate = [&](std::string str, size_t width) -> std::string {
		if (str.length() > width)
			return str.substr(0, width) + "...";
		return str;
	};

	auto link = submission.GetDownloadLink();
	auto filename = submission.GetFilename();

	std::string id = std::to_string(submission.GetSubmissionID()); id.append("-");
	filename.insert(0, id);

	std::wstring savedir = path + L"\\" + AnsiToWstring(filename);

	CURLcode returncode;

	for (int i = 5; i > 1; --i)
	{
		returncode = curlDownload(link, savedir);

		if (returncode == CURLE_OK)
			break;

		// Gotta be grammar correct
		if (i == 1)
			log_console(xlog::LogLevel::warning, "\n Download error for '%s', %d retry left\n", truncate(submission.GetCDNFilename(), 21).c_str(), i);
		else
			log_console(xlog::LogLevel::warning, "\n Download error for '%s', %d retries left\n", truncate(submission.GetCDNFilename(), 21).c_str(), i);
	}

	if (returncode != CURLE_OK)
		log_console(xlog::LogLevel::error, "\n Failed to download '%s', %s!\n", truncate(submission.GetCDNFilename(), 21), curl_easy_strerror(returncode));

	int curprogress = progress->operator int(); curprogress++;
	progress->operator=(curprogress);
	return returncode;
}
