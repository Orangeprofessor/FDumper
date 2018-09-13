#pragma once

#include "CBaseDumper.h"

#include "curl/curl.h"

class CE621Dumper : public CBaseDumper
{
public:
	CE621Dumper(std::string tags);
	~CE621Dumper();
	
	virtual int Download()override;
	virtual int QueryAPI()override;

	bool CommandSyntax();

private:
	std::string m_tags;

	std::shared_ptr<CURL> m_curl;
};