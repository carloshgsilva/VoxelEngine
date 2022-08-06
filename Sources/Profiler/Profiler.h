#pragma once

#include "Core/Module.h"

#include <fstream>
#include <sstream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <mutex>

#define ACTIVATE_PROFILER 0

class Profiler : public ModuleDef<Profiler> {
public:

	struct ProfileResult {
		std::string Name;
		std::chrono::steady_clock::time_point StartTime{ std::chrono::milliseconds(0)};
		std::chrono::steady_clock::duration ElapsedTime{0};
		std::thread::id ThreadID;
	};
	class ProfilerTimer {
		std::chrono::steady_clock::time_point _StartTime;
		const char* _Name;
	public:
		ProfilerTimer(const char* Name) {
			_StartTime = std::chrono::steady_clock::now();
			_Name = Name;
		}
		~ProfilerTimer() {
			auto end = std::chrono::steady_clock().now();
			auto elapsedTime = end.time_since_epoch() - _StartTime.time_since_epoch();
		
			ProfileResult Result;
			Result.Name = _Name;
			Result.StartTime = _StartTime;
			Result.ElapsedTime = elapsedTime;
			Result.ThreadID = std::this_thread::get_id();
			Profiler::WriteProfile(Result);
		}
	};

	static void Begin(std::string file){
#if ACTIVATE_PROFILER
		Get()._OutStream = std::ofstream(file);
		auto& s = Get()._OutStream;

		s << "{\"otherData\": {},\"traceEvents\":[{}";
		Get()._ProfilerStartTime = std::chrono::steady_clock::now().time_since_epoch();
#endif
	}
	static void WriteProfile(ProfileResult& R) {
		std::stringstream json;

		using MicroSecs = std::chrono::duration<double, std::micro>;
		
		json << std::setprecision(3) << std::fixed;
		json << ", {";
		json << "\"cat\":\"function\",";
		json << "\"dur\":" << std::chrono::duration_cast<MicroSecs>(R.ElapsedTime).count() << ',';
		json << "\"name\":\"" << R.Name << "\",";
		json << "\"ph\":\"X\",";
		json << "\"pid\":0,";
		json << "\"tid\":" << R.ThreadID << ",";
		json << "\"ts\":" << std::chrono::duration_cast<MicroSecs>( (R.StartTime - Profiler::Get()._ProfilerStartTime).time_since_epoch() ).count();
		json << "}";

		std::lock_guard lock(Get()._Mutex);
		Get()._OutStream << json.str();
	}
	static void End(){
#if ACTIVATE_PROFILER
		auto& s = Get()._OutStream;
		s << "]}";
		s.close();
#endif
	}


	static void BeginGPU() {
		Get()._GPUResult.Name = "GPU";
		Get()._GPUResult.StartTime = std::chrono::steady_clock::now();
	}
	static void EndGPU() {
		if (Get()._GPUResult.StartTime.time_since_epoch().count() == 0)return;
		Get()._GPUResult.ElapsedTime = std::chrono::steady_clock().now().time_since_epoch() - Get()._GPUResult.StartTime.time_since_epoch();
		Profiler::WriteProfile(Get()._GPUResult);
	}

private:
	ProfileResult _GPUResult;
	std::ofstream _OutStream;
	std::mutex _Mutex;
	std::chrono::steady_clock::duration _ProfilerStartTime;
	friend class ProfilerTimer;
};

#if ACTIVATE_PROFILER
	#define PROFILE_FUNC() Profiler::ProfilerTimer __profile_func_timer##__LINE__("" __FUNCTION__);
	#define PROFILE_SCOPE(Name) Profiler::ProfilerTimer __profile_timer##__LINE__(Name);
#else
	#define PROFILE_FUNC()
	#define PROFILE_SCOPE(Name)
#endif

