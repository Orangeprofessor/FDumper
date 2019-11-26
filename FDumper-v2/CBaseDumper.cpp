#include "stdafx.h"

#include "CBaseDumper.h"

#include "../contrib/rapidjson/istreamwrapper.h"
#include "../contrib/rapidjson/ostreamwrapper.h"
#include "../contrib/rapidjson/writer.h"
#include "../contrib/rapidjson/document.h"

void CBaseDumper::Main(const DownloadContext& ctx)
{
	curl_ptr curl;

	curl.SetUrl(m_config.config().apiaddress);
	curl.SetOpt(CURLOPT_TIMEOUT, 10);
	curl.SetOpt(CURLOPT_NOBODY, 1);

	std::string buffer;
	auto code = curl.DownloadToBuffer(buffer);

	if (code != CURLE_OK) {
		xlog::Error("Connection error! %s, aborting...", curl.GetError(code).c_str());
		return;
	}
	Action(ctx);
}

bool CBaseDumper::ValidUser(std::string user)
{
	char urlbuff[200];
	sprintf_s(urlbuff, "%s/user/%s.json", m_config.config().apiaddress.c_str(), user.c_str());

	curl_ptr curl; std::string temp;

	curl.SetUrl(urlbuff);
	curl.DownloadToBuffer(temp);

	if (curl.GetHTTPCode() != 200)
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

void CBaseDumper::CreateGalleryCfg(const DownloadContext& ctx)
{
	auto path = ctx.path;
	path.operator/=(L"userdata\\config.json");

	std::ofstream ofs(path);

	rapidjson::Document config;
	config.Parse("{}");

	rapidjson::Value cfghandle, cfgrating, cfggallery;

	cfghandle.SetString(ctx.username.c_str(), ctx.username.length(), config.GetAllocator());
	config.AddMember("handle", cfghandle, config.GetAllocator());

	cfgrating.SetInt(ctx.ratings);
	config.AddMember("rating", cfgrating, config.GetAllocator());

	cfggallery.SetInt(ctx.gallery);
	config.AddMember("gallery", cfggallery, config.GetAllocator());

	rapidjson::OStreamWrapper osw(ofs);

	rapidjson::Writer<rapidjson::OStreamWrapper> writer(osw);
	config.Accept(writer);

	ofs.close();
}
