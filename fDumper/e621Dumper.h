#pragma once

#include "CBaseDumper.h"

#include "curl/curl.h"

class CE621Dumper : public CBaseDumper
{
public:
	CE621Dumper(std::string tags);
	~CE621Dumper();
	
	virtual int Download()override;

private:
	std::string m_tags;

	CURL* m_pCurl;
};