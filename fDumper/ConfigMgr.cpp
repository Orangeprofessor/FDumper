#include "pch.h"

#include "ConfigMgr.h"
#include <libda3m0n/Misc/Utils.h>

#define CURRENT_PROFILE L"\\curprofile.xpr"

bool ConfigMgr::Save(const std::wstring& path /*= L""*/)
{
	try
	{
		auto filepath = path.empty() ? (libdaemon::Utils::GetExeDirectory() + CURRENT_PROFILE) : path;

		acut::XmlDoc<char> xml;
		xml.create_document();

		xml.set("cfg.apiurl", _config.apiurl.c_str());
		xml.set("cfg.rating", _config.rating);
		xml.set("cfg.scraps", _config.scraps);
		
		xml.write_document(filepath);

		return true;
	}
	catch (const std::runtime_error&)
	{
		return false;
	}
}

bool ConfigMgr::Load(const std::wstring& path /*= L""*/)
{
	try
	{
		auto filepath = path.empty() ? (libdaemon::Utils::GetExeDirectory() + CURRENT_PROFILE) : path;
		if (!acut::file_exists(filepath))
			return false;

		acut::XmlDoc<char> xml;
		xml.read_from_file(filepath);

		xml.get_if_present("cfg.apiurl", _config.apiurl);
		xml.get_if_present("cfg.rating", _config.rating);
		xml.get_if_present("cfg.scraps", _config.scraps);

		return true;
	}
	catch (const std::runtime_error&)
	{
		return false;
	}
}
