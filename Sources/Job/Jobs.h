#pragma once

#include "Core/Module.h"

#include "Core/Core.h"

#include <queue>
#include <thread>

class Jobs : public ModuleDef<Jobs> {
public:
	Jobs();

	struct Context {
		std::atomic<uint32> ActiveJobs{ 0 };
	};
	struct Job {
		Context* _Context;
		std::function<void()> _Callback;
	};

	static uint32 GetThreadCount() { return Get()._ThreadPool.size(); }

	static Context MainThreadContext;

	static void Run(std::function<void()> JobCallback, Context& JobContext = MainThreadContext);
	static void ParallelFor(uint32 Count, std::function<void(int Index, int Group)> JobCallback, Context& JobContext = MainThreadContext);
	static void Complete(Context& JobContext = MainThreadContext);

private:
	std::vector<std::thread> _ThreadPool;

	std::mutex _WakeMutex;
	std::condition_variable _ThreadWakeCondition;

	std::mutex _JobsMutex;
	std::queue<Job> _Jobs;

	static bool _TryToGetJob(Job& j);
	static bool _Work();
	static void _JobThread();
};