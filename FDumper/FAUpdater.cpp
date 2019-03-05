#include "pch.h"

#include "FAUpdater.h"
#include "FASubmission.h"

#include "../contrib/rapidjson/istreamwrapper.h"

#include "ctpl_stl.hpp"


int CFAUpdater::Action(const DownloadContext& ctx)
{
	//if (m_allusers)
	//{
	//	UpdateAll();
	//}
	//else
	//{
	//	for (; arg.i < arg.c; ++arg.i)
	//	{
	//		std::string name = WstringToAnsi(arg.v[arg.i]);

	//		log_console(xlog::LogLevel::critical, "Processing gallery '%s'\n", name.c_str());

	//		if (Update(name)) {
	//			log_console(xlog::LogLevel::error, "Couldn't update gallery for '%s'\n", name.c_str());
	//		}

	//		log_console(xlog::LogLevel::critical, "Finished processing gallery '%s'\n", name.c_str());
	//		log_console(xlog::LogLevel::verbose, "======================================\n");
	//	}
	//}
	return 0;
}

int CFAUpdater::Update(std::string name)
{
	//const std::wstring wdir = m_path + L"\\" + AnsiToWstring(name);

	//namespace fs = std::filesystem;

	//// paste from cppreference, why so much to just check a folder lol
	//auto directory_exists = [&](const fs::path& p, fs::file_status s = fs::file_status{}) -> bool
	//{
	//	if (fs::status_known(s) ? fs::exists(s) : fs::exists(p))
	//		return true;
	//	else
	//		return false;
	//};


	//if (m_favesonly)
	//{
	//	const std::wstring favorites_dir = wdir + L"\\" + std::wstring(L"Favorites");

	//	if (!directory_exists(favorites_dir))
	//	{
	//		log_console(xlog::LogLevel::critical, "No favorites found for %s!\n", name.c_str());
	//		return 0;
	//	}		

	//	std::ifstream favstream(wdir + L"\\" + std::wstring(L"favorites") + L"\\" + std::wstring(L"favorites.json"));
	//	if (!favstream.good())
	//	{
	//		log_console(xlog::LogLevel::error, "Favorites config not found for %s!\n", name.c_str());
	//		return -1;
	//	}

	//	rapidjson::Document favjson;
	//	rapidjson::IStreamWrapper favisw(favstream);

	//	favjson.ParseStream(favisw);

	//	int favrating = std::stoi(favjson[0].GetString());

	//	auto favorites = GetUserFavoriteGalleryPages(name, favrating);

	//	std::vector<int> currentSubs;

	//	
	//	log_console(xlog::LogLevel::normal, "Scanning submissions in favorites folder for %s...", name.c_str());
	//	startspin();

	//	for (auto& p : fs::directory_iterator(favorites_dir))
	//	{
	//		if (!p.is_regular_file())
	//			continue;

	//		auto filename = p.path().filename().wstring();
	//		auto ext = filename.substr(filename.find_last_of(L".") + 1);

	//		if (!(!ext.compare(L"png") || !ext.compare(L"jpg") || !ext.compare(L"jpeg")
	//			|| !ext.compare(L"gif") || !ext.compare(L"swf")))
	//			continue;

	//		auto faidstr = filename.substr(0, filename.find_first_of(L"-"));
	//		if (faidstr.empty())
	//			continue;
	//		auto faID = std::stoi(faidstr);

	//		currentSubs.push_back(faID);
	//	}

	//	stopspin();
	//	std::printf("Done!\n");

	//	
	//	log_console(xlog::LogLevel::normal, "Comparing new and legacy submission lists...");
	//	startspin();

	//	favorites.erase(std::remove_if(favorites.begin(), favorites.end(), [&](int id)
	//	{
	//		return std::find(currentSubs.begin(), currentSubs.end(), id) != currentSubs.end();
	//	}), favorites.end());

	//	stopspin();
	//	std::printf("Done!\n");

	//	log_console(xlog::LogLevel::critical, "%d submission(s) not found!\n", favorites.size());

	//	if (!favorites.empty())
	//	{
	//		std::vector<FASubmission> subvec; int progress = 1;

	//		if (favorites.size() > 300)
	//		{
	//			while (true)
	//			{
	//				auto time = favorites.size();
	//				// i hate this

	//				auto days = time / (24 * 3600);

	//				time = time % (24 * 3600);
	//				auto hours = time / 3600;

	//				time %= 3600;
	//				auto minutes = time / 60;

	//				time %= 60;
	//				auto seconds = time;

	//				std::string message = "Warning, gathering the submission data will take approximately ";

	//				if (days > 0)
	//				{
	//					message += std::to_string(days) + " days, " + std::to_string(hours) + " hours, and " + std::to_string(minutes) + " minutes.";
	//				}
	//				else if (hours > 0)
	//				{
	//					message += std::to_string(hours) + " hours and " + std::to_string(minutes) + " minutes.";
	//				}
	//				else if (minutes > 0)
	//				{
	//					message += std::to_string(minutes) + " minutes.";
	//				}

	//				message.append(" Proceed? (Y/N)");

	//				log_console(xlog::LogLevel::warning, message.c_str());

	//				std::string command;
	//				std::getline(std::cin, command);

	//				std::printf("\n");

	//				if (command.empty())
	//				{
	//					log_console(xlog::LogLevel::warning, "Invalid response! Please enter Y or N");
	//					continue;
	//				}

	//				if (!command.compare("Y") || !command.compare("y"))
	//					break;
	//				else if (!command.compare("N") || !command.compare("n"))
	//					return 0;
	//				else
	//				{
	//					log_console(xlog::LogLevel::warning, "Invalid response! Please enter Y or N");
	//					continue;
	//				}
	//			}
	//		}

	//		for (auto id : favorites)
	//		{
	//			FASubmission sub(id);

	//			log_console(xlog::LogLevel::normal, "Downloading new favorites submission data...");

	//			int width = 35;
	//			std::printf("|");
	//			int perc = (int)std::ceil((((float)progress / (float)favorites.size()) * 100));
	//			int pos = ((float)progress / (float)favorites.size()) * width;
	//			for (int i = 0; i < width; ++i)
	//			{
	//				if (i < pos) std::printf("%c", 219);
	//				else std::cout << " ";
	//			}
	//			std::cout << "|" << perc << "%\r";
	//			std::cout.flush();

	//			sub.Setup(g_api);

	//			++progress;

	//			subvec.push_back(sub);
	//		}
	//		std::printf("\n");

	//		if (int failed = DownloadInternal(subvec, favorites_dir))
	//		{
	//			// nonzero number of failed downloads, something went wrong, lets tell the user
	//			log_console(xlog::LogLevel::error, "%d submissions failed to download, try updating the gallery to redownload the submissions\n", failed);
	//			return -1;
	//		}
	//	}
	//	return 0;
	//}

	//std::ifstream is(wdir + L"\\" + std::wstring(L"config.json"));
	//if (!is.good())
	//{
	//	log_console(xlog::LogLevel::error, "Gallery config not found for %s!\n", name.c_str());
	//	return 0;
	//}

	//rapidjson::Document doc;
	//rapidjson::IStreamWrapper isw(is);

	//doc.ParseStream(isw);

	//int rating = std::stoi(doc[0].GetString());
	//int gallery = std::stoi(doc[1].GetString());

	//if (gallery == SCRAPS_ONLY)
	//{
	//	const std::wstring scraps_dir = wdir + L"\\" + std::wstring(L"Scraps");

	//	auto scraps = GetUserScrapGalleryPages(name, rating);

	//	std::vector<int> currentImgs;

	//	log_console(xlog::LogLevel::normal, "Scanning images in scrap folder for %s...", name.c_str());
	//	startspin();

	//	for (auto& p : fs::directory_iterator(scraps_dir))
	//	{
	//		if (!p.is_regular_file())
	//			continue;

	//		auto filename = p.path().filename().wstring();
	//		auto ext = filename.substr(filename.find_last_of(L".") + 1);

	//		if (!(!ext.compare(L"png") || !ext.compare(L"jpg") || !ext.compare(L"jpeg")
	//			|| !ext.compare(L"gif") || !ext.compare(L"swf")))
	//			continue;

	//		auto faidstr = filename.substr(0, filename.find_first_of(L"-"));
	//		if (faidstr.empty())
	//			continue;
	//		auto faID = std::stoi(faidstr);

	//		currentImgs.push_back(faID);
	//	}

	//	stopspin();
	//	std::printf("Done!\n");
	//	
	//	log_console(xlog::LogLevel::normal, "Comparing new and legacy submission lists...");
	//	startspin();

	//	scraps.erase(std::remove_if(scraps.begin(), scraps.end(), [&](int id)
	//	{
	//		return std::find(currentImgs.begin(), currentImgs.end(), id) != currentImgs.end();
	//	}), scraps.end());

	//	stopspin();
	//	std::printf("Done!\n");

	//	log_console(xlog::LogLevel::critical, "%d submission(s) not found!\n", scraps.size());

	//	if (!scraps.empty())
	//	{
	//		std::vector<FASubmission> subvec; int progress = 1;
	//		for (auto id : scraps)
	//		{
	//			FASubmission sub(id);

	//			log_console(xlog::LogLevel::normal, "Downloading new scrap submission data...");

	//			int width = 35;
	//			std::printf("|");
	//			int perc = (int)std::ceil((((float)progress / (float)scraps.size()) * 100));
	//			int pos = ((float)progress / (float)scraps.size()) * width;
	//			for (int i = 0; i < width; ++i)
	//			{
	//				if (i < pos) std::printf("%c", 219);
	//				else std::cout << " ";
	//			}
	//			std::cout << "|" << perc << "%\r";
	//			std::cout.flush();

	//			sub.Setup(g_api);

	//			++progress;

	//			subvec.push_back(sub);
	//		}
	//		std::printf("\n");

	//		if (int failed = DownloadInternal(subvec, scraps_dir))
	//		{
	//			// nonzero number of failed downloads, something went wrong, lets tell the user
	//			log_console(xlog::LogLevel::error, "%d submissions failed to download, try updating the gallery to redownload the submissions\n", failed);
	//			return -1;
	//		}
	//	}
	//	return 0;
	//}

	//{
	//	auto maingallery = GetUserMainGalleryPages(name, rating);

	//	std::vector<int> currentImgs;
	//	
	//	log_console(xlog::LogLevel::normal, "Scanning images in main folder for '%s'...", name.c_str());
	//	startspin();

	//	for (auto& p : fs::directory_iterator(wdir))
	//	{
	//		if (!p.is_regular_file())
	//			continue;

	//		auto filename = p.path().filename().wstring();
	//		auto ext = filename.substr(filename.find_last_of(L".") + 1);

	//		if (!(!ext.compare(L"png") || !ext.compare(L"jpg") || !ext.compare(L"jpeg")
	//			|| !ext.compare(L"gif") || !ext.compare(L"swf")))
	//			continue;

	//		auto faidstr = filename.substr(0, filename.find_first_of(L"-"));
	//		if (faidstr.empty())
	//			continue;
	//		auto faID = std::stoi(faidstr);

	//		currentImgs.push_back(faID);
	//	}

	//	stopspin();
	//	std::printf("Done!\n");
	//	
	//	log_console(xlog::LogLevel::normal, "Comparing new and legacy submission lists...");
	//	startspin();

	//	maingallery.erase(std::remove_if(maingallery.begin(), maingallery.end(), [&](int id)
	//	{
	//		return std::find(currentImgs.begin(), currentImgs.end(), id) != currentImgs.end();
	//	}), maingallery.end());

	//	stopspin();
	//	std::printf("Done!\n");

	//	log_console(xlog::LogLevel::critical, "%d submission(s) not found!\n", maingallery.size());

	//	if (!maingallery.empty())
	//	{
	//		std::vector<FASubmission> subvec; int progress = 1;
	//		for (auto id : maingallery)
	//		{
	//			FASubmission sub(id);

	//			log_console(xlog::LogLevel::normal, "Downloading new submission data...");

	//			int width = 35;
	//			std::printf("|");
	//			int perc = (int)std::ceil((((float)progress / (float)maingallery.size()) * 100));
	//			int pos = ((float)progress / (float)maingallery.size()) * width;
	//			for (int i = 0; i < width; ++i)
	//			{
	//				if (i < pos) std::printf("%c", 219);
	//				else std::cout << " ";
	//			}
	//			std::cout << "|" << perc << "%\r";
	//			std::cout.flush();

	//			sub.Setup(g_api);

	//			++progress;

	//			subvec.push_back(sub);
	//		}
	//		std::printf("\n");

	//		if (int failed = DownloadInternal(subvec, wdir))
	//		{
	//			// nonzero number of failed downloads, something went wrong, lets tell the user
	//			log_console(xlog::LogLevel::error, "%d submissions failed to download, try updating the gallery to redownload the submissions\n", failed);
	//			return -1;
	//		}
	//	}
	//}

	//if (gallery != NO_SCRAPS)
	//{		
	//	const std::wstring scraps_dir = wdir + L"\\" + std::wstring(L"Scraps");

	//	auto scraps = GetUserScrapGalleryPages(name, rating);

	//	std::vector<int> currentImgs;
	//
	//	log_console(xlog::LogLevel::normal, "Scanning images in scrap folder for '%s'...", name.c_str());
	//	startspin();

	//	for (auto& p : fs::directory_iterator(scraps_dir))
	//	{
	//		if (!p.is_regular_file())
	//			continue;

	//		auto filename = p.path().filename().wstring();
	//		auto ext = filename.substr(filename.find_last_of(L".") + 1);

	//		if (!(!ext.compare(L"png") || !ext.compare(L"jpg") || !ext.compare(L"jpeg")
	//			|| !ext.compare(L"gif") || !ext.compare(L"swf")))
	//			continue;

	//		auto faidstr = filename.substr(0, filename.find_first_of(L"-"));
	//		if (faidstr.empty())
	//			continue;
	//		auto faID = std::stoi(faidstr);

	//		currentImgs.push_back(faID);
	//	}

	//	stopspin();
	//	std::printf("Done!\n");

	//	log_console(xlog::LogLevel::normal, "Comparing new and legacy submission lists...");
	//	startspin();

	//	scraps.erase(std::remove_if(scraps.begin(), scraps.end(), [&](int id)
	//	{
	//		return std::find(currentImgs.begin(), currentImgs.end(), id) != currentImgs.end();
	//	}), scraps.end());

	//	stopspin();
	//	std::printf("Done!\n");

	//	log_console(xlog::LogLevel::critical, "%d submission(s) not found!\n", scraps.size());

	//	if (!scraps.empty())
	//	{
	//		std::vector<FASubmission> subvec; int progress = 1;
	//		for (auto id : scraps)
	//		{
	//			FASubmission sub(id);

	//			log_console(xlog::LogLevel::normal, "Downloading new scrap submission data...");

	//			int width = 35;
	//			std::printf("|");
	//			int perc = (int)std::ceil((((float)progress / (float)scraps.size()) * 100));
	//			int pos = ((float)progress / (float)scraps.size()) * width;
	//			for (int i = 0; i < width; ++i)
	//			{
	//				if (i < pos) std::printf("%c", 219);
	//				else std::cout << " ";
	//			}
	//			std::cout << "|" << perc << "%\r";
	//			std::cout.flush();

	//			sub.Setup(g_api);

	//			++progress;

	//			subvec.push_back(sub);
	//		}
	//		std::printf("\n");

	//		if (int failed = DownloadInternal(subvec, scraps_dir))
	//		{
	//			// nonzero number of failed downloads, something went wrong, lets tell the user
	//			log_console(xlog::LogLevel::error, "%d submissions failed to download, try updating the gallery to redownload the submissions\n", failed);
	//			return -1;
	//		}
	//	}
	//}

	//if (!m_nofaves)
	//{
	//	const std::wstring favorites_dir = wdir + L"\\" + std::wstring(L"Favorites");

	//	if (!directory_exists(favorites_dir))
	//	{
	//		log_console(xlog::LogLevel::critical, "No favorites found for %s!\n", name.c_str());
	//		return 0;
	//	}

	//	std::ifstream favstream(wdir + L"\\" + std::wstring(L"favorites") + L"\\" + std::wstring(L"favorites.json"));
	//	if (!favstream.good())
	//	{
	//		log_console(xlog::LogLevel::error, "Favorites config not found for %s!\n", name.c_str());
	//		return -1;
	//	}

	//	rapidjson::Document favjson;
	//	rapidjson::IStreamWrapper favisw(favstream);

	//	favjson.ParseStream(favisw);

	//	int favrating = std::stoi(doc[0].GetString());

	//	auto favorites = GetUserFavoriteGalleryPages(name, favrating);

	//	std::vector<int> currentSubs;

	//	startspin();
	//	log_console(xlog::LogLevel::normal, "Scanning submissions in favorites folder for %s...", name.c_str());

	//	for (auto& p : fs::directory_iterator(favorites_dir))
	//	{
	//		if (!p.is_regular_file())
	//			continue;

	//		auto filename = p.path().filename().wstring();
	//		auto ext = filename.substr(filename.find_last_of(L".") + 1);

	//		if (!(!ext.compare(L"png") || !ext.compare(L"jpg") || !ext.compare(L"jpeg")
	//			|| !ext.compare(L"gif") || !ext.compare(L"swf")))
	//			continue;

	//		auto faidstr = filename.substr(0, filename.find_first_of(L"-"));
	//		if (faidstr.empty())
	//			continue;
	//		auto faID = std::stoi(faidstr);

	//		currentSubs.push_back(faID);
	//	}
	//
	//	std::printf("Done!\n");
	//	stopspin();

	//	startspin();
	//	log_console(xlog::LogLevel::normal, "Comparing new and legacy submission lists...");

	//	favorites.erase(std::remove_if(favorites.begin(), favorites.end(), [&](int id)
	//	{
	//		return std::find(currentSubs.begin(), currentSubs.end(), id) != currentSubs.end();
	//	}), favorites.end());

	//	stopspin();
	//	std::printf("Done!\n");

	//	log_console(xlog::LogLevel::critical, "%d submission(s) not found!\n", favorites.size());

	//	if (!favorites.empty())
	//	{
	//		std::vector<FASubmission> subvec; int progress = 1;

	//		if (favorites.size() > 300)
	//		{
	//			while (true)
	//			{
	//				auto time = favorites.size();
	//				// i hate this

	//				auto days = time / (24 * 3600);

	//				time = time % (24 * 3600);
	//				auto hours = time / 3600;

	//				time %= 3600;
	//				auto minutes = time / 60;

	//				time %= 60;
	//				auto seconds = time;

	//				std::string message = "Warning, gathering the submission data will take approximately ";

	//				if (days > 0)
	//				{
	//					message += std::to_string(days) + " days, " + std::to_string(hours) + " hours, and " + std::to_string(minutes) + " minutes.";
	//				}
	//				else if (hours > 0)
	//				{
	//					message += std::to_string(hours) + " hours and " + std::to_string(minutes) + " minutes.";
	//				}
	//				else if (minutes > 0)
	//				{
	//					message += std::to_string(minutes) + " minutes.";
	//				}

	//				message.append(" Proceed? (Y/N)");

	//				log_console(xlog::LogLevel::warning, message.c_str());

	//				std::string command;
	//				std::getline(std::cin, command);

	//				std::printf("\n");

	//				if (command.empty())
	//				{
	//					log_console(xlog::LogLevel::warning, "Invalid response! Please enter Y or N");
	//					continue;
	//				}

	//				if (!command.compare("Y") || !command.compare("y"))
	//					break;
	//				else if (!command.compare("N") || !command.compare("n"))
	//					return 0;
	//				else
	//				{
	//					log_console(xlog::LogLevel::warning, "Invalid response! Please enter Y or N");
	//					continue;
	//				}
	//			}
	//		}

	//		for (auto id : favorites)
	//		{
	//			FASubmission sub(id);

	//			log_console(xlog::LogLevel::normal, "Downloading new favorites submission data...");

	//			int width = 35;
	//			std::printf("|");
	//			int perc = (int)std::ceil((((float)progress / (float)favorites.size()) * 100));
	//			int pos = ((float)progress / (float)favorites.size()) * width;
	//			for (int i = 0; i < width; ++i)
	//			{
	//				if (i < pos) std::printf("%c", 219);
	//				else std::cout << " ";
	//			}
	//			std::cout << "|" << perc << "%\r";
	//			std::cout.flush();

	//			sub.Setup(g_api);

	//			++progress;

	//			subvec.push_back(sub);
	//		}
	//		std::printf("\n");

	//		if (int failed  = DownloadInternal(subvec, favorites_dir))
	//		{
	//			// nonzero number of failed downloads, something went wrong, lets tell the user
	//			log_console(xlog::LogLevel::error, "%d submissions failed to download, try updating the gallery to redownload the submissions\n", failed);
	//			return -1;
	//		}
	//	}
	//}
	return 0;
}

