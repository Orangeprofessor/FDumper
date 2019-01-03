#include "pch.h"

#include "FADumper.h"
#include "FASubmission.h"

#include "../contrib/rapidjson/ostreamwrapper.h"
#include "../contrib/rapidjson/writer.h"
#include "../contrib/rapidjson/istreamwrapper.h"
#include "../contrib/rapidjson/document.h"

#include "ctpl_stl.hpp"


void CFADumper::PrintDescription()
{
	log_console(xlog::LogLevel::normal,
		"FurAffinity user gallery dumper\n"
		"Usage: dump [flags] [user1] [user2] ...\n\n"
		"Content filtering: (dont specify one for dumping all submissions)\n"
		" --sfw-only		Only dump SFW submissions\n"
		" --nsfw-only		Only dump NSFW submissions\n"
		" --scraps-only		Only dump images from the scraps folder\n"
		" --no-scraps		Only dump images from the main gallery\n\n");

	BaseClass::PrintDescription();
}

bool CFADumper::Argument(arg_t & arg)
{
	std::wstring s = arg.v[arg.i];

	if (!s.compare(L"--sfw-only"))
	{
		m_rating = SFW_ONLY;
		return ++arg.i <= arg.c;
	}
	else if (!s.compare(L"--nsfw-only"))
	{
		m_rating = NSFW_ONLY;
		return ++arg.i <= arg.c;
	}
	else if (!s.compare(L"--scraps-only"))
	{
		m_gallery = SCRAPS_ONLY;
		return ++arg.i <= arg.c;
	}
	else if (!s.compare(L"--no-scraps"))
	{
		m_gallery = NO_SCRAPS;
		return ++arg.i <= arg.c;
	}
	else
	{
		return BaseClass::Argument(arg);
	}
}

void CFADumper::Action(arg_t & arg)
{
	for (; arg.i < arg.c; ++arg.i)
	{
		m_savedir.erase(); m_savedir.assign(m_path);

		m_uHandle = WstringToAnsi(arg.v[arg.i]);

		if (!ValidUser(m_uHandle))
		{
			log_console(xlog::LogLevel::error, "User '%s' doesn't exist!\n", m_uHandle.c_str());
			continue;
		}

		CreateDirectoryW((m_savedir + L"\\" + AnsiToWstring(m_uHandle)).c_str(), NULL);

		char jsonbuff[80];
		sprintf_s(jsonbuff, "[\n \"%d\", \"%d\" \n]", m_rating, m_gallery);

		rapidjson::Document doc;
		doc.Parse(jsonbuff);

		std::ofstream ofs(m_savedir + L"\\" + AnsiToWstring(m_uHandle) + L"\\" + std::wstring(L"config.json"));
		rapidjson::OStreamWrapper osw(ofs);

		rapidjson::Writer<rapidjson::OStreamWrapper> writer(osw);
		doc.Accept(writer);

		ofs.close();

		log_console(xlog::LogLevel::critical, "Processing user gallery '%s'\n", m_uHandle.c_str());

		if (Download())
		{
			log_console(xlog::LogLevel::error, "Errors while processing user gallery '%s'\n", m_uHandle.c_str());
			continue;
		}

		log_console(xlog::LogLevel::critical, "Finished processing user gallery '%s'\n", m_uHandle.c_str());
	}
}

int CFADumper::Download()
{
	m_savedir += L"\\" + AnsiToWstring(m_uHandle);
	CreateDirectoryW(m_savedir.c_str(), NULL);

	log_console(xlog::LogLevel::normal, "Dumping gallery '%s' to '%s'\n", m_uHandle.c_str(), WstringToAnsi(m_savedir).c_str());

	if (m_gallery == faGalleryFlags::SCRAPS_ONLY)
	{
		m_savedir += L"\\" + std::wstring(L"Scraps");

		CreateDirectoryW(m_savedir.c_str(), NULL);

		auto scrapgallery = GetScrapGallery();

		if (scrapgallery.empty())
		{
			log_console(xlog::LogLevel::warning, "Nothing to download!\n", m_uHandle.c_str());
			return 0;
		}

		// download
		if (int failed = DownloadInternal(scrapgallery))
		{
			// nonzero number of failed downloads, something went wrong, lets tell the user
			log_console(xlog::LogLevel::error, "%d submissions failed to download, try updating the gallery to redownload the submissions\n", failed);
			return -1;
		}

		return 0;
	}

	auto fullgallery = GetMainGallery();

	if (fullgallery.empty())
	{
		log_console(xlog::LogLevel::warning, "Nothing to download!\n", m_uHandle.c_str());
		return 0;
	}

	// download
	if (int failed = DownloadInternal(fullgallery))
	{
		// nonzero number of failed downloads, something went wrong, lets tell the user
		log_console(xlog::LogLevel::error, "%d submissions failed to download, try updating the gallery to redownload the submissions\n", failed);
		return -1;
	}

	if (m_gallery != faGalleryFlags::NO_SCRAPS)
	{
		auto scrapgallery = GetScrapGallery();

		m_savedir += L"\\" + std::wstring(L"Scraps");
		CreateDirectoryW(m_savedir.c_str(), NULL);

		if (scrapgallery.empty())
		{
			log_console(xlog::LogLevel::warning, "Nothing to download!\n", m_uHandle.c_str());
			return 0;
		}

		// download
		if (int failed = DownloadInternal(scrapgallery))
		{
			// nonzero number of failed downloads, something went wrong, lets tell the user
			log_console(xlog::LogLevel::error, "%d submissions failed to download, try updating the gallery to redownload the submissions\n", failed);
			return -1;
		}
	}

	return 0;
}

