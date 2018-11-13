#pragma once

class FDumper
{
public:
	void Start(std::wstring cmd);
	void WaitForCommand();
	bool ParseCommand(std::wstring cmd);
};