void CFAUpdater::UpdateAll()
{
	//namespace fs = std::filesystem;

	//std::vector<std::string> validusers;

	//for (auto& p : fs::directory_iterator(m_path))
	//{
	//	if (!p.is_directory())
	//		continue;

	//	auto dir = p.path().wstring();
	//	auto user = dir.substr(dir.find_last_of(L"\\") + 1);
	//	std::ifstream ifs(dir + L"\\" + std::wstring(L"config.json"));
	//	if (ifs.good()) {
	//		validusers.push_back(WstringToAnsi(user));
	//	}
	//	ifs.close();
	//}

	//std::for_each(validusers.begin(), validusers.end(), [&](std::string user)
	//{
	//	log_console(xlog::LogLevel::critical, "Processing gallery '%s'\n", user.c_str());

	//	if (Update(user)) {
	//		log_console(xlog::LogLevel::error, "Couldn't update gallery for '%s'\n", user.c_str());
	//	}

	//	log_console(xlog::LogLevel::critical, "Finished processing gallery '%s'\n", user.c_str());
	//	log_console(xlog::LogLevel::verbose, "======================================\n");
	//});
}


std::vector<int> CFAUpdater::GetUserMainGalleryPages(std::string user, int rating)
{
	std::vector<int> gallery;

	//auto curlDownload = [&](const std::string& url, std::string& buffer, int& httpcode) -> CURLcode
	//{
	//	CURL* pCurl = curl_easy_init();

	//	curl_easy_setopt(pCurl, CURLOPT_URL, url.c_str());
	//	curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, writebuffercallback);
	//	curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &buffer);
	//	CURLcode code = curl_easy_perform(pCurl);

	//	if (code == CURLE_OK) 
	//		curl_easy_getinfo(pCurl, CURLINFO_RESPONSE_CODE, &httpcode);
	//	
	//	curl_easy_cleanup(pCurl);
	//	return code;
	//};


	//log_console(xlog::LogLevel::normal, "Downloading new submission pages...");
	//startspin();

	//int curpage = 1;
	//while (true)
	//{
	//	char urlbuff[200] = {};
	//	if (rating == SFW_ONLY) {
	//		sprintf_s(urlbuff, "%s/user/%s/gallery.json?sfw=1&page=%d", g_api.c_str(), user.c_str(), curpage);
	//	}
	//	else {
	//		sprintf_s(urlbuff, "%s/user/%s/gallery.json?page=%d", g_api.c_str(), user.c_str(), curpage);
	//	}

	//	std::string buffer;
	//	int httpcode = 0;

	//	if (CURLcode err = curlDownload(urlbuff, buffer, httpcode))
	//	{
	//		stopspin();
	//		log_console(xlog::LogLevel::warning, "\nCouldn't download page %d!, %s, retrying...", curpage, curl_easy_strerror(err));
	//		startspin();
	//		continue;
	//	}

	//	if (httpcode != 200)
	//	{
	//		stopspin();
	//		log_console(xlog::LogLevel::warning, "\ndownload error! retrying...");
	//		startspin();
	//		continue;
	//	}

	//	rapidjson::Document doc;
	//	doc.Parse(buffer.c_str());

	//	for (auto itr = doc.Begin(); itr != doc.End(); ++itr)
	//	{
	//		std::string jsonElm = itr->GetString();
	//		int subID = std::stoi(jsonElm);

	//		gallery.push_back(subID);
	//	}

	//	++curpage;
	//	if (doc.Size() < 72)
	//		break;
	//}

	//if (rating == NSFW_ONLY)
	//{
	//	curpage = 1;
	//	std::vector<int> sfwIDs;

	//	while (true)
	//	{
	//		char urlbuff[200] = {};
	//		sprintf_s(urlbuff, "%s/user/%s/gallery.json?sfw=1&page=%d", g_api.c_str(), user.c_str(), curpage);

	//		std::string buffer;
	//		int httpcode = 0;

	//		if (CURLcode err = curlDownload(urlbuff, buffer, httpcode))
	//		{
	//			stopspin();
	//			log_console(xlog::LogLevel::warning, "\nCouldn't download page %d!, %s, retrying...", curpage, curl_easy_strerror(err));
	//			stopspin();
	//			continue;
	//		}

	//		if (httpcode != 200)
	//		{
	//			stopspin();
	//			log_console(xlog::LogLevel::warning, "\ndownload error! retrying...");
	//			startspin();
	//			continue;
	//		}

	//		rapidjson::Document doc;
	//		doc.Parse(buffer.c_str());

	//		for (auto itr = doc.Begin(); itr != doc.End(); ++itr)
	//		{
	//			std::string jsonElm = itr->GetString();
	//			int subID = std::stoi(jsonElm);

	//			sfwIDs.push_back(subID);
	//		}

	//		auto docsize = doc.Size();
	//		if (docsize < 72)
	//			break;
	//		++curpage;
	//	}

	//	gallery.erase(std::remove_if(gallery.begin(), gallery.end(), [&](int sub)
	//	{
	//		return std::find(sfwIDs.begin(), sfwIDs.end(), sub) != sfwIDs.end();
	//	}), gallery.end());
	//}

	//stopspin();
	//std::printf("Done!\n");

	//log_console(xlog::LogLevel::critical, "%d total images\n", gallery.size());

	return gallery;
}

