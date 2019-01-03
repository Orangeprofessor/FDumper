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

		if (!ValidUser(m_uHandle))
		{
			log_console(xlog::LogLevel::error, "User '%s' doesn't exist!\n", m_uHandle.c_str());
			continue;
		}

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

		log_console(xlog::LogLevel::critical, "Processing user favorites '%s'\n", m_uHandle.c_str());

		if (Download())
		{
			log_console(xlog::LogLevel::error, "Errors while processing user favorites '%s'\n", m_uHandle.c_str());
			continue;
		}

		log_console(xlog::LogLevel::critical, "Finished processing user favorites '%s'\n", m_uHandle.c_str());
	}
}

int CFAFavorites::Download()
{
	// Create user folder
	m_savedir += L"\\" + AnsiToWstring(m_uHandle);
	CreateDirectoryW(m_savedir.c_str(), NULL);

	// Now the favorites folder
	m_savedir += L"\\" + std::wstring(L"favorites");
	CreateDirectoryW(m_savedir.c_str(), NULL);

	log_console(xlog::LogLevel::normal, "Dumping %s's favorites to '%s'\n", m_uHandle.c_str(), WstringToAnsi(m_savedir).c_str());

	auto favorites = GetFavoritesGallery();

	if (favorites.empty())
	{
		log_console(xlog::LogLevel::critical, "No images to download\n");
		return 0;
	}

	if (int failed = DownloadInternal(favorites))
	{
		// nonzero number of failed downloads, something went wrong, lets tell the user
		log_console(xlog::LogLevel::error, "%d submissions failed to download, try updating the gallery to redownload the submissions\n", failed);
		return -1;
	}
	return 0;
}

std::vector<FASubmission> CFAFavorites::GetFavoritesGallery()
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

	log_console(xlog::LogLevel::normal, "Downloading favorites pages...");
	startspin();

	int favorite_id = 0;
	while (true)
	{
		char urlbuff[200] = {};

		if (m_rating == SFW_ONLY)
			sprintf_s(urlbuff, "%s/user/%s/favorites.json?full=1&sfw=1&next=%d", m_api.c_str(), m_uHandle.c_str(), favorite_id);
		else
			sprintf_s(urlbuff, "%s/user/%s/favorites.json?full=1&next=%d", m_api.c_str(), m_uHandle.c_str(), favorite_id);

		std::string buffer;
		int httpcode = 0;
	
		if (CURLcode err = curlDownload(urlbuff, buffer, httpcode))
		{
			startspin();
			log_console(xlog::LogLevel::warning, "\nCouldn't download favorite page id %d!, %s, retrying...", favorite_id, curl_easy_strerror(err));
			stopspin();
			continue;
		}

		if (httpcode != 200)
		{
			startspin();
			log_console(xlog::LogLevel::warning, "\ndownload error! retrying...");
			stopspin();
			continue;
		}

		rapidjson::Document doc;
		doc.Parse(buffer.c_str());

		for (auto itr = doc.Begin(); itr != doc.End(); ++itr)
		{
			int subID = std::stoi(itr->FindMember("id")->value.GetString());
			favorite_id = std::stoi(itr->FindMember("fav_id")->value.GetString());

			tempIDs.push_back(subID);
		}

		auto docsize = doc.Size();
		if (docsize < 72)
			break;
	}

	if (m_rating == NSFW_ONLY)
	{
		std::vector<int> sfwIDs;

		favorite_id = 0;
		while (true)
		{
			char urlbuff[200] = {};
			sprintf_s(urlbuff, "%s/user/%s/favorites.json?full=1&sfw=1&next=%d", m_api.c_str(), m_uHandle.c_str(), favorite_id);

			std::string buffer;
			int httpcode = 0;
	
			if (CURLcode err = curlDownload(urlbuff, buffer, httpcode))
			{
				startspin();
				log_console(xlog::LogLevel::warning, "\nCouldn't download favorite page id %d!, %s, retrying...", favorite_id, curl_easy_strerror(err));
				stopspin();
				continue;
			}

			if (httpcode != 200)
			{
				startspin();
				log_console(xlog::LogLevel::warning, "\ndownload error! retrying...");
				stopspin();
				continue;
			}

			rapidjson::Document doc;
			doc.Parse(buffer.c_str());

			for (auto itr = doc.Begin(); itr != doc.End(); ++itr)
			{
				int subID = std::stoi(itr->FindMember("id")->value.GetString());
				favorite_id = std::stoi(itr->FindMember("fav_id")->value.GetString());

				sfwIDs.push_back(subID);
			}

			auto docsize = doc.Size();
			if (docsize < 72)
				break;
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

	if (tempIDs.size() > 300) // 5 minutes
	{
		while (true)
		{
			auto time = tempIDs.size();
			// i hate this

			auto days = time / (24 * 3600);

			time = time % (24 * 3600);
			auto hours = time / 3600;

			time %= 3600;
			auto minutes = time / 60;

			time %= 60;
			auto seconds = time;

			std::string message = "Warning, gathering the submission data will take approximately ";

			if (days > 0)
			{
				message += std::to_string(days) + " days, " + std::to_string(hours) + " hours, and " + std::to_string(minutes) + " minutes.";
			}
			else if (hours > 0)
			{
				message += std::to_string(hours) + " hours and " + std::to_string(minutes) + " minutes.";
			}
			else if (minutes > 0)
			{
				message += std::to_string(minutes) + " minutes.";
			}

			message.append(" Proceed? (Y/N)");

			log_console(xlog::LogLevel::warning, message.c_str());

			std::string command;
			std::getline(std::cin, command);

			std::printf("\n");

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

int CFAFavorites::DownloadInternal(std::vector<FASubmission> gallery)
{
	ThreadLock<int> progress;
	ThreadLock<int> consolelock;
	progress.operator=(1);

	ctpl::thread_pool threads(std::thread::hardware_concurrency());
	std::vector<std::shared_future<CURLcode>> threadresults;

	for (auto submission : gallery) {
		auto result = threads.push(ThreadedImageDownload, submission, m_savedir, &progress, &consolelock);
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

CURLcode CFAFavorites::ThreadedImageDownload(int threadID, FASubmission submission, std::wstring path, ThreadLock<int>* progress, ThreadLock<int>* consolelock)
{
	auto curlDownload = [&](const std::string& url, const std::wstring& path) -> CURLcode
	{
		FILE* fp = nullptr;
		_wfopen_s(&fp, path.c_str(), L"wb");

		CURL* pCurl = curl_easy_init();

		curl_easy_setopt(pCurl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, writefilecallback);
		curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, fp);

		CURLcode code = curl_easy_perform(pCurl);

		return std::fclose(fp), code;
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
