#ifndef PCH_H
#define PCH_H


// Windows headers
#include <windows.h>
#include <windowsx.h>
#include <CommCtrl.h>
#include <commdlg.h>

// C includes
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cmath>
#include <ctime>

// C++ includes
#include <iostream>
#include <fstream>
#include <algorithm>
#include <thread>
#include <exception>
#include <vector>
#include <string>
#include <filesystem>

// 6.0 CommCtrls look the best tbh
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")


#endif //PCH_H