std::vector<int> CFAUpdater::GetUserScrapGalleryPages(std::string user, int rating)
{
	std::vector<int> scraps;

	//log_console(xlog::LogLevel::normal, "Downloading new scrap submission pages...");
	//startspin();

	//auto curlDownload = [&](const std::string& url, std::string& buffer, int& httpcode) -> CURLcode
	//{
	//	CURL* pCurl = curl_easy_init();

	//	curl_easy_setopt(pCurl, CURLOPT_URL, url.c_str());
	//	curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, writebuffercallback);
	//	curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &buffer);
	//	CURLcode code = curl_easy_perform(pCurl);
	//	
	//	if (code == CURLE_OK) 
	//		curl_easy_getinfo(pCurl, CURLINFO_RESPONSE_CODE, &httpcode);

	//	curl_easy_cleanup(pCurl);
	//	return code;
	//};
	//const DownloadContext& ctx
	//int curpage = 1;
	//while (true)
	//{
	//	char urlbuff[200] = {};
	//	if (rating == SFW_ONLY) {
	//		sprintf_s(urlbuff, "%s/user/%s/scraps.json?sfw=1&page=%d", g_api.c_str(), user.c_str(), curpage);
	//	}
	//	else {
	//		sprintf_s(urlbuff, "%s/user/%s/scraps.json?page=%d", g_api.c_str(), user.c_str(), curpage);
	//	}

	//	std::string buffer;
	//	int httpcode = 0;

	//	if (CURLcode err = curlDownload(urlbuff, buffer, httpcode))
	//	{
	//		startspin();
	//		log_console(xlog::LogLevel::warning, "\nCouldn't download page %d!, %s, retrying...\n", curpage, curl_easy_strerror(err));
	//		stopspin();
	//		continue;
	//	}

	//	if (httpcode != 200)
	//	{
	//		startspin();
	//		log_console(xlog::LogLevel::warning, "\ndownload error! retrying...\n");
	//		stopspin();
	//		continue;
	//	}

	//	rapidjson::Document doc;
	//	doc.Parse(buffer.c_str());

	//	for (auto itr = doc.Begin(); itr != doc.End(); ++itr)
	//	{
	//		std::string jsonElm = itr->GetString();
	//		int subID = std::stoi(jsonElm);

	//		scraps.push_back(subID);
	//	}

	//	++curpage;
	//	if (doc.Size() < 72)
	//		break;
	//}

	//if (rating == NSFW_ONLY)
	//{
	//	curpage = 1;
	//	std::vector<int> sfwIDs;

	//	while (true)
	//	{
	//		char urlbuff[200] = {};
	//		sprintf_s(urlbuff, "%s/user/%s/scraps.json?sfw=1&page=%d", g_api.c_str(), user.c_str(), curpage);

	//		std::string buffer;
	//		int httpcode = 0;

	//		if (CURLcode err = curlDownload(urlbuff, buffer, httpcode))
	//		{
	//			startspin();
	//			log_console(xlog::LogLevel::warning, "\nCouldn't download page %d!, %s, retrying...\n", curpage, curl_easy_strerror(err));
	//			stopspin();
	//			continue;
	//		}

	//		if (httpcode != 200)
	//		{
	//			startspin();
	//			log_console(xlog::LogLevel::warning, "\ndownload error! retrying...\n");
	//			stopspin();
	//			continue;
	//		}

	//		rapidjson::Document doc;
	//		doc.Parse(buffer.c_str());

	//		for (auto itr = doc.Begin(); itr != doc.End(); ++itr)
	//		{
	//			std::string jsonElm = itr->GetString();
	//			int subID = std::stoi(jsonElm);

	//			sfwIDs.push_back(subID);
	//		}

	//		auto docsize = doc.Size();
	//		if (docsize < 72)
	//			break;

	//		++curpage;
	//	}

	//	scraps.erase(std::remove_if(scraps.begin(), scraps.end(), [&](int sub)
	//	{
	//		return std::find(sfwIDs.begin(), sfwIDs.end(), sub) != sfwIDs.end();
	//	}), scraps.end());

	//}

	//stopspin();
	//std::printf("Done!\n");

	//log_console(xlog::LogLevel::critical, "%d total images\n", scraps.size());

	return scraps;
}

