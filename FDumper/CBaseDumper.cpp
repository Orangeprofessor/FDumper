#include "pch.h"

#include "CBaseDumper.h"
#include <ShlObj.h>

#include "../contrib/rapidjson/istreamwrapper.h"
#include "../contrib/rapidjson/ostreamwrapper.h"
#include "../contrib/rapidjson/writer.h"
#include "../contrib/rapidjson/document.h"

#include "control/Message.hpp"

void CBaseDumper::Main(const DownloadContext& ctx)
{
	CURL* pCurl = curl_easy_init();

	curl_easy_setopt(pCurl, CURLOPT_URL, m_config.config().apiaddress.c_str());
	curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 10);
	curl_easy_setopt(pCurl, CURLOPT_NOBODY, 1);

	auto res = curl_easy_perform(pCurl);

	std::string err = curl_easy_strerror(res);

	curl_easy_cleanup(pCurl);

	if (res != CURLE_OK) {		
		xlog::Error("Connection error! %s, aborting...", err.c_str());
		return;
	}

	Action(ctx);
}

bool CBaseDumper::ValidUser(std::string user)
{
	char urlbuff[200];
	sprintf_s(urlbuff, "%s/user/%s.json", m_config.config().apiaddress.c_str(), user.c_str());

	int httpcode; std::string temp;
	CURL* pCurl = curl_easy_init();

	curl_easy_setopt(pCurl, CURLOPT_URL, urlbuff);
	curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, writebuffercallback);
	curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &temp);
	CURLcode code = curl_easy_perform(pCurl);

	if (code == CURLE_OK)
		curl_easy_getinfo(pCurl, CURLINFO_RESPONSE_CODE, &httpcode);
	else
		return curl_easy_cleanup(pCurl), false;

	curl_easy_cleanup(pCurl);

	if (httpcode != 200)
		return false;

	return true;
}

std::pair<int, int> CBaseDumper::ParseGalleryCfg(const std::wstring& path)
{
	std::ifstream ifs(path);
	
	rapidjson::Document config;
	rapidjson::IStreamWrapper isw(ifs);

	config.ParseStream(isw);

	ifs.close();
	
	int rating = config.FindMember("rating")->value.GetInt();
	int gallery = config.FindMember("gallery")->value.GetInt();
	
	return std::make_pair(rating, gallery);
}

void CBaseDumper::CreateGalleryCfg(const std::wstring& path, int rating, int gallery)
{
	std::ofstream ofs(path + L"\\" + std::wstring(L"config.json"));

	rapidjson::Document config;
	config.Parse("{}");

	rapidjson::Value cfgrating, cfggallery;

	cfgrating.SetInt(rating);
	config.AddMember("rating", cfgrating, config.GetAllocator());

	cfggallery.SetInt(gallery);
	config.AddMember("gallery", cfggallery, config.GetAllocator());
	
	rapidjson::OStreamWrapper osw(ofs);

	rapidjson::Writer<rapidjson::OStreamWrapper> writer(osw);
	config.Accept(writer);

	ofs.close();
}
