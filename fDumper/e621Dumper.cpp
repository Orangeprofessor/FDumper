#include "pch.h"

#include "e621Dumper.h"

CE621Dumper::CE621Dumper(std::string tags) : m_tags(tags), m_curl(std::make_shared<CURL>(curl_easy_init()))
{

}

CE621Dumper::~CE621Dumper()
{
	curl_easy_cleanup(m_curl.get());
}

int CE621Dumper::Download()
{
	return 0;
}

int CE621Dumper::QueryAPI()
{
	return 0;
}

bool CE621Dumper::CommandSyntax()
{

}