std::vector<int> CFAUpdater::GetUserFavoriteGalleryPages(std::string user, int rating)
{
	std::vector<int> favorites;

	/*auto curlDownload = [&](const std::string& url, std::string& buffer, int& httpcode) -> CURLcode
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

	log_console(xlog::LogLevel::normal, "Downloading favorites pages...");
	startspin();

	int favorite_id = 0;
	while (true)
	{
		char urlbuff[200] = {};

		if (rating == SFW_ONLY)
			sprintf_s(urlbuff, "%s/user/%s/favorites.json?full=1&sfw=1&next=%d", g_api.c_str(), user.c_str(), favorite_id);
		else
			sprintf_s(urlbuff, "%s/user/%s/favorites.json?full=1&next=%d", g_api.c_str(), user.c_str(), favorite_id);

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
			log_console(xlog::LogLevel::warning, "\ndownload error! retrying...\n");
			stopspin();
			continue;
		}

		rapidjson::Document doc;
		doc.Parse(buffer.c_str());

		for (auto itr = doc.Begin(); itr != doc.End(); ++itr)
		{
			int subID = std::stoi(itr->FindMember("id")->value.GetString());
			favorite_id = std::stoi(itr->FindMember("fav_id")->value.GetString());

			favorites.push_back(subID);
		}

		auto docsize = doc.Size();
		if (docsize < 72)
			break;
	}

	if (rating == NSFW_ONLY)
	{
		std::vector<int> sfwIDs;

		favorite_id = 0;
		while (true)
		{
			char urlbuff[200] = {};
			sprintf_s(urlbuff, "%s/user/%s/favorites.json?full=1&sfw=1&next=%d", g_api.c_str(), user.c_str(), favorite_id);

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

		favorites.erase(std::remove_if(favorites.begin(), favorites.end(), [&](int id)
		{
			return std::find(sfwIDs.begin(), sfwIDs.end(), id) != sfwIDs.end();
		}), favorites.end());
	}

	stopspin();
	std::printf("Done!\n");

	log_console(xlog::LogLevel::critical, "%d total submissions\n", favorites.size());*/

	return favorites;
}

