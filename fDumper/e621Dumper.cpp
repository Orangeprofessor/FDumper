#include "pch.h"

#include "e621Dumper.h"

CE621Dumper::CE621Dumper(std::string tags) : m_tags(tags), m_pCurl(curl_easy_init())
{

}

CE621Dumper::~CE621Dumper()
{
	curl_easy_cleanup(m_pCurl);
}

int CE621Dumper::Download()
{
	return 0;
}

