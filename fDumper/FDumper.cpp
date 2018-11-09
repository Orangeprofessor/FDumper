#include "pch.h"

#include "FDumper.h"
#include "fADumper.h"
#include "FAUpdater.h"

void FDumper::Start()
{
	int argc = 0;
	auto pCmdLine = GetCommandLineW();
	auto argv = CommandLineToArgvW(pCmdLine, &argc);

	if (argc == 1)
	{
		WaitForCommand();
		return;
	}
	
	ParseCommand(pCmdLine);
}

void FDumper::WaitForCommand()
{
	while (true)
	{
		std::string command;

		std::printf("FDumper> ");
		std::getline(std::cin, command);

		if (command.empty())
			continue;

		if (ParseCommand(AnsiToWstring(command)))
			break;
	}
}

bool FDumper::ParseCommand(std::wstring cmd)
{
	int argc = 0;
	auto argv = CommandLineToArgvW(cmd.c_str(), &argc);;

	std::wstring mode = argv[0];

	if (!mode.compare(L"dump"))
	{
		CFADumper dumper;
		dumper.Main(argc, argv);
	}
	else if (!mode.compare(L"update"))
	{
		CFAUpdater updater;
		updater.Main(argc, argv);
	}
	else if (!mode.compare(L"exit"))
	{
		return true;
	}
	else
	{
		log_console(xlog::LogLevel::error,
			"Invalid command!\n"
			"See the readme for more info\n");
	}

	return false;
}