int CFAUpdater::DownloadInternal(std::vector<FASubmission> gallery, std::wstring folder)
{
	/*ThreadLock<int> progress;
	ThreadLock<int> consolelock;
	progress.operator=(1);

	ctpl::thread_pool threads(std::thread::hardware_concurrency());
	std::vector<std::shared_future<CURLcode>> threadresults;

	for (auto submission : gallery) {
		auto result = threads.push(ThreadedImageDownload, submission, folder, &progress, &consolelock);
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

	return faileddownloads;*/

	return 0;
}

CURLcode CFAUpdater::ThreadedImageDownload(int threadID, FASubmission submission, std::wstring path, ThreadLock<int>* progress, ThreadLock<int>* consolelock)
{
	//auto curlDownload = [&](const std::string& url, const std::wstring& path) -> CURLcode
	//{
	//	FILE* fp = nullptr;
	//	_wfopen_s(&fp, path.c_str(), L"wb");

	//	CURL* pCurl = curl_easy_init();

	//	curl_easy_setopt(pCurl, CURLOPT_URL, url.c_str());
	//	curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, writefilecallback);
	//	curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, fp);

	//	CURLcode code = curl_easy_perform(pCurl);

	//	return std::fclose(fp), code;
	//};

	//auto truncate = [&](std::string str, size_t width) -> std::string {
	//	if (str.length() > width)
	//		return str.substr(0, width) + "...";
	//	return str;
	//};

	//auto link = submission.GetDownloadLink();
	//auto filename = submission.GetFilename();

	//std::string id = std::to_string(submission.GetSubmissionID()); id.append("-");
	//filename.insert(0, id);

	//std::wstring savedir = path + L"\\" + AnsiToWstring(filename);

	//CURLcode returncode = curlDownload(link, savedir);

	//if (returncode != CURLE_OK)
	//{
	//	consolelock->execute([&](int s) {
	//		log_console(xlog::LogLevel::error, "\nFailed to download '%s', %s!\n", truncate(submission.GetCDNFilename(), 21).c_str(), curl_easy_strerror(returncode));
	//	});

	//	// Delete the file so we can tell the user to update later
	//	DeleteFile(savedir.c_str());
	//}

	//// temporarily unlock & lock mutex for progress bar
	//int curprogress = progress->operator int();

	//// increment that shit
	//curprogress++;

	//// unlock & lock mutex for progress bar again and assign the new value
	//progress->operator=(curprogress);

	//return returncode;

	return CURLE_OK;
}