std::vector<FASubmission> CFADumper::GetMainGallery()
{
	std::vector<FASubmission> gallery;
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

	// lil spinny spin spin (idk why i do this)
	auto terminate = std::make_unique<bool>(false);

	ctpl::thread_pool spinner(1);
	auto spinny = [&](int id, std::unique_ptr<bool>* term) -> void {
		while (!(*term->get()) == true)
		{
			static int pos = 0;
			char cursor[4] = { '/','-','\\','|' };
			printf("%c\b", cursor[pos]);
			std::cout.flush();
			pos = (pos + 1) % 4;
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
	};

	auto startspin = [&]() -> void {
		*terminate = false;
		spinner.push(spinny, &terminate);
	};

	auto stopspin = [&]() -> void {
		*terminate = true;
		spinner.stop();
	};

	log_console(xlog::LogLevel::normal, "Downloading submission pages...");
	startspin();

	int curpage = 1;
	while (true)
	{
		char urlbuff[200] = {};
		if (m_rating == SFW_ONLY) {
			sprintf_s(urlbuff, "%s/user/%s/gallery.json?sfw=1&page=%d", m_api.c_str(), m_uHandle.c_str(), curpage);
		}
		else {
			sprintf_s(urlbuff, "%s/user/%s/gallery.json?page=%d", m_api.c_str(), m_uHandle.c_str(), curpage);
		}

		std::string buffer;
		int httpcode = 0;

		if (CURLcode err = curlDownload(urlbuff, buffer, httpcode))
		{
			stopspin();
			log_console(xlog::LogLevel::warning, "\nCouldn't download page %d!, %s, retrying...", curpage, curl_easy_strerror(err));
			startspin();
			continue;
		}

		if (httpcode != 200)
		{
			stopspin();
			log_console(xlog::LogLevel::warning, "\ndownload error! retrying...");
			startspin();
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

	if (m_rating == NSFW_ONLY)
	{
		curpage = 1;
		std::vector<int> sfwIDs;

		while (true)
		{
			char urlbuff[200] = {};
			sprintf_s(urlbuff, "%s/user/%s/gallery.json?sfw=1&page=%d", m_api.c_str(), m_uHandle.c_str(), curpage);

			std::string buffer;
			int httpcode = 0;

			if (CURLcode err = curlDownload(urlbuff, buffer, httpcode))
			{
				stopspin();
				log_console(xlog::LogLevel::warning, "\nCouldn't download page %d!, %s, retrying...", curpage, curl_easy_strerror(err));
				startspin();
				continue;
			}

			if (httpcode != 200)
			{
				stopspin();
				log_console(xlog::LogLevel::warning, "\ndownload error! retrying...");
				startspin();
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

	stopspin();
	std::printf("Done!\n");

	log_console(xlog::LogLevel::critical, "%d total images\n", tempIDs.size());

	if (tempIDs.empty())
		return std::vector<FASubmission>();

	int progress = 1;

	for (auto IDs : tempIDs)
	{
		FASubmission submission(IDs);

		log_console(xlog::LogLevel::normal, "Downloading submission data...");

		int width = 35;
		std::printf("|");
		int perc = (int)std::ceil((((float)progress / (float)tempIDs.size()) * 100));
		int pos = ((float)progress / (float)tempIDs.size()) * width;
		for (int i = 0; i < width; ++i)
		{
			if (i < pos) std::printf("%c", 219);
			else std::cout << " ";
		}
		std::cout << "|" << perc << "%\r";
		std::cout.flush();

		submission.Setup(m_api);

		++progress;

		gallery.push_back(submission);
	}

	std::printf("\n");

	return gallery;
}

std::vector<FASubmission> CFADumper::GetScrapGallery()
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

	// lil spinny spin spin (idk why i do this)
	auto terminate = std::make_unique<bool>(false);

	ctpl::thread_pool spinner(1);
	auto spinny = [&](int id, std::unique_ptr<bool>* term) -> void {
		while (!(*term->get()) == true)
		{
			static int pos = 0;
			char cursor[4] = { '/','-','\\','|' };
			printf("%c\b", cursor[pos]);
			std::cout.flush();
			pos = (pos + 1) % 4;
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
	};

	auto startspin = [&]() -> void {
		*terminate = false;
		spinner.push(spinny, &terminate);
	};

	auto stopspin = [&]() -> void {
		*terminate = true;
		spinner.stop();
	};


	int curpage = 1;
	log_console(xlog::LogLevel::normal, "Downloading scrap submission pages...");
	startspin();

	while (true)
	{
		char urlbuff[200] = {};

		if (m_rating == SFW_ONLY) {
			sprintf_s(urlbuff, "%s/user/%s/scraps.json?sfw=1&page=%d", m_api.c_str(), m_uHandle.c_str(), curpage);
		}
		else {
			sprintf_s(urlbuff, "%s/user/%s/scraps.json?page=%d", m_api.c_str(), m_uHandle.c_str(), curpage);
		}

		std::string buffer;
		int httpcode = 0;

		if (CURLcode err = curlDownload(urlbuff, buffer, httpcode))
		{
			stopspin();
			log_console(xlog::LogLevel::warning, "\nCouldn't download page %d!, %s, retrying...", curpage, curl_easy_strerror(err));
			startspin();
			continue;
		}

		if (httpcode != 200)
		{
			stopspin();
			log_console(xlog::LogLevel::warning, "\ndownload error! retrying...\n");
			startspin();
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

	if (m_rating == NSFW_ONLY)
	{
		curpage = 1;
		std::vector<int> sfwIDs;

		while (true)
		{
			char urlbuff[200] = {};
			sprintf_s(urlbuff, "%s/user/%s/scraps.json?sfw=1&page=%d", m_api.c_str(), m_uHandle.c_str(), curpage);

			std::string buffer;
			int httpcode = 0;

			if (CURLcode err = curlDownload(urlbuff, buffer, httpcode))
			{
				startspin();
				log_console(xlog::LogLevel::warning, "\nCouldn't download page %d!, %s, retrying...\n", curpage, curl_easy_strerror(err));
				stopspin();
				continue;
			}

			if (httpcode != 200)
			{
				startspin();
				log_console(xlog::LogLevel::warning, "\ndownload error! retrying...\n");
				stopspin();
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

	stopspin();
	std::printf("Done!\n");

	log_console(xlog::LogLevel::critical, "%d total images\n", tempIDs.size());

	if (tempIDs.empty())
		return std::vector<FASubmission>();

	int progress = 1;

	for (auto IDs : tempIDs)
	{
		FASubmission submission(IDs);

		log_console(xlog::LogLevel::normal, "Downloading scrap submission data...");

		int width = 35;
		std::printf("|");
		int perc = (int)std::ceil((((float)progress / (float)tempIDs.size()) * 100));
		int pos = ((float)progress / (float)tempIDs.size()) * width;
		for (int i = 0; i < width; ++i)
		{
			if (i < pos) std::printf("%c", 219);
			else std::cout << " ";
		}
		std::cout << "|" << perc << "%\r";
		std::cout.flush();

		submission.Setup(m_api);

		++progress;

		scraps.push_back(submission);
	}

	std::printf("\n");

	return scraps;
}

int CFADumper::DownloadInternal(std::vector<FASubmission> gallery)
{
	ThreadLock<int> progress;
	ThreadLock<int> genericmut;
	progress.operator=(1);

	ctpl::thread_pool threads(std::thread::hardware_concurrency());
	std::vector<std::shared_future<CURLcode>> threadresults;

	for (auto submission : gallery) {
		auto result = threads.push(ThreadedImageDownload, submission, m_savedir, &progress, &genericmut);
		threadresults.push_back(result.share());
	}

	int perc = 0;
	while (threads.size() != threads.n_idle() || perc < 100)
	{
		if (perc < 100)
		{
			log_console(xlog::LogLevel::normal, "Downloading submissions...");

			int width = 35;
			std::printf("|");

			float size = (float)gallery.size();
			float progsize = (float)progress.operator int();
			perc = (int)std::ceil(((progsize / size) * 100));
			int pos = (progress / size) * width;
			for (int i = 0; i < width; ++i)
			{
				if (i < pos) std::printf("%c", 219);
				else std::cout << " ";
			}
			std::cout << "|" << perc << "%\r";
			std::cout.flush();
		}
	}

	std::printf("\n");

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

	CURLcode returncode = curlDownload(link, savedir);

	if (returncode != CURLE_OK)
	{
		consolelock->execute([&](int s) {
			log_console(xlog::LogLevel::error, "\nFailed to download '%s', %s!\n", truncate(submission.GetCDNFilename(), 21).c_str(), curl_easy_strerror(returncode));
		});

		// Delete the file so we can tell the user to update later
		DeleteFile(savedir.c_str());
	}

	// temporarily unlock & lock mutex for progress bar
	int curprogress = progress->operator int();

	// increment that shit
	curprogress++;

	// unlock & lock mutex for progress bar again and assign the new value
	progress->operator=(curprogress);

	return returncode;
}
