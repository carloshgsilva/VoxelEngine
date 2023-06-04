#pragma once

#include <string>

#include "IO/Log.h"

class SystemIO {
public:
	static void Run(std::string s) {
		system(s.c_str());
	}
	/*
	static void Run(std::wstring s) {
		int len = WideCharToMultiByte(CP_ACP, 0, s.c_str(), s.size() + 1, 0, 0, 0, 0);

		std::string str(len, ' ');
		WideCharToMultiByte(CP_ACP, 0, s.c_str(), s.size()+1, str.data(), len, 0, 0);

		Log::info("Final System Call: {}", str);

		system(str.c_str());
	}
	*/
};