#include "pch.h"

#include "FASubmission.h"

#include "../contrib/rapidjson/ostreamwrapper.h"
#include "../contrib/rapidjson/writer.h"

#include "fADumper.h"
#include "Log.h"


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

		Download();

		log_console(xlog::LogLevel::normal, "Finished processing user gallery '%s'\n", m_uHandle.c_str());
	}
}

int CFADumper::Download()
{
	m_savedir += L"\\" + AnsiToWstring(m_uHandle);
	CreateDirectoryW(m_savedir.c_str(), NULL);

	log_console(xlog::LogLevel::normal, "Processing user gallery '%s'\n", m_uHandle.c_str());
	log_console(xlog::LogLevel::normal, "Dumping gallery '%s' to '%s'\n", m_uHandle.c_str(), WstringToAnsi(m_savedir).c_str());

	if (m_gallery == faGalleryFlags::SCRAPS_ONLY)
	{
		m_savedir += L"\\" + std::wstring(L"Scraps");

		CreateDirectoryW(m_savedir.c_str(), NULL);

		auto fullgallery = GetScrapGallery();

		// download
		DownloadInternal(fullgallery);

		return 0;
	}

	auto fullgallery = GetMainGallery();

	DownloadInternal(fullgallery);

	if (m_gallery != faGalleryFlags::NO_SCRAPS)
	{
		auto scrapgallery = GetScrapGallery();

		m_savedir += L"\\" + std::wstring(L"Scraps");
		CreateDirectoryW(m_savedir.c_str(), NULL);

		DownloadInternal(scrapgallery);
	}



	return 0;
}

std::vector<FASubmission> CFADumper::GetMainGallery()
{
	std::vector<FASubmission> gallery;
	std::vector<int> tempIDs;

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

	log_console(xlog::LogLevel::normal, "Downloading submission pages...");

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

		tempIDs.erase(std::remove_if(tempIDs.begin(), tempIDs.end(), [&](int id)
		{
			return std::find(sfwIDs.begin(), sfwIDs.end(), id) != sfwIDs.end();
		}), tempIDs.end());
	}
	std::printf("Done!\n");

	log_console(xlog::LogLevel::normal, "%d total images\n", tempIDs.size());

	int progress = 1;

	for (auto IDs : tempIDs)
	{
		FASubmission submission(IDs);

		log_console(xlog::LogLevel::normal, "Downloading submission data...");

		int barWidth = 45;
		std::cout << "[";
		int pos = ((float)progress / (float)tempIDs.size()) * barWidth;
		for (int i = 0; i < barWidth; ++i) {
			if (i < pos) std::cout << "=";
			else if (i == pos) std::cout << ">";
			else std::cout << " ";
		}
		std::cout << "] " << int(((float)progress / (float)tempIDs.size()) * 100) << "%\r";
		std::cout.flush();;

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
	log_console(xlog::LogLevel::normal, "Downloading scrap submission pages...");
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

		if (CURLcode err = curlDownload(urlbuff, buffer))
		{
			log_console(xlog::LogLevel::warning, "Couldn't download page %d!, %s, retrying...", curpage, curl_easy_strerror(err));
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

		tempIDs.erase(std::remove_if(tempIDs.begin(), tempIDs.end(), [&](int id)
		{
			return std::find(sfwIDs.begin(), sfwIDs.end(), id) != sfwIDs.end();
		}), tempIDs.end());
	}

	std::printf("Done!\n");


	log_console(xlog::LogLevel::normal, "%d total images\n", tempIDs.size());

	int progress = 1;

	for (auto IDs : tempIDs)
	{
		FASubmission submission(IDs);

		log_console(xlog::LogLevel::normal, "Downloading scrap submission data...");

		int barWidth = 45;
		std::cout << "[";
		int pos = ((float)progress / (float)tempIDs.size()) * barWidth;
		for (int i = 0; i < barWidth; ++i) {
			if (i < pos) std::cout << "=";
			else if (i == pos) std::cout << ">";
			else std::cout << " ";
		}
		std::cout << "] " << int(((float)progress / (float)tempIDs.size()) * 100) << "%\r";
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
	int progress = 1;

	for (auto submission : gallery)
	{
		auto link = submission.GetDownloadLink();
		auto filename = link.substr(link.find_last_of("/") + 1);

		std::string id = std::to_string(submission.GetSubmissionID()); id.append("-");
		filename.insert(0, id);

		std::wstring savedir = m_savedir + std::wstring(L"\\") +
			AnsiToWstring(filename);

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

		log_console(xlog::LogLevel::normal, "Downloading submissions...");

		auto truncate = [&](std::string str, size_t width) -> std::string {
			if (str.length() > width)
				return str.substr(0, width) + "...";
			return str;
		};

		int barWidth = 45;
		std::cout << "[";
		int pos = ((float)progress / (float)gallery.size()) * barWidth;
		for (int i = 0; i < barWidth; ++i) {
			if (i < pos) std::cout << "=";
			else if (i == pos) std::cout << ">";
			else std::cout << " ";
		}
		std::cout << "] " << int(((float)progress / (float)gallery.size()) * 100) << "% (" << truncate(filename, 21) << ")\r";
		std::cout.flush();

		if (auto code = curlDownload(link, savedir))
		{
			log_console(xlog::LogLevel::warning, "Couldn't download submission %s!, %s, retrying...\n", filename.c_str(), curl_easy_strerror(code));
			continue;
		}

		++progress;
	}

	std::printf("\n");

	return 0;
}
