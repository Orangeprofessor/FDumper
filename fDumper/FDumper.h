#pragma once

class FDumper
{
public:
	void Start();
	void WaitForCommand();
	bool ParseCommand(std::wstring cmd);
};