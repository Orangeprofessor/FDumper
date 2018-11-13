#include "pch.h"

#include "FAFavorites.h"
#include "FASubmission.h"

#include "../contrib/rapidjson/ostreamwrapper.h"
#include "../contrib/rapidjson/writer.h"
#include "../contrib/rapidjson/istreamwrapper.h"
#include "../contrib/rapidjson/document.h"

#include "ctpl_stl.hpp"

void CFAFavorites::PrintDescription()
{
	log_console(xlog::LogLevel::normal,
		"FurAffinity user favorites dumper\n"
		"Usage: favorites [flags] [user1] [user2] ...\n\n"
		"Content filtering: (dont specify one for dumping all favorites)\n"
		" --sfw-only		Only dump SFW submissions\n"
		" --nsfw-only		Only dump NSFW submissions\n\n");

	BaseClass::PrintDescription();
}

bool CFAFavorites::Argument(arg_t & arg)
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
	else
	{
		return BaseClass::Argument(arg);
	}
}

void CFAFavorites::Action(arg_t & arg)
{
	for (; arg.i < arg.c; ++arg.i)
	{
		m_savedir.erase(); m_savedir.assign(m_path);

		m_uHandle = WstringToAnsi(arg.v[arg.i]);
		CreateDirectoryW((m_savedir + L"\\" + AnsiToWstring(m_uHandle) + L"\\" + std::wstring(L"favorites")).c_str(), NULL);

		char jsonbuff[80];
		sprintf_s(jsonbuff, "[\n \"%d\" \n]", m_rating);

		rapidjson::Document doc;
		doc.Parse(jsonbuff);

		std::ofstream ofs(m_savedir + L"\\" + AnsiToWstring(m_uHandle) + L"\\" + std::wstring(L"favorites") + L"\\" + std::wstring(L"favorites.json"));
		rapidjson::OStreamWrapper osw(ofs);

		rapidjson::Writer<rapidjson::OStreamWrapper> writer(osw);
		doc.Accept(writer);

		ofs.close();
		Download();

		log_console(xlog::LogLevel::critical, "Finished processing user favorites '%s'\n", m_uHandle.c_str());
	}
}

void CFAFavorites::Download()
{
	m_savedir += L"\\" + AnsiToWstring(m_uHandle) + L"\\" + std::wstring(L"favorites");
	CreateDirectoryW(m_savedir.c_str(), NULL);

	log_console(xlog::LogLevel::critical, "Processing user favorites '%s'\n", m_uHandle.c_str());
	log_console(xlog::LogLevel::normal, "Dumping %s's favorites to '%s'\n", m_uHandle.c_str(), WstringToAnsi(m_savedir).c_str());

	auto favorites = GetFavoritesGallery();

	if (favorites.empty())
	{
		log_console(xlog::LogLevel::normal, "No images to download\n");
		return;
	}

	DownloadInternal(favorites);
}

std::vector<FASubmission> CFAFavorites::GetFavoritesGallery()
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

	log_console(xlog::LogLevel::normal, "Downloading favorites pages...");

	int curpage = 1;
	std::string prevbuffer;
	while (false)
	{
		char urlbuff[200] = {};

		if (m_rating == SFW_ONLY)
			sprintf_s(urlbuff, "%s/user/%s/favorites.json?sfw=1&page=%d", m_api.c_str(), m_uHandle.c_str(), curpage);
		else
			sprintf_s(urlbuff, "%s/user/%s/favorites.json?page=%d", m_api.c_str(), m_uHandle.c_str(), curpage);

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

		if (!buffer.compare(prevbuffer))
			break;

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

		prevbuffer.assign(buffer);
		++curpage;
	}

	for (int i = 0; i < 788492; ++i) {
		tempIDs.push_back(i);
	}

	prevbuffer.clear();
	if (m_rating == NSFW_ONLY)
	{
		curpage = 1;
		std::vector<int> sfwIDs;

		while (true)
		{
			char urlbuff[200] = {};
			sprintf_s(urlbuff, "%s/user/%s/favorites.json?sfw=1&page=%d", m_api.c_str(), m_uHandle.c_str(), curpage);

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

			if (!buffer.compare(prevbuffer))
				break;

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
			prevbuffer.assign(buffer);
			++curpage;
		}

		tempIDs.erase(std::remove_if(tempIDs.begin(), tempIDs.end(), [&](int id)
		{
			return std::find(sfwIDs.begin(), sfwIDs.end(), id) != sfwIDs.end();
		}), tempIDs.end());
	}
	std::printf("Done!\n");

	log_console(xlog::LogLevel::normal, "%d total images\n", tempIDs.size());

	if (tempIDs.size() > 300) // 5 minutes
	{
		while (true)
		{
			char timebuff[32] = {};
			auto time = tempIDs.size();

			time_t seconds(time); tm res;
			gmtime_s(&res, &seconds);

			std::string message = "Warning, gathering the submission data will take approximately ";

			if (res.tm_mday > 0) 
			{
				message += std::to_string(res.tm_mday) + " days, " + std::to_string(res.tm_hour) + " hours, and " + std::to_string(res.tm_min) + " minutes.";
			}
			else if (res.tm_hour > 0) 
			{
				message += std::to_string(res.tm_hour) +  " hours and " + std::to_string(res.tm_min) + " minutes.";
			}
			else if (res.tm_min > 0) 
			{
				message += std::to_string(res.tm_min) + " minutes.";
			}

			message.append(" Proceed? (Y/N)\n");

			log_console(xlog::LogLevel::warning, message.c_str());

			std::string command;
			std::getline(std::cin, command);

			if (command.empty())
			{
				log_console(xlog::LogLevel::warning, "Invalid response! Please enter Y or N");
				continue;
			}

			if (!command.compare("Y") || !command.compare("y"))
				break;
			else if (!command.compare("N") || !command.compare("n"))
				return std::vector<FASubmission>();	
			else
			{
				log_console(xlog::LogLevel::warning, "Invalid response! Please enter Y or N");
				continue;
			}
		}	
	}

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

void CFAFavorites::DownloadInternal(std::vector<FASubmission> gallery)
{
	ThreadLock<int> progress;
	progress.operator=(1);

	ctpl::thread_pool threads(std::thread::hardware_concurrency());

	for (auto submission : gallery) {
		threads.push(ThreadedImageDownload, submission, m_savedir, &progress);
	}

	do
	{
		log_console(xlog::LogLevel::normal, "Downloading favorites...");
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
}

CURLcode CFAFavorites::ThreadedImageDownload(int threadID, FASubmission submission, std::wstring path, ThreadLock<int>* progress)
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
