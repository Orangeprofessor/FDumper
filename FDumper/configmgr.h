#pragma once

class ConfigMgr
{
public:
	struct ConfigData
	{
		std::wstring defaultDir;
		std::string apiaddress;
		bool askforsave;
	};

public:
	bool Save();
	bool Load();

	inline ConfigData& config() { return m_config; }

private:
	ConfigData m_config;
};