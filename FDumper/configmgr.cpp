#include "pch.h"

#include "configmgr.h"
#include "CBaseDumper.h"

#include "../contrib/rapidjson/istreamwrapper.h"
#include "../contrib/rapidjson/ostreamwrapper.h"
#include "../contrib/rapidjson/writer.h"
#include "../contrib/rapidjson/document.h"

bool ConfigMgr::Save()
{
	try
	{
		wchar_t temppath[MAX_PATH];
		GetTempPath(MAX_PATH, temppath);

		std::wstring filepath(temppath);
		filepath.append(L"\\fdumperconfig.json");

		rapidjson::Document config;
		config.Parse("{}");

		rapidjson::Value savedir, apiaddr, asksave;
		// I would have done this in unicode but i couldnt figure out how to write it to a document
		auto defualtsave = WstringToAnsi(m_config.defaultDir);
		savedir.SetString(defualtsave.c_str(), defualtsave.length(), config.GetAllocator());
		config.AddMember("defaultsaves", savedir, config.GetAllocator());

		apiaddr.SetString(m_config.apiaddress.c_str(), m_config.apiaddress.length(), config.GetAllocator());
		config.AddMember("apiaddress", apiaddr, config.GetAllocator());

		asksave.SetBool(m_config.askforsave);
		config.AddMember("askforsave", asksave, config.GetAllocator());

		std::ofstream ofs(filepath);
		rapidjson::OStreamWrapper osw(ofs);

		rapidjson::Writer<rapidjson::OStreamWrapper> writer(osw);
		config.Accept(writer);

		ofs.close();

		return true;
	}
	catch (const std::exception& err)
	{
		// ill handle this later
		return false;
	}
}

bool ConfigMgr::Load()
{
	try
	{
		wchar_t temppath[MAX_PATH];
		GetTempPath(MAX_PATH, temppath);

		std::wstring filepath(temppath);
		filepath.append(L"\\fdumperconfig.json");

		rapidjson::Document config;

		std::ifstream ifs(filepath);
		rapidjson::IStreamWrapper isw(ifs);

		config.ParseStream(isw);

		std::string ansidir = config.FindMember("defaultsaves")->value.GetString();
		m_config.defaultDir = AnsiToWstring(ansidir);

		m_config.apiaddress = config.FindMember("apiaddress")->value.GetString();

		m_config.askforsave = config.FindMember("askforsave")->value.GetBool();

		return true;
	}
	catch (const std::exception& err)
	{
		return false;
	}
}
