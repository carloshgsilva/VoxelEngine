#include "Jobs.h"

Jobs::Context Jobs::MainThreadContext = {};

bool Jobs::_TryToGetJob(Job& j) {
    Jobs& J = Jobs::Get();

    std::unique_lock lock(J._JobsMutex);

    if (J._Jobs.size() > 0) {
        j = J._Jobs.front();
        J._Jobs.pop();
        return true;
    } else {
        return false;
    }
}

bool Jobs::_Work() {
    Job j;
    if (_TryToGetJob(j)) {
        j._Callback();
        j._Context->ActiveJobs.fetch_sub(1);
        return true;
    } else {
        return false;
    }
}

void Jobs::_JobThread() {
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    Jobs& J = Jobs::Get();
    while (true) {
        if (!_Work()) {
            std::unique_lock lock(J._WakeMutex);
            J._ThreadWakeCondition.wait(lock);
        }
    }
}

Jobs::Jobs() {
    int threadCount = std::thread::hardware_concurrency();
    Log::info("[Jobs] Thread Count: {}", threadCount);

    for (int i = 0; i < threadCount; i++) {
        _ThreadPool.push_back(std::thread(_JobThread));
        std::thread& t = _ThreadPool.back();

#ifdef _WIN32
        HANDLE h = t.native_handle();
        DWORD_PTR affinityMask = 1ull << i;
        DWORD affinity_result = SetThreadAffinityMask(h, affinityMask);
        assert(affinity_result > 0);

        // BOOL priority_result = SetThreadPriority(h, THREAD_PRIORITY_HIGHEST);
        // assert(priority_result > 0);

        // HRESULT hr = SetThreadDescription(h, fmt::format("Job Worker {}", i).c_str());
        // assert(SUCCEEDED(hr));
#endif

        t.detach();
    }
}

void Jobs::Run(std::function<void()> JobCallback, Context& JobContext) {
    Jobs& J = Jobs::Get();

    Job j;
    j._Context = &JobContext;
    j._Callback = JobCallback;

    {
        std::unique_lock lock(J._JobsMutex);
        J._Jobs.push(j);
    }

    j._Context->ActiveJobs.fetch_add(1);
    J._ThreadWakeCondition.notify_all();
}

void Jobs::ParallelFor(uint32 Count, std::function<void(int Index, int Group)> JobCallback, Context& JobContext) {
    uint32 threadCount = GetThreadCount();
    uint32 jobSize = Count / threadCount;

    uint32 start = 0;
    uint32 end = 0;

    for (int t = 0; t < threadCount; t++) {
        start = end;
        end += jobSize;
        if (end >= Count) {
            end = Count;
        }

        Run(
            [=]() {
                for (int i = start; i < end; i++) {
                    JobCallback(i, t);
                }
            },
            JobContext);

        if (end == Count) break;
    }
}

void Jobs::Complete(Context& JobContext) {
    Get()._ThreadWakeCondition.notify_all();
    while (JobContext.ActiveJobs.load() > 0) {
        _Work();
    }
}
