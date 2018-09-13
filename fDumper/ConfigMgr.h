#pragma once

#include "rapidxml/rapidxml_wrap.hpp"


class ConfigMgr
{
public:
	struct ConfigData
	{
		std::string apiurl;
		int rating = 0;
		int scraps = 0;
	};

public:
	bool Save(const std::wstring& path = L"");
	bool Load(const std::wstring& path = L"");

	inline ConfigData& config() { return _config; }

private:
	ConfigData _config;
};


