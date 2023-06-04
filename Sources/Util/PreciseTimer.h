#pragma once

#include "IO/Log.h"

#include <string>

#if WIN32
#include <profileapi.h>

class PreciseTimer {
	std::string identifier;
	LARGE_INTEGER frequency;        // ticks per second
	LARGE_INTEGER t1, t2;           // ticks
	double elapsedTime;

public:
	PreciseTimer(std::string identifier) : identifier(identifier) {
		QueryPerformanceFrequency(&frequency);
		QueryPerformanceCounter(&t1);
	}

	~PreciseTimer() {
		QueryPerformanceCounter(&t2);
		elapsedTime = (t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart;

		Log::info("{}: {}ms", identifier, elapsedTime);
	}

};

#else
//TODO: PreciseTimer on other os
class PreciseTimer {
	std::string identifier;
	double elapsedTime;

public:
	PreciseTimer(std::string identifier) : identifier(identifier) {
		
	}

	~PreciseTimer() {
		
	}

};

#endif