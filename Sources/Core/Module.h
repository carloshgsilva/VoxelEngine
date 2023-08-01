#pragma once

#include "Core/Core.h"
#include "IO/Log.h"

//Base class for Module
//When defining an Module use ModuleDef<T>
class Module {
};

//Inherited by the Engine's Modules
template<class T>
class ModuleDef {
	friend class Engine; // Only the Engine class can Initialize and Get an EngineSystem
	inline static const int SYSTEM_STATE_UNINITIALIZED = 0;
	inline static const int SYSTEM_STATE_INITIALIZING = 1;
	inline static const int SYSTEM_STATE_INITIALIZED = 2;
	inline static int initialization_state = SYSTEM_STATE_UNINITIALIZED;

	static void Initialize() {
		CHECK(initialization_state == SYSTEM_STATE_UNINITIALIZED);
		Get();
	}

public:
	static T& Get() {
		CHECK(initialization_state != SYSTEM_STATE_INITIALIZING);//Trying to get before being initialized
		if (initialization_state == SYSTEM_STATE_UNINITIALIZED)initialization_state = SYSTEM_STATE_INITIALIZING;
		static T Instance;
		if (initialization_state == SYSTEM_STATE_INITIALIZING)initialization_state = SYSTEM_STATE_INITIALIZED;

		return Instance;
	}

};