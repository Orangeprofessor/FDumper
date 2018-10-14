#pragma once

#include "pch.h"
#include "ctpl_stl.hpp"

extern ctpl::thread_pool g_threads;

class CBaseDumper
{
public:
	virtual ~CBaseDumper() {}
	virtual int Download() = 0;
